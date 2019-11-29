#include "ball.h"
#include "hole.h"

typedef struct BALLNODE
{
	PBALL ball;
	struct BALLNODE *next;
} BALLNODE;
typedef BALLNODE *PBALLNODE;

typedef struct PRIVATEBALL
{
	BOOL leftWallHit;
	BOOL rightWallHit;
	BOOL topWallHit;
	BOOL bottomWallHit;
	float delta_s;
	PBALLNODE hitBalls;

	BOOL (*hit)(PBALL ball);
	void (*setVector)(PBALL ball, COORDINATES vector);
	BOOL(*addBall)(PBALLNODE ballList, PBALL ball);
	void (*removeBall)(PBALLNODE ballList, PBALL ball);
	BOOL(*insideHole)(PBALL ball);
} PRIVATEBALL;
typedef PRIVATEBALL *PPRIVATEBALL;

void *start(PBALL ball, float speed, COORDINATES vector)
{
	ball->speed = speed;
	((PPRIVATEBALL)ball->privateData)->setVector(ball, vector);
}

void *stop(PBALL ball)
{
	ball->vector = (COORDINATES){0, 0};
	ball->speed = 0;
	((PPRIVATEBALL)ball->privateData)->delta_s = 0;
}

void *move(PBALL ball)
{
	if (((ball->vector.x == 0) && (ball->vector.y == 0)) || (ball->speed == 0) || !ball->active)
		return;

	((PPRIVATEBALL)ball->privateData)->delta_s += ball->speed;
	while ((int)(((PPRIVATEBALL)ball->privateData)->delta_s) > 0)
	{
		BOOL vectorT = (ball->vector.x == 0) && (ball->vector.y < 0);
		BOOL vectorB = (ball->vector.x == 0) && (ball->vector.y > 0);
		BOOL vectorR = (ball->vector.x > 0) && (ball->vector.y == 0);
		BOOL vectorL = (ball->vector.x < 0) && (ball->vector.y == 0);
		if (vectorT)
			ball->coordinates.y--;
		else if (vectorB)
			ball->coordinates.y++;
		else if (vectorR)
			ball->coordinates.x++;
		else if (vectorL)
			ball->coordinates.x--;
		else
		{
			ball->coordinates.x += ball->vector.x / ball->speed;
			ball->coordinates.y += ball->vector.y / ball->speed;
		}
		((PPRIVATEBALL)ball->privateData)->delta_s--;
		ball->start(ball, ball->speed - BALL_SPEED_LOSS_VALUE, ball->vector);
		if (!((PPRIVATEBALL)ball->privateData)->hit(ball))
			return;
	}
}

