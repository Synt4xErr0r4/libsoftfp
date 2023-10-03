/***** FLOATING-POINT ENVIRONMENT *****/

# define SOFTFP_HAS_FENV 1

# ifdef SOFTFP_FENV
#  define feenableexcept  __softfp_feenableexcept
#  define fedisableexcept __softfp_fedisableexcept
#  define feclearexcept   __softfp_feclearexcept
#  define feraiseexcept   __softfp_feraiseexcept
#  define fetestexcept    __softfp_fetestexcept
#  define fegetexceptflag __softfp_fegetexceptflag
#  define fesetexceptflag __softfp_fesetexceptflag
#  define fegetround      __softfp_fegetround
#  define fesetround      __softfp_fesetround
#  define fegetenv        __softfp_fegetenv
#  define fesetenv        __softfp_fesetenv
#  define feholdexcept    __softfp_feholdexcept
#  define feupdateenv     __softfp_feupdateenv

#  define FE_DIVBYZERO  (1 << 0)
#  define FE_INEXACT    (1 << 1)
#  define FE_INVALID    (1 << 2)
#  define FE_OVERFLOW   (1 << 3)
#  define FE_UNDERFLOW  (1 << 4)
#  define FE_DENORM     (1 << 5) /* nonstandard */
#  define FE_ALL_EXCEPT (FE_DIVBYZERO | FE_INEXACT | FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW | FE_DENORM)

#  define FE_TONEAREST  0
#  define FE_DOWNWARD   1
#  define FE_UPWARD     2
#  define FE_TOWARDZERO 3

typedef uint16_t fenv_t;
typedef uint8_t  fexcept_t;

extern const fenv_t *const __softfp_dfl_env;

#  define FE_DFL_ENV __softfp_dfl_env

int __softfp_feenableexcept(int excepts);
int __softfp_fedisableexcept(int excepts);
int __softfp_feclearexcept(int excepts);
int __softfp_feraiseexcept(int excepts);
int __softfp_fetestexcept(int excepts);
int __softfp_fegetexceptflag(fexcept_t *flagp, int excepts);
int __softfp_fesetexceptflag(const fexcept_t *flagp, int excepts);
int __softfp_fegetround(void);
int __softfp_fesetround(int round);
int __softfp_fegetenv(fenv_t *envp);
int __softfp_fesetenv(const fenv_t *envp);
int __softfp_feholdexcept(fenv_t *envp);
int __softfp_feupdateenv(const fenv_t *envp);

# endif
