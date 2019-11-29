#include <Windows.h>
#include <windowsx.h> // cursor
#include <math.h> // sqrt

#pragma once

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR szCmdLine, int nCmdShow);
LRESULT CALLBACK WindowsProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL pointInside(int centerX, int centerY, int radius, int pointX, int pointY);
void drawBackground();
void drawRails();
void drawHoles();
void drawBalls();
void drawCue();
void createHoles();
void createBalls();
void destroyHoles();
void destroyBalls();

HDC hDC, hCmpDC;
HBITMAP hBmp;
RECT clientRect;
PAINTSTRUCT ps;
char *windowClassName = "windowMain";
char *windowCaption = "Billiards";