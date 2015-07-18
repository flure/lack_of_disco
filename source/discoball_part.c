#include <nds.h>
#include <maxmod9.h>

#include <myutil.h>
#include <util3d.h>
#include <demosystem.h>
#include <paths.h>

#include <string.h>


#include <data.h>
#include <discoball_part.h>
#include <discoball_fx.h>

void DiscoballPartInit(void);
void DiscoballPartExec(int time);
void DiscoballPartDeinit(void);

int MainBgIdBack,MainBgIdFront, SubBgId;
u8* MainBgBackPixels;
u8* MainBgFrontPixels;
u16* SubBgPixels;


static int32 CurrentTime;

//#define NB_RAYCOLORS 5
//static rgb RayColors[NB_RAYCOLORS] = {
//	RGB15(31, 31, 0),
//	RGB15(31, 0, 0),
//	RGB15(0, 31, 0),
//	RGB15(0, 31, 31),
//	RGB15(31, 0, 31)
//};

static tPath* DiscoBallPath;

static void FadeIn(int32 time)
{
	int32 bright, t;
	
	t = (time<<12) / 1000;
	bright = (16 * ((1<<12)-t)) >> 12;
	
	REG_MASTER_BRIGHT = (2<<14) | bright;
	REG_MASTER_BRIGHT_SUB = REG_MASTER_BRIGHT;
}

void InitDiscoballPath(void)
{
	DiscoBallPath = (tPath*)malloc(sizeof(tPath));
	DiscoBallPath->nb_path_points = 5;
	DiscoBallPath->path_points = (tPathPoint*)malloc(DiscoBallPath->nb_path_points * sizeof(tPathPoint));
	
	DiscoBallPath->path_points[0].time = 0;
	DiscoBallPath->path_points[0].x = floattof32(-0.5);
	DiscoBallPath->path_points[0].y = floattof32( 0.0);
	DiscoBallPath->path_points[0].z = floattof32(-6.0);
	
	DiscoBallPath->path_points[1].time = 3000;
	DiscoBallPath->path_points[1].x = floattof32( 0.0);
	DiscoBallPath->path_points[1].y = floattof32( 0.0);
	DiscoBallPath->path_points[1].z = floattof32(-4.0);
	
	DiscoBallPath->path_points[2].time = 6000;
	DiscoBallPath->path_points[2].x = floattof32( 1.0);
	DiscoBallPath->path_points[2].y = floattof32( 1.0);
	DiscoBallPath->path_points[2].z = floattof32(-3.0);
	
	DiscoBallPath->path_points[3].time = 10000;
	DiscoBallPath->path_points[3].x = floattof32( 0.8);
	DiscoBallPath->path_points[3].y = floattof32( 0.5);
	DiscoBallPath->path_points[3].z = floattof32(-2.5);
	
	DiscoBallPath->path_points[4].time = 20000;
	DiscoBallPath->path_points[4].x = floattof32( 0.3);
	DiscoBallPath->path_points[4].y = floattof32( 0.3);
	DiscoBallPath->path_points[4].z = floattof32(-2.0);
}

mm_word DiscobalPartSync( mm_word msg, mm_word param )
{
    switch( msg )
    {
    case MMCB_SONGMESSAGE:
    	switch(param) {
    	case 0: 
    		/* flash blanc */
    		//DiscoballFxStartFlash(CurrentTime);
    		StartFlash(CurrentTime);
    		break;
    	case 1:
    		/* rayons */
        	DiscoballFxAddRay(-1, RGB15(21+rand()%10, 21+rand()%10, 21+rand()%10), CurrentTime, 500, FALSE);
        	DiscoballFxAddRay(-1, RGB15(21+rand()%10, 21+rand()%10, 21+rand()%10), CurrentTime, 500, FALSE);
        	DiscoballFxAddRay(-1, RGB15(21+rand()%10, 21+rand()%10, 21+rand()%10), CurrentTime, 500, FALSE);
        	break;
        case 2:
        	/* Explosions */
        	DiscoballFxExplodeStart(CurrentTime);
        	break;
        default:
        	break;
    	}
        break;
//    case MMCB_SONGFINISHED:
//        // A song has finished playing
    }
    
    return 0;
}


