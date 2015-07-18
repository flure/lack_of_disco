#include <util3d.h>

#include <nds.h>

static float current_fov;

#ifdef USING_3DENGINE

/* Variables privées **********************************************************/
static uint32 lights_param;
/******************************************************************************/

/* Rendu d'un objet 3D ********************************************************
 ******************************************************************************/
void _RenderObject(T_Obj3D* obj)
{
	T_Mesh* mesh = obj->mesh;
	T_Transform* transform = obj->transform;
	int iface, ivtx;
	uint32 polyfmt;
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslate3f32(transform->translation[0], transform->translation[1], transform->translation[2]);
	glRotateXi(transform->rotation[0]);
	glRotateYi(transform->rotation[1]);
	glRotateZi(transform->rotation[2]);
	
	polyfmt = POLY_ALPHA(mesh->alpha) | POLY_CULL_BACK;
	//polyfmt = POLY_ALPHA(mesh->alpha) | POLY_CULL_NONE;
	if(mesh->lighting) {
		polyfmt |= lights_param;
	}
	
	if(mesh->texturing && mesh->lighting) {
		polyfmt |= POLY_MODULATION;
		//polyfmt |= POLY_TOON_HIGHLIGHT;
	}

	glPolyFmt(polyfmt);
	
	if(mesh->texturing) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, mesh->texture_id);
	}

	if(mesh->display_list != NULL) {
		glCallList(mesh->display_list);
	} else {
		glBegin(GL_TRIANGLES);
		for(iface=0; iface<mesh->nb_faces; iface++) {
			ivtx = mesh->faces[iface][0];
			if(mesh->texturing) {
				glTexCoord2f32(mesh->texcoords[ivtx][0], mesh->texcoords[ivtx][1]);
			}
			if(mesh->lighting) {
				glNormal(mesh->normals[ivtx]);
			} else {
				glColor3b(mesh->colors[ivtx][0], mesh->colors[ivtx][1], mesh->colors[ivtx][2]);
			}
			glVertex3v16(mesh->vertices[ivtx][0], mesh->vertices[ivtx][1], mesh->vertices[ivtx][2]);
			
			ivtx = mesh->faces[iface][1];
			if(mesh->texturing) {
				glTexCoord2f32(mesh->texcoords[ivtx][0], mesh->texcoords[ivtx][1]);
			}
			if(mesh->lighting) {
				glNormal(mesh->normals[ivtx]);
			} else {
				glColor3b(mesh->colors[ivtx][0], mesh->colors[ivtx][1], mesh->colors[ivtx][2]);
			}
			glVertex3v16(mesh->vertices[ivtx][0], mesh->vertices[ivtx][1], mesh->vertices[ivtx][2]);
		
			ivtx = mesh->faces[iface][2];
			if(mesh->texturing) {
				glTexCoord2f32(mesh->texcoords[ivtx][0], mesh->texcoords[ivtx][1]);
			}
			if(mesh->lighting) {
				glNormal(mesh->normals[ivtx]);
			} else {
				glColor3b(mesh->colors[ivtx][0], mesh->colors[ivtx][1], mesh->colors[ivtx][2]);
			}
			glVertex3v16(mesh->vertices[ivtx][0], mesh->vertices[ivtx][1], mesh->vertices[ivtx][2]);
		}
		glEnd();
	}
	
	glPopMatrix(1);
}



/* Met à jour les paramètres de caméra ****************************************
 ******************************************************************************/
void _UpdateCamera(T_Camera* camera)
{
	gluLookAtf32(camera->position[0], camera->position[1], camera->position[2],
				 camera->lookat[0],   camera->lookat[1],   camera->lookat[2],
				 camera->up[0],       camera->up[1],       camera->up[2]);
}

/* Met à jour les paramètres de lumière ***************************************
 ******************************************************************************/
void _UpdateLight(T_Light* light, int id)
{
	lights_param |= (1<<id); /* POLY_FORMAT_LIGHT<id> */
	glLight(id, light->color, light->direction[0], light->direction[1], light->direction[2]);
}


