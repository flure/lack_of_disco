#include <usings.h>

#include <discoball_fx.h>
#include <data.h>
#include <myutil.h>

#include <malloc.h>
#include <string.h>

#include <nds.h>

#include <util3d.h>


/* Dimensions de la boule disco */
#define NB_SEGMENTS 16
#define NB_RINGS	16

/* Définition du mesh de la boule disco */
static Vertex3D* Vertices;
static PackedNormal3D* QuadNormals;
static PackedNormal3D* TriNormals;
static int32 NbVertices;
static tQuad* Quads;
static tTri* Tris;
static int32 NbQuads;
static int32 NbTris;

/* Définition d'un rayon */
typedef struct ray_of_light {
	bool always_and_change; /* quand le rayon est fini, il reste actif mais change de quad et de couleur */
	bool active;
	int iquad;
	rgb color;
	int32 start_time;
	int32 duration;
} tRayOfLight;

/* Liste des rayons en cours */
#define NB_RAYS 25
static tRayOfLight *Rays;

/* Stockage des paramètres de l'explosion */
typedef int32 tRot[3];
static int32 ExplStartTime;
static int32 *QuadsExplSpeeds;
static tRot *QuadsExplRots;
static int32 *TrisExplSpeeds;
static tRot *TrisExplRots;

void DiscoballFxExplodeStart(int32 time)
{
	ExplStartTime = time;
}

void DiscoballFxExplode(time)
{
	int i, vtx, alpha;
	int32 x, y, z, rx, ry, rz, t;
	int32 theta;
	
	t = time - ExplStartTime;
	
	alpha = ABS(31-((t*31)>>12)) % 31;
	if(alpha == 0) return;
	
	glEnable(GL_BLEND);
	
	glPolyFmt(POLY_ID(1) | POLY_ALPHA(alpha) | POLY_CULL_NONE);
	glColor(RGB15(31, 31, 31));
	for(i=0; i<NbQuads; i++) {
		x = (Vertices[Quads[i][0]][0] + Vertices[Quads[i][1]][0] + Vertices[Quads[i][2]][0] + Vertices[Quads[i][3]][0]) >> 2;
		y = (Vertices[Quads[i][0]][1] + Vertices[Quads[i][1]][1] + Vertices[Quads[i][2]][1] + Vertices[Quads[i][3]][1]) >> 2;
		z = (Vertices[Quads[i][0]][2] + Vertices[Quads[i][1]][2] + Vertices[Quads[i][2]][2] + Vertices[Quads[i][3]][2]) >> 2;

		x = (((QuadsExplSpeeds[i] * t) >> 12) * x) >> 12;
		y = (((QuadsExplSpeeds[i] * t) >> 12) * y) >> 12;
		z = (((QuadsExplSpeeds[i] * t) >> 12) * z) >> 12;
		
		rx = QuadsExplRots[i][0];
		ry = QuadsExplRots[i][1];
		rz = QuadsExplRots[i][2];
		
		glPushMatrix();
		glTranslate3f32(x, y, z);
		glRotatef32i(time, rx, ry, rz);
		
		glBegin(GL_QUAD);
			vtx = Quads[i][0];
			//glNormal(QuadNormals[i]);
			glVertex3v16(Vertices[vtx][0], Vertices[vtx][1], Vertices[vtx][2]);
			vtx = Quads[i][1];
			//glNormal(QuadNormals[i]);
			glVertex3v16(Vertices[vtx][0], Vertices[vtx][1], Vertices[vtx][2]);
			vtx = Quads[i][2];
			//glNormal(QuadNormals[i]);
			glVertex3v16(Vertices[vtx][0], Vertices[vtx][1], Vertices[vtx][2]);
			vtx = Quads[i][3];
			//glNormal(QuadNormals[i]);
			glVertex3v16(Vertices[vtx][0], Vertices[vtx][1], Vertices[vtx][2]);
		glEnd();
		
		glPopMatrix(1);
	}
	
	for(i=0; i<NbTris; i++) {
		x = (Vertices[Tris[i][0]][0] + Vertices[Tris[i][1]][0] + Vertices[Tris[i][2]][0]) / 3;
		y = (Vertices[Tris[i][0]][1] + Vertices[Tris[i][1]][1] + Vertices[Tris[i][2]][2]) / 3;
		z = (Vertices[Tris[i][0]][2] + Vertices[Tris[i][1]][2] + Vertices[Tris[i][2]][1]) / 3;

		x = (((TrisExplSpeeds[i] * t) >> 12) * x) >> 12;
		y = (((TrisExplSpeeds[i] * t) >> 12) * y) >> 12;
		z = (((TrisExplSpeeds[i] * t) >> 12) * z) >> 12;
		
		rx = TrisExplRots[i][0];
		ry = TrisExplRots[i][1];
		rz = TrisExplRots[i][2];
		
		glPushMatrix();
		glTranslate3f32(x, y, z);
		glRotatef32i(time, rx, ry, rz);
		
		glBegin(GL_TRIANGLE);
			vtx = Tris[i][0];
			//glNormal(TriNormals[i]);
			glVertex3v16(Vertices[vtx][0], Vertices[vtx][1], Vertices[vtx][2]);
			vtx = Tris[i][1];
			//glNormal(TriNormals[i]);
			glVertex3v16(Vertices[vtx][0], Vertices[vtx][1], Vertices[vtx][2]);
			vtx = Tris[i][2];
			//glNormal(TriNormals[i]);
			glVertex3v16(Vertices[vtx][0], Vertices[vtx][1], Vertices[vtx][2]);
		glEnd();
		
		glPopMatrix(1);
	}
	
	if(t > 1000) t = 1000;
	
	theta = t * (DEGREES_IN_CIRCLE/4) / 1000;
	alpha = (16 * sinLerp(theta))>>12;
	
	REG_MASTER_BRIGHT = (1<<14) | alpha;
	REG_MASTER_BRIGHT_SUB = REG_MASTER_BRIGHT;
}


