#include <sys/types.h>
#include <libgte.h>

/* Multiplies m on the right by a rotation about Z; rz is in 1/4096 degree. */
void gte_rotate_z_matrix(MATRIX *m, int rz)
{
    int angle;
    int c;
    int s;
    MATRIX rot;

    angle = rz / 360;
    c = rcos(angle);
    s = rsin(angle);
    if (rz != 0) {
        rot.m[0][1] = -s;
        rot.m[0][0] = c;
        rot.m[0][2] = 0;
        rot.m[1][0] = s;
        rot.m[1][1] = c;
        rot.m[1][2] = 0;
        rot.m[2][0] = 0;
        rot.m[2][1] = 0;
        rot.m[2][2] = 4096;
        rot.t[0] = 0;
        rot.t[1] = 0;
        rot.t[2] = 0;
        MulMatrix(m, &rot);
    }
}