/* Effectue le rendu de la scène **********************************************
 ******************************************************************************/
void RenderScene(T_Scene* scene)
{
	int ilight, iobj;

	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();	
	
	// Positionne la caméra
	_UpdateCamera(scene->camera);

	// Met à jour l'éclairage
	lights_param = 0;
	for(ilight=0; ilight<4; ilight++) {
		if(scene->lights[ilight] != NULL) {
			if(scene->lights[ilight]->on) {
				_UpdateLight(scene->lights[ilight], ilight);
			}		
		}
	}
	
	if(lights_param != 0) {
		// Initialise les propriétés d'éclairage
//		glMaterialf(GL_AMBIENT, RGB15(8,8,8));
//    	glMaterialf(GL_DIFFUSE, RGB15(16,16,16));
    	glMaterialf(GL_SPECULAR, BIT(15) | RGB15(31,31,31));
//    glMaterialf(GL_EMISSION, RGB15(16,16,16));
		glMaterialShinyness();
	}
	
	// Affiche les objets
	for(iobj=0; iobj<scene->nb_objects; iobj++) {
		_RenderObject(&(scene->objects[iobj]));
	}
}

void Init3DEngine(void)
{
	// initialize the geometry engine
    current_fov = 0;
    
    // enable antialiasing
    glEnable(GL_ANTIALIAS);
    
    // Set our viewport to be the same size as the screen
    glViewport(0,0,255,191);
        
	ChangeFov(70);
	
	glClearColor(0,0,0,31);
    glClearPolyID(63);
    glClearDepth(0x7FFF);
    
    glSetToonTableRange( 0, 8, RGB15(0,0,0) );
    glSetToonTableRange( 8, 16, RGB15(8,8,8) );
    glSetToonTableRange( 16, 31, RGB15(16,16,16) );
    glSetToonTableRange( 31, 31, RGB15(31,31,31) );
    
    glResetTextures();
}


/* Alloue et initialise une camera ********************************************
 * Cette camera est positionnée en (0,0,0) et regarde dans la direction -Z    *
 ******************************************************************************/
T_Camera* CreateCamera(void)
{
	T_Camera* camera = (T_Camera*)malloc(sizeof(T_Camera));
	if(camera==NULL) return NULL;
	
	camera->position[0] = 0;
	camera->position[1] = 0;
	camera->position[2] = 0;
	
	camera->lookat[0] = 0;
	camera->lookat[1] = 0;
	camera->lookat[2] = -1 * (1<<12); /* -1 */
	
	camera->up[0] = 0;
	camera->up[1] = 1 * (1<<12); /* 1 */
	camera->up[2] = 0;
	
	return camera;
}

/* Alloue et initialise une scene 3D ******************************************
 * Crée une scene par défaut avec :
 * 0 objet
 * 0 light (ligthing désactivé)
 * 1 caméra positionnée en (0,0,0) et regardant dans la direction -Z 
 ******************************************************************************/
T_Scene* CreateScene(void)
{
	int ilight;
	T_Scene* scene = (T_Scene*) malloc(sizeof(T_Scene));
	if(scene==NULL) return NULL;
	
	scene->objects = NULL;
	scene->nb_objects = 0;
	scene->textures = NULL;
	scene->nb_textures = 0;	
	scene->lighting = FALSE;
	
	for(ilight=0; ilight<4; ilight++) {
		scene->lights[ilight] = NULL;
	}

	scene->camera = CreateCamera();

	return scene;
}

/* Ajouter une instance d'objet à une scene 3D ********************************
 ******************************************************************************/
void AddObject(T_Scene* scene, T_Obj3D* obj)
{
	T_Obj3D* tmpobj;
	
	if(scene->objects==NULL) {
		scene->objects = (T_Obj3D*)malloc(sizeof(T_Obj3D));
		scene->nb_objects = 1;
		scene->objects[0] = *obj;
	} else {
		tmpobj = scene->objects;
		scene->objects = realloc(tmpobj, (scene->nb_objects + 1) * sizeof(T_Obj3D));
		if(scene->objects==NULL) {
			scene->objects = tmpobj;
		} else {
			scene->nb_objects++;
			scene->objects[scene->nb_objects - 1] = *obj;
		}
	}
}

