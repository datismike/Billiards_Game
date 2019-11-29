#include "hole.h"
#include <stdlib.h>

PCOORDINATES newHole(COORDINATES coordinates)
{
	PCOORDINATES hole = (PCOORDINATES)malloc(sizeof(COORDINATES));
	hole->x = coordinates.x;
	hole->y = coordinates.y;
	return hole;
}

void freeHole(PCOORDINATES hole)
{
	free(hole);
}