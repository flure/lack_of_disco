#ifndef PATHS_H_
#define PATHS_H_

#include <nds.h>
#include <interpolation.h>

typedef struct path_point {
	int32 time;
	int32 x;
	int32 y;
	int32 z;
} tPathPoint;

typedef struct path {
	int nb_path_points;
	tPathPoint* path_points;
} tPath;

/* Trouve la prochaine coordonnée sur le chemin défini par path
 * à l'instant time, en utilisant la fonction d'interpolation lerp
 * Le résultat est donné dans la variable xyz
 */
void PathFollow(tPath* path, int32 time, InterpolateFunc lerp, int32 xyz[3]); 

/* Trouve la prochaine coordonnée sur le chemin défini par path
 * à l'instant time, en utilisant la fonction d'interpolation par spline lerp
 * Le résultat est donné dans la variable xyz
 */
void PathSplineFollow(tPath* path, int32 time, SplineFunc lerp, int32 xyz[3]);

#endif /*PATHS_H_*/
