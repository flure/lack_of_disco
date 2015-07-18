#ifndef UTIL3D_H
#define UTIL3D_H

#include <nds.h>

#include <usings.h>

#ifdef USING_UTIL3D
#define USING_3DTYPES
#define USING_3DENGINE
#define USING_ChangeFov
#define USING_SetViewPort3D
#endif /* USING_UTIL3D */

/* Types communs **************************************************************
 ******************************************************************************/
#ifdef USING_3DTYPES
typedef v16 Vertex3D[3];
typedef int32 PackedNormal3D;
typedef float Vec2D[2];
typedef int32 Tex2D[2];
typedef int Face[3];
typedef u8 Color[3];
typedef int tQuad[4];
typedef int tTri[3];
#endif /* USING_3DTYPES */

#ifdef USING_3DENGINE
/* Initialise le moteur 3D. ***************************************************
 * Doit être appelée avant l'utilisation de RenderScene ***********************
 ******************************************************************************/
void Init3DEngine(void);

/* Définition d'une texture ***************************************************
 ******************************************************************************/
typedef struct t_texture {
	sImage* image;
	int texture_params; /* liste de GL_TEXTURE_PARAM_ENUM combinés avec des | */
	int id; /* identifiant de la texture */
} T_Texture;

/* Création d'une texture à partir d'un PCX en mémoire ************************
 * texture_params : 0 ou une combinaison par | de valeurs GL_TEXTURE_PARAM_ENUM
 ******************************************************************************/
T_Texture* CreateTextureFromPCX(unsigned char* pcx, int texture_params);

/* Création d'une texture à partir d'une image en mémoire *********************
 * texture_params : 0 ou une combinaison par | de valeurs GL_TEXTURE_PARAM_ENUM
 ******************************************************************************/
T_Texture* CreateTextureFromImage(sImage* img, int texture_params);

/* Création d'une texture à partir d'une image en mémoire *********************
 * Eventuellement compressée
 * texture_params : 0 ou une combinaison par | de valeurs GL_TEXTURE_PARAM_ENUM
 ******************************************************************************/
T_Texture* CreateTextureFromMemory(u8* img, GL_TEXTURE_TYPE_ENUM tex_type, int texture_params, int bpp, int width, bool compression, DecompressType compressType);

/* Libère la mémoire occupée par une texture **********************************
 ******************************************************************************/
void FreeTexture(T_Texture *texture);


/* Définition d'un objet 3D ***************************************************
 ******************************************************************************/
typedef struct t_mesh {
	Vertex3D* vertices;  /* les sommets */
	PackedNormal3D* normals;   /* les normales, packées (3 v10 dans un int32) */
	Color* colors;    /* les couleurs */
	Tex2D* texcoords; /* les coordonnées de texture */
	int nb_vertices;  /* le nombre de sommets. Il y a autant de normals, colors, texcoords */
	bool lighting; /* TRUE : utilise les normales et l'éclairage - FALSE : utilise les couleurs et pas l'éclairage */
	
	Face* faces;      /* les facettes */
	int nb_faces;     /* le nombre de facettes */
	
	int texture_id;      /* l'identifiant de texture */
	bool texturing;
	u8 alpha;           /* 0 = transparent - 31 = opaque */
	
	
	u32* display_list; /* pour affichage rapide par display list */
} T_Mesh;

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
T_Mesh* CreateMesh(void);

/* Alloue la mémoire pour nb_vertices sommets *********************************
 * Alloue en même temps la mémoire pour les normales, couleurs et coordonnées
 * de texture.
 * ATTENTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * Les données de vertex, normales, couleurs et coordonnées de texture déjà !!!
 * existantes sont supprimées et leur mémoire libérée !!!!!!!!!!!!!!!!!!!!!!!!!
 ******************************************************************************/
void AllocVertices(T_Mesh* mesh, int nb_vertices);

/* Alloue la mémoire pour nb_faces faces **************************************
 ******************************************************************************/
void AllocFaces(T_Mesh*, int nb_faces);

/* Libère la mémoire occupée par un objet 3D **********************************
 ******************************************************************************/
void FreeMesh(T_Mesh* mesh);





/* Définition d'une position et orientation dans l'espace *********************
 ******************************************************************************/
typedef struct t_transform {
	int32 translation[3]; /* 1.19.12 */
	int rotation[3];
} T_Transform;

