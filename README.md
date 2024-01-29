# libsoftfp

```diff
- - - !WORK IN PROGRESS! - - -
```

A software floating-point library.

Supports the following IEEE 754 standards:

- `binary16`
- `binary32`
- `binary64`
- `binary128`
- `binary256`
- `decimal32`
- `decimal64`
- `decimal128`

x87's 80-bit extended precision format is also supported (here called `binary80` for consistency).

## Building

Before building the library, you need to configure it. Running

```bash
./configure.sh
```

will use the default configuration. You can use

```bash
./configure.sh --help
```

to show a list of all possible configuration options.
This also generates the header file `include/softfp.h`

Then you can build the library using CMake:

```bash
mkdir build
cd build
cmake ..
make
make install # optional
```

(Note that running `./configure.sh` automatically creates the `build` directory and runs `cmake ..` unless `--no-cmake`
is specified)

## Usage

For all the supported types, the following functions are provided:

- `T __addX3(T a, T b)` (returns `a + b`)
- `T __subX3(T a, T b)` (returns `a - b`)
- `T __mulX3(T a, T b)` (returns `a * b`)
- `T __divX3(T a, T b)` (returns `a / b`)
- `T __negX3(T a)` (returns `-a`)
- `int32_t __fixXsi(T a)` (returns `(int32_t) a`)
- `int64_t __fixXdi(T a)` (returns `(int64_t) a`)
- `uint32_t __fixunsXsi(T a)` (returns `(uint32_t) a`)
- `uint64_t __fixunsXdi(T a)` (returns `(uint64_t) a`)
- `void __fixXbitint(void *r, int32_t rprec, T a)` (returns `(_BitInt(rprec)) a`)
- `T __floatXsi(int32_t a)` (returns `(sfloatN_t) a`)
- `T __floatXdi(int64_t a)` (returns `(sfloatN_t) a`)
- `T __floatunXsi(uint32_t a)` (returns `(sfloatN_t) a`)
- `T __floatunXdi(uint64_t a)` (returns `(sfloatN_t) a`)
- `T __floatbitintX(void *r, int32_t rprec)` (returns `(sfloatN_t) a`, where `a` is a `_BitInt` with `rprec` bits)
- `int __cmpX2(T a, T b)` (returns `a <=> b`)
- `int __unordX2(T a, T b)` (returns `a <=> b`)
- `int __eqX2(T a, T b)` (returns `a == b`)
- `int __neX2(T a, T b)` (returns `a != b`)
- `int __ltX2(T a, T b)` (returns `a < b`)
- `int __leX2(T a, T b)` (returns `a <= b`)
- `int __gtX2(T a, T b)` (returns `a > b`)
- `int __geX2(T a, T b)` (returns `a >= b`)

(All of these are practically analogous to
[GCC's routines for floating point emulation](https://gcc.gnu.org/onlinedocs/gccint/Soft-float-library-routines.html)
and [decimal floating point emulation](https://gcc.gnu.org/onlinedocs/gccint/Decimal-float-library-routines.html).
There you can also find the exact semantics of the comparison functions.)

Decimal floating-point types are available in two modes: DPD (densly packed decimal) and BID (binary integer decimal).
When running `./configure.sh`, you can chose which one you want to use. If you, however, decide to include both modes,
separate versions of the aforementioned functions are created for each mode. The functions then start with `__dpd_` or
`__bid_` rather than `__` (e.g., `__dpd_addX3` for the DPD version of `__addX3` and `__bid_addX3` for the BID version).

`T` is the floating-point type, `X` is its corresponding identifier:

| Type       | T             | X  |
| ---------- | ------------- | -- |
| binary16   | sfloat16_t    | hf |
| binary32   | sfloat32_t    | sf |
| binary64   | sfloat64_t    | df |
| binary80   | sfloat80_t    | tf |
| binary128  | sfloat128_t   | xf |
| binary256  | sfloat256_t   | yf |
| decimal32  | sdecimal32_t  | sd |
| decimal64  | sdecimal64_t  | dd |
| decimal128 | sdecimal128_t | td |

There are also function for converting between types:

- `T __extendXY2(S a)` and
- `T __truncXY2(S a)`

This converts a source type `S` (with identifier `X`)
into a target type `T` (with identifier `Y`).  
`extend` is used when `S` is less precise than `T`,
`trunc` is used when `S` is more precise than `T`,
as visualized in this table:

|            | binary16 | binary32 | decimal32 | binary64 | decimal64 | binary80 | binary128 | decimal128 | binary256  |
| ---------- | -------- | -------- | --------- | -------- | --------- | -------- | --------- | ---------- | ---------- |
| binary16   |          | extend   | extend    | extend   | extend    | extend   | extend    | extend     | extend     |
| binary32   | trunc    |          | extend    | extend   | extend    | extend   | extend    | extend     | extend     |
| decimal32  | trunc    | trunc    |           | extend   | extend    | extend   | extend    | extend     | extend     |
| binary64   | trunc    | trunc    | trunc     |          | extend    | extend   | extend    | extend     | extend     |
| decimal64  | trunc    | trunc    | trunc     | trunc    |           | extend   | extend    | extend     | extend     |
| binary80   | trunc    | trunc    | trunc     | trunc    | trunc     |          | extend    | extend     | extend     |
| binary128  | trunc    | trunc    | trunc     | trunc    | trunc     | trunc    |           | extend     | extend     |
| decimal128 | trunc    | trunc    | trunc     | trunc    | trunc     | trunc    | trunc     |            | extend     |
| binary256  | trunc    | trunc    | trunc     | trunc    | trunc     | trunc    | trunc     | trunc      |            |

For example, for converting from `binary16` to `binary32`, you need the `extend` function.

For the binary types (and *not* for the decimal types), there are also four complex functions:

- `C __mulX3(T a_Re, T b_Im, T c_Re, T d_Im)` (`(a + i*b) * (c + i*d)`)
- `C __divX3(T a_Re, T b_Im, T c_Re, T d_Im)` (`(a + i*b) / (c + i*d)`)
- `C __cmulX3(C a, C b)` (`a * b`)
- `C __cdivX3(C a, C b)` (`a / b`)

`C` is the complex variant of `T` (i.e., `scfloatN_t` instead of `sfloatN_t`).  
`X` is the identifier from before, except that the second letter is `c` (instead of `f`, e.g. `hf` becomes `hc`).

`mul` and `cmul` as well as `div` and `cdiv` perform the same operation,
except that `mul` and `div` take the complex numbers' separate components,
whereas `cmul` and `cdiv` take the complex numbers themselves as their parameters.

## Implementation status

- binary
  - [x] arithmetic (`add`, `sub`, `mul`, `div`, `neg`)
  - [x] integer conversions (`fix`, `float`)
  - [x] comparisons (`cmp`, `unord`, `eq`, ...)
  - [ ] complex arithmetic (`mul`, `div`, `cmul`, `cdiv`)
- decimal
  - [ ] arithmetic
  - [ ] integer conversions
  - [ ] comparisons
- type conversion (`trunc`, `extend`)
  - [ ] binary to binary
  - [ ] decimal to decimal
  - [ ] binary to decimal et vice versa
- [ ] edge cases tested

## License

This project is licensed under the [MIT License](LICENSE).
