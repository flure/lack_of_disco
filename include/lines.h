#ifndef LINES_H_
#define LINES_H_

#include <nds.h>

#include <usings.h>

extern u8 *LinesScreen8;
extern u16 *LinesScreen16;
/* !!! ATTENTION !!!
 * Les fonctions suivantes ne font pas de tests sur la validité des paramètres !
 * Veillez donc à ce que les paramètres soient cohérents, sous peine de plantage !
 * 
 * De plus, pour des raisons d'optimisation, ces fonctions supposent que l'écran a une largeur de 256 pixels.
 */

#ifdef USING_DrawLine_8bpp_H
//void DrawLine_8bpp_H(register int32 x1, register int32 x2, register int32 y, register u8 color);
#define DrawLine_8bpp_H(x1, x2, y, color) memset(LinesScreen8 + x1 + (y<<8), color, x2 - x1)
#endif /* USING_DrawLine_8bpp_H */

#ifdef USING_DrawLine_8bpp_V
void DrawLine_8bpp_V(register int32 y1, register int32 y2, register int32 x, register u8 color);
#endif /* USING_DrawLine_8bpp_V */

#ifdef USING_DrawLine_8bpp
void DrawLine_8bpp(register int32 x1, register int32 y1, register int32 x2, register int32 y2, register u8 color);
#endif /* USING_DrawLine_8bpp */

#ifdef USING_DrawLine_16bpp_H
//void DrawLine_16bpp_H(register int32 x1, register int32 x2, register int32 y, register u16 color);
#define DrawLine_16bpp_H(x1, x2, y, color) dmaFillHalfWords(color, LinesScreen16 + x1 + (y<<8), (x2-x1)<<1)
#endif /* USING_DrawLine_16bpp_H */

#ifdef USING_DrawLine_16bpp_V
void DrawLine_16bpp_V(register int32 y1, register int32 y2, register int32 x, register u16 color);
#endif /* USING_DrawLine_16bpp_V */

#ifdef USING_SetLinesScreen8bpp
void SetLinesScreen8bpp(register u8* screen);
#endif /* USING_SetLinesScreen8bpp */

#ifdef USING_SetLinesScreen16bpp
void SetLinesScreen16bpp(register u16* screen);
#endif /* USING_SetLinesScreen16bpp */

#endif /*LINES_H_*/
