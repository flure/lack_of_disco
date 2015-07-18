#include <pqtorus.h>

void _BuildSections(TPQTorus *torus);
void _PQTorusBuildVertices(TPQTorus *torus);
void _PQTorusBuildFaceNormals(TPQTorus *torus);
void _PQTorusBuildVertexNormals(TPQTorus *torus);


TPQTorus* CreatePQTorus(int p, int q, int nbsections, int nbslices, int32 sectionradius, int32 radius)
{
	TPQTorus *torus;
	int i, sec, slice, curvertex, nextslice;
	int32 theta;
	
	torus = malloc(sizeof(*torus));
	
	torus->nb_sections = nbsections;
	torus->nb_slices = nbslices;
	torus->section_radius = sectionradius;
	torus->nb_vertex = torus->nb_slices * torus->nb_sections;
	torus->radius = radius;
	torus->p = p;
	torus->q = q;

	// build the curvepoints
	torus->curve_points = malloc(nbsections * sizeof(* (torus->curve_points)));
	for(sec=0; sec<nbsections; ++sec )
	{
		theta = (sec * DEGREES_IN_CIRCLE) / nbsections;
		
		PQTorusGetCurvePoint(torus, theta, torus->curve_points[sec]);
	}
	
	_BuildSections(torus);
	
	torus->vertices = malloc(torus->nb_vertex * sizeof(Vertex3D));
	torus->normals = malloc(torus->nb_vertex * sizeof(PackedNormal3D));
	torus->indices = malloc(torus->nb_slices * sizeof(int32 *));
	torus->tex_coords = malloc(torus->nb_vertex * sizeof(Tex2D));
	
	// create as many index arrays as slices (=quad strips)
	for(i=0; i<torus->nb_slices; ++i )	{
		torus->indices[i] = malloc((torus->nb_sections+1) * 2 * sizeof(int32));
		//IndexArray[i] = new unsigned int [(NbSections+1)*2];
	}

	// build the index arrays
	for (slice=0; slice<torus->nb_slices; ++slice) {
		curvertex = 0;
		nextslice = slice+1;
		if(nextslice == torus->nb_slices) nextslice=0;

		for (sec=0; sec<torus->nb_sections; ++sec) {
			
			torus->indices[slice][curvertex++] = sec * torus->nb_slices + nextslice;
			torus->indices[slice][curvertex++] = sec * torus->nb_slices + slice;
		}

		torus->indices[slice][curvertex++] = nextslice;
		torus->indices[slice][curvertex++] = slice;
	}
	
	for (sec=0; sec<torus->nb_sections; ++sec) {
		for (slice=0; slice<torus->nb_slices; ++slice) {
			torus->tex_coords[(sec * torus->nb_slices)][0] = sinLerp(slice * (DEGREES_IN_CIRCLE/2) / torus->nb_slices);
			torus->tex_coords[(sec * torus->nb_slices)][1] = 16 * sinLerp(sec * (DEGREES_IN_CIRCLE/2) / torus->nb_sections);
		}
	}
	
	// prebuild wave coefficients for wobbling
	torus->wave_coefs = malloc(torus->nb_sections * sizeof(int32));
	//WaveCoefficients = new float[NbSections];
	for (i=0; i<torus->nb_sections; ++i) {
		torus->wave_coefs[i] = torus->section_radius;
	}
	
	// set no wavesec
	PQTorusSetWaves(torus, 0, 0, 0);

	// allocate space for temp normals calculations
	torus->face_normals = malloc(torus->nb_sections * (torus->nb_slices) * sizeof(PackedNormal3D)*2); //DBG : le *2 pour corriger un bug dont je ne connais pas la cause
	//FaceNormals = new TVector3[NbSections*NbSlices];
	
	return torus;
}

void PQTorusGetCurvePoint(TPQTorus *torus, int32 theta, Vertex3D out)
{
	int32 r;
	r = (torus->radius * (((2<<12)+sinLerp(torus->q * theta))/2)) >> 12;
	out[0] = (r * cosLerp(torus->p * theta)) >> 12;
	out[1] = (r * cosLerp(torus->q * theta)) >> 12;
	out[2] = (r * sinLerp(torus->p * theta)) >> 12;
}

void DestroyPQTorus(TPQTorus *torus)
{
	int i;
	
	free(torus->wave_coefs);
	free(torus->oriented_circles);
	free(torus->vertices);
	free(torus->normals);
	free(torus->tex_coords);
	free(torus->face_normals);
	
	for (i=0; i<torus->nb_slices; ++i)	{
		free(torus->indices[i]);
	}
	free(torus->indices);
	
	
	free(torus);
}

