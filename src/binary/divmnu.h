#include <stdint.h>

static int nlz16(uint16_t n) {
    unsigned z;
    for(z = 0; z < 16; ++z) {
            if(n & 0x8000u)
                break;
            n = n << 1;
    }
    return z;
}

static int divmnu(uint16_t q[], uint16_t r[], const uint16_t u[], const uint16_t v[], int m, int n) {
    const unsigned b = 65536;
    uint16_t un[16], vn[16];
    unsigned qhat;
    unsigned rhat;
    unsigned p;
    int s, i, j, t, k;

    if(m < n || n <= 0 || !v[n - 1])
        return 1;

    if(n == 1) {
        k = 0;
        for(j = m - 1; j >= 0; --j) {
            q[j] = (k*b + u[j]) / v[0];
            k = (k * b + u[j]) - q[j] * v[0];
        }
        if(r) r[0] = k;
        return 0;
    }

    s = nlz16(v[n - 1]); /* 0 <= s <= 16. */
    for(i = n - 1; i > 0; --i)
        vn[i] = (v[i] << s) | (v[i - 1] >> (16 - s));
    vn[0] = v[0] << s;
    un[m] = u[m-1] >> (16 - s);
    for(i = m - 1; i > 0; --i)
        un[i] = (u[i] << s) | (u[i - 1] >> (16 - s));
    un[0] = u[0] << s;

    for(j = m - n; j >= 0; --j) {
        qhat = (un[j+n]*b + un[j+n-1])/vn[n-1];
        rhat = (un[j+n]*b + un[j+n-1]) - qhat*vn[n-1];
again:
        if(qhat >= b || qhat*vn[n-2] > b*rhat + un[j+n-2]) {
            qhat = qhat - 1;
            rhat = rhat + vn[n-1];
            if(rhat < b) goto again;
        }

        k = 0;
        for(i = 0; i < n; ++i) {
            p = qhat*vn[i];
            t = un[i + j] - k - (p & 0xFFFF);
            un[i+j] = t;
            k = (p >> 16) - (t >> 16);
        }
        t = un[j+n] - k;
        un[j + n] = t;

        q[j] = qhat;

        if(t < 0) {
            q[j] = q[j] - 1;
            k = 0;
            for(i = 0; i < n; ++i) {
                t = un[i + j] + vn[i] + k;
                un[i + j] = t;
                k = t >> 16;
            }
            un[j+n] = un[j+n] + k;
        }
    }

    if(r)
        for(i = 0; i < n; ++i)
            r[i] = (un[i] >> s) | (un[i + 1] << (16 - s));

    return 0;
}