#include <tunnel_fx.h>
#include <data.h>
#include <myutil.h>
#include <lines.h>

#include <string.h>
#include <malloc.h>

#define VOXEL_XEYE			128
#define VOXEL_YEYE			-10
#define VOXEL_EYE_DIST		128
#define VOXEL_SCALE			5
#define VOXEL_FAR 			48
#define VOXEL_LOD_LIMIT1 	32
#define VOXEL_LOD_LIMIT2 	40
#define VOXEL_LOD_LIMIT3 	44
#define VOXEL_HORIZON		5

#define VOXEL_WIDTH			256

/* Les pixels dans lesquels on va dessiner */
static u16 *ScreenPixels;

static u16 *VoxelPixels;
static u16 *OffscreenPixels;
static int *VoxelHeights;
static int *LastHeights;
static u16 *LastColors;
static u8 *VoxelUs;
static int32 *VoxelPolar;

/* Initialise le tunnel et tout ce qui va autour */
void TunnelFxInit(u16* pixels)
{
	int c, u, v, x, y;
	float xray;
	int32 R, theta, maxdist, Rn;
	
	/* L'écran dans lequel on va dessiner le résultat
	 * TODO : page flipping
	 */
	ScreenPixels = pixels;
	
	/* Double buffer
	 * TODO : page flipping
	 */
	OffscreenPixels = malloc(256*192 * sizeof(*OffscreenPixels));
	
	/* On va dessiner le voxel dedans */
	VoxelPixels = malloc(VOXEL_WIDTH*96 * sizeof(*VoxelPixels));
	
	/* Deux tables de precalc pour le voxel */
	VoxelHeights = malloc(256*VOXEL_FAR * sizeof(*VoxelHeights));
	VoxelUs = malloc(VOXEL_WIDTH*VOXEL_FAR * sizeof(*VoxelUs));
	
	/* Deux tables pour le LOD */
	LastHeights = malloc(VOXEL_WIDTH * sizeof(*LastHeights));
	LastColors = malloc(VOXEL_WIDTH * sizeof(*LastColors));
	
	/* Precalc pour tordre le voxel et en faire un tunnel */
	VoxelPolar = malloc(256 * 192 * sizeof(*VoxelPolar));
	
	/* Précalcul des hauteurs projetées en fonction de la distance */ 
	for(c=0; c<256; c++) {
		for(v=0; v<VOXEL_FAR; v++) {
			VoxelHeights[c + (v*256)] = ABS( (c - VOXEL_YEYE) * (VOXEL_SCALE) / (v+1) + VOXEL_HORIZON );
		}
	}
	
	/* Précalcul des u projetées en fonction de la distance */
	for(x=0; x<VOXEL_WIDTH; x++) {
		xray = (x - VOXEL_XEYE) / (float)VOXEL_EYE_DIST;
		for(v=0; v<VOXEL_FAR; v++) {
			u = (x + (int)(xray * v)) % VOXEL_WIDTH;
			
			VoxelUs[x + (v<<8)] = u;
		}
	}
	
	/* La table de conversion polaire */
	maxdist = sqrt32( 128*128 + 96*96 );
	for(x=0; x<256; x++) {
		for(y=0; y<192; y++) {
			R = sqrt32( (x-128)*(x-128) + (y-96)*(y-96) );
		
			theta = ATAN2(y, x);
		
			Rn = (R * (1<<12)) / maxdist;			
			
			u = ((Rn * 96) / (1<<12)) % 96;
			v = ((theta * (VOXEL_WIDTH) / ATAN_LUT_DEGREES)) % VOXEL_WIDTH;
		
			VoxelPolar[x + (y<<8)] = u + (v*96);
		}
	}
	
	SetLinesScreen16bpp(VoxelPixels);
}

void TunnelFxDraw(int time, int32 speed_forward, int32 speed_rotate)
{
	int x, y, u, v, v8;
	u8 height;
	//int avance = time>>6, tourne = time>>5;
	int32 avance, tourne;
	int final_h, h_max;
	int32 offset;
	u16 color;
	
	avance = time * 8; //(time * 8 * speed_forward)>>12;
	tourne = 0; //(time * speed_rotate)>>12;
	
	//BG_PALETTE_SUB[0] = 0;

	// On lance en asynchrone l'update de l'écran précédent, pendant qu'on fait les calculs
	dmaCopyWordsAsynch(3, OffscreenPixels, ScreenPixels, 256*192*2);
	
	dmaFillWords(0, VoxelPixels, VOXEL_WIDTH*96*2);

	for(x=0; x<VOXEL_WIDTH; x++) {
		h_max = 95;
		v8 = 0;
		for(v=0; v<VOXEL_FAR; v++) {
//			if((v > VOXEL_LOD_LIMIT) && (v&2)/*&& (x&1)*/) {
//				//height = LastHeights[v];
//				//color = LastColors[v];
//				
//				v8 += VOXEL_WIDTH;
//				continue;
			if((v>VOXEL_LOD_LIMIT3) && (v&3)) {
				v8 += VOXEL_WIDTH;
				continue;
			} else if((v>VOXEL_LOD_LIMIT2) && (v&2)) {
				v8 += VOXEL_WIDTH;
				continue;
			} else if((v>VOXEL_LOD_LIMIT1) && (v&1)) {
				v8 += VOXEL_WIDTH;
				continue;
			} else {
				u = VoxelUs[x+v8];
				
				offset = ((u+tourne)&0xFF) + (((v+avance)&0xFF)<<8);
				//offset = ((u+tourne)%VOXEL_WIDTH) + (((v+avance)%VOXEL_WIDTH)*VOXEL_WIDTH);
				height = tunnel_heightmapBitmap[offset];
				color = tunnel_colorsBitmap[offset];
				
				LastHeights[v] = height;
				LastColors[v] = color;
			//DBG}
			
			final_h = VoxelHeights[height + v8];
			if(final_h < h_max) {
				//DrawLine_16bpp_H(final_h, h_max, x, color);
				//dmaFillHalfWords(color, LinesScreen16 + final_h + (x*96), (h_max-final_h)*2);
				DMA_FILL(3) = (vuint32)color;
				DMA_SRC(3) = (uint32)&DMA_FILL(3);
				DMA_DEST(3) = (uint32)(LinesScreen16 + final_h + (x*96));
				DMA_CR(3) = DMA_SRC_FIX | DMA_COPY_HALFWORDS | (h_max-final_h);
				//while(DMA_CR(3) & DMA_BUSY);
				h_max = final_h;
				
				if(h_max <= 0) break;
			}
			
			v8 += VOXEL_WIDTH;
			} //DBG
		}
	}
	
	// On attend que la copie par DMA soit terminée avant de modifier la source
	while(dmaBusy(3)) { /* rien */ };
	
	// Conversion polaire
	offset = 0;
	for(y=0; y<192; y++) {
		for(x=0; x<256; x++) {
			OffscreenPixels[offset] = VoxelPixels[ VoxelPolar[offset] ];
			
			offset++;
		}
	}

	//BG_PALETTE_SUB[0] = RGB15(31, 0, 0);
}


/* Libère la mémoire allouée */
void TunnelFxDeinit(void)
{
	free(VoxelPixels);
	free(VoxelHeights);
	free(VoxelUs);
	free(LastHeights);
	free(LastColors);
	free(VoxelPolar);
}

#undef VOXEL_XEYE
#undef VOXEL_YEYE
#undef VOXEL_EYE_DIST
#undef VOXEL_SCALE
#undef VOXEL_FAR
#undef VOXEL_LOD_LIMIT
#undef VOXEL_HORIZON
