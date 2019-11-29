#include "source.h"
#include "field.h"
#include "ball.h"
#include "hole.h"
#include "cue.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR szCmdLine, int nCmdShow)
{
	firstBall = NULL;

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_DBLCLKS;
	wcex.lpfnWndProc = WindowsProcedure;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = windowClassName;
	wcex.hIconSm = wcex.hIcon;
	RegisterClassEx(&wcex);

	HWND hWnd = CreateWindow(
		wcex.lpszClassName,
		windowCaption,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		FIELD_WIDTH, FIELD_HEIGHT,
		NULL, NULL, hInstance, NULL
	);

	MSG msg;
	while (GetMessage(&msg, hWnd, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

LRESULT CALLBACK WindowsProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_LBUTTONDOWN:
		{
			for (PBALLLISTNODE node = firstBall; node != NULL; node = node->next)
				if (pointInside(node->ball->coordinates.x + BALL_RADIUS, node->ball->coordinates.y + BALL_RADIUS, BALL_RADIUS, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
				{
					node->ball->activeCue = TRUE;
					node->ball->cue = (COORDINATES){node->ball->coordinates.x + BALL_RADIUS, node->ball->coordinates.y + BALL_RADIUS};
					break;
				}
			break;
		}
		case WM_MOUSEMOVE:
		{
			for (PBALLLISTNODE node = firstBall; node != NULL; node = node->next)
				if (node->ball->activeCue)
				{
					node->ball->cue = (COORDINATES){GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
					//InvalidateRect(hWnd, NULL, FALSE);
					break;;
				}
			break;
		}
		case WM_LBUTTONUP:
		{
			for (PBALLLISTNODE node = firstBall; node != NULL; node = node->next)
				if (node->ball->activeCue)
				{
					node->ball->activeCue = FALSE;
					COORDINATES mouse = (COORDINATES){GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
					float speed;
					if (pointInside(node->ball->coordinates.x + BALL_RADIUS, node->ball->coordinates.y + BALL_RADIUS, CUE_MAX_PADDING, mouse.x, mouse.y))
					{
						float a = node->ball->coordinates.x + BALL_RADIUS - mouse.x;
						float b = mouse.y - (node->ball->coordinates.y + BALL_RADIUS);
						float c = sqrt(a * a + b * b);
						speed = (c * BALL_MAX_SPEED) / CUE_MAX_PADDING;
					}
					else
						speed = BALL_MAX_SPEED;
					node->ball->stop(node->ball);
					node->ball->start(node->ball, speed, (COORDINATES) { -1 * (node->ball->cue.x - (node->ball->coordinates.x + BALL_RADIUS)), -1 * (node->ball->cue.y - (node->ball->coordinates.y + BALL_RADIUS)) });
					InvalidateRect(hWnd, NULL, FALSE);
					break;
				}
			break;
		}
		case WM_TIMER:
		{
			for (PBALLLISTNODE node = firstBall; node != NULL; node = node->next)
				if (((node->ball->vector.x == 0) && (node->ball->vector.y == 0)) || (node->ball->speed == 0))
					node->ball->stop(node->ball);
				else
					node->ball->move(node->ball);
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		}
		case WM_PAINT:
		{
			SaveDC(hCmpDC);

			drawBackground();
			drawRails();
			drawHoles();
			drawBalls();
			drawCue();

			SaveDC(hDC);
			hDC = BeginPaint(hWnd, &ps);
			BitBlt(hDC, 0, 0, fieldWidth, fieldHeight, hCmpDC, 0, 0, SRCCOPY);
			EndPaint(hWnd, &ps);
			RestoreDC(hDC, -1);
			RestoreDC(hCmpDC, -1);
		}
		case WM_CREATE:
		{
			ShowWindow(hWnd, SW_SHOWNORMAL);
			UpdateWindow(hWnd);
			SetTimer(hWnd, TIMER_ID, TIMER_INTERVAL, NULL);
			break;
		}
		case WM_SIZE:
		{
			GetClientRect(hWnd, &clientRect);
			fieldWidth = clientRect.right - clientRect.left;
			fieldHeight = clientRect.bottom - clientRect.top;

			destroyHoles();
			destroyBalls();
			createHoles();
			createBalls();

			ReleaseDC(hWnd, hCmpDC);
			DeleteDC(hCmpDC);
			DeleteObject(hBmp);
			hDC = GetDC(0);
			hCmpDC = CreateCompatibleDC(hDC);
			hBmp = CreateCompatibleBitmap(hDC, clientRect.right, clientRect.bottom);
			SelectObject(hCmpDC, hBmp);
			ReleaseDC(0, hDC);
			break;
		}
		case WM_DESTROY:
		{
			destroyHoles();
			destroyBalls();
			ExitProcess(0);
		}
		default:
		{
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
	}
}

BOOL pointInside(int centerX, int centerY, int radius, int pointX, int pointY)
{
	int a = abs(centerX - pointX);
	int b = abs(centerY - pointY);
	int c = sqrt(a * a + b * b);
	if (c <= radius)
		return TRUE;
	else
		return FALSE;
}

void drawBackground()
{
	LOGBRUSH lBrush;
	HBRUSH brush;
	lBrush.lbColor = FIELD_BACKGROUND_COLOR;
	lBrush.lbStyle = BS_SOLID;
	brush = CreateBrushIndirect(&lBrush);
	SelectObject(hCmpDC, brush);
	FillRect(hCmpDC, &clientRect, brush);
	DeleteObject(brush);
}

void drawRails()
{
	LOGBRUSH lBrush;
	HBRUSH brush;
	lBrush.lbColor = FIELD_RAIL_BRUSH_COLOR;
	lBrush.lbStyle = BS_SOLID;
	brush = CreateBrushIndirect(&lBrush);
	SelectObject(hCmpDC, brush);
	HPEN pen = CreatePen(PS_SOLID, 1, FIELD_RAIL_BRUSH_COLOR);
	SelectObject(hCmpDC, pen);
	Rectangle(hCmpDC, 0, 0, FIELD_RAIL_PADDING, fieldHeight);
	Rectangle(hCmpDC, 0, fieldHeight, fieldWidth, fieldHeight - FIELD_RAIL_PADDING);
	Rectangle(hCmpDC, fieldWidth - FIELD_RAIL_PADDING, 0, fieldWidth, fieldHeight);
	Rectangle(hCmpDC, 0, 0, fieldWidth, FIELD_RAIL_PADDING);
	DeleteObject(pen);
	DeleteObject(brush);
	pen = CreatePen(PS_SOLID, FIELD_RAIL_PEN_WIDTH, FIELD_RAIL_PEN_COLOR);
	SelectObject(hCmpDC, pen);
	MoveToEx(hCmpDC, FIELD_RAIL_PEN_WIDTH / 2, 0, NULL);
	LineTo(hCmpDC, FIELD_RAIL_PEN_WIDTH / 2, fieldHeight - FIELD_RAIL_PEN_WIDTH / 2);
	LineTo(hCmpDC, fieldWidth - FIELD_RAIL_PEN_WIDTH / 2, fieldHeight - FIELD_RAIL_PEN_WIDTH / 2);
	LineTo(hCmpDC, fieldWidth - FIELD_RAIL_PEN_WIDTH / 2, FIELD_RAIL_PEN_WIDTH / 2);
	LineTo(hCmpDC, FIELD_RAIL_PEN_WIDTH / 2, FIELD_RAIL_PEN_WIDTH / 2);
	MoveToEx(hCmpDC, FIELD_RAIL_PADDING - FIELD_RAIL_PEN_WIDTH / 2, FIELD_RAIL_PADDING - FIELD_RAIL_PEN_WIDTH / 2, NULL);
	LineTo(hCmpDC, FIELD_RAIL_PADDING - FIELD_RAIL_PEN_WIDTH / 2, fieldHeight - FIELD_RAIL_PADDING + FIELD_RAIL_PEN_WIDTH / 2);
	LineTo(hCmpDC, fieldWidth - FIELD_RAIL_PADDING + FIELD_RAIL_PEN_WIDTH / 2, fieldHeight - FIELD_RAIL_PADDING + FIELD_RAIL_PEN_WIDTH / 2);
	LineTo(hCmpDC, fieldWidth - FIELD_RAIL_PADDING + FIELD_RAIL_PEN_WIDTH / 2, FIELD_RAIL_PADDING - FIELD_RAIL_PEN_WIDTH / 2);
	LineTo(hCmpDC, FIELD_RAIL_PADDING - FIELD_RAIL_PEN_WIDTH / 2, FIELD_RAIL_PADDING - FIELD_RAIL_PEN_WIDTH / 2);
	DeleteObject(pen);
}

void drawHoles()
{
	LOGBRUSH lBrush;
	HBRUSH brush;
	lBrush.lbColor = HOLE_BRUSH_COLOR;
	lBrush.lbStyle = BS_SOLID;
	brush = CreateBrushIndirect(&lBrush);
	SelectObject(hCmpDC, brush);
	HPEN pen = CreatePen(PS_SOLID, 1, HOLE_PEN_COLOR);
	SelectObject(hCmpDC, pen);
	for (int i = 0; i < HOLE_COUNT; i++)
		Ellipse(hCmpDC, holes[i]->x, holes[i]->y, holes[i]->x + HOLE_DIAMETER, holes[i]->y + HOLE_DIAMETER);
	DeleteObject(pen);
	DeleteObject(brush);
}

void drawBalls()
{
	LOGBRUSH lBrush;
	HBRUSH brush;
	lBrush.lbColor = BALL_BRUSH_COLOR;
	lBrush.lbStyle = BS_SOLID;
	brush = CreateBrushIndirect(&lBrush);
	SelectObject(hCmpDC, brush);
	HPEN pen = CreatePen(PS_SOLID, 1, BALL_PEN_COLOR);
	SelectObject(hCmpDC, pen);
	for (PBALLLISTNODE node = firstBall; node != NULL; node = node->next)
		if (node->ball->active)
			Ellipse(hCmpDC, node->ball->coordinates.x, node->ball->coordinates.y, node->ball->coordinates.x + BALL_DIAMETER, node->ball->coordinates.y + BALL_DIAMETER);
	DeleteObject(pen);
	DeleteObject(brush);
}

void drawCue()
{
	HPEN pen = CreatePen(PS_SOLID, CUE_WIDTH, CUE_COLOR);
	SelectObject(hCmpDC, pen);
	for (PBALLLISTNODE node = firstBall; node != NULL; node = node->next)
		if (node->ball->activeCue && node->ball->active)
		{
			MoveToEx(hCmpDC, node->ball->coordinates.x + BALL_RADIUS, node->ball->coordinates.y + BALL_RADIUS, NULL);
			Ellipse(hCmpDC, node->ball->coordinates.x + BALL_RADIUS - CUE_RADIUS, node->ball->coordinates.y + BALL_RADIUS - CUE_RADIUS, node->ball->coordinates.x + BALL_RADIUS + CUE_RADIUS, node->ball->coordinates.y + BALL_RADIUS + CUE_RADIUS);
			if (pointInside(node->ball->coordinates.x + BALL_RADIUS, node->ball->coordinates.y + BALL_RADIUS, CUE_MAX_PADDING, node->ball->cue.x, node->ball->cue.y))
			{
				LineTo(hCmpDC, node->ball->cue.x, node->ball->cue.y);
				Ellipse(hCmpDC, node->ball->cue.x - CUE_RADIUS, node->ball->cue.y - CUE_RADIUS, node->ball->cue.x + CUE_RADIUS, node->ball->cue.y + CUE_RADIUS);
			}
			else
			{
				int a = abs(node->ball->cue.x - (node->ball->coordinates.x + BALL_RADIUS));
				int b = abs(node->ball->cue.y - (node->ball->coordinates.y + BALL_RADIUS));
				float c = sqrt(a * a + b * b);
				float tgA = (float)a / (float)b;
				float cosA = 1 / sqrt(1 + tgA * tgA);
				float delta_y = CUE_MAX_PADDING * cosA;
				float delta_x = CUE_MAX_PADDING * sqrt(1 - cosA * cosA);
				float x, y;
				if (node->ball->cue.x <= node->ball->coordinates.x)
					x = node->ball->cue.x + (a - delta_x);
				else
					x = node->ball->cue.x - (a - delta_x);
				if (node->ball->cue.y <= node->ball->coordinates.y)
					y = node->ball->cue.y + (b - delta_y);
				else
					y = node->ball->cue.y - (b - delta_y);
				LineTo(hCmpDC, x, y);
				Ellipse(hCmpDC, x - CUE_RADIUS, y - CUE_RADIUS, x + CUE_RADIUS, y + CUE_RADIUS);
			}
			break;
		}
	DeleteObject(pen);
}

void createHoles()
{
	holes[0] = newHole((COORDINATES) { FIELD_RAIL_PADDING, FIELD_RAIL_PADDING });
	holes[1] = newHole((COORDINATES) { FIELD_RAIL_PADDING, fieldHeight / 2 - HOLE_RADIUS });
	holes[2] = newHole((COORDINATES) { FIELD_RAIL_PADDING, fieldHeight - FIELD_RAIL_PADDING - HOLE_DIAMETER });
	holes[3] = newHole((COORDINATES) { fieldWidth - FIELD_RAIL_PADDING - HOLE_DIAMETER, FIELD_RAIL_PADDING });
	holes[4] = newHole((COORDINATES) { fieldWidth - FIELD_RAIL_PADDING - HOLE_DIAMETER, fieldHeight / 2 - HOLE_RADIUS });
	holes[5] = newHole((COORDINATES) { fieldWidth - FIELD_RAIL_PADDING - HOLE_DIAMETER, fieldHeight - FIELD_RAIL_PADDING - HOLE_DIAMETER });
}

void destroyHoles()
{
	for (int i = 0; i < HOLE_COUNT; i++)
		freeHole(holes[i]);
}

void createBalls()
{
	int y = 100;
	int dx = 2;
	int dy = BALL_RADIUS + BALL_RADIUS / sqrt(3) + 3;

	// 1
	PBALLLISTNODE node = (PBALLLISTNODE)malloc(sizeof(BALLLISTNODE));
	firstBall = node;
	node->ball = newBall((COORDINATES) { fieldWidth / 2 - BALL_RADIUS - 2 * BALL_DIAMETER - 2 * dx, y });
	node->prev = NULL;

	// 2
	node->next = (PBALLLISTNODE)malloc(sizeof(BALLLISTNODE));
	node->next->prev = node;
	node = node->next;
	node->ball = newBall((COORDINATES) { fieldWidth / 2 - BALL_RADIUS - BALL_DIAMETER - dx, y });

	// 3
	node->next = (PBALLLISTNODE)malloc(sizeof(BALLLISTNODE));
	node->next->prev = node;
	node = node->next;
	node->ball = newBall((COORDINATES) { fieldWidth / 2 - BALL_RADIUS, y });

	// 4
	node->next = (PBALLLISTNODE)malloc(sizeof(BALLLISTNODE));
	node->next->prev = node;
	node = node->next;
	node->ball = newBall((COORDINATES) { fieldWidth / 2 + BALL_RADIUS + dx, y });

	// 5
	node->next = (PBALLLISTNODE)malloc(sizeof(BALLLISTNODE));
	node->next->prev = node;
	node = node->next;
	node->ball = newBall((COORDINATES) { fieldWidth / 2 + BALL_RADIUS + BALL_DIAMETER + 2 * dx, y });

	// 6
	y = node->ball->coordinates.y;
	node->next = (PBALLLISTNODE)malloc(sizeof(BALLLISTNODE));
	node->next->prev = node;
	node = node->next;
	node->ball = newBall((COORDINATES) { fieldWidth / 2 - 2 * BALL_DIAMETER - dx - dx / 2, y + dy });

	// 7
	node->next = (PBALLLISTNODE)malloc(sizeof(BALLLISTNODE));
	node->next->prev = node;
	node = node->next;
	node->ball = newBall((COORDINATES) { fieldWidth / 2 - BALL_DIAMETER - dx / 2, y + dy });

	// 8
	node->next = (PBALLLISTNODE)malloc(sizeof(BALLLISTNODE));
	node->next->prev = node;
	node = node->next;
	node->ball = newBall((COORDINATES) { fieldWidth / 2 + dx / 2, y + dy });

	// 9 
	node->next = (PBALLLISTNODE)malloc(sizeof(BALLLISTNODE));
	node->next->prev = node;
	node = node->next;
	node->ball = newBall((COORDINATES) { fieldWidth / 2 + BALL_DIAMETER + dx + dx / 2, y + dy });

	// 10
	y = node->ball->coordinates.y;
	node->next = (PBALLLISTNODE)malloc(sizeof(BALLLISTNODE));
	node->next->prev = node;
	node = node->next;
	node->ball = newBall((COORDINATES) { fieldWidth / 2 - BALL_RADIUS - BALL_DIAMETER - dx, y + dy });

	// 11
	node->next = (PBALLLISTNODE)malloc(sizeof(BALLLISTNODE));
	node->next->prev = node;
	node = node->next;
	node->ball = newBall((COORDINATES) { fieldWidth / 2 - BALL_RADIUS, y + dy });

	// 12
	node->next = (PBALLLISTNODE)malloc(sizeof(BALLLISTNODE));
	node->next->prev = node;
	node = node->next;
	node->ball = newBall((COORDINATES) { fieldWidth / 2 + BALL_RADIUS + dx, y + dy });

	// 13
	y = node->ball->coordinates.y;
	node->next = (PBALLLISTNODE)malloc(sizeof(BALLLISTNODE));
	node->next->prev = node;
	node = node->next;
	node->ball = newBall((COORDINATES) { fieldWidth / 2 - BALL_DIAMETER - dx / 2, y + dy });

	// 14
	node->next = (PBALLLISTNODE)malloc(sizeof(BALLLISTNODE));
	node->next->prev = node;
	node = node->next;
	node->ball = newBall((COORDINATES) { fieldWidth / 2 + dx / 2, y + dy });

	// 15
	y = node->ball->coordinates.y;
	node->next = (PBALLLISTNODE)malloc(sizeof(BALLLISTNODE));
	node->next->prev = node;
	node = node->next;
	node->ball = newBall((COORDINATES) { fieldWidth / 2 - BALL_RADIUS, y + dy });

	// 0
	y = node->ball->coordinates.y;
	node->next = (PBALLLISTNODE)malloc(sizeof(BALLLISTNODE));
	node->next->prev = node;
	node = node->next;
	node->ball = newBall((COORDINATES) { fieldWidth / 2 - BALL_RADIUS, y + BALL_DIAMETER + 150 });
	node->next = NULL;
}

void destroyBalls()
{
	for (PBALLLISTNODE node = firstBall; node != NULL;)
	{
		freeBall(node->ball);
		PBALLLISTNODE temp = node;
		node = node->next;
		free(temp);
	}
	firstBall = NULL;
}