/* Ajoute un rayon de lumière */
void DiscoballFxAddRay(int iquad, rgb light_color, int32 start_time, int32 duration, bool always)
{
	int i, q1, q2, q3, q4, q5, s, r;
	
	q1 = -1;
	q2 = -1;
	q3 = -1;
	q4 = -1;
	q5 = -1;
	/* si iquad vaut -1, gestion de 5 rayons placés aléatoirement */
	if(iquad == -1) {
		s = rand()%NB_SEGMENTS;
		r = rand()%NB_RINGS;
		q1 = s * r;

		s = rand()%NB_SEGMENTS;
		r = rand()%NB_RINGS;
		q2 = s * r;
		
		s = rand()%NB_SEGMENTS;
		r = rand()%NB_RINGS;
		q3 = s * r;
		
		s = rand()%NB_SEGMENTS;
		r = rand()%NB_RINGS;
		q4 = s * r;
		
		s = rand()%NB_SEGMENTS;
		r = rand()%NB_RINGS;
		q5 = s * r;
	}
	
	for(i=0; i<NB_RAYS; i++) {
		if(!Rays[i].active) {
			Rays[i].always_and_change = always;
			Rays[i].active = TRUE;
			Rays[i].color = light_color;
			Rays[i].start_time = start_time;
			Rays[i].duration = duration;
			if(iquad >= 0) {
				Rays[i].iquad = iquad;
				break;
			} else {
				/* gestion de deux rayons diamétralement opposés */
				if(q1 >= 0) {
					Rays[i].iquad = q1;
					q1 = -1;
				} else if(q2 >= 0) {
					Rays[i].iquad = q2;
					q2 = -1;
				} else if(q3 >= 0) {
					Rays[i].iquad = q3;
					q3 = -1;
				} else if(q4 >= 0) {
					Rays[i].iquad = q4;
					q4 = -1;
				} else if(q5 >= 0) {
					Rays[i].iquad = q5;
					q5 = -1;
					break;
				}
			}
		}
	}
}

