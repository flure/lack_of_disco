#ifndef TUNNEL_FX_H_
#define TUNNEL_FX_H_

#include <nds.h>

/* Initialise le tunnel et tout ce qui va autour */
void TunnelFxInit(u16* pixels);

void TunnelFxDraw(int time, int32 speed_forward, int32 speed_rotate);

/* Libère la mémoire allouée */
void TunnelFxDeinit(void);


#endif /*TUNNEL_FX_H_*/
