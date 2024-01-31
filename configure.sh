#!/bin/bash
# (c) 2024, Thomas Kasper
# Licensed under the MIT License. See `LICENSE` for more information

make_struct_type() {
    printf "struct { char _[%d]; }" $(($1 / 8))
}

make_complex_type() {
    if [[ "$1" == "float" ]] || [[ "$1" == "double" ]] || [[ "$1" == "long double" ]]; then
        echo "_Complex $1"
    else
        printf "struct { sbinary%s_t Re; sbinary%s_t Im; }" "$2" "$2"
    fi
}

COL1="\e[20G"
COL2="\e[31G"
COL3="\e[55G"

NO_FENV=0 # 0=standard 1=custom

MODE_FULL=0 # generate full library support
MODE_CAST=1 # generate only cast support (trunc, extend)
MODE_NONE=2 # generate nothing

MODE_BIN16=$MODE_FULL
MODE_BIN32=$MODE_CAST
MODE_BIN64=$MODE_CAST
MODE_BIN128=$MODE_FULL
MODE_BIN256=$MODE_FULL
MODE_DEC32=$MODE_FULL
MODE_DEC64=$MODE_FULL
MODE_DEC128=$MODE_FULL

BIN80_SIZE=80

REPR_DEC=both

NO_CMAKE=0

NEED_IMMINTRIN=0

ARCH=`uname -m`

if [ "$ARCH" = "x86_64"  ] || [ "$ARCH" = "i686" ]; then
    X86=1
    MODE_BIN80=$MODE_CAST
    TYPE_BIN80="long double"
    TYPE_BIN128="__m128"
    TYPE_BIN256="__m256"
    NEED_IMMINTRIN=1
else
    X86=0
    MODE_BIN80=$MODE_FULL
    TYPE_BIN80=`make_struct_type 80`
    TYPE_BIN128=`make_struct_type 128`
    TYPE_BIN256=`make_struct_type 256`
fi

TYPE_BIN16=uint16_t
TYPE_BIN32=float
TYPE_BIN64=double
TYPE_DEC32=`make_struct_type 32`
TYPE_DEC64=`make_struct_type 64`
TYPE_DEC128=`make_struct_type 128`

CTYPE_BIN16=`make_complex_type "$TYPE_BIN16" 16`
CTYPE_BIN32=`make_complex_type "$TYPE_BIN32" 32`
CTYPE_BIN64=`make_complex_type "$TYPE_BIN64" 64`
CTYPE_BIN80=`make_complex_type "$TYPE_BIN80" 80`
CTYPE_BIN128=`make_complex_type "$TYPE_BIN128" 128`
CTYPE_BIN256=`make_complex_type "$TYPE_BIN256" 256`

_prnt_msg() {
    name=$1
    color=$2
    shift 2
    msg="$@"
    printf "\e[90m[\e[${color}m$name\e[90m] \e[${color}m$msg\e[0m\n"
}

prnt_err() {
    msg=$1
    shift
    _prnt_msg ERROR 31 `printf "$msg" "$@"` 1>&2
    exit 1
}

prnt_warn() {
    msg=$1
    shift
    _prnt_msg WARN 33 `printf "$msg" "$@"` 1>&2
}

prnt_info() {
    msg=$1
    shift
    _prnt_msg INFO 94 `printf "$msg" "$@"`
}

prnt_cfg() {
    types="\e[96m$3\e[0m"

    if [[ $# -eq 4 ]]; then
        types="$types,$COL3\e[96m$4\e[0m"
    fi

    if [[ "$2" -eq $MODE_NONE ]]; then
        prnt_info "%s:$COL1\e[91mdisabled" $1
    elif [[ "$2" -eq $MODE_CAST ]]; then
        prnt_info "%s:$COL1\e[33mcast only\e[0m,$COL2%s" $1 "$types"
    else
        prnt_info "%s:$COL1\e[92mfull\e[0m,$COL2%s" $1 "$types"
    fi
}

parse_mode() {
    case $1 in
    *=no|*=none)
        echo $MODE_NONE
        ;;
    *=cast)
        echo $MODE_CAST
        ;;
    *=full)
        echo $MODE_FULL
        ;;
    *)
        if [[ "$1" == *"="* ]] && [[ "$1" != *"=" ]]; then
            prnt_err "Unknown mode '%s' for '%s'" "${1#*=}" "${1%=*}"
        else
            prnt_err "Missing mode for '%s'" "$1"
        fi
        return 1
        ;;
    esac
}