/* Dessine un rayon de lumière */
void DrawRayOfLight(int iquad, rgb lightcolor, int alphamax)
{
	int i, p0, p1, p2, p3;
	float il, il1, l;
	
	
	p0 = Quads[iquad][0];
	p1 = Quads[iquad][1];
	p2 = Quads[iquad][2];
	p3 = Quads[iquad][3];
	
	glPolyFmt(POLY_ID(3+alphamax) | POLY_ALPHA(alphamax) | POLY_CULL_NONE);
	glBegin(GL_QUADS);
		glColor(lightcolor);
		glVertex3v16(Vertices[p0][0]*1.05, Vertices[p0][1]*1.05, Vertices[p0][2]*1.05);
		glVertex3v16(Vertices[p1][0]*1.05, Vertices[p1][1]*1.05, Vertices[p1][2]*1.05);
		glVertex3v16(Vertices[p2][0]*1.05, Vertices[p2][1]*1.05, Vertices[p2][2]*1.05);
		glVertex3v16(Vertices[p3][0]*1.05, Vertices[p3][1]*1.05, Vertices[p3][2]*1.05);
	glEnd();

	l = 0.05; //(0.05 * 32) / alphamax;

	for(i=0; i<alphamax; i++) {
		il = i*l;
		il1 = (i+1)*l;
		glPolyFmt(POLY_ID(2+i) | POLY_ALPHA(alphamax-i) | POLY_CULL_NONE);
		glBegin(GL_QUADS);
			glColor(lightcolor);
			glVertex3v16(Vertices[p0][0] + Vertices[p0][0]*il,  Vertices[p0][1] + Vertices[p0][1]*il,  Vertices[p0][2] + Vertices[p0][2]*il);
			glVertex3v16(Vertices[p1][0] + Vertices[p1][0]*il,  Vertices[p1][1] + Vertices[p1][1]*il,  Vertices[p1][2] + Vertices[p1][2]*il);
			glVertex3v16(Vertices[p1][0] + Vertices[p1][0]*il1, Vertices[p1][1] + Vertices[p1][1]*il1, Vertices[p1][2] + Vertices[p1][2]*il1);
			glVertex3v16(Vertices[p0][0] + Vertices[p0][0]*il1, Vertices[p0][1] + Vertices[p0][1]*il1, Vertices[p0][2] + Vertices[p0][2]*il1);
	
			glVertex3v16(Vertices[p1][0] + Vertices[p1][0]*il,  Vertices[p1][1] + Vertices[p1][1]*il,  Vertices[p1][2] + Vertices[p1][2]*il);
			glVertex3v16(Vertices[p2][0] + Vertices[p2][0]*il,  Vertices[p2][1] + Vertices[p2][1]*il,  Vertices[p2][2] + Vertices[p2][2]*il);
			glVertex3v16(Vertices[p2][0] + Vertices[p2][0]*il1, Vertices[p2][1] + Vertices[p2][1]*il1, Vertices[p2][2] + Vertices[p2][2]*il1);
			glVertex3v16(Vertices[p1][0] + Vertices[p1][0]*il1, Vertices[p1][1] + Vertices[p1][1]*il1, Vertices[p1][2] + Vertices[p1][2]*il1);
	
			glVertex3v16(Vertices[p2][0] + Vertices[p2][0]*il,  Vertices[p2][1] + Vertices[p2][1]*il,  Vertices[p2][2] + Vertices[p2][2]*il);
			glVertex3v16(Vertices[p3][0] + Vertices[p3][0]*il,  Vertices[p3][1] + Vertices[p3][1]*il,  Vertices[p3][2] + Vertices[p3][2]*il);
			glVertex3v16(Vertices[p3][0] + Vertices[p3][0]*il1, Vertices[p3][1] + Vertices[p3][1]*il1, Vertices[p3][2] + Vertices[p3][2]*il1);
			glVertex3v16(Vertices[p2][0] + Vertices[p2][0]*il1, Vertices[p2][1] + Vertices[p2][1]*il1, Vertices[p2][2] + Vertices[p2][2]*il1);
	
			glVertex3v16(Vertices[p3][0] + Vertices[p3][0]*il,  Vertices[p3][1] + Vertices[p3][1]*il,  Vertices[p3][2] + Vertices[p3][2]*il);
			glVertex3v16(Vertices[p0][0] + Vertices[p0][0]*il,  Vertices[p0][1] + Vertices[p0][1]*il,  Vertices[p0][2] + Vertices[p0][2]*il);
			glVertex3v16(Vertices[p0][0] + Vertices[p0][0]*il1, Vertices[p0][1] + Vertices[p0][1]*il1, Vertices[p0][2] + Vertices[p0][2]*il1);
			glVertex3v16(Vertices[p3][0] + Vertices[p3][0]*il1, Vertices[p3][1] + Vertices[p3][1]*il1, Vertices[p3][2] + Vertices[p3][2]*il1);
		glEnd();
	} 
}