void _BuildSections(TPQTorus *torus)
{
	// build the section shape
	Vertex3D *circle;
	int slice, sec, nextsec;
	Vertex3D v1, v2;
	int32 t[3], n[3], b[3];
	
		
	circle = malloc(torus->nb_slices * sizeof(Vertex3D));
	
	for (slice=0; slice<torus->nb_slices; ++slice) {
		circle[slice][0] = (v16)sinLerp(DEGREES_IN_CIRCLE * slice / torus->nb_slices);
		circle[slice][1] = (v16)cosLerp(DEGREES_IN_CIRCLE * slice / torus->nb_slices);
		circle[slice][2] = 0;
	}

	// build the oriented sections
	torus->oriented_circles = malloc(torus->nb_slices * torus->nb_sections * sizeof(Vertex3D));
	for (sec=0; sec<torus->nb_sections; ++sec) {
		nextsec = sec+1;
		if(nextsec==torus->nb_sections) nextsec = 0;

		t[0] = (int32)(torus->curve_points[nextsec][0] - torus->curve_points[sec][0]) * 5;
		t[1] = (int32)(torus->curve_points[nextsec][1] - torus->curve_points[sec][1]) * 5;
		t[2] = (int32)(torus->curve_points[nextsec][2] - torus->curve_points[sec][2]) * 5;
		
		n[0] = (int32)(torus->curve_points[nextsec][0] + torus->curve_points[sec][0]) * 5;
		n[1] = (int32)(torus->curve_points[nextsec][1] + torus->curve_points[sec][1]) * 5;
		n[2] = (int32)(torus->curve_points[nextsec][2] + torus->curve_points[sec][2]) * 5;

		
		crossf32(t, n, b);
		crossf32(b, t, n);
		normalizef32(n);
		normalizef32(b);
		
		for (slice=0; slice<torus->nb_slices; ++slice) {
			v1[0] = ((v16)n[0] * circle[slice][0]) >> 12;
			v1[1] = ((v16)n[1] * circle[slice][0]) >> 12;
			v1[2] = ((v16)n[2] * circle[slice][0]) >> 12;
			
			v2[0] = ((v16)b[0] * circle[slice][1]) >> 12;
			v2[1] = ((v16)b[1] * circle[slice][1]) >> 12;
			v2[2] = ((v16)b[2] * circle[slice][1]) >> 12;

			torus->oriented_circles[sec * torus->nb_slices + slice][0] = v1[0] + v2[0];
			torus->oriented_circles[sec * torus->nb_slices + slice][1] = v1[1] + v2[1];
			torus->oriented_circles[sec * torus->nb_slices + slice][2] = v1[2] + v2[2];
		}
	}

	free(circle);
}

void PQTorusSetWaves(TPQTorus *torus, int32 amplitude, int nbwaves, int32 wavespeed)
{
	torus->wave_amplitude = amplitude;
	torus->nb_waves = nbwaves;
	torus->wave_speed = wavespeed;
}

void _PQTorusBuildVertices(TPQTorus *torus)
{
	unsigned int slice, sec;
	Vertex3D vtx;

	for (slice=0; slice<torus->nb_slices; ++slice) {

		for (sec=0; sec<torus->nb_sections; ++sec) {
			vtx[0] = (torus->oriented_circles[sec * torus->nb_slices + slice][0] * torus->wave_coefs[sec]) >> 12;
			vtx[1] = (torus->oriented_circles[sec * torus->nb_slices + slice][1] * torus->wave_coefs[sec]) >> 12;
			vtx[2] = (torus->oriented_circles[sec * torus->nb_slices + slice][2] * torus->wave_coefs[sec]) >> 12;

			torus->vertices[sec * torus->nb_slices + slice][0] = vtx[0] + torus->curve_points[sec][0];
			torus->vertices[sec * torus->nb_slices + slice][1] = vtx[1] + torus->curve_points[sec][1];
			torus->vertices[sec * torus->nb_slices + slice][2] = vtx[2] + torus->curve_points[sec][2];
		}
	}
}

void PQTorusUpdate(TPQTorus *torus, int time)
{
	int sec;
	int32 ttime = torus->wave_speed*time/1000, wavesec;

	
	for (sec=0; sec<torus->nb_sections; sec++) {
		wavesec = DEGREES_IN_CIRCLE * torus->nb_waves * sec / torus->nb_sections;
		torus->wave_coefs[sec] = torus->section_radius + ((torus->wave_amplitude * sinLerp(ttime + wavesec) / 2) >> 12);
	}

	
	_PQTorusBuildVertices(torus);
	_PQTorusBuildFaceNormals(torus);
	_PQTorusBuildVertexNormals(torus);
}

