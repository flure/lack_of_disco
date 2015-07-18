#include <nds.h>

#include <demosystem.h>
#include <util3d.h>

#include <tunnel_part.h>
#include <tunnel_fx.h>

#include <DiscoElephant_mesh.h>
#include <boule.h>

#include <data.h>

#include <myutil.h>


static int TunnelBgId;
static u16* TunnelBgPixels;
static int FrontLayerId;
static u8* FrontLayerPixels;

static T_Obj3D* DiscoElephant;
static T_Scene* Scene;

static T_Billboard* flares[4];
static T_Texture* flare_tex;
static T_Texture* tex;

static int32 CurrentTime;

mm_word TunnelPartSync(mm_word msg, mm_word param)
{
	switch( msg )
    {
    case MMCB_SONGMESSAGE:
    	switch(param) {
    	case 0: 
    		/* flash blanc */
    		StartFlash(CurrentTime);
    		break;

        default:
        	break;
    	}
//    case MMCB_SONGFINISHED:
//        // A song has finished playing
    }
    
    return 0;
}



static void FadeIn(int32 time)
{
	int32 bright, t;
	
	t = (time<<12) / 1000;
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

void TunnelPartInit(void)
{
	T_Mesh* mesh;
	int i;
	
	/* 
	 * Initialisation vidéo
	 */ 
	/* Modes video */
	videoSetMode(MODE_5_3D);
	videoSetModeSub(MODE_5_2D);
	lcdMainOnBottom();
	
	vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
	vramSetBankB(VRAM_B_MAIN_BG_0x06020000);
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);
	vramSetBankD(VRAM_D_TEXTURE_SLOT3);
	
	videoBgEnable(DISPLAY_BG0_ACTIVE | DISPLAY_BG2_ACTIVE);
	videoBgEnableSub(DISPLAY_BG2_ACTIVE | DISPLAY_BG3_ACTIVE);
	
	/* Le BG qui servira pour le tunnel */
	TunnelBgId = bgInitSub(2, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	TunnelBgPixels = (u16*)bgGetGfxPtr(TunnelBgId);
	bgSetPriority(TunnelBgId, 2); /* Le tunnel en fond */

	/* L'avant plan de l'éléphant */	
	FrontLayerId = bgInit(2, BgType_Bmp8, BgSize_B8_256x256, 8, 0);
	FrontLayerPixels = (u8*)bgGetGfxPtr(FrontLayerId);
	bgSetPriority(FrontLayerId, 0); /* En avant plan */
	decompress(bouleBitmap, FrontLayerPixels, LZ77Vram);
	decompress(boulePal, BG_PALETTE, LZ77Vram); 
	
	bgSetPriority(0, 1); /* La 3D entre les deux */
	
	
	/* Initialisation de la 3D */
    Init3DEngine();
    
	TunnelFxInit(TunnelBgPixels);
	        
    mesh = CreateMesh();
    mesh->display_list = DiscoElephant_mesh;
    mesh->alpha = 31;
    mesh->texturing = TRUE;
    mesh->lighting = TRUE;
    DiscoElephant = CreateObj3D(mesh);
    Scene = CreateScene();
    AddObject(Scene, DiscoElephant);
    
    Scene->lights[0] = CreateLight(floattov10(0), floattov10(0), floattov10(-1), RGB15(16, 0, 01), TRUE);
    Scene->lights[1] = CreateLight(floattov10(0), floattov10(0), floattov10(-1), RGB15(0, 16, 0), TRUE);
    Scene->lights[2] = CreateLight(floattov10(0), floattov10(0), floattov10(-1), RGB15(0, 0, 16), TRUE);
    Scene->lights[3] = CreateLight(floattov10(0), floattov10(0), floattov10(-1), RGB15(16, 16, 0), TRUE);
    
    glClearColor(0, 0, 0, 0);
    
    flare_tex = CreateTextureFromMemory((u8*)flareBitmap, GL_RGBA, 0, 16, 64, TRUE, LZ77Vram);
    
    for(i=0; i<4; i++) {
    	flares[i] = malloc(sizeof(*flares[i]));
    }
    
    flares[0]->position[0] = 0;
    flares[0]->position[1] = 0;
    flares[0]->position[2] = floattof32(-1);
    flares[0]->poly_alpha = 16;
    flares[0]->color = RGB15(31, 0, 0);
    flares[0]->size = floattov16(0.15f);
    flares[0]->texture_id = flare_tex->id;
    flares[0]->poly_id = 5;
    
    flares[1]->position[0] = 0;
    flares[1]->position[1] = 0;
    flares[1]->position[2] = floattof32(-1);
    flares[1]->poly_alpha = 16;
    flares[1]->color = RGB15(0, 31, 0);
    flares[1]->size = floattov16(0.15f);
    flares[1]->texture_id = flare_tex->id;
    flares[1]->poly_id = 6;
    
    flares[2]->position[0] = 0;
    flares[2]->position[1] = 0;
    flares[2]->position[2] = floattof32(-1);
    flares[2]->poly_alpha = 16;
    flares[2]->color = RGB15(0, 0, 31);
    flares[2]->size = floattov16(0.15f);
    flares[2]->texture_id = flare_tex->id;
    flares[2]->poly_id = 6;
    
    flares[3]->position[0] = 0;
    flares[3]->position[1] = 0;
    flares[3]->position[2] = floattof32(-1);
    flares[3]->poly_alpha = 16;
    flares[3]->color = RGB15(31, 31, 0);
    flares[3]->size = floattov16(0.15f);
    flares[3]->texture_id = flare_tex->id;
    flares[3]->poly_id = 7;
    
    tex = CreateTextureFromMemory((u8*)DiscoElephantBitmap, GL_RGB, 0, 16, 64, TRUE, LZ77Vram);
    DiscoElephant->mesh->texture_id = tex->id;
    DiscoElephant->mesh->texturing = TRUE;
    
	/* Initialisation du fade in depuis le blanc */
	REG_MASTER_BRIGHT = (1<<14);
	REG_MASTER_BRIGHT_SUB = REG_MASTER_BRIGHT;
	
	StartFlash(-1000); // pour éviter un flash à la première seconde
	
	glMaterialf(GL_AMBIENT, RGB15(16,16,16));
    glMaterialf(GL_SPECULAR, BIT(15) | RGB15(31,31,31));
	glMaterialShinyness();
}

void TunnelPartExec(int time)
{
	int i;
	int32 dir[3];
	
	CurrentTime = time;
	
	
	
	TunnelFxDraw((time>>6), sinLerp(time>>1), cosLerp(time>>1));
	
	
	
	glEnable(GL_BLEND);
		
	DiscoElephant->transform->translation[2] = floattof32(-1) + ABS(sinLerp(time)/2);
	DiscoElephant->transform->rotation[0] = (DEGREES_IN_CIRCLE/16);
	DiscoElephant->transform->rotation[1] = sinLerp(time*2) / 2;

	for(i=0; i<4; i++) {
		dir[0] = (DiscoElephant->transform->translation[0] - flares[i]->position[0]);
		dir[1] = (DiscoElephant->transform->translation[1] - flares[i]->position[1]);
		dir[2] = (DiscoElephant->transform->translation[2] - flares[i]->position[2]);
		
		normalizef32(dir);
		
		dir[0] *= 0.9;
		dir[1] *= 0.9;
		dir[2] *= 0.9;
		
		Scene->lights[i]->direction[0] = f32tov10(dir[0]);
		Scene->lights[i]->direction[1] = f32tov10(dir[1]);
		Scene->lights[i]->direction[2] = f32tov10(dir[2]);
	}

	RenderScene(Scene);	
		
	for(i=0; i<4; i++) {
		flares[i]->position[0] = DiscoElephant->transform->translation[0] + (int32)sinLerp((time<<1) + (i+1) * DEGREES_IN_CIRCLE/4)/2;
		flares[i]->position[1] = DiscoElephant->transform->translation[1] + (int32)cosLerp((time<<2) + (i+1) * DEGREES_IN_CIRCLE/4)/2;
		flares[i]->position[2] = DiscoElephant->transform->translation[2] + (int32)sinLerp((time<<3) + DEGREES_IN_CIRCLE/(i+1))/2;
		DrawBillboard(flares[i], false, 0, 0, 0, 0);
	}
	
	if(time < 1000) {
		FadeIn(time);
	} else if(time >= 28000) {
		FadeOut(time);
	} else {
		DrawFlash(time);
	}
	
	glFlush(0);
	swiWaitForVBlank();
}

void TunnelPartDeinit(void)
{
	int i;
	
	TunnelFxDeinit();
	FreeScene(Scene);
	for(i=0; i<4; i++) {
    	free(flares[i]);
    }	
    free(flare_tex);
    free(tex);
}

void CreateTunnelPart(T_Part* part)
{
	DemoCreatePart(part, TunnelPartInit, TunnelPartExec, TunnelPartDeinit, TunnelPartSync, 30*1000); //30*1000);
}

