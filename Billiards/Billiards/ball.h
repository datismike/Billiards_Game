#include "field.h"
#include <stdlib.h>
#include <math.h>

#pragma once

#define BOOL int
#define TRUE 1
#define FALSE 0

#define BALL_RADIUS 10
#define BALL_DIAMETER (2 * BALL_RADIUS)
#define BALL_BRUSH_COLOR 0x00A0A0A0
#define BALL_PEN_COLOR 0x00000000
#define BALL_SPEED_LOSS_VALUE 0.03
#define BALL_MAX_SPEED 25

typedef struct COORDINATES
{
	float x;
	float y;
} COORDINATES;
typedef COORDINATES *PCOORDINATES;

typedef struct BALL
{
	COORDINATES coordinates;
	COORDINATES vector;
	float speed;
	COORDINATES cue;
	BOOL activeCue;
	BOOL active;

	void (*start)(struct BALL *, float, COORDINATES);
	void (*stop)(struct BALL *);
	void (*move)(struct BALL *);

	void *privateData;
} BALL;
typedef BALL *PBALL;

typedef struct BALLLISTNODE
{
	PBALL ball;
	struct BALLLISTNODE *next;
	struct BALLLISTNODE *prev;
} BALLLISTNODE;
typedef BALLLISTNODE *PBALLLISTNODE;

PBALLLISTNODE firstBall;
PBALL newBall(COORDINATES);
void freeBall(PBALL);