void PQTorusRender(TPQTorus *torus)
{
	int slice, vtx, i;
	
	for(slice = 0; slice < torus->nb_slices; slice++) {
		glBegin(GL_TRIANGLE_STRIP);
		for(i=0; i<(torus->nb_sections + 1) * 2; i++) {
			vtx = torus->indices[slice][i];
			
			glNormal(torus->normals[vtx]);
			glVertex3v16(torus->vertices[vtx][0], torus->vertices[vtx][1], torus->vertices[vtx][2]);
		}
		glEnd();
	}
}

void _PQTorusBuildFaceNormals(TPQTorus* torus)
{
	int32 u[3], v[3], n[3];
	int slice, nextslice, nextsec, sec, v1, v2;

	for (slice=0; slice<torus->nb_slices; ++slice) {
		nextslice = slice+1; 
		if (nextslice==torus->nb_slices) nextslice=0;
			
		for (sec=0; sec<torus->nb_sections; ++sec) {
			nextsec = sec+1; 
			if (nextsec==torus->nb_sections) nextsec=0;
		
			v1 = (nextsec*torus->nb_slices + nextslice);
			v2 = (sec*torus->nb_slices + slice);
			
			u[0] = (torus->vertices[v1][0] - torus->vertices[v2][0])<<2;
			u[1] = (torus->vertices[v1][1] - torus->vertices[v2][1])<<2;
			u[2] = (torus->vertices[v1][2] - torus->vertices[v2][2])<<2;
			
			v1 = (sec*torus->nb_slices + nextslice);
			v2 = (nextsec*torus->nb_slices + slice);
			v[0] = (torus->vertices[v1][0] - torus->vertices[v2][0])<<2;
			v[1] = (torus->vertices[v1][1] - torus->vertices[v2][1])<<2;
			v[2] = (torus->vertices[v1][2] - torus->vertices[v2][2])<<2;
			
			crossf32(u, v, n);
			normalizef32(n);
			torus->face_normals[sec * torus->nb_slices + slice][0] = (v16)n[0];
			torus->face_normals[sec * torus->nb_slices + slice][1] = (v16)n[1];
			torus->face_normals[sec * torus->nb_slices + slice][2] = (v16)n[2];
		}
	}
}

void _PQTorusBuildVertexNormals(TPQTorus *torus)
{
	int32 n[3];
	int slice, sec, predslice, predsec;

	
	for (slice=0; slice<torus->nb_slices; ++slice) {
		predslice = slice - 1;
		if (predslice < 0) predslice = torus->nb_slices - 1;
			
		for (sec=0; sec<torus->nb_sections; ++sec) {
			predsec = sec - 1;
			if (predsec < 0) predsec = torus->nb_sections - 1;
			
			n[0] = torus->face_normals[sec*torus->nb_slices + slice][0] + torus->face_normals[predsec*torus->nb_slices + slice][0];
			n[1] = torus->face_normals[sec*torus->nb_slices + slice][1] + torus->face_normals[predsec*torus->nb_slices + slice][1];
			n[2] = torus->face_normals[sec*torus->nb_slices + slice][2] + torus->face_normals[predsec*torus->nb_slices + slice][2];
			
			n[0] = n[0] + torus->face_normals[sec*torus->nb_slices + predslice][0];
			n[1] = n[1] + torus->face_normals[sec*torus->nb_slices + predslice][1];
			n[2] = n[2] + torus->face_normals[sec*torus->nb_slices + predslice][2];
			
			n[0] = n[0] + torus->face_normals[predsec*torus->nb_slices + predslice][0];
			n[1] = n[1] + torus->face_normals[predsec*torus->nb_slices + predslice][1];
			n[2] = n[2] + torus->face_normals[predsec*torus->nb_slices + predslice][2];

			n[0] >>= 2;
			n[1] >>= 2;
			n[2] >>= 2;
			
			if(torus->inverted_normals) {
				n[0] = -n[0];
				n[1] = -n[1];
				n[2] = -n[2];
			}
			
			torus->normals[sec * torus->nb_slices + slice ] = NORMAL_PACK(f32tov10(n[0]), f32tov10(n[1]), f32tov10(n[2]));
		}
	}
}