BOOL *hit(PBALL ball)
{
	if (((PPRIVATEBALL)ball->privateData)->insideHole(ball))
		for (PBALLLISTNODE node = firstBall; node != NULL; node = node->next)
			if (node->ball == ball)
			{
				node->ball->active = FALSE;
				return;
			}
	if (!ball->active)
		return;

	if (((int)ball->coordinates.x == FIELD_RAIL_PADDING) && (((PPRIVATEBALL)ball->privateData)->leftWallHit == FALSE))
	{
		((PPRIVATEBALL)ball->privateData)->leftWallHit = TRUE;
		float tgF = fabs(ball->vector.y) / fabs(ball->vector.x);
		ball->coordinates.y -= (ball->coordinates.x - FIELD_RAIL_PADDING) * tgF;
		ball->coordinates.x = FIELD_RAIL_PADDING;
		ball->vector.x *= -1;
		ball->start(ball, ball->speed, ball->vector);
	}
	if ((int)ball->coordinates.x > FIELD_RAIL_PADDING)
		((PPRIVATEBALL)ball->privateData)->leftWallHit = FALSE;

	if (((int)ball->coordinates.y == FIELD_RAIL_PADDING) && (((PPRIVATEBALL)ball->privateData)->topWallHit == FALSE))
	{
		((PPRIVATEBALL)ball->privateData)->topWallHit = TRUE;
		float tgF = fabs(ball->vector.x) / fabs(ball->vector.y);
		ball->coordinates.x -= (ball->coordinates.y - FIELD_RAIL_PADDING) * tgF;
		ball->coordinates.y = FIELD_RAIL_PADDING;
		ball->vector.y *= -1;
		ball->start(ball, ball->speed, ball->vector);
	}
	if ((int)ball->coordinates.y > FIELD_RAIL_PADDING)
		((PPRIVATEBALL)ball->privateData)->topWallHit = FALSE;

	if (((int)ball->coordinates.x == fieldWidth - FIELD_RAIL_PADDING - BALL_DIAMETER) && (((PPRIVATEBALL)ball->privateData)->rightWallHit == FALSE))
	{
		((PPRIVATEBALL)ball->privateData)->rightWallHit = TRUE;
		float tgF = fabs(ball->vector.y) / fabs(ball->vector.x);
		ball->coordinates.y += (fieldWidth - fabs(ball->coordinates.x) - FIELD_RAIL_PADDING - BALL_DIAMETER) * tgF;
		ball->coordinates.x = fieldWidth - FIELD_RAIL_PADDING - BALL_DIAMETER;
		ball->vector.x *= -1;
		ball->start(ball, ball->speed, ball->vector);
	}
	if ((int)ball->coordinates.x < fieldWidth - FIELD_RAIL_PADDING - BALL_DIAMETER)
		((PPRIVATEBALL)ball->privateData)->rightWallHit = FALSE;

	if (((int)ball->coordinates.y == fieldHeight - FIELD_RAIL_PADDING - BALL_DIAMETER) && (((PPRIVATEBALL)ball->privateData)->bottomWallHit == FALSE))
	{
		((PPRIVATEBALL)ball->privateData)->bottomWallHit = TRUE;
		float tgF = fabs(ball->vector.x) / fabs(ball->vector.y);
		ball->coordinates.x += (fieldHeight - fabs(ball->coordinates.y) - FIELD_RAIL_PADDING - BALL_DIAMETER) * tgF;
		ball->coordinates.y = fieldHeight - FIELD_RAIL_PADDING - BALL_DIAMETER;
		ball->vector.y *= -1;
		ball->start(ball, ball->speed, ball->vector);
	}
	if ((int)ball->coordinates.y < fieldHeight - FIELD_RAIL_PADDING - BALL_DIAMETER)
		((PPRIVATEBALL)ball->privateData)->bottomWallHit = FALSE;

	for (PBALLLISTNODE node = firstBall; node != NULL; node = node->next)
	{
		if ((node->ball == ball) || !node->ball->active)
			continue;

		float dx = ball->coordinates.x - node->ball->coordinates.x;
		float dy = ball->coordinates.y - node->ball->coordinates.y;
		float dz;
		if (dx == 0)
			dz = fabs(dy);
		else if (dy == 0)
			dz = fabs(dx);
		else
			dz = sqrt(dx * dx + dy * dy);

		if (dz < BALL_DIAMETER)
		{
			if (!((PPRIVATEBALL)ball->privateData)->addBall(((PPRIVATEBALL)ball->privateData)->hitBalls, node->ball))
			{
				((PPRIVATEBALL)node->ball->privateData)->addBall(((PPRIVATEBALL)node->ball->privateData)->hitBalls, ball);
				continue;
			}

			BOOL centralCollision;
			if (dx == 0)
				centralCollision = fabs(ball->vector.x / ball->vector.y) == fabs(dx / dy);
			else
				centralCollision = fabs(ball->vector.y / ball->vector.x) == fabs(dy / dx);
			if (centralCollision)
			{
				COORDINATES vector = ball->vector;
				float speed = ball->speed;
				ball->stop(ball);
				ball->start(ball, node->ball->speed, node->ball->vector);
				node->ball->stop(node->ball);
				node->ball->start(node->ball, speed, vector);
			}
			else
			{
				float sinF = fabs(dy / dz);
				float cosF = fabs(dx / dz);
				if ((node->ball->vector.x == 0) && (node->ball->vector.y))
				{
					COORDINATES vector;
					vector.x = ball->vector.x * cosF + ball->vector.y * sinF;
					vector.y = -ball->vector.x * sinF + ball->vector.y * cosF;

					node->ball->vector.x = vector.x * cosF;
					node->ball->vector.y = vector.x * sinF;
					node->ball->start(node->ball, fabs(vector.x), node->ball->vector);

					ball->vector.x = -vector.y * sinF;
					ball->vector.y = vector.y * cosF;
					ball->start(ball, fabs(vector.y), ball->vector);
				}
				else
				{
					COORDINATES vector1;
					vector1.x = ball->vector.x * cosF + ball->vector.y * sinF;
					vector1.y = -ball->vector.x * sinF + ball->vector.y * cosF;

					COORDINATES vector2;
					vector2.x = node->ball->vector.x * cosF + node->ball->vector.y * sinF;
					vector2.y = -node->ball->vector.x * sinF + node->ball->vector.y * cosF;

					ball->vector.x = vector2.x * cosF - vector1.y * sinF;
					ball->vector.y = vector2.x * sinF + vector1.y * cosF;
					ball->start(ball, sqrt(vector2.x * vector2.x + vector1.y * vector1.y), ball->vector);

					node->ball->vector.x = vector1.x * cosF - vector2.y * sinF;
					node->ball->vector.y = vector1.x * sinF + vector2.y * cosF;
					node->ball->start(node->ball, sqrt(vector1.x * vector1.x + vector2.y * vector2.y), node->ball->vector);
				}
			}
		}
		else
		{
			((PPRIVATEBALL)ball->privateData)->removeBall(((PPRIVATEBALL)ball->privateData)->hitBalls, node->ball);
			((PPRIVATEBALL)node->ball->privateData)->removeBall(((PPRIVATEBALL)node->ball->privateData)->hitBalls, ball);
		}
	}
}