/* Ajouter une texture à une scene 3D *****************************************
 ******************************************************************************/
void AddTexture(T_Scene* scene, T_Texture* texture)
{
	T_Texture* tmptex;
	if(scene->textures==NULL) {
		scene->textures = (T_Texture*)malloc(sizeof(T_Texture));
		scene->nb_textures = 1;
		scene->textures[0] = *texture;
	} else {
		tmptex = scene->textures;
		scene->textures = realloc(tmptex, (scene->nb_textures + 1) * sizeof(T_Texture));
		if(scene->textures==NULL) {
			scene->textures = tmptex;
		} else {
			scene->nb_textures++;
			scene->textures[scene->nb_textures - 1] = *texture;
		}
	}
}

/* Libère la mémoire occupée par une scene 3D *********************************
 ******************************************************************************/
void FreeScene(T_Scene* scene)
{
	int ilight, iobj, itex;
	
	for(ilight=0; ilight<4; ilight++) {
		FreeLight(scene->lights[ilight]);
	}
	
	for(iobj=0; iobj<scene->nb_objects; iobj++) {
		FreeMesh(scene->objects[iobj].mesh);
		FreeObj3D(&(scene->objects[iobj]));
	}
	
	FreeCamera(scene->camera);
	
	for(itex=0; itex<scene->nb_textures; itex++) {
		FreeTexture(&(scene->textures[itex]));
		free(scene->textures);
	}
}

/* Libère la mémoire occupée par une camera ***********************************
 ******************************************************************************/
void FreeCamera(T_Camera* camera)
{
	free(camera);
}

/* Alloue la mémoire d'une instance d'objet ***********************************
 * Initialisée en utilisant l'objet (mesh) passé en paramètre. Une 
 * transformation par défaut est appliquée : pas de  translation ni rotation.
 ******************************************************************************/
T_Obj3D* CreateObj3D(T_Mesh* mesh)
{
	T_Obj3D* obj = (T_Obj3D*)malloc(sizeof(T_Obj3D));
	if(obj==NULL) return NULL;
	
	obj->mesh = mesh;
	
	obj->transform = (T_Transform*)malloc(sizeof(T_Transform));
	obj->transform->translation[0] = 0;
	obj->transform->translation[1] = 0;
	obj->transform->translation[2] = 0;
	obj->transform->rotation[0] = 0;
	obj->transform->rotation[1] = 0;
	obj->transform->rotation[2] = 0;
	
	return obj;
}

/* Libère la mémoire occupée par un objet 3D **********************************
 ******************************************************************************/
void FreeObj3D(T_Obj3D* object)
{
	free(object->transform);
}

/* Alloue la mémoire d'un objet 3D ********************************************
 * Initialise aux valeurs par défaut :
 * vertices, normals et colors : vides
 * nb_vertices : 0
 * faces : vide
 * nb_faces : 0
 * texture_id : 0
 * alpha : 255 (opaque)
 * display_list : NULL
 ******************************************************************************/
T_Mesh* CreateMesh(void)
{
	T_Mesh* mesh = (T_Mesh*)malloc(sizeof(T_Mesh));
	if(mesh==NULL) return NULL;
	
	mesh->vertices = NULL;
	mesh->normals = NULL;
	mesh->colors = NULL;
	mesh->texcoords = NULL;
	mesh->nb_vertices = 0;
	
	mesh->texture_id = 0;
	mesh->alpha = 31;
	mesh->texturing = FALSE;
	mesh->lighting = FALSE;
	
	mesh->display_list = NULL;
	
	return mesh;
}

/* Alloue la mémoire pour nb_vertices sommets *********************************
 * Alloue en même temps la mémoire pour les normales, couleurs et coordonnées
 * de texture.
 * ATTENTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * Les données de vertex, normales, couleurs et coordonnées de texture déjà !!!
 * existantes sont supprimées et leur mémoire libérée !!!!!!!!!!!!!!!!!!!!!!!!!
 ******************************************************************************/