/* Dessine tous les rayons en cours */
void DiscoballFxDrawRays(time)
{
	int32 t, theta, alphamax;
	int i;
	
	for(i=0; i<NB_RAYS; i++) {
		if(Rays[i].active) {
			if(time < Rays[i].start_time) continue;
			if(time > (Rays[i].start_time + Rays[i].duration)) {
				if(Rays[i].always_and_change) {
					Rays[i].active = TRUE;
					Rays[i].start_time = time;
					Rays[i].color = RGB15(16+rand()%16, 16+rand()%16, 16+rand()%16);
					Rays[i].iquad = rand()%(NB_SEGMENTS * NB_RINGS);
				} else {
					Rays[i].active = FALSE;
					continue;
				}
			}
			
			t = Rays[i].start_time - time;
			
			if((t >= Rays[i].duration)) {
				if(Rays[i].always_and_change) {
					t = 0;
					Rays[i].start_time = time;
					Rays[i].active = TRUE;
					Rays[i].color = RGB15(16+rand()%16, 16+rand()%16, 16+rand()%16);
					Rays[i].iquad = rand()%(NB_SEGMENTS * NB_RINGS);
				} else {				
					Rays[i].active = FALSE;
					continue;
				}
			} else {
				theta = t * (DEGREES_IN_CIRCLE/2) / Rays[i].duration;			
				alphamax = (ABS(sinLerp(theta) * 31)) >> 12;
				DrawRayOfLight(Rays[i].iquad, Rays[i].color, alphamax);
			}
		} 
	}
}

/* Initialise les paramètres d'explosion (appeler après DiscoballFxCreateBall) */
void DiscoballFxExplodeInit(void)
{
	int i;
	
	ExplStartTime = -10000;
	QuadsExplSpeeds = (int32*)malloc(NbQuads * sizeof(int32));
	TrisExplSpeeds = (int32*)malloc(NbTris * sizeof(int32));
	QuadsExplRots = (tRot*)malloc(NbQuads * sizeof(tRot));
	TrisExplRots = (tRot*)malloc(NbTris * sizeof(tRot));
	
	for(i=0; i<NbQuads; i++) {
		QuadsExplSpeeds[i] = floattof32(15.0);
		QuadsExplRots[i][0] = rand()%(floattof32(6));
		QuadsExplRots[i][1] = rand()%(floattof32(6));
		QuadsExplRots[i][2] = rand()%(floattof32(6));
	}
	
	for(i=0; i<NbTris; i++) {
		TrisExplSpeeds[i] = floattof32(15.0);
		TrisExplRots[i][0] = rand()%(floattof32(6));
		TrisExplRots[i][1] = rand()%(floattof32(6));
		TrisExplRots[i][2] = rand()%(floattof32(6));
	}
}

void DiscoballFxExplodeDeinit(void)
{
	free(QuadsExplSpeeds);
	free(TrisExplSpeeds);
	free(QuadsExplRots);
	free(TrisExplRots);
}

