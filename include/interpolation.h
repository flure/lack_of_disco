#ifndef INTERPOLATION_H_
#define INTERPOLATION_H_

#include <nds.h>

typedef int32 (*InterpolateFunc) (int32, int32, int32);
typedef int32 (*SplineFunc) (int32, int32, int32, int32, int32);

/* Interpolation linéaire entre a et b à l'instant t
 * a, b et t sont de type f32
 * t doit être compris entre 0 et 1
 */
int32 InterpolateLinearf32(int32 a, int32 b, int32 t);

/* Interpolation "smooth" entre a et b à l'instant t
 * a, b et t sont de type f32
 * t doit être compris entre 0 et 1
 */
int32 InterpolateSmoothf32(int32 a, int32 b, int32 t);

/* Interpolation cosinusoidale entre a et b à l'instant t
 * a, b et t sont de type f32
 * t doit être compris entre 0 et 1
 */
int32 InterpolateCosf32(int32 a, int32 b, int32 t);

/* Interpolation spline avec la méthode Catmull-Rom
 * entre les points b et c à l'instant t
 * Les points a et d doivent être fournis
 * t doit être compris entre 0 et 1
 */
int32 SplineCatmullRomf32(int32 a, int32 b, int32 c, int32 d, int32 t);

#endif /*INTERPOLATION_H_*/