parse_type() {
    case $1 in
    *=*)
        echo ${1#*=}
        ;;
    *)
        prnt_err "Missing type for '%s'" "$1"
        return 1
        ;;
    esac
}

parse_repr() {
    case $1 in
    *=bid|*=dpd|*=both)
        echo ${1#*=}
        ;;
    *)
        if [[ "$1" == *"="* ]] && [[ "$1" != *"=" ]]; then
            prnt_err "Unknown representation '%s'" "${1#*=}"
        else
            prnt_err "Missing representation for '%s'" "$1"
        fi
        return 1
        ;;
    esac
}

parse_preset() {
    case $1 in
    *=int)
        if [[ $3 -gt 64 ]]; then
            prnt_err "Preset 'int' for '' is not supported" "${1%=*}"
            return 1
        fi
        echo "uint${3}_t"
        ;;
    *=struct)
        echo `make_struct_type $3`
        ;;
    *=builtin)
        if [[ $2 -eq 1 ]]; then
            echo "_Decimal$3"
        elif [[ $3 -eq 32 ]]; then
            echo "float"
        elif [[ $3 -eq 64 ]]; then
            echo "double"
        elif [[ $3 -eq 80 ]]; then
            if [[ $X86 -eq 0 ]]; then
                prnt_warn "Using type nonstandard type __float80 for binary80"
                echo "__float80"
            else
                echo "long double"
            fi
        elif [[ $3 -eq 128 ]] && [[ $X86 -eq 0 ]]; then
            prnt_warn "Assuming long double has binary128 format"
            echo "long double"
        else
            echo "_Float$3"
        fi
        ;;
    *=mmx)
        if [[ $X86 -eq 0 ]]; then
            prnt_err "Cannot use MMX preset on non-x86 architecture"
        fi
        NEED_IMMINTRIN=1

        if [[ $3 -eq 80 ]]; then
            prnt_warn "Expanding binary80 to 128 bits for MMX preset"
            echo "__m128"
        else
            echo "__m$3"
        fi
        ;;
    *)
        if [[ "$1" == *"="* ]] && [[ "$1" != *"=" ]]; then
            prnt_err "Unknown preset '%s' for '%s'" "${1#*=}" "${1%=*}"
        else
            prnt_err "Missing preset for '%s'" "$1"
        fi
        return 1
        ;;
    esac
}

mkopt() {
    msg="$@"
    fmt=`echo $msg | sed -e "s/[<>|{},=]/#e[90m\\0#e[0m/g"`
    echo -ne "\e[37m--\e[93m${fmt//#/\\}"
}

prnt_opt() {
    echo -ne " \e[90m[$@\e[90m]"
}