/* Crée le mesh de la boule disco */
void DiscoballFxCreateBall(void)
{
	int rings, segs, ivtx, iquad, itri, vtx0, vtx1, vtx2, vtx3;
	s16 theta, phi, dtheta, dphi, theta2, phi2;
	int32 v1[3], v2[3], n[3];
	PackedNormal3D pn;
	
	NbTris = NB_SEGMENTS * 2;
	NbQuads = NB_SEGMENTS * (NB_RINGS-2);
	NbVertices = 4 * NbQuads + 3 * NbTris;
	
	Vertices = (Vertex3D*)malloc(NbVertices * sizeof(Vertex3D));
	QuadNormals = (PackedNormal3D*)malloc(NbQuads * sizeof(PackedNormal3D));
	TriNormals = (PackedNormal3D*)malloc(NbTris * sizeof(PackedNormal3D));
	Quads = (tQuad*)malloc(NbQuads * sizeof(tQuad));
	Tris = (tTri*)malloc(NbTris * sizeof(tTri));
	
	dtheta = degreesToAngle(180) / NB_RINGS;
	dphi = degreesToAngle(360) / NB_SEGMENTS;	
	
	/* Building the quads */
	ivtx = 0;
	iquad = 0;
	
	theta = degreesToAngle(-90) + dtheta;
	theta2 = theta + dtheta;
	for(rings=1; rings<NB_RINGS-1; rings++) {	
		phi = 0;
		phi2 = phi + dphi;
		for(segs=0; segs<NB_SEGMENTS; segs++) {
			/* ref. http://local.wasp.uwa.edu.au/~pbourke/miscellaneous/sphere_cylinder/ */
			
			Vertices[ivtx][0] = (cosLerp(theta) * cosLerp(phi)) >> 12;
			Vertices[ivtx][1] = sinLerp(theta);
			Vertices[ivtx][2] = (cosLerp(theta) * sinLerp(phi)) >> 12;
			Quads[iquad][0] = ivtx;
			vtx0 = ivtx;
			ivtx++;
			
			Vertices[ivtx][0] = (cosLerp(theta2) * cosLerp(phi)) >> 12;
			Vertices[ivtx][1] = sinLerp(theta2);
			Vertices[ivtx][2] = (cosLerp(theta2) * sinLerp(phi)) >> 12;
			Quads[iquad][1] = ivtx;
			vtx1 = ivtx;
			ivtx++;
			
			Vertices[ivtx][0] = (cosLerp(theta2) * cosLerp(phi2)) >> 12;
			Vertices[ivtx][1] = sinLerp(theta2);
			Vertices[ivtx][2] = (cosLerp(theta2) * sinLerp(phi2)) >> 12;
			Quads[iquad][2] = ivtx;
			vtx2 = ivtx;
			ivtx++;
			
			Vertices[ivtx][0] = (cosLerp(theta) * cosLerp(phi2)) >> 12;
			Vertices[ivtx][1] = sinLerp(theta);
			Vertices[ivtx][2] = (cosLerp(theta) * sinLerp(phi2)) >> 12;
			Quads[iquad][3] = ivtx;
			vtx3 = ivtx;
			ivtx++;
			
			v1[0] = (Vertices[vtx2][0] - Vertices[vtx0][0]) * 10;
			v1[1] = (Vertices[vtx2][1] - Vertices[vtx0][1]) * 10;
			v1[2] = (Vertices[vtx2][2] - Vertices[vtx0][2]) * 10; 
			
			v2[0] = (Vertices[vtx3][0] - Vertices[vtx1][0]) * 10;
			v2[1] = (Vertices[vtx3][1] - Vertices[vtx1][1]) * 10;
			v2[2] = (Vertices[vtx3][2] - Vertices[vtx1][2]) * 10;
			
			crossf32(v1, v2, n);
			normalizef32(n);
			pn = NORMAL_PACK( f32tov10(n[0]), f32tov10(n[1]), f32tov10(n[2]) );
			QuadNormals[iquad] = pn;			
			
			iquad++;
			
			phi += dphi;
			phi2 += dphi;
		}
		
		theta += dtheta;
		theta2 += dtheta;
	}
	
	/* Building the triangle caps */
	phi = 0;
	phi2 = phi + dphi;
	itri = 0;
	for(segs=0; segs<NB_SEGMENTS; segs++) {
		/* top */
		theta = degreesToAngle(90) - dtheta;		
		Vertices[ivtx][0] = 0;
		Vertices[ivtx][1] = floattov16(1);
		Vertices[ivtx][2] = 0;
		Tris[itri][0] = ivtx;
		vtx0 = ivtx;
		ivtx++;
		
		Vertices[ivtx][0] = (cosLerp(theta) * cosLerp(phi2)) >> 12;
		Vertices[ivtx][1] = sinLerp(theta);
		Vertices[ivtx][2] = (cosLerp(theta) * sinLerp(phi2)) >> 12;
		Tris[itri][1] = ivtx;
		vtx1 = ivtx;
		ivtx++;
		
		Vertices[ivtx][0] = (cosLerp(theta) * cosLerp(phi)) >> 12;
		Vertices[ivtx][1] = sinLerp(theta);
		Vertices[ivtx][2] = (cosLerp(theta) * sinLerp(phi)) >> 12;
		Tris[itri][2] = ivtx;
		vtx2 = ivtx;
		ivtx++;
				
		v1[0] = (Vertices[vtx1][0] - Vertices[vtx0][0]) * 10;
		v1[1] = (Vertices[vtx1][1] - Vertices[vtx0][1]) * 10;
		v1[2] = (Vertices[vtx1][2] - Vertices[vtx0][2]) * 10; 
		
		v2[0] = (Vertices[vtx2][0] - Vertices[vtx0][0]) * 10;
		v2[1] = (Vertices[vtx2][1] - Vertices[vtx0][1]) * 10;
		v2[2] = (Vertices[vtx2][2] - Vertices[vtx0][2]) * 10;
		
		crossf32(v1, v2, n);
		normalizef32(n);
		pn = NORMAL_PACK( f32tov10(n[0]), f32tov10(n[1]), f32tov10(n[2]) );
		TriNormals[itri] = pn;		
		
		itri++;
		
		/* bottom */
		theta = degreesToAngle(-90) + dtheta;
		Vertices[ivtx][0] = 0;
		Vertices[ivtx][1] = floattov16(-1);
		Vertices[ivtx][2] = 0;
		Tris[itri][0] = ivtx;
		vtx0 = ivtx;
		ivtx++;
		
		Vertices[ivtx][0] = (cosLerp(theta) * cosLerp(phi)) >> 12;
		Vertices[ivtx][1] = sinLerp(theta);
		Vertices[ivtx][2] = (cosLerp(theta) * sinLerp(phi)) >> 12;
		Tris[itri][1] = ivtx;
		vtx1 = ivtx;
		ivtx++;
		
		Vertices[ivtx][0] = (cosLerp(theta) * cosLerp(phi2)) >> 12;
		Vertices[ivtx][1] = sinLerp(theta);
		Vertices[ivtx][2] = (cosLerp(theta) * sinLerp(phi2)) >> 12;
		vtx2 = ivtx;
		Tris[itri][2] = ivtx;
		ivtx++;
		
		v1[0] = (Vertices[vtx1][0] - Vertices[vtx0][0]) * 10;
		v1[1] = (Vertices[vtx1][1] - Vertices[vtx0][1]) * 10;
		v1[2] = (Vertices[vtx1][2] - Vertices[vtx0][2]) * 10; 
		
		v2[0] = (Vertices[vtx2][0] - Vertices[vtx0][0]) * 10;
		v2[1] = (Vertices[vtx2][1] - Vertices[vtx0][1]) * 10;
		v2[2] = (Vertices[vtx2][2] - Vertices[vtx0][2]) * 10;
		
		crossf32(v1, v2, n);
		normalizef32(n);
		pn = NORMAL_PACK( f32tov10(n[0]), f32tov10(n[1]), f32tov10(n[2]) );
		TriNormals[itri] = pn;
		itri++;
		
		phi += dphi;
		phi2 += dphi;
	}
}

