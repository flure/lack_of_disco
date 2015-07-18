#include <pqtorus_fx.h>
#include <util3d.h>
#include <pqtorus.h>
#include <pqtorusmap.h>
#include <credits_flure.h>
#include <credits_boub.h>
#include <credits_stv.h>
#include <credits_taggy.h>
#include <credits_profil.h>
#include <myutil.h>

#define NB_CREDITS 5

TPQTorus *torus[NB_CREDITS];
T_Texture *torus_tex = NULL;

static u8 *credits[NB_CREDITS];

extern int bgCreditsId;
extern u16* bgCreditsPixels;
static int32 TorusZ = 0;
static int CurrentCredit = 0;
static int CurrentTorus = 0;
static int32 CreditStartTime = -5000;
#define TORUS_CHANGE_TIME 500
#define CREDIT_DURATION 3000
static int CreditsPQ[NB_CREDITS][2] = {
	{5, 1},
	{2, 4},
	{3, 1},
	{7, 3},
	{2, 5}
};


void ShowNextCredit(int time)
{
	CurrentCredit++;
	if(CurrentCredit == NB_CREDITS) CurrentCredit = 0;
	
	dmaCopyWords(3, credits[CurrentCredit], bgCreditsPixels, 256*192*2);
	BLEND_CR = BLEND_ALPHA | BLEND_SRC_BG3 | BLEND_DST_BG0;
	BLEND_AB = (16<<8);
	
	CreditStartTime = time;	
}

void UpdateNextTorus(int time)
{
	int32 theta, t;
	if(time < CreditStartTime) return;
	if(time > (CreditStartTime + TORUS_CHANGE_TIME)) return;
	
	t = time - CreditStartTime;
	
	theta = t * (DEGREES_IN_CIRCLE/2) / TORUS_CHANGE_TIME;
	
	if(theta >= (DEGREES_IN_CIRCLE/4 - 100)) {
		CurrentTorus = CurrentCredit;
	}
		
	TorusZ = (ABS(sinLerp(theta)) * 3);
}


void UpdateCurrentCredit(time)
{
	int32 theta, t, alpha;
	
	if( time < CreditStartTime ) return;
	if( time > (CreditStartTime + CREDIT_DURATION)) return;
	
	t = time - CreditStartTime;
	
	theta = t * (DEGREES_IN_CIRCLE/2) / CREDIT_DURATION;			
	alpha = (ABS(sinLerp(theta) * 16)) >> 12;
	
	BLEND_AB = alpha | (16<<8);
}

void PQTorusFxInit()
{
	int i;
	
	for(i=0; i<NB_CREDITS; i++) {
		torus[i] = CreatePQTorus(CreditsPQ[i][0], CreditsPQ[i][1], 200, 7, floattof32(0.15), floattof32(0.5)); 
    	PQTorusSetWaves(torus[i], 0, 24, floattof32(7));
	}
	
	/* Chargement des gfx pour les credits */
	credits[0] = malloc(256*192*2);
	decompress(credits_boubBitmap,		credits[0], LZ77);
	credits[1] = malloc(256*192*2);
	decompress(credits_flureBitmap,		credits[1], LZ77);	
	credits[2] = malloc(256*192*2);
	decompress(credits_stvBitmap, 		credits[2], LZ77);
	credits[3] = malloc(256*192*2);
	decompress(credits_taggyBitmap, 	credits[3], LZ77);
	credits[4] = malloc(256*192*2);
	decompress(credits_profilBitmap, 	credits[4], LZ77);
    
	glMaterialf(GL_AMBIENT, RGB15(12, 12, 16));
    glMaterialf(GL_SPECULAR, BIT(15) | RGB15(31,31,31));
	glMaterialShinyness();
		
	glResetTextures();
	torus_tex = CreateTextureFromMemory((u8*)pqtorusmapBitmap, GL_RGB, GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | TEXGEN_NORMAL, 16, 128, TRUE, LZ77Vram);
	
	glClearColor(12, 12, 16, 31);
}

void PQTorusFxDraw(int time)
{
	int32 keys;
	GLvector tex_scale = { 32<<16, -32<<16, 1<<16 };	
	
	scanKeys();
	keys = keysDown();
	
//	if(keys & KEY_A) {
//		ShowNextCredit(time);
//	}
	UpdateCurrentCredit(time);
	UpdateNextTorus(time);
	
	torus[CurrentCredit]->wave_amplitude = 0.1 * sinLerp(time<<3);
	PQTorusUpdate(torus[CurrentTorus], time);
			
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glScalev( &tex_scale );		//scale normals up from (-1,1) range into texcoords
	glRotatef32i(time<<1, 10<<12, 10<<12, 10<<12);
		
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLight(0, ARGB16(1, 31, 31, 16), 0, 0, floattov10(-1));
	glTranslate3f32(floattof32(0.5), floattof32(-0.5), floattof32(-2) - TorusZ);
	glRotatef32i(time<<1, 10<<12, 10<<12, 10<<12);
	glScalef32(floattof32(2));
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, torus_tex->id);
	glPolyFmt(POLY_ALPHA(31) | POLY_ID(10) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0 | POLY_MODULATION);
	PQTorusRender(torus[CurrentTorus]);	
}


void PQTorusFxDeinit(void)
{
	int i;

	for(i=0; i<NB_CREDITS; i++) {
		DestroyPQTorus(torus[i]);
	}
	FreeTexture(torus_tex);
	
	for(i=0; i<NB_CREDITS; i++) {
		free(credits[i]);
	}
}