void AllocVertices(T_Mesh* mesh, int nb_vertices)
{
	free(mesh->vertices);
	free(mesh->normals);
	free(mesh->colors);		
	free(mesh->texcoords);
	
	mesh->nb_vertices = nb_vertices;
	mesh->vertices = (Vertex3D*)malloc(nb_vertices * sizeof(Vertex3D));
	mesh->normals = (PackedNormal3D*)malloc(nb_vertices * sizeof(PackedNormal3D));
	mesh->colors = (Color*)malloc(nb_vertices * sizeof(Color));
	mesh->texcoords = (Tex2D*)malloc(nb_vertices * sizeof(Tex2D));
}

/* Alloue la mémoire pour nb_faces faces **************************************
 ******************************************************************************/
void AllocFaces(T_Mesh* mesh, int nb_faces)
{
	free(mesh->faces);
	
	mesh->nb_faces = nb_faces;
	mesh->faces = (Face*)malloc(nb_faces * sizeof(Face));
}

/* Libère la mémoire occupée par un objet 3D **********************************
 ******************************************************************************/
void FreeMesh(T_Mesh* mesh)
{
	free(mesh->vertices);
	free(mesh->normals);
	free(mesh->colors);
	free(mesh->faces);
	free(mesh->texcoords);
	free(mesh);
}






/* Crée une light avec les paramètres donnés **********************************
 ******************************************************************************/
T_Light* CreateLight(v10 dir_x, v10 dir_y, v10 dir_z, rgb color, bool onoff)
{
	T_Light* light = (T_Light*)malloc(sizeof(T_Light));
	if(light==NULL) return NULL;
	
	light->direction[0] = dir_x;
	light->direction[1] = dir_y;
	light->direction[2] = dir_z;
	
	light->color = color;
	
	light->on = onoff;
	
	return light;
}

/* Libère la mémoire allouée à un light ***************************************
 ******************************************************************************/
void FreeLight(T_Light* light)
{
	free(light);
}

/* Création d'une texture à partir d'un PCX en mémoire ************************
 * texture_params : 0 ou une combinaison par | de valeurs GL_TEXTURE_PARAM_ENUM
 ******************************************************************************/
T_Texture* CreateTextureFromPCX(unsigned char* pcx, int texture_params)
{
	sImage* img;
	img = (sImage*)malloc(sizeof(sImage));
	if(img == NULL) {
		return NULL;
	}
	loadPCX(pcx, img);
	
	return CreateTextureFromImage(img, texture_params);
}

/* Création d'une texture à partir d'une image en mémoire *********************
 * texture_params : 0 ou une combinaison par | de valeurs GL_TEXTURE_PARAM_ENUM
 ******************************************************************************/
T_Texture* CreateTextureFromImage(sImage* img, int texture_params)
{
	T_Texture* texture;
	int type;
	int size;
	int id;
	if(img==NULL) {
		return NULL;
	}
	texture = (T_Texture*)malloc(sizeof(T_Texture));
	if(texture==NULL) {
		return NULL;
	}
	if(img->bpp == 8) {
		image8to16(img);// on ne gère que le 16bpp pour le moment
	}
	type = GL_RGB; // on ne gère que le 16bpp pour le moment
	texture->image = img;
	texture->texture_params = texture_params;
	
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &id);
	texture->id = id;
	glBindTexture(GL_TEXTURE_2D, texture->id);
	
	switch(texture->image->width) {
		case 8 : size = TEXTURE_SIZE_8; break;
		case 16 : size = TEXTURE_SIZE_16; break;
		case 32 : size = TEXTURE_SIZE_32; break;
		case 64 : size = TEXTURE_SIZE_64; break;
		case 128 : size = TEXTURE_SIZE_128; break;
		case 256 : size = TEXTURE_SIZE_256; break;
		case 512 : size = TEXTURE_SIZE_512; break;
		case 1024 : size = TEXTURE_SIZE_1024; break;
		default : size = TEXTURE_SIZE_256;
	}
	
	glTexImage2D(0, 0, type, size, size, 0, texture->texture_params, texture->image->image.data8);

	return texture;
}