void DiscoballPartInit(void)
{
	/* 
	 * Initialisation vidéo
	 */ 
	/* Modes video */
	videoSetMode(MODE_5_3D);
	videoSetModeSub(MODE_5_2D);
	
	vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
	vramSetBankB(VRAM_B_MAIN_BG_0x06020000);
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);
	vramSetBankD(VRAM_D_TEXTURE_SLOT3);
	vramSetBankE(VRAM_E_TEX_PALETTE);
	
	videoBgEnable(DISPLAY_BG0_ACTIVE | DISPLAY_BG2_ACTIVE | DISPLAY_BG3_ACTIVE);
	videoBgEnableSub(DISPLAY_BG3_ACTIVE);
	
	/* Les Gfx de la scène 3D */
	/* Arrière-plan */
	MainBgIdBack = bgInit(2, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	MainBgBackPixels = (u8*)bgGetGfxPtr(MainBgIdBack);
	decompress(LackOfDisco_backBitmap, MainBgBackPixels, LZ77Vram);
	memcpy(BG_PALETTE, LackOfDisco_backPal, LackOfDisco_backPalLen);
	bgSetPriority(MainBgIdBack, 2);
	/* Avant-plan */
	MainBgIdFront = bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 4, 0);
	MainBgFrontPixels = (u8*)bgGetGfxPtr(MainBgIdFront);
	decompress(LackOfDisco_frontBitmap, MainBgFrontPixels, LZ77Vram);
	bgSetPriority(MainBgIdFront, 0);
	bgSetPriority(0, 1); /* La 3D par dessus le bg d'arrière plan, par dessous le bg d'avant plan 
	                        La 3D en priorité 1 comme ça il suffit de faire passer l'avant-plan en 2 pour faire passer la discoball par-dessus */

	/* Le Gfx 2D */
	SubBgId = bgInitSub(2, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	SubBgPixels = (u16*)bgGetGfxPtr(SubBgId);
	decompress(LogoElephantBitmap, SubBgPixels, LZ77Vram);
	
	/* Initialisation de la 3D */
	glViewport(0, 0, 255, 191);
	glClearColor(31, 31, 31, 0);
	glClearDepth(GL_MAX_DEPTH);
	glClearPolyID(63);
	glEnable(GL_ANTIALIAS);
    
    ChangeFov(70);
    
    DiscoballFxInit();
    InitDiscoballPath();
    
    /* Pour être sûr que la transparence des rayons de lumière va fonctionner avec l'arrière-plan */	
	BLEND_CR = BLEND_ALPHA | BLEND_SRC_BG0 | BLEND_DST_BG2;

	/* Initialisation du fade in depuis le noir */
	REG_MASTER_BRIGHT = (2<<14);
	REG_MASTER_BRIGHT_SUB = REG_MASTER_BRIGHT;
}

void DiscoballPartExec(int time)
{	
	//int32 keys;
	int32 xyz[3];
	
	CurrentTime = time;
	
//	scanKeys();
//	keys = keysDown();
//	
//	if(keys & KEY_A) {
//		DiscoballFxAddRay(rand()%(16*16-1), RGB15(16+rand()%16,16+rand()%16,16+rand()%16), CurrentTime, 1000, true);
//	}
	
	PathFollow(DiscoBallPath, time, InterpolateCosf32, xyz);
	glLoadIdentity();
	glLight(0, ARGB16(1, 31, 31, 31), 0, 0, floattov10(-1));	
	glTranslate3f32(xyz[0], xyz[1], xyz[2] );
	glRotatef32i(time<<2, 0.1*(cosLerp(time<<4)<<4), 1<<16, 0.1*(sinLerp(time<<4)<<4));
	DiscoballFxDraw(time);
	
	if(time < 1000) {
		FadeIn(time);
	}
	
	if(time > 10000) {
		bgHide(MainBgIdFront);
	}
	
	glFlush(0);
	swiWaitForVBlank();
}

void DiscoballPartDeinit(void)
{
	DiscoballFxDeInit();
	free(DiscoBallPath->path_points);
    free(DiscoBallPath);
}

void CreateDiscoballPart(T_Part* part)
{
	DemoCreatePart(part, DiscoballPartInit, DiscoballPartExec, DiscoballPartDeinit, DiscobalPartSync, 27*1000); //30*1000);
}

#undef NB_RAYCOLORS