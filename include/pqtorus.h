#ifndef PQTORUS_H_
#define PQTORUS_H_

#include <util3d.h>
#include <nds.h>


typedef struct t_pqtorus {
	int p;
	int q;
	Vertex3D *curve_points;
	int32 radius;
	int32 section_radius;
	int nb_slices;
	int nb_sections;
	int nb_vertex;
	
	int32 wave_amplitude;
	int	nb_waves;
	int32 *wave_coefs;
	int32 wave_speed;
		
	Vertex3D *oriented_circles;
	Vertex3D *vertices;
	PackedNormal3D *normals;
	int32 **indices;
	Tex2D *tex_coords;
	
	Vertex3D *face_normals;
	
	bool inverted_normals;
	
} TPQTorus;

TPQTorus* CreatePQTorus(int p, int q, int nbsections, int nbslices, int32 sectionradius, int32 radius);
void DestroyPQTorus(TPQTorus *torus);
void PQTorusSetWaves(TPQTorus *torus, int32 amplitude, int nbwaves, int32 wavespeed);
void PQTorusUpdate(TPQTorus *torus, int time);
void PQTorusRender(TPQTorus *torus);
void PQTorusGetCurvePoint(TPQTorus *torus, int32 theta, Vertex3D out);


#endif /*PQTORUS_H_*/