/* Création d'une texture à partir d'une image en mémoire *********************
 * Eventuellement compressée
 * texture_params : 0 ou une combinaison par | de valeurs GL_TEXTURE_PARAM_ENUM
 ******************************************************************************/
T_Texture* CreateTextureFromMemory(u8* img, GL_TEXTURE_TYPE_ENUM tex_type, int texture_params, int bpp, int width, bool compression, DecompressType compressType)
{
	u8 *img_decompress;
	int size;
	T_Texture* texture;
	
	if(compression) {
		img_decompress = malloc(width*width*(bpp/8));
		decompress(img, img_decompress, compressType);
	} else {
		img_decompress = img;
	}
	
	texture = malloc(sizeof(T_Texture));
	if(texture==NULL) {
		return NULL;
	}
	texture->image = NULL;
	texture->texture_params = texture_params;
	
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &(texture->id));
	glBindTexture(GL_TEXTURE_2D, texture->id);
	
	switch(width) {
		case 8 : size = TEXTURE_SIZE_8; break;
		case 16 : size = TEXTURE_SIZE_16; break;
		case 32 : size = TEXTURE_SIZE_32; break;
		case 64 : size = TEXTURE_SIZE_64; break;
		case 128 : size = TEXTURE_SIZE_128; break;
		case 256 : size = TEXTURE_SIZE_256; break;
		case 512 : size = TEXTURE_SIZE_512; break;
		case 1024 : size = TEXTURE_SIZE_1024; break;
		default : size = TEXTURE_SIZE_256;
	}
	
	glTexImage2D(0, 0, tex_type, size, size, 0, texture->texture_params,  img_decompress);
	
	if(compression) {
		free(img_decompress);
	}

	return texture;
}


/* Libère la mémoire occupée par une texture **********************************
 ******************************************************************************/
void FreeTexture(T_Texture *texture)
{
	if(texture->image != NULL) {
		imageDestroy(texture->image);
	}
	free(texture);
}

#endif /* USING_3DENGINE */

#ifdef USING_BILLBOARDS
void DrawBillboard(T_Billboard* billboard, bool use_uv, t16 left, t16 up, t16 right, t16 down)
{
	v16 half_size;

	half_size = billboard->size / 2;
	
	glPushMatrix();
	glLoadIdentity();
	glTranslate3f32(billboard->position[0], billboard->position[1], billboard->position[2]);
	
	glPolyFmt(POLY_ID(billboard->poly_id) | POLY_ALPHA(billboard->poly_alpha) | POLY_CULL_NONE);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, billboard->texture_id);

	glColor(billboard->color);
	glBegin(GL_QUAD);
		if(use_uv) {
			glTexCoord2t16(left, up);
		} else {
			glTexCoord2f32(0, 0);
		}
		//glVertex3f(-1, 1, 0);
		glVertex3v16(-half_size, half_size, 0);
		
		if(use_uv) {
			glTexCoord2t16(left, down);
		} else {
			glTexCoord2f32(0, 1<<12);
		}
		//glVertex3f(-1, -1, 0);
		glVertex3v16(-half_size, -half_size, 0);
		
		if(use_uv) {
			glTexCoord2t16(right, down);
		} else {
			glTexCoord2f32(1<<12, 1<<12);
		}
		//glVertex3f(1, -1, 0);
		glVertex3v16(half_size, -half_size, 0);
		
		if(use_uv) {
			glTexCoord2t16(right, up);
		} else {
			glTexCoord2f32(1<<12, 0);
		}
		//glVertex3f(1, 1, 0);
		glVertex3v16(half_size, half_size, 0);
	glEnd();
	
	glPopMatrix(1);
}

