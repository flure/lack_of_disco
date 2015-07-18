#ifndef MYUTIL_H
#define MYUTIL_H

#include <nds.h>

#define ABS(x) ((x) > 0 ? (x) : (-(x)))
#define MAX(a, b) ((a)>(b) ? (a) : (b))
#define MIN(a, b) ((a)<(b) ? (a) : (b))

#define ATAN_LUT_DEGREES 256
#define ATAN2(y, x) Atan_Lut[(x&0xFF) + ((y%192)<<8)]
extern int Atan_Lut[];

/* Ajoute un flash à l'instant time */
void StartFlash(int32 time);
void DrawFlash(int32 time);

#endif /* MYUTIL_H */