while [[ $# -gt 0 ]]; do
    case $1 in
    --mode-bin80-size*)
        # TODO
        ;;
    --mode-bin16*)
        MODE_BIN16=`parse_mode "$1"`
        ;;
    --mode-bin32*)
        MODE_BIN32=`parse_mode "$1"`
        ;;
    --mode-bin64*)
        MODE_BIN64=`parse_mode "$1"`
        ;;
    --mode-bin80*)
        MODE_BIN80=`parse_mode "$1"`
        ;;
    --mode-bin128*)
        MODE_BIN128=`parse_mode "$1"`
        ;;
    --mode-bin256*)
        MODE_BIN256=`parse_mode "$1"`
        ;;
    --mode-dec32*)
        MODE_DEC32=`parse_mode "$1"`
        ;;
    --mode-dec64*)
        MODE_DEC64=`parse_mode "$1"`
        ;;
    --mode-dec128*)
        MODE_DEC128=`parse_mode "$1"`
        ;;
    --type-bin16*)
        TYPE_BIN16=`parse_type "$1"`
        ;;
    --type-bin32*)
        TYPE_BIN32=`parse_type "$1"`
        ;;
    --type-bin64*)
        TYPE_BIN64=`parse_type "$1"`
        ;;
    --type-bin80*)
        TYPE_BIN80=`parse_type "$1"`
        ;;
    --type-bin128*)
        TYPE_BIN128=`parse_type "$1"`
        ;;
    --type-bin256*)
        TYPE_BIN256=`parse_type "$1"`
        ;;
    --type-dec32*)
        TYPE_DEC32=`parse_type "$1"`
        ;;
    --type-dec64*)
        TYPE_DEC64=`parse_type "$1"`
        ;;
    --type-dec128*)
        TYPE_DEC128=`parse_type "$1"`
        ;;
    --ctype-bin16*)
        CTYPE_BIN16=`parse_type "$1"`
        ;;
    --ctype-bin32*)
        CTYPE_BIN32=`parse_type "$1"`
        ;;
    --ctype-bin64*)
        CTYPE_BIN64=`parse_type "$1"`
        ;;
    --ctype-bin80*)
        CTYPE_BIN80=`parse_type "$1"`
        ;;
    --ctype-bin128*)
        CTYPE_BIN128=`parse_type "$1"`
        ;;
    --ctype-bin256*)
        CTYPE_BIN256=`parse_type "$1"`
        ;;
    --ctype-dec32*)
        CTYPE_DEC32=`parse_type "$1"`
        ;;
    --ctype-dec64*)
        CTYPE_DEC64=`parse_type "$1"`
        ;;
    --ctype-dec128*)
        CTYPE_DEC128=`parse_type "$1"`
        ;;
    --preset-bin16*)
        TYPE_BIN16=`parse_preset "$1" 0 16`
        CTYPE_BIN16=`make_complex_type "$TYPE_BIN16" 16`
        ;;
    --preset-bin32*)
        TYPE_BIN32=`parse_preset "$1" 0 32`
        CTYPE_BIN32=`make_complex_type "$TYPE_BIN32" 32`
        ;;
    --preset-bin64*)
        TYPE_BIN64=`parse_preset "$1" 0 64`
        CTYPE_BIN64=`make_complex_type "$TYPE_BIN64" 64`
        ;;
    --preset-bin80*)
        TYPE_BIN80=`parse_preset "$1" 0 $BIN80_SIZE`
        CTYPE_BIN80=`make_complex_type "$TYPE_BIN80" 80`
        ;;
    --preset-bin128*)
        TYPE_BIN128=`parse_preset "$1" 0 128`
        CTYPE_BIN128=`make_complex_type "$TYPE_BIN128" 128`
        ;;
    --preset-bin256*)
        TYPE_BIN256=`parse_preset "$1" 0 256`
        CTYPE_BIN256=`make_complex_type "$TYPE_BIN256" 256`
        ;;
    --preset-dec32*)
        TYPE_DEC32=`parse_preset "$1" 1 32`
        ;;
    --preset-dec64*)
        TYPE_DEC64=`parse_preset "$1" 1 64`
        ;;
    --preset-dec128*)
        TYPE_DEC128=`parse_preset "$1" 1 128`
        ;;
    --dec-repr*)
        REPR_DEC=`parse_repr "$1"`
        ;;
    --no-cmake)
        NO_CMAKE=1
        ;;
    --no-fenv)
        NO_FENV=1
        ;;
    --print)
        prnt_info "Current configuration:"
        prnt_info "type${COL1}mode${COL2}type${COL3}complex type"
        prnt_cfg binary16   "$MODE_BIN16"  "$TYPE_BIN16"  "$CTYPE_BIN16"
        prnt_cfg binary32   "$MODE_BIN32"  "$TYPE_BIN32"  "$CTYPE_BIN32"
        prnt_cfg binary64   "$MODE_BIN64"  "$TYPE_BIN64"  "$CTYPE_BIN64"
        prnt_cfg binary80   "$MODE_BIN80"  "$TYPE_BIN80"  "$CTYPE_BIN80"
        prnt_cfg binary128  "$MODE_BIN128" "$TYPE_BIN128" "$CTYPE_BIN128"
        prnt_cfg binary256  "$MODE_BIN256" "$TYPE_BIN256" "$CTYPE_BIN256"
        prnt_cfg decimal32  "$MODE_DEC32"  "$TYPE_DEC32"
        prnt_cfg decimal64  "$MODE_DEC64"  "$TYPE_DEC64"
        prnt_cfg decimal128 "$MODE_DEC128" "$TYPE_DEC128"
        prnt_info "decimal representation:     \e[92m$REPR_DEC"

        if [[ $NO_FENV -eq 1 ]]; then
            FENV="custom"
        else
            FENV="standard"
        fi

        prnt_info "floating-point environment: \e[92m$FENV"
        exit 0
        ;;
    --help)
        RS="\e[0m"  # Reset
        OR="\e[33m" # Orange
        GY="\e[37m" # Gray
        DG="\e[90m" # Dark Gray
        RE="\e[91m" # Red
        GR="\e[92m" # Green
        YE="\e[93m" # Yellow
        BL="\e[94m" # Blue
        MA="\e[95m" # Magenta

        # print usage
        echo -e "${BL}libsoftfp$RS version 0.1.0"
        echo -e "(c) 2024, Thomas Kasper"
        echo

        BIN_TYPES="bin{${GR}16,${GR}32,${GR}64,${GR}80,${GR}128,${GR}256}"
        DEC_TYPES="dec{${GR}32,${GR}64,${GR}128}"
        
        OPT_MODES="${RE}no|${OR}cast|${GR}full"

        OPT_MODE_BIN="`mkopt "mode-$BIN_TYPES=<$OPT_MODES>"`"
        OPT_MODE_DEC="`mkopt "mode-$DEC_TYPES=<$OPT_MODES>"`"
        OPT_TYPE_BIN="`mkopt "type-$BIN_TYPES=<${BL}TYPE>"`"
        OPT_TYPE_DEC="`mkopt "type-$DEC_TYPES=<${BL}TYPE>"`"
        OPT_CTYPE_BIN="`mkopt "ctype-$BIN_TYPES=<${BL}TYPE>"`"
        OPT_CTYPE_DEC="`mkopt "ctype-$DEC_TYPES=<${BL}TYPE>"`"
        OPT_PRESET_BIN="`mkopt "preset-$BIN_TYPES=<${BL}int|${BL}struct|${BL}builtin|${BL}mmx>"`"
        OPT_PRESET_DEC="`mkopt "preset-$DEC_TYPES=<${BL}int|${BL}struct|${BL}builtin|${BL}mmx>"`"
        OPT_BIN80_SIZE="`mkopt "bin80-size=<${BL}80|${BL}96|${BL}128>"`"
        OPT_DEC_REPR="`mkopt "dec-repr=<${BL}both|${BL}bid|${BL}dpd>"`"
        OPT_NO_CMAKE="`mkopt no-cmake`"
        OPT_NO_FENV="`mkopt no-fenv`"
        OPT_PRINT="`mkopt print`"
        OPT_HELP="`mkopt help`"

        echo -ne "$BL$0"

        prnt_opt $OPT_MODE_BIN ; echo
        prnt_opt $OPT_MODE_DEC ; echo
        prnt_opt $OPT_TYPE_BIN ; echo
        prnt_opt $OPT_TYPE_DEC ; echo
        prnt_opt $OPT_CTYPE_BIN ; echo
        prnt_opt $OPT_CTYPE_DEC ; echo
        prnt_opt $OPT_PRESET_BIN ; echo
        prnt_opt $OPT_PRESET_DEC ; echo
        prnt_opt $OPT_BIN80_SIZE
        prnt_opt $OPT_DEC_REPR ; echo
        prnt_opt $OPT_NO_CMAKE 
        prnt_opt $OPT_NO_FENV
        prnt_opt $OPT_PRINT
        prnt_opt $OPT_HELP

        echo -e "$RS\n"

        SPC="                         " # 25 spaces

        # print docs
        echo -e "Help for ${BL}$0${RS}:"
        echo -e " $OPT_MODE_BIN\n $OPT_MODE_DEC"
        echo -e "$SPC${RS}Specify whether to generate ${RE}no$RS, only ${OR}cast$RS or ${GR}full$RS"
        echo -e "$SPC  library support for a certain type"
        
        echo -e " $OPT_TYPE_BIN\n $OPT_TYPE_DEC"
        echo -e " $OPT_CTYPE_BIN\n $OPT_CTYPE_DEC"
        echo -e "$SPC${RS}Specify a custom (complex) type for a certain type"

        echo -e " $OPT_PRESET_BIN\n $OPT_PRESET_DEC"
        echo -e "$SPC${RS}Use a preset for a certain type:"
        echo -e "$SPC ${BL}int$RS uses ${MA}uintN_t$RS (${MA}N$RS <= 64, requires $BL<stdint.h>$RS),"
        echo -e "$SPC ${BL}struct$RS uses ${MA}struct { uint8_t _[N/8]; }$RS,"
        echo -e "$SPC ${BL}builtin$RS uses ${MA}float$RS (binary32), ${MA}double$RS (binary64), ${MA}long"
        echo -e "$SPC  double$RS (binary80 or binary128), ${MA}_FloatN$RS or ${MA}_DecimalN$RS"
        echo -e "$SPC ${BL}mmx$RS uses ${MA}__mN$RS (x86-only; requires $BL<immintrin.h>$RS)"
        echo -e "$SPC (make sure your compiler supports these types!)"

        echo -e " $OPT_BIN80_SIZE"
        echo -e "$SPC${RS}Specifies the size of the ${BL}binary80$RS structure (only"
        echo -e "$SPC affects subsequent ${BL}--preset-bin80=struct$RS options)"

        echo -e " $OPT_DEC_REPR"
        echo -e "$SPC${RS}Specify the representation for decimal"
        echo -e "$SPC floating-point numbers. When ${GR}both$RS is"
        echo -e "$SPC selected, separate functions for BID"
        echo -e "$SPC and DPD are generated, prefixed with"
        echo -e "$SPC '__bid_' or '__dpd_', respectively."

        echo -e " $OPT_NO_CMAKE$RS              Don't create a build dir and run cmake"
        echo -e " $OPT_NO_FENV$RS               Don't use the standard floating-point environment"
        echo -e "$SPC provided by $BL<fenv.h>$RS"
        echo -e " $OPT_PRINT$RS                 Print the selected config and exit"
        echo -e " $OPT_HELP$RS                  Print this help and exit"

        exit 0
        ;;
    *)
        prnt_err "Unknown option '%s'" "$1"
        ;;
    esac
    [[ $? -eq 0 ]] || exit 1
    shift