void *setVector(PBALL ball, COORDINATES vector)
{
	if (vector.x == 0)
	{
		ball->vector.x = 0;
		if (vector.y > 0)
			ball->vector.y = ball->speed;
		else if (vector.y < 0)
			ball->vector.y = -ball->speed;
	}
	else if (vector.y == 0)
	{
		ball->vector.y = 0;
		if (vector.x > 0)
			ball->vector.x = ball->speed;
		else if (vector.x < 0)
			ball->vector.x = -ball->speed;
	}
	else
	{
		float z = sqrt(vector.x * vector.x + vector.y * vector.y);
		ball->vector.x = (vector.x * ball->speed) / z;
		ball->vector.y = (vector.y * ball->speed) / z;
	}
}

BOOL *addBall(PBALLNODE ballList, PBALL ball)
{
	if (ballList == NULL)
	{
		ballList = (PBALLNODE)malloc(sizeof(BALLNODE));
		ballList->ball = ball;
		ballList->next = NULL;
		return TRUE;
	}
	else
	{
		for (PBALLNODE node = ballList; node != NULL; node = node->next)
			if (node->ball == ball)
				return FALSE;
			else if (node->next == NULL)
			{
				node->next = (PBALLNODE)malloc(sizeof(BALLNODE));
				node = node->next;
				node->ball = ball;
				node->next = NULL;
				return TRUE;
			}
	}
}

void *removeBall(PBALLNODE ballList, PBALL ball)
{
	if (ballList == NULL)
		return;
	if (ballList->ball == ball)
	{
		PBALLNODE temp = ballList;
		ballList = ballList->next;
		free(temp);
	}
	else
	{
		for (PBALLNODE node = ballList; node != NULL; node = node->next)
			if (node->next->ball == ball)
			{
				PBALLNODE temp = node->next;
				node->next = node->next->next;
				free(temp);
				break;
			}
	}
}

BOOL *insideHole(PBALL ball)
{
	for (int i = 0; i < HOLE_COUNT; i++)
	{
		float dx = fabs(ball->coordinates.x - holes[i]->x);
		float dy = fabs(ball->coordinates.y - holes[i]->y);
		float dz;
		if (dx == 0)
			dz = dy;
		else if (dy == 0)
			dz = dx;
		else
			dz = sqrt(dx * dx + dy * dy);
		if (dz <= HOLE_RADIUS)
			return TRUE;
	}
	return FALSE;
}

PBALL newBall(COORDINATES coordinates)
{
	PBALL ball = (PBALL)malloc(sizeof(BALL));
	ball->coordinates = coordinates;
	ball->vector = (COORDINATES){0, 0};
	ball->speed = 0;
	ball->cue = (COORDINATES) {0, 0};
	ball->activeCue = FALSE;
	ball->active = TRUE;
	ball->start = &start;
	ball->stop = &stop;
	ball->move = &move;

	ball->privateData = (PPRIVATEBALL)malloc(sizeof(PRIVATEBALL));
	((PPRIVATEBALL)ball->privateData)->leftWallHit = FALSE;
	((PPRIVATEBALL)ball->privateData)->rightWallHit = FALSE;
	((PPRIVATEBALL)ball->privateData)->topWallHit = FALSE;
	((PPRIVATEBALL)ball->privateData)->bottomWallHit = FALSE;
	((PPRIVATEBALL)ball->privateData)->delta_s = 0;
	((PPRIVATEBALL)ball->privateData)->hitBalls = NULL;
	((PPRIVATEBALL)ball->privateData)->hit = &hit;
	((PPRIVATEBALL)ball->privateData)->setVector = &setVector;
	((PPRIVATEBALL)ball->privateData)->addBall = &addBall;
	((PPRIVATEBALL)ball->privateData)->removeBall = &removeBall;
	((PPRIVATEBALL)ball->privateData)->insideHole = &insideHole;

	return ball;
}

void freeBall(PBALL ball)
{
	free(ball);
}