#include <interpolation.h>

/* cf. http://sol.gfxile.net/interpolation/ */

/* Interpolation linéaire entre a et b à l'instant t
 * a, b et t sont de type f32
 * t doit être compris entre 0 et 1
 */
int32 InterpolateLinearf32(int32 a, int32 b, int32 t)
{
	return ((a * ((1<<12) - t))>>12) + ((b * t)>>12);
}

#define SMOOTHSTEPf32(x) ( ((((x) * (x))>>12) * ((3<<12) - 2 * (x)))>>12 )

/* Interpolation "smooth" entre a et b à l'instant t
 * a, b et t sont de type f32
 * t doit être compris entre 0 et 1
 */
int32 InterpolateSmoothf32(int32 a, int32 b, int32 t)
{
	int32 v = SMOOTHSTEPf32(t);
	
	return ((a * ((1<<12) - v))>>12) + ((b * v)>>12);
}

#define SMOOTHCOSf32(x) ((1<<12) - cosLerp((((x) * (DEGREES_IN_CIRCLE/2)))>>12)) / 2;

/* Interpolation cosinusoidale entre a et b à l'instant t
 * a, b et t sont de type f32
 * t doit être compris entre 0 et 1
 */
int32 InterpolateCosf32(int32 a, int32 b, int32 t)
{
	// v = (1-cos(t*PI))/2;
	int32 v = SMOOTHCOSf32(t); //((1<<12) - cosLerp(((t * (DEGREES_IN_CIRCLE/2)))>>12)) / 2;
	v = SMOOTHCOSf32(v); 
	
	return ((a * ((1<<12) - v))>>12) + ((b * v)>>12);
}

int32 _CatmullRom(int32 a, int32 b, int32 c, int32 d, int32 t)
{
	// cf http://sol.gfxile.net/interpolation/index.html#c8
	int32 t2, t3;
	t2 = (t*t)>>12;
	t3 = (t*t2)>>12;
	/*
	 * return 0.5f * (
           (2 * p1) + 
           (-p0 + p2) * t + 
           (2 * p0 - 5 * p1 + 4 * p2 - p3) * t * t + 
           (-p0 + 3 * p1 - 3 * p2 + p3) * t * t * t 
           );
	 * 
	 */
	 
	 return	((b<<1) +
			(((-a + c) * t) >> 12) +
			((((a<<1) - (5*b) + (c<<2) - d) * t2) >> 12) +
			(((-a + (3*b) - (3*c) + d) * t3) >> 12)) >> 1;
}

/* Interpolation spline avec la méthode Catmull-Rom
 * entre les points b et c à l'instant t
 * Les points a et d doivent être fournis
 * t doit être compris entre 0 et 1
 */
int32 SplineCatmullRomf32(int32 a, int32 b, int32 c, int32 d, int32 t)
{
	// cf http://sol.gfxile.net/interpolation/index.html#c8
	int32 v = _CatmullRom(a, b, c, d, t);
	
	return ((b * ((1<<12) - v))>>12) + ((c * v)>>12);	
}

#undef SMOOTHSTEPf32