/* Définition d'une instance d'un objet dans l'espace *************************
 ******************************************************************************/
typedef struct t_obj3d {
	T_Mesh* mesh;
	T_Transform* transform;
} T_Obj3D;

/* Alloue la mémoire d'une instance d'objet ***********************************
 * Initialisée en utilisant l'objet (mesh) passé en paramètre. Une 
 * transformation par défaut est appliquée : pas de  translation ni rotation.
 ******************************************************************************/
T_Obj3D* CreateObj3D(T_Mesh* mesh);

/* Libère la mémoire occupée par une instance d'un objet 3D *******************
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * ATTENTION : la mémoire occupée par l'objet 3D (variable mesh) devra être
 * libérée séparément par FreeMesh, car un mesh peut être utilisé par plusieurs
 * T_Obj3D différents
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 ******************************************************************************/
void FreeObj3D(T_Obj3D* object);




/* Une camera : permet de définir le point de vue de la scène *****************
 ******************************************************************************/
typedef struct t_camera {
	int32 position[3]; /* position de la camera, en 1.19.12 */
	int32 lookat[3];   /* le point d'intérêt, en 1.19.12 */
	int32 up[3];       /* indique où se trouve le haut, en 1.19.12 ! */
} T_Camera;

/* Alloue et initialise une camera ********************************************
 * Cette camera est positionnée en (0,0,0) et regarde dans la direction -Z    *
 ******************************************************************************/
T_Camera* CreateCamera(void);

/* Libère la mémoire occupée par une camera ***********************************
 ******************************************************************************/
void FreeCamera(T_Camera* camera);




/* Pour que ce ne soit pas tout noir !! ***************************************
 ******************************************************************************/
typedef struct t_light {
	v10 direction[3];
	rgb color;
	bool on; /* TRUE si la lumière est allumée */
} T_Light;

/* Crée une light avec les paramètres donnés **********************************
 ******************************************************************************/
T_Light* CreateLight(v10 dir_x, v10 dir_y, v10 dir_z, rgb color, bool onoff);

/* Libère la mémoire allouée à un light ***************************************
 ******************************************************************************/
void FreeLight(T_Light* light);

typedef struct t_scene {
	T_Obj3D* objects;
	int nb_objects;
	
	T_Light* lights[4];
	bool lighting;
	
	T_Texture* textures;
	int nb_textures;	
	
	T_Camera* camera;
} T_Scene;

/* Alloue et initialise une scene 3D ******************************************
 * Crée une scene par défaut avec :                                           *
 * 0 objet                                                                    *
 * 0 light (ligthing désactivé)                                               *
 * 1 caméra positionnée en (0,0,0) et regardant dans la direction -Z          *
 ******************************************************************************/
T_Scene* CreateScene(void);

/* Ajouter une instance d'objet à une scene 3D ********************************
 ******************************************************************************/
void AddObject(T_Scene* scene, T_Obj3D* obj);

/* Ajouter une texture à une scene 3D *****************************************
 ******************************************************************************/
void AddTexture(T_Scene* scene, T_Texture* texture);

/* Libère la mémoire occupée par une scene 3D *********************************
 ******************************************************************************/
void FreeScene(T_Scene* scene);

/* Effectue le rendu de la scène **********************************************
 ******************************************************************************/
void RenderScene(T_Scene* scene);

#endif /* USING_3DENGINE */

#ifdef USING_BILLBOARDS

typedef struct t_billboard {
	int32 position[3];
	v16 size;
	rgb color;
	int texture_id;
	int poly_alpha; 
	int poly_id;
} T_Billboard;

void DrawBillboard(T_Billboard* billboard, bool use_uv, t16 left, t16 up, t16 right, t16 down);
void DrawOrientedBillboard(T_Billboard* billboard, Vertex3D cam_position, bool use_uv, t16 left, t16 up, t16 right, t16 down, v16 ratio);

#endif /* USING_BILLBOARDS */

/* Change le champ de vision de la caméra *************************************
 ******************************************************************************/
#ifdef USING_ChangeFov
void ChangeFov(float fov);
#endif /* USING_ChangeFov */

#ifdef USING_SetViewPort3D
void SetViewPort3D(uint8 x1, uint8 y1, uint8 x2, uint8 y2, float fov);
#endif /* USING_SetViewPort3D */

//void _RenderObject(T_Obj3D* obj);

#endif // UTIL3D_H
