#include <lines.h>
#include <myutil.h>

#include <string.h>

u8 *LinesScreen8;
u16 *LinesScreen16;

/*#ifdef USING_DrawLine_8bpp_H
void DrawLine_8bpp_H(register int32 x1, register int32 x2, register int32 y, register u8 color)
{
	register int32 nb_px;
	
	nb_px = x2-x1;
	
	memset(LinesScreen8 + x1 + (y<<8), color, nb_px);
}
#endif*/ /* USING_DrawLine_8bpp_H */

#ifdef USING_DrawLine_8bpp_V
void DrawLine_8bpp_V(register int32 y1, register int32 y2, register int32 x, register u8 color)
{
	register int32 i;
	register u8 *screen_ptr = LinesScreen8 + x + (y1<<8);
	
	for(i=y1; i<=y2; i++) {
		*screen_ptr = color;
		screen_ptr += 256;
	}
}
#endif /* USING_DrawLine_8bpp_V */

#ifdef USING_DrawLine_16bpp_H
//void DrawLine_16bpp_H(register int32 x1, register int32 x2, register int32 y, register u16 color)
//{
//	register int32 i, nb_pixels = x2-x1;
//	register u16* screen_ptr = LinesScreen16 + x1 + (y<<8);
//	
//	//memset(screen_ptr, color, (x2-x1)*2);
//	dmaFillHalfWords(color, screen_ptr, (x2-x1)*2);
//	
//	/*for(i=0; i<=nb_pixels; i++) {
//		*screen_ptr = color;
//		screen_ptr++;
//	}*/
//}
#endif /* USING_DrawLine_16bpp_H */

#ifdef USING_DrawLine_16bpp_V
void DrawLine_16bpp_V(register int32 y1, register int32 y2, register int32 x, register u16 color)
{
	register int32 i;
	register u16 *screen_ptr = LinesScreen16 + x + (y1<<8);
	
	for(i=y1; i<=y2; i++) {
		*screen_ptr = color;
		screen_ptr += 256;
	}
}
#endif /* USING_DrawLine_16bpp_V */


#ifdef USING_DrawLine_8bpp
void DrawLine_8bpp(register int32 x1, register int32 y1, register int32 x2, register int32 y2, register u8 color)
{
	int32 dx, dy;
	register int32 xinc1, xinc2;
	register int32 den, num, numadd, nb_px, i;
	register int32 offsinc1, offsinc2;
	register u8* screen_ptr;
	
	if(x1 == x2) {
		if(y1 < y2) {
			DrawLine_8bpp_V(y1, y2, x1, color);
		} else {
			DrawLine_8bpp_V(y2, y1, x1, color);
		}
		return;
	}
	
	if(y1 == y2) {
		if(x1 < x2) {
			DrawLine_8bpp_H(x1, x2, y1, color);
		} else {
			DrawLine_8bpp_H(x2, x1, y1, color);
		}
		return;
	}
	
	dx = ABS(x2 - x1);
	dy = ABS(y2 - y1);
	
	if(x2 >= x1)                 // The x-values are increasing
	{
		xinc1 = 1;
		xinc2 = 1;
	}
	else                          // The x-values are decreasing
	{
		xinc1 = -1;
		xinc2 = -1;
	}

	if(y2 >= y1)                 // The y-values are increasing
	{
		offsinc1 = 256;
		offsinc2 = 256;
	}
	else                          // The y-values are decreasing
	{
		offsinc1 = -256;
		offsinc2 = -256;
	}
	
	if (dx >= dy)         // There is at least one x-value for every y-value
	{
		xinc1 = 0;                  // Don't change the x when numerator >= denominator
		offsinc2 = 0;
		den = dx;
		num = dx >> 1;
		numadd = dy;
		nb_px = dx;         // There are more x-values than y-values
	}
	else                          // There is at least one y-value for every x-value
	{
		xinc2 = 0;                  // Don't change the x for every iteration
  		offsinc1 = 0;
  		den = dy;
  		num = dy >> 1;
  		numadd = dx;
  		nb_px = dy;         // There are more y-values than x-values
	}

	screen_ptr = LinesScreen8 + x1 + (y1<<8);
	for (i = 0; i <= nb_px; i++)
	{
	  	*screen_ptr = color;
	  	
	  	num += numadd;              // Increase the numerator by the top of the fraction
	  	if (num >= den)             // Check if numerator >= denominator
	  	{
	    	num -= den;               // Calculate the new numerator value
	    	screen_ptr += offsinc1;
	    	screen_ptr += xinc1;
	  	}
	  	screen_ptr += offsinc2;
	  	screen_ptr += xinc2;
	}
}
#endif /* USING_DrawLine_8bpp */

#ifdef USING_SetLinesScreen8bpp
void SetLinesScreen8bpp(register u8* screen)
{
	LinesScreen8 = screen;
}
#endif /* USING_SetLinesScreen8bpp */

#ifdef USING_SetLinesScreen16bpp
void SetLinesScreen16bpp(register u16* screen)
{
	LinesScreen16 = screen;
}
#endif /* USING_SetLinesScreen16bpp */