/* Initialise la boule disco et tout ce qui va autour */
void DiscoballFxInit(void)
{
	/* Crée le mesh de la boule disco */
	DiscoballFxCreateBall();
	
	/* Prépare l'explosion */
	DiscoballFxExplodeInit();
	
	 
	/* Le material de la boule à facettes */
	glMaterialf(GL_AMBIENT, RGB15(0, 0, 0));
	glMaterialf(GL_DIFFUSE, RGB15(24, 24, 31));
	glMaterialf(GL_SPECULAR, BIT(15) | RGB15(31,31,31));
	glMaterialShinyness();
	
	
	
	/*for(i=0; i<NB_RAYS; i++) {
		AddRay(i*5, RGB15(31, i<<7, i<<3), 1000*(i+1), 1000);
	}*/
	
	/*AddRay(16*7, RGB15(31, 31, 20), 1000, 1000);
	AddRay(16*7, RGB15(31, 31, 20), 3000, 1000);
	AddRay(16*7, RGB15(31, 31, 20), 5000, 1000);
	AddRay(16*7, RGB15(31, 31, 20), 7000, 1000);
	AddRay(16*7, RGB15(31, 31, 20), 9000, 1000);*/
	
	//glMaterialShinyness();
	
	//FlashStartTime = -1000; // pour éviter un flash à la première seconde
	StartFlash(-1000); // pour éviter un flash à la première seconde
	Rays = (tRayOfLight*)malloc(sizeof(tRayOfLight) * NB_RAYS);
}