done

rm -rf configure-tmp
mkdir -p configure-tmp
cd configure-tmp

touch typedefs.h

if [[ $NEED_IMMINTRIN -ne 0 ]]; then
    echo -e "#include <immintrin.h>\n" >> typedefs.h
fi

gen_typedef() {
    if [[ $2 -ne $MODE_NONE ]]; then
        prnt_info "Generating typedef for $5$1..."

        echo "/* $5$1 */" >> typedefs.h
        echo "typedef $3 s$5${1}_t;" >> typedefs.h

        if [[ "$5" == "binary" ]]; then
            echo "typedef $4 sc$5${1}_t;" >> typedefs.h
        fi

        echo >> typedefs.h
    fi
}

gen_typedef 16  $MODE_BIN16  "$TYPE_BIN16"  "$CTYPE_BIN16"  binary
gen_typedef 32  $MODE_BIN32  "$TYPE_BIN32"  "$CTYPE_BIN32"  binary
gen_typedef 64  $MODE_BIN64  "$TYPE_BIN64"  "$CTYPE_BIN64"  binary
gen_typedef 80  $MODE_BIN80  "$TYPE_BIN80"  "$CTYPE_BIN80"  binary
gen_typedef 128 $MODE_BIN128 "$TYPE_BIN128" "$CTYPE_BIN128" binary
gen_typedef 256 $MODE_BIN256 "$TYPE_BIN256" "$CTYPE_BIN256" binary
gen_typedef 32  $MODE_DEC32  "$TYPE_DEC32"  ""              decimal
gen_typedef 64  $MODE_DEC64  "$TYPE_DEC64"  ""              decimal
gen_typedef 128 $MODE_DEC128 "$TYPE_DEC128" ""              decimal

