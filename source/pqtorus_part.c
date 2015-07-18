#include <nds.h>

#include <demosystem.h>
#include <util3d.h>

#include <pqtorus_part.h>
#include <pqtorus_fx.h>

#include <myutil.h>

#include <popsy_golden.h>

int bgSubId;
int bgCreditsId;
u16* bgSubPixels;
u16* bgCreditsPixels;

static int32 CurrentTime;
static int CreditCount = 0;

void ShowNextCredit(int time);

static void FadeIn(int32 time)
{
	int32 bright, t;
	
	t = (time<<12) / 2000;
	bright = (16 * ((1<<12)-t)) >> 12;
	
	REG_MASTER_BRIGHT = (1<<14) | bright;
	REG_MASTER_BRIGHT_SUB = REG_MASTER_BRIGHT;
}

static void FadeOut(int32 time)
{
	int32 bright;
	int step = 2000 / 16;
	
	bright = (time - 28000) / step;
	
	REG_MASTER_BRIGHT = (1<<14) | bright;
	REG_MASTER_BRIGHT_SUB = REG_MASTER_BRIGHT;
}

mm_word PQTorusPartSync(mm_word msg, mm_word param)
{
	switch( msg )
    {
    case MMCB_SONGMESSAGE:
    	switch(param) {
    	case 0: 
    		StartFlash(CurrentTime);
    		break;
    	case 3:
    		if(CreditCount >= 5) break;
    		ShowNextCredit(CurrentTime);
    		CreditCount++;
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

void PQTorusPartInit(void)
{
	/* 
	 * Initialisation vidéo
	 */ 
	/* Modes video */
	videoSetMode(MODE_5_3D);
	videoSetModeSub(MODE_5_2D);
	lcdMainOnTop();
	
	vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
	vramSetBankB(VRAM_B_MAIN_BG_0x06020000);
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);
	vramSetBankD(VRAM_D_TEXTURE_SLOT3);
	
	videoBgEnable(DISPLAY_BG0_ACTIVE | DISPLAY_BG3_ACTIVE);
	videoBgEnableSub(DISPLAY_BG2_ACTIVE);
	
	bgSubId = bgInitSub(2, BgType_Bmp16, BgSize_B16_256x256, 8, 0);
	bgSubPixels = (u16*)bgGetGfxPtr(bgSubId);
	
	bgCreditsId = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	bgCreditsPixels = (u16*)bgGetGfxPtr(bgCreditsId);
	bgSetPriority(bgCreditsId, 2);
	bgSetPriority(0, 3);
	
	decompress(popsy_goldenBitmap, bgSubPixels, LZ77Vram); 
	
	/* Initialisation de la 3D */
    //Init3DEngine();
	glViewport(0, 0, 255, 191);
	glClearColor(31, 31, 31, 0);
	glClearDepth(GL_MAX_DEPTH);
	glClearPolyID(63);
	glEnable(GL_ANTIALIAS);
	ChangeFov(70);
    
    PQTorusFxInit();
}

void PQTorusPartExec(int time)
{
	CurrentTime = time;
	PQTorusFxDraw(time);
	
	if(time <= 2000) {
		FadeIn(time);
	} else if(time >= 28000) {
		FadeOut(time);
	} else {
		DrawFlash(time);
	}
	
	glFlush(0);
	swiWaitForVBlank();
}

void PQTorusPartDeinit(void)
{
	PQTorusFxDeinit();
}

void CreatePQTorusPart(T_Part* part)
{
	DemoCreatePart(part, PQTorusPartInit, PQTorusPartExec, PQTorusPartDeinit, PQTorusPartSync, 30*1000); //30*1000);
}