void DrawOrientedBillboard(T_Billboard* billboard, Vertex3D cam_position, bool use_uv, t16 left, t16 up, t16 right, t16 down, v16 ratio)
{
	v16 vertical_half_size, horizontal_half_size;
	int32 n[3], r[3], u[3];
	Vertex3D a, b, c, d;
	
	u[0] = 0;
	u[1] = floattof32(1);
	u[2] = 0;
	
	n[0] = (int32)cam_position[0] - billboard->position[0];
	n[1] = (int32)cam_position[1] - billboard->position[1];
	n[2] = (int32)cam_position[2] - billboard->position[2];
	
	normalizef32(n);
	
	crossf32(u, n, r);
	normalizef32(r);
	crossf32(n, r, u);
	normalizef32(u);
	
	vertical_half_size = billboard->size / 2;
	horizontal_half_size = (vertical_half_size * ratio) >> 12;
	
	r[0] = (r[0] * horizontal_half_size)>>12;
	r[1] = (r[1] * horizontal_half_size)>>12;
	r[2] = (r[2] * horizontal_half_size)>>12;
	
	u[0] = (u[0] * vertical_half_size)>>12;
	u[1] = (u[1] * vertical_half_size)>>12;
	u[2] = (u[2] * vertical_half_size)>>12;
	
	a[0] = (v16)billboard->position[0] + u[0] - r[0];
	a[1] = (v16)billboard->position[1] + u[1] - r[1];
	a[2] = (v16)billboard->position[2] + u[2] - r[2];
	
	b[0] = (v16)billboard->position[0] - u[0] - r[0];
	b[1] = (v16)billboard->position[1] - u[1] - r[1];
	b[2] = (v16)billboard->position[2] - u[2] - r[2];
	
	c[0] = (v16)billboard->position[0] - u[0] + r[0];
	c[1] = (v16)billboard->position[1] - u[1] + r[1];
	c[2] = (v16)billboard->position[2] - u[2] + r[2];
	
	d[0] = (v16)billboard->position[0] + u[0] + r[0];
	d[1] = (v16)billboard->position[1] + u[1] + r[1];
	d[2] = (v16)billboard->position[2] + u[2] + r[2];
	
	glPolyFmt(POLY_ID(billboard->poly_id) | POLY_ALPHA(billboard->poly_alpha) | POLY_CULL_NONE);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, billboard->texture_id);

	glColor(billboard->color);
	glBegin(GL_QUAD);
		if(use_uv) {
			glTexCoord2t16(left, up);
		} else {
			glTexCoord2f32(0, 0);
		}
		glVertex3v16(a[0], a[1], a[2]);
		
		if(use_uv) {
			glTexCoord2t16(left, down);
		} else {
			glTexCoord2f32(0, 1<<12);
		}
		glVertex3v16(b[0], b[1], b[2]);
		
		if(use_uv) {
			glTexCoord2t16(right, down);
		} else {
			glTexCoord2f32(1<<12, 1<<12);
		}
		glVertex3v16(c[0], c[1], c[2]);
		
		if(use_uv) {
			glTexCoord2t16(right, up);
		} else {
			glTexCoord2f32(1<<12, 0);
		}
		glVertex3v16(d[0], d[1], d[2]);
	glEnd();
}
#endif /* USING_BILLBOARDS */


#ifdef USING_SetViewPort3D
void SetViewPort3D(uint8 x1, uint8 y1, uint8 x2, uint8 y2, float fov)
{
	glViewport(x1, y1, x2, y2);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(fov, ((float)(x2-x1)) / ((float)(y2-y1)), 0.1, 100);
    glMatrixMode(GL_MODELVIEW);
}
#endif /* USING_SetViewPort3D */


#ifdef USING_ChangeFov
/* Change le champ de vision de la caméra *************************************
 ******************************************************************************/
void ChangeFov(float fov)
{
	if(current_fov != fov) {
		current_fov = fov;
		glMatrixMode(GL_PROJECTION);
    	glLoadIdentity();
		gluPerspective(current_fov, 256.0 / 192.0, 0.1, 100);
		
		glMatrixMode(GL_MODELVIEW);
	}
}
#endif /* USING_ChangeFov */