cp ../template/softfp.template.h softfp.1.h
sed $'/%typedefs%/{r typedefs.h\nd}' softfp.1.h > softfp.2.h

touch functions.h

CFG_BIN16="16   $MODE_BIN16  hf hc binary"
CFG_BIN32="32   $MODE_BIN32  sf sc binary"
CFG_BIN64="64   $MODE_BIN64  df dc binary"
CFG_BIN80="80   $MODE_BIN80  tf tc binary"
CFG_BIN128="128 $MODE_BIN128 xf xc binary"
CFG_BIN256="256 $MODE_BIN256 yf yc binary"
CFG_DEC32="32   $MODE_DEC32  sd -  decimal"
CFG_DEC64="64   $MODE_DEC64  dd -  decimal"
CFG_DEC128="128 $MODE_DEC128 td -  decimal"

CFG_ALL="$CFG_BIN16 $CFG_BIN32 $CFG_BIN64 $CFG_BIN80 $CFG_BIN128 $CFG_BIN256 $CFG_DEC32 $CFG_DEC64 $CFG_DEC128"

process_template() {
    file="$1"
    shift

    while [[ $# -gt 1 ]]; do
        if ! grep -q "%$1" "$file"; then
            shift 2
            continue
        fi

        echo -n $2 > ./subst
        awk "function readfile (file) { getline var < file ; return var } BEGIN { RS=\"^$\" }; gsub(/%$1/,readfile(\"subst\"))" $file > $file.1
        cp -f $file.1 $file
        shift 2
    done
}

gen_decimal_prefix() {
    for kind in $@; do
        if [[ "$kind" == "decimal" ]]; then
            if [[ "$REPR_DEC" == "both" ]]; then
                echo "__dpd_ __bid_"
                return
            fi
            break
        fi
    done
    echo "__"
}

gen_truncextend() {
    bits=$1
    id=$3
    cid=$4
    kind=$5
    shift 5

    while [[ $# -gt 4 ]]; do    
        target_bits=$1
        target_mode=$2
        target_id=$3
        target_cid=$4
        target_kind=$5
        shift 5

        [[ $target_mode -eq $MODE_NONE ]] && continue
        [[ "$id" == "$target_id" ]] && continue

        if [[ $bits -lt $target_bits ]] || ([[ $bits -eq $target_bits ]] && [[ $target_kind == "decimal" ]]); then
            fnkind="extend"
        else
            fnkind="trunc"
        fi

        for prefix in `gen_decimal_prefix $kind $target_kind`; do
            echo "/** @brief converts \`a\` into a s$target_kind${target_bits}_t */" >> functions.h
            echo "s$target_kind${target_bits}_t $prefix$fnkind$id${target_id}2(s$kind${bits}_t a);" >> functions.h
            echo >> functions.h
        done
    done
}

gen_functions() {
    if [[ $2 -ne $MODE_NONE ]]; then
        bits=$1
        id=$3
        cid=$4
        kind=$5

        prnt_info "Generating function prototypes for $kind$bits..."

        for prefix in `gen_decimal_prefix $kind`; do

            # convert __X_ to X
            readable_prefix=${prefix%_}
            readable_prefix=${readable_prefix#__}

            echo -e "\n\n/* $kind$bits $readable_prefix */" >> functions.h

            cat ../template/functions.template.h >> functions.h
            process_template functions.h T "s$kind${bits}_t" I $id P "$prefix"

            if [[ "$kind" == "binary" ]]; then
                cat ../template/complex.template.h >> functions.h
                process_template functions.h T "s$kind${bits}_t" C "sc$kind${bits}_t" I $cid
            fi
        done

        if [[ "$kind" == "decimal" ]] && [[ "$REPR_DEC" == "both" ]]; then
            cat ../template/decimal.template.h >> functions.h
            process_template functions.h T "s$kind${bits}_t" I $id
        fi

        gen_truncextend $@ $CFG_ALL
    fi
}

gen_functions $CFG_BIN16
gen_functions $CFG_BIN32
gen_functions $CFG_BIN64
gen_functions $CFG_BIN80
gen_functions $CFG_BIN128
gen_functions $CFG_BIN256
gen_functions $CFG_DEC32
gen_functions $CFG_DEC64
gen_functions $CFG_DEC128

mkdir -p ../include
sed $'/%functions%/{r functions.h\nd}' softfp.2.h > softfp.3.h

if [[ $NO_FENV -eq 1 ]]; then
    sed $'/%fenv%/{r ../template/fenv.template.h\nd}' softfp.3.h > ../include/softfp.h
else
    sed -e 's/%fenv%/#include <fenv.h>/g' softfp.3.h > ../include/softfp.h
fi

gen_isdpd() {
    ([[ "$1" == "dpd" ]] || ([[ "$1" == "" ]] && [[ "$7" == "decimal" ]] && [[ $REPR_DEC == "dpd" ]])) && return 0
    return 1
}

gen_filename() {
    if [[ "$7" != "decimal" ]]; then
        suffix=""
    else
        gen_isdpd $@
        if [[ $? -eq 0 ]]; then
            suffix="-dpd"
        else
            suffix="-bid"
        fi
    fi

    echo "$7$2$suffix"
}

gen_truncextend_impls() {
    prefix=$1
    shift

    bits=$1
    id=$3
    kind=$6
    shift 11

    while [[ $# -gt 4 ]]; do    
        target_bits=$1
        target_mode=$2
        target_id=$3
        target_kind=$6

        if [[ $target_mode -eq $MODE_NONE ]] || [[ "$id" == "$target_id" ]]; then
            shift 11
            continue
        fi

        if [[ $bits -lt $target_bits ]] || ([[ $bits -eq $target_bits ]] && [[ $target_kind == "decimal" ]]); then
            fnkind="extend"
        else
            fnkind="trunc"
        fi

        if [[ "$kind" == "decimal" ]]; then
            prefixes="$prefix"
        else
            prefixes=`gen_decimal_prefix $target_kind`
        fi

        for prefix in $prefixes; do
            readable_prefix=${prefix%_}
            readable_prefix=${readable_prefix#__}
            filename=`gen_filename "$readable_prefix" $@`

            echo "#include \"../config/$filename.h\"" >> impl.h
            echo "#include \"../common.h\"" >> impl.h
        done

        shift 11
    done
}

gen_impls() {
    if [[ $2 -ne $MODE_NONE ]]; then
        prnt_info "Generating implementations for $6$1..."

        for prefix in `gen_decimal_prefix $6`; do

            # convert __X_ to X
            readable_prefix=${prefix%_}
            readable_prefix=${readable_prefix#__}

            filename=`gen_filename "$readable_prefix" $@`
            cfgfile=../src/config/$filename.h
            implfile=../src/impl/$filename.c

            touch config.h
            cat ../template/config.$6.template.h > config.h

            gen_isdpd "$readable_prefix" $@
            dpd=$((1-$?))

            process_template config.h\
                B $1 M $2 I $3 CX $5\
                FE $7 FJ $8 FF $9 FC $4\
                DC ${10} DS ${11} DR "$dpd"

            cp -f config.h $cfgfile

            cat ../template/license.template.h > impl.h
            echo "#include \"../config/$filename.h\"" >> impl.h
            echo "#include \"../common.h\"" >> impl.h

            gen_truncextend_impls "$readable_prefix" $@ $IMPL_ALL

            cp -f impl.h $implfile
        done
    fi
}

stdcomplex() {
    if [[ "$1" == "_Complex" ]]; then
        echo "1"
    else
        echo "0"
    fi
}

#           bits mode         id cid stdcomplex                 kind  exp J frac comb sig
IMPL_BIN16="16   $MODE_BIN16  hf hc  `stdcomplex $CTYPE_BIN16`  binary  5  0 10  _  _"
IMPL_BIN32="32   $MODE_BIN32  sf sc  `stdcomplex $CTYPE_BIN32`  binary  8  0 23  _  _"
IMPL_BIN64="64   $MODE_BIN64  df dc  `stdcomplex $CTYPE_BIN64`  binary  11 0 52  _  _"
IMPL_BIN80="80   $MODE_BIN80  tf tc  `stdcomplex $CTYPE_BIN80`  binary  15 1 63  _  _"
IMPL_BIN128="128 $MODE_BIN128 xf xc  `stdcomplex $CTYPE_BIN128` binary  15 0 112 _  _"
IMPL_BIN256="256 $MODE_BIN256 yf yc  `stdcomplex $CTYPE_BIN256` binary  19 0 236 _  _"
IMPL_DEC32="32   $MODE_DEC32  sd _   0                          decimal _  _ _   11 20"
IMPL_DEC64="64   $MODE_DEC64  dd _   0                          decimal _  _ _   13 50"
IMPL_DEC128="128 $MODE_DEC128 td _   0                          decimal _  _ _   17 110"

IMPL_ALL="$IMPL_BIN16 $IMPL_BIN32 $IMPL_BIN64 $IMPL_BIN80 $IMPL_BIN128 $IMPL_BIN256 $IMPL_DEC32 $IMPL_DEC64 $IMPL_DEC128"

mkdir -p ../src/config
mkdir -p ../src/impl

gen_impls $IMPL_BIN16
gen_impls $IMPL_BIN32
gen_impls $IMPL_BIN64
gen_impls $IMPL_BIN80
gen_impls $IMPL_BIN128
gen_impls $IMPL_BIN256
gen_impls $IMPL_DEC32
gen_impls $IMPL_DEC64
gen_impls $IMPL_DEC128

cd ..
rm -rf configure-tmp

if [[ $NO_CMAKE -eq 0 ]]; then
    mkdir -p build
    cd build
    cmake ..
fi

prnt_info "Done!"
