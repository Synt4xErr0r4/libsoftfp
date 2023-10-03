#define SOFTFP_FENV
#include "softfp.h"

#ifdef SOFTFP_HAS_FENV

# include <signal.h>

# define EXCEPTION_OFFSET 0
# define MASK_OFFSET      6
# define ROUND_OFFSET     12
# define HOLD_OFFSET      14

const fenv_t dfl_env = 0;
const fenv_t *const __softfp_dfl_env = &dfl_env;

static void check_exceptions(fenv_t fenv) {
    if(fenv & (1 << HOLD_OFFSET))
        return;

    if((fenv >> EXCEPTION_OFFSET) & (fenv >> MASK_OFFSET) & FE_ALL_EXCEPT)
        raise(SIGFPE);
}

static _Thread_local fenv_t fenv = dfl_env;

int __softfp_fedisableexcept(int excepts) {
    fenv &= ~((excepts & FE_ALL_EXCEPT) << MASK_OFFSET);
    return 0;
}

int __softfp_feenableexcept(int excepts) {
    fenv |= (excepts & FE_ALL_EXCEPT) << MASK_OFFSET;
    check_exceptions(fenv);
    return 0;
}

int __softfp_feclearexcept(int excepts) {
    fenv &= ~((excepts & FE_ALL_EXCEPT) << EXCEPTION_OFFSET);
    return 0;
}

int __softfp_feraiseexcept(int excepts) {
    fenv |= (excepts & FE_ALL_EXCEPT) << EXCEPTION_OFFSET;
    check_exceptions(fenv);
    return 0;
}

int __softfp_fetestexcept(int excepts) {
    return (fenv >> EXCEPTION_OFFSET) & excepts & FE_ALL_EXCEPT;
}

int __softfp_fegetexceptflag(fexcept_t *flagp, int excepts) {
    *flagp = __softfp_fetestexcept(excepts);
    return 0;
}

int __softfp_fesetexceptflag(const fexcept_t *flagp, int excepts) {
    fenv |= (excepts & FE_ALL_EXCEPT) << EXCEPTION_OFFSET;
    return 0;
}

int __softfp_fegetround(void) {
    return (fenv >> ROUND_OFFSET) & 3;
}

int __softfp_fesetround(int round) {
    fenv |= (round & 3) << ROUND_OFFSET;
    return 0;
}

int __softfp_fegetenv(fenv_t *envp) {
    *envp = fenv;
    return 0;
}

int __softfp_fesetenv(const fenv_t *envp) {
    fenv = *envp;
    return 0;
}

int __softfp_feholdexcept(fenv_t *envp) {
    *envp = fenv;
    fenv |= 1 << HOLD_OFFSET;
    return 0;
}

int __softfp_feupdateenv(const fenv_t *envp) {
    fenv_t old_fenv = fenv & ~(1 << HOLD_OFFSET);
    fenv = *envp;
    check_exceptions(old_fenv);
    return 0;
}

#endif