/* Dessine la boule disco et les flashs et rayons de lumière */
void DiscoballFxDraw(int32 time)
{
	int i, vtx;
	
	if((ExplStartTime <= 0) || (time < ExplStartTime)) {
		
		glEnable(GL_BLEND);
		
		glPolyFmt(POLY_ID(1) | POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0 | POLY_MODULATION );
		glBegin(GL_QUADS);	
		for(i=0; i<NbQuads; i++) {
			vtx = Quads[i][0];
			glNormal(QuadNormals[i]);
			glVertex3v16(Vertices[vtx][0], Vertices[vtx][1], Vertices[vtx][2]);
			
			vtx = Quads[i][1];
			glVertex3v16(Vertices[vtx][0], Vertices[vtx][1], Vertices[vtx][2]);
			
			vtx = Quads[i][2];
			glVertex3v16(Vertices[vtx][0], Vertices[vtx][1], Vertices[vtx][2]);
			
			vtx = Quads[i][3];
			glVertex3v16(Vertices[vtx][0], Vertices[vtx][1], Vertices[vtx][2]);
		}
		glEnd();
		
		glBegin(GL_TRIANGLES);
		for(i=0; i<NbTris; i++) {
			vtx = Tris[i][0];
			glNormal(TriNormals[i]);
			glVertex3v16(Vertices[vtx][0], Vertices[vtx][1], Vertices[vtx][2]);
			
			vtx = Tris[i][1];
			glVertex3v16(Vertices[vtx][0], Vertices[vtx][1], Vertices[vtx][2]);
			
			vtx = Tris[i][2];
			glVertex3v16(Vertices[vtx][0], Vertices[vtx][1], Vertices[vtx][2]);
		}
		glEnd();
		
		DiscoballFxDrawRays(time);	
		DrawFlash(time);
	} else {
		DiscoballFxExplode(time);
	}
}


/* Libère la mémoire allouée */
void DiscoballFxDeInit(void)
{
	free(Vertices);
	free(QuadNormals);
	free(TriNormals);
	free(Quads);
	free(Tris);
	free(Rays);
	
	DiscoballFxExplodeDeinit();
}

/* Supprime les constantes préprocesseur */
#undef NB_SEGMENTS
#undef NB_RINGS
#undef NB_RAYS
#undef FLASH_DURATION