#include <paths.h>

#include <math.h>

/* Trouve la prochaine coordonnée sur le chemin défini par path
 * à l'instant time, en utilisant la fonction d'interpolation lerp
 */
void PathFollow(tPath* path, int32 time, InterpolateFunc lerp, int32 xyz[3])
{
	int i, p0, p1;
	int32 t;
	
	p0 = -1;
	p1 = -1;
	
	/* trouve les deux points entre lesquels on interpole */
	for(i=0; i<path->nb_path_points-1; i++)
	{
		if((time >= path->path_points[i].time) && (time < path->path_points[i+1].time)) {
			p0 = i;
			p1 = i+1;
			break;
		}
	}

	/* Si aucun point trouvé, on reste sur le dernier point */	
	if(p0 == -1) {
		p0 = path->nb_path_points-1;
		xyz[0] = path->path_points[p0].x;
		xyz[1] = path->path_points[p0].y;
		xyz[2] = path->path_points[p0].z;
		
		return;
	}
	
	/* t est l'équivalent de time rapporté entre 0 et 1 */
	t = divf32(time - path->path_points[p0].time, path->path_points[p1].time - path->path_points[p0].time);
	
	/* on calcule les coordonnées en fonction de t */
	xyz[0] = lerp(path->path_points[p0].x, path->path_points[p1].x, t);
	xyz[1] = lerp(path->path_points[p0].y, path->path_points[p1].y, t);
	xyz[2] = lerp(path->path_points[p0].z, path->path_points[p1].z, t);
}

/* Trouve la prochaine coordonnée sur le chemin défini par path
 * à l'instant time, en utilisant la fonction d'interpolation par spline lerp
 * Le résultat est donné dans la variable xyz
 */
void PathSplineFollow(tPath* path, int32 time, SplineFunc slerp, int32 xyz[3])
{
	int i, p0, p1;
	int32 t;
	int32 start[3], end[3];
	
	p0 = -1;
	p1 = -1;
	
	/* trouve les deux points entre lesquels on interpole */
	for(i=0; i<path->nb_path_points-1; i++)
	{
		if((time >= path->path_points[i].time) && (time < path->path_points[i+1].time)) {
			p0 = i;
			p1 = i+1;
			break;
		}
	}

	/* Si aucun point trouvé, on reste sur le dernier point */	
	if(p0 == -1) {
		p0 = path->nb_path_points-1;
		xyz[0] = path->path_points[p0].x;
		xyz[1] = path->path_points[p0].y;
		xyz[2] = path->path_points[p0].z;
		
		return;
	}
	
	/* Si on est aux extrémités de la courbe, on ajoute un point de contrôle
	 * de de manière arbitraire */
	if(p0 == 0) {
		start[0] = path->path_points[0].x * 1.2;
		start[1] = path->path_points[0].y * 1.2;
		start[2] = path->path_points[0].z * 1.2;
	} else {
		start[0] = path->path_points[p0-1].x;
		start[1] = path->path_points[p0-1].y;
		start[2] = path->path_points[p0-1].z;
	}
	if(p1 == (path->nb_path_points-1)) {
		end[0] = path->path_points[p1].x * 1.2;
		end[1] = path->path_points[p1].y * 1.2;
		end[2] = path->path_points[p1].z * 1.2;
	} else {
		end[0] = path->path_points[p1+1].x;
		end[1] = path->path_points[p1+1].y;
		end[2] = path->path_points[p1+1].z;
	}
	
	
	/* t est l'équivalent de time rapporté entre 0 et 1 */
	t = divf32(time - path->path_points[p0].time, path->path_points[p1].time - path->path_points[p0].time);
	
	/* on calcule les coordonnées en fonction de t */
	xyz[0] = slerp(start[0], path->path_points[p0].x, path->path_points[p1].x, end[0], t);
	xyz[1] = slerp(start[1], path->path_points[p0].y, path->path_points[p1].y, end[1], t);
	xyz[2] = slerp(start[2], path->path_points[p0].z, path->path_points[p1].z, end[2], t);
}