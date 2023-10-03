/** @brief calculates `(a+i*b) * (c+i*d)` according to C99 Annex G */
%C __mul%I3(%T a, %T b, %T c, %T d);

/** @brief calculates `(a+i*b) / (c+i*d)` according to C99 Annex G */
%C __div%I3(%T a, %T b, %T c, %T d);

/** @brief calculates `a * b` according to C99 Annex G */
%C __cmul%I3(%C a, %C b);

/** @brief calculates `a / b` according to C99 Annex G */
%C __cdiv%I3(%C a, %C b);