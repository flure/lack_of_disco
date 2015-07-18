#ifndef DISCOBALL_FX_H_
#define DISCOBALL_FX_H_

#include <nds.h>

/* Initialise la boule disco et tout ce qui va autour */
void DiscoballFxInit(void);

/* Dessine la boule disco et les flashs et rayons de lumière */
void DiscoballFxDraw(int32 time);

/* Libère la mémoire allouée */
void DiscoballFxDeInit(void);

/* Ajoute un rayon de lumière */
void DiscoballFxAddRay(int iquad, rgb color, int32 start_time, int32 duration, bool always);

/* Ajoute un flash à l'instant time */
void DiscoballFxStartFlash(int32 time);

/* Lance l'explosion de la boule */
void DiscoballFxExplodeStart(int32 time);

#endif /*DISCOBALL_FX_H_*/
