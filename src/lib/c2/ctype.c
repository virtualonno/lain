#include <ctype.h>

char _ctype_[] = {
    0, _C, _C, _C, _C, _C, _C, _C,
    _C, _C, _S, _S, _S, _S, _S, _C,
    _C, _C, _C, _C, _C, _C, _C, _C,
    _C, _C, _C, _C, _C, _C, _C, _C,
    _C, _P|_S, _P, _P, _P, _P, _P, _P,
    _P, _P, _P, _P, _P, _P, _P, _P,
    _P, _N, _N, _N, _N, _N, _N, _N,
    _N, _N, _N, _P, _P, _P, _P, _P,
    _P, _P, _U|_X, _U|_X, _U|_X, _U|_X, _U|_X, _U|_X,
    _U, _U, _U, _U, _U, _U, _U, _U,
    _U, _U, _U, _U, _U, _U, _U, _U,
    _U, _U, _U, _U, _P, _P, _P, _P,
    _P, _P, _L|_X, _L|_X, _L|_X, _L|_X, _L|_X, _L|_X,
    _L, _L, _L, _L, _L, _L, _L, _L,
    _L, _L, _L, _L, _L, _L, _L, _L,
    _L, _L, _L, _L, _P, _P, _P, _P,
    _C,
};

char toupper(char c)
{
    if (islower(c)) {
        c -= 'a' - 'A';
    }
    return c;
}

char tolower(char c)
{
    if (isupper(c)) {
        c += 'a' - 'A';
    }
    return c;
}
