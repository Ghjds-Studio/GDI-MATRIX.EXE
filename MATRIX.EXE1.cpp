//Code By Ghjds
//MATRIX.EXE-GDI
#include <cmath>
#include <Windows.h>
#include <stdlib.h>
#include <time.h>
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"msimg32.lib")

typedef union _RGBQ {
	COLORREF rgb;
	struct
	{
		BYTE r;
		BYTE g;
		BYTE b;
		BYTE Re;
	};
} _RGBQ, * RGBQ;
typedef struct {
	FLOAT h;
	FLOAT s;
	FLOAT l;
} HSL;
namespace Colors
{
	HSL rgb2hsl(RGBQUAD rgb)
	{
		HSL hsl;

		BYTE r = rgb.rgbRed;
		BYTE g = rgb.rgbGreen;
		BYTE b = rgb.rgbBlue;

		FLOAT _r = (FLOAT)r / 255.f;
		FLOAT _g = (FLOAT)g / 255.f;
		FLOAT _b = (FLOAT)b / 255.f;

		FLOAT rgbMin = min(min(_r, _g), _b);
		FLOAT rgbMax = max(max(_r, _g), _b);

		FLOAT fDelta = rgbMax - rgbMin;
		FLOAT deltaR;
		FLOAT deltaG;
		FLOAT deltaB;

		FLOAT h = 0.f;
		FLOAT s = 0.f;
		FLOAT l = (FLOAT)((rgbMax + rgbMin) / 2.f);

		if (fDelta != 0.f)
		{
			s = l < .5f ? (FLOAT)(fDelta / (rgbMax + rgbMin)) : (FLOAT)(fDelta / (2.f - rgbMax - rgbMin));
			deltaR = (FLOAT)(((rgbMax - _r) / 6.f + (fDelta / 2.f)) / fDelta);
			deltaG = (FLOAT)(((rgbMax - _g) / 6.f + (fDelta / 2.f)) / fDelta);
			deltaB = (FLOAT)(((rgbMax - _b) / 6.f + (fDelta / 2.f)) / fDelta);

			if (_r == rgbMax)      h = deltaB - deltaG;
			else if (_g == rgbMax) h = (1.f / 3.f) + deltaR - deltaB;
			else if (_b == rgbMax) h = (2.f / 3.f) + deltaG - deltaR;
			if (h < 0.f)           h += 1.f;
			if (h > 1.f)           h -= 1.f;
		}

		hsl.h = h;
		hsl.s = s;
		hsl.l = l;
		return hsl;
	}

	RGBQUAD hsl2rgb(HSL hsl)
	{
		RGBQUAD rgb;

		FLOAT r = hsl.l;
		FLOAT g = hsl.l;
		FLOAT b = hsl.l;

		FLOAT h = hsl.h;
		FLOAT sl = hsl.s;
		FLOAT l = hsl.l;
		FLOAT v = (l <= .5f) ? (l * (1.f + sl)) : (l + sl - l * sl);

		FLOAT m;
		FLOAT sv;
		FLOAT fract;
		FLOAT vsf;
		FLOAT mid1;
		FLOAT mid2;

		INT sextant;

		if (v > 0.f)
		{
			m = l + l - v;
			sv = (v - m) / v;
			h *= 6.f;
			sextant = (INT)h;
			fract = h - sextant;
			vsf = v * sv * fract;
			mid1 = m + vsf;
			mid2 = v - vsf;

			switch (sextant)
			{
			case 0:
				r = v;
				g = mid1;
				b = m;
				break;
			case 1:
				r = mid2;
				g = v;
				b = m;
				break;
			case 2:
				r = m;
				g = v;
				b = mid1;
				break;
			case 3:
				r = m;
				g = mid2;
				b = v;
				break;
			case 4:
				r = mid1;
				g = m;
				b = v;
				break;
			case 5:
				r = v;
				g = m;
				b = mid2;
				break;
			}
		}

		rgb.rgbRed = (BYTE)(r * 255.f);
		rgb.rgbGreen = (BYTE)(g * 255.f);
		rgb.rgbBlue = (BYTE)(b * 255.f);

		return rgb;
	}
}

COLORREF RandGreenToBlack() {
	int brightness = rand() % 255 + 1;
	return RGB(0, brightness, 0);
}

volatile BOOL g_running = TRUE;
int g_stage = 0;
DWORD g_stageStartTime = 0;
HANDLE g_hSoundThread = NULL;

int g_treeMaxDepth = 14;
BOOL g_treeFullyGrown = FALSE;
POINT g_zoomTarget = { 0, 0 };
BOOL g_zoomTargetSet = FALSE;
double g_treeZoomScale = 1.0;

double g_mandelCenterX = -0.5, g_mandelCenterY = 0.0;
double g_mandelRange = 3.0;

double g_tunnelPhase = 0.0;

void DrawPythagorasTree(HDC hdc, double x, double y, double length, double angle,
	int depth, int maxDepth, POINT* pLeaf, BOOL* pFound) {
	if (depth > maxDepth || length < 1.0) return;

	double x2 = x + length * cos(angle);
	double y2 = y - length * sin(angle);

	HPEN pen = CreatePen(PS_SOLID, 1, RandGreenToBlack());
	HPEN oldPen = (HPEN)SelectObject(hdc, pen);
	MoveToEx(hdc, (int)x, (int)y, NULL);
	LineTo(hdc, (int)x2, (int)y2);
	SelectObject(hdc, oldPen);
	DeleteObject(pen);

	if (depth == maxDepth && !(*pFound)) {
		pLeaf->x = (int)x2;
		pLeaf->y = (int)y2;
		*pFound = TRUE;
		return;
	}

	if (depth < maxDepth) {
		double newLength = length / sqrt(2.0);
		DrawPythagorasTree(hdc, x2, y2, newLength, angle + 3.14159 / 4.0,
			depth + 1, maxDepth, pLeaf, pFound);
		DrawPythagorasTree(hdc, x2, y2, newLength, angle - 3.14159 / 4.0,
			depth + 1, maxDepth, pLeaf, pFound);
	}
}

void DrawMandelbrot(HDC hdc, int width, int height,
	double centerX, double centerY, double range) {
	int calcW = width / 2;
	int calcH = height / 2;
	if (calcW < 1) calcW = 1;
	if (calcH < 1) calcH = 1;

	HDC memDC = CreateCompatibleDC(hdc);
	HBITMAP bitmap = CreateCompatibleBitmap(hdc, calcW, calcH);
	HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, bitmap);

	int maxIter = 100 + (int)(156.0 * log(3.0 / (range + 0.000001)) / log(10.0));
	if (maxIter > 256) maxIter = 256;

	double xMin = centerX - range * width / (double)height;
	double xMax = centerX + range * width / (double)height;
	double yMin = centerY - range;
	double yMax = centerY + range;
	double dx = (xMax - xMin) / calcW;
	double dy = (yMax - yMin) / calcH;

	for (int py = 0; py < calcH; ++py) {
		double y0 = yMax - py * dy;
		for (int px = 0; px < calcW; ++px) {
			double x0 = xMin + px * dx;
			double x = 0.0, y = 0.0;
			int iter = 0;
			while (x * x + y * y <= 4.0 && iter < maxIter) {
				double xtemp = x * x - y * y + x0;
				y = 2.0 * x * y + y0;
				x = xtemp;
				++iter;
			}
			COLORREF color;
			if (iter == maxIter) {
				color = RGB(0, 0, 0);
			}
			else {
				int brightness = (iter * 8) % 256;
				color = RGB(0, brightness, 0);
			}
			SetPixel(memDC, px, py, color);
		}
	}

	StretchBlt(hdc, 0, 0, width, height, memDC, 0, 0, calcW, calcH, SRCCOPY);

	SelectObject(memDC, oldBmp);
	DeleteObject(bitmap);
	DeleteDC(memDC);
}

void DrawTunnel(HDC hdc, int width, int height, double phase) {
	int cx = width / 2;
	int cy = height / 2;
	double maxRadius = sqrt(cx * cx + cy * cy);

	int numBoxes = 40;
	for (int i = 0; i < numBoxes; ++i) {
		double t = (double)i / numBoxes;
		double radius = maxRadius * (1.0 - t * 0.95);
		double angle = phase + t * 2.0;

		double corners[4][2] = {
			{ cx - radius, cy - radius },
			{ cx + radius, cy - radius },
			{ cx + radius, cy + radius },
			{ cx - radius, cy + radius }
		};

		double cosA = cos(angle);
		double sinA = sin(angle);
		POINT pts[5];
		for (int j = 0; j < 4; ++j) {
			double rx = corners[j][0] - cx;
			double ry = corners[j][1] - cy;
			pts[j].x = (int)(cx + rx * cosA - ry * sinA);
			pts[j].y = (int)(cy + rx * sinA + ry * cosA);
		}
		pts[4] = pts[0];

		int brightness = (int)(255 * (1.0 - t));
		HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, brightness, 0));
		HPEN oldPen = (HPEN)SelectObject(hdc, pen);
		Polyline(hdc, pts, 5);
		SelectObject(hdc, oldPen);
		DeleteObject(pen);
	}

	HPEN centerPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
	HPEN oldPen = (HPEN)SelectObject(hdc, centerPen);
	for (int i = 0; i < 8; ++i) {
		double angle = phase * 3.0 + i * 3.14159 / 4.0;
		MoveToEx(hdc, cx, cy, NULL);
		LineTo(hdc, cx + (int)(cos(angle) * 30), cy + (int)(sin(angle) * 30));
	}
	SelectObject(hdc, oldPen);
	DeleteObject(centerPen);
}

DWORD WINAPI NewStage0(LPVOID lpvd) {
	HDC hdc = GetDC(0);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	DWORD startTime = GetTickCount();
	while (GetTickCount() - startTime < 30000) {
		for (int i = 0; i < 300; ++i) {
			HPEN pen = CreatePen(PS_SOLID, 1, RandGreenToBlack());
			HPEN oldPen = (HPEN)SelectObject(hdc, pen);
			MoveToEx(hdc, rand() % w, rand() % h, NULL);
			LineTo(hdc, rand() % w, rand() % h);
			SelectObject(hdc, oldPen);
			DeleteObject(pen);
		}
		for (int i = 0; i < 500; ++i) {
			SetPixel(hdc, rand() % w, rand() % h, RandGreenToBlack());
		}
		Sleep(50);
	}
	ReleaseDC(0, hdc);
	return 0;
}

DWORD WINAPI NewStage1(LPVOID lpvd) {
	HDC hdc = GetDC(0);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	DWORD startTime = GetTickCount();
	while (GetTickCount() - startTime < 30000) {
		for (int i = 0; i < 200; ++i) {
			HPEN pen = CreatePen(PS_SOLID, 1, RandGreenToBlack());
			HPEN oldPen = (HPEN)SelectObject(hdc, pen);
			HBRUSH brush = CreateSolidBrush(RandGreenToBlack());
			HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
			int left = rand() % w;
			int top = rand() % h;
			Rectangle(hdc, left, top, left + rand() % 200 + 10, top + rand() % 200 + 10);
			SelectObject(hdc, oldBrush);
			DeleteObject(brush);
			SelectObject(hdc, oldPen);
			DeleteObject(pen);
		}
		Sleep(50);
	}
	ReleaseDC(0, hdc);
	return 0;
}

DWORD WINAPI NewStage2(LPVOID lpvd) {
	HDC hdc = GetDC(0);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	DWORD startTime = GetTickCount();
	while (GetTickCount() - startTime < 30000) {
		for (int i = 0; i < 200; ++i) {
			HPEN pen = CreatePen(PS_SOLID, 1, RandGreenToBlack());
			HPEN oldPen = (HPEN)SelectObject(hdc, pen);
			HBRUSH brush = CreateSolidBrush(RandGreenToBlack());
			HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
			int left = rand() % w;
			int top = rand() % h;
			Ellipse(hdc, left, top, left + rand() % 200 + 10, top + rand() % 200 + 10);
			SelectObject(hdc, oldBrush);
			DeleteObject(brush);
			SelectObject(hdc, oldPen);
			DeleteObject(pen);
		}
		Sleep(50);
	}
	ReleaseDC(0, hdc);
	return 0;
}

DWORD WINAPI NewStage3(LPVOID lpvd) {
	HDC hdc = GetDC(0);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	DWORD startTime = GetTickCount();
	while (GetTickCount() - startTime < 30000) {
		for (int i = 0; i < 150; ++i) {
			HPEN pen = CreatePen(PS_SOLID, 1, RandGreenToBlack());
			HPEN oldPen = (HPEN)SelectObject(hdc, pen);
			HBRUSH brush = CreateSolidBrush(RandGreenToBlack());
			HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
			int nPoints = rand() % 6 + 3;
			POINT points[8];
			for (int j = 0; j < nPoints; ++j) {
				points[j].x = rand() % w;
				points[j].y = rand() % h;
			}
			Polygon(hdc, points, nPoints);
			SelectObject(hdc, oldBrush);
			DeleteObject(brush);
			SelectObject(hdc, oldPen);
			DeleteObject(pen);
		}
		Sleep(50);
	}
	ReleaseDC(0, hdc);
	return 0;
}

DWORD WINAPI NewStage4(LPVOID lpvd) {
	HDC hdc = GetDC(0);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	DWORD startTime = GetTickCount();
	while (GetTickCount() - startTime < 30000) {
		for (int i = 0; i < 100; ++i) {
			HPEN pen = CreatePen(PS_SOLID, 1, RandGreenToBlack());
			HPEN oldPen = (HPEN)SelectObject(hdc, pen);
			if (rand() % 2) {
				POINT pts[4];
				for (int j = 0; j < 4; ++j) {
					pts[j].x = rand() % w;
					pts[j].y = rand() % h;
				}
				PolyBezier(hdc, pts, 4);
			}
			else {
				int left = rand() % w, top = rand() % h;
				int right = left + rand() % 200 + 10, bottom = top + rand() % 200 + 10;
				Arc(hdc, left, top, right, bottom,
					left + rand() % (right - left), top + rand() % (bottom - top),
					left + rand() % (right - left), top + rand() % (bottom - top));
			}
			SelectObject(hdc, oldPen);
			DeleteObject(pen);
		}
		Sleep(50);
	}
	ReleaseDC(0, hdc);
	return 0;
}

DWORD WINAPI NewStage5(LPVOID lpvd) {
	HDC hdc = GetDC(0);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	DWORD startTime = GetTickCount();
	BOOL treeFullyGrown = FALSE;
	POINT zoomTarget = { 0, 0 };
	BOOL zoomTargetSet = FALSE;
	double treeZoomScale = 1.0;
	int treeMaxDepth = 14;
	DWORD growDuration = 25000;
	DWORD zoomDuration = 5000;

	while (GetTickCount() - startTime < 30000) {
		DWORD elapsed = GetTickCount() - startTime;
		int currentMaxDepth;
		double progress;

		if (elapsed < growDuration) {
			progress = (double)elapsed / growDuration;
			currentMaxDepth = (int)(progress * treeMaxDepth);
			if (currentMaxDepth < 1) currentMaxDepth = 1;
			treeFullyGrown = FALSE;
			zoomTargetSet = FALSE;
			treeZoomScale = 1.0;
		}
		else {
			currentMaxDepth = treeMaxDepth;
			treeFullyGrown = TRUE;
			double zoomElapsed = elapsed - growDuration;
			treeZoomScale = 1.0 + 19.0 * (zoomElapsed / zoomDuration);
		}

		BOOL leafFound = FALSE;
		DrawPythagorasTree(hdc, w / 2, h - 50, 80.0, 3.14159 / 2.0,
			0, currentMaxDepth, &zoomTarget, &leafFound);
		if (leafFound && !zoomTargetSet) {
			zoomTargetSet = TRUE;
		}
		Sleep(50);
	}
	ReleaseDC(0, hdc);
	return 0;
}

DWORD WINAPI NewStage6(LPVOID lpvd) {
	HDC hdc = GetDC(0);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	DWORD startTime = GetTickCount();
	double centerX = -0.5, centerY = 0.0;
	double rangeStart = 3.0;
	double rangeEnd = 0.0001;

	while (GetTickCount() - startTime < 30000) {
		DWORD elapsed = GetTickCount() - startTime;
		double progress = (double)elapsed / 30000.0;
		if (progress > 1.0) progress = 1.0;
		double currentRange = rangeStart * exp(progress * log(rangeEnd / rangeStart));
		DrawMandelbrot(hdc, w, h, centerX, centerY, currentRange);
		Sleep(50);
	}
	ReleaseDC(0, hdc);
	return 0;
}

DWORD WINAPI NewStage7(LPVOID lpvd) {
	HDC hdc = GetDC(0);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	DWORD startTime = GetTickCount();

	HFONT hFont = CreateFontA(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, FIXED_PITCH, "Consolas");
	HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
	SetBkMode(hdc, TRANSPARENT);

	while (GetTickCount() - startTime < 30000) {
		HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
		RECT rect = { 0, 0, w, h };
		FillRect(hdc, &rect, blackBrush);
		DeleteObject(blackBrush);

		for (int i = 0; i < 30; ++i) {
			int x = rand() % (w - 300);
			int y = rand() % (h - 60);
			SetTextColor(hdc, RandGreenToBlack());
			TextOutA(hdc, x, y, "MATRIX.EXE", 10);
		}
		for (int i = 0; i < 10; ++i) {
			int x = rand() % (w - 300);
			int y = rand() % (h - 60);
			SetTextColor(hdc, RandGreenToBlack());
			TextOutA(hdc, x, y, "Made By Ghjds", 13);
		}
		Sleep(50);
	}

	SelectObject(hdc, oldFont);
	DeleteObject(hFont);
	ReleaseDC(0, hdc);
	return 0;
}

DWORD WINAPI NewStage8(LPVOID lpvd) {
	HDC hdc = GetDC(0);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	DWORD startTime = GetTickCount();
	double tunnelPhase = 0.0;

	while (GetTickCount() - startTime < 30000) {
		tunnelPhase += 0.05;
		DrawTunnel(hdc, w, h, tunnelPhase);
		for (int y = 0; y < h; y += 2) {
			int brightness = (int)(128 + 127 * sin(y * 0.05 + tunnelPhase));
			HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, brightness, 0));
			HPEN oldPen = (HPEN)SelectObject(hdc, pen);
			MoveToEx(hdc, 0, y, NULL);
			LineTo(hdc, w, y);
			SelectObject(hdc, oldPen);
			DeleteObject(pen);
		}
		Sleep(50);
	}
	ReleaseDC(0, hdc);
	return 0;
}

DWORD WINAPI NewStage9(LPVOID lpvd) {
	HDC hdc = GetDC(0);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	DWORD startTime = GetTickCount();
	int cx = w / 2, cy = h / 2;
	double radius = 0;
	double angle = 0;
	while (GetTickCount() - startTime < 30000) {
		HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
		RECT rect = { 0, 0, w, h };
		FillRect(hdc, &rect, blackBrush);
		DeleteObject(blackBrush);
		radius += 2.0;
		angle += 0.05;
		if (radius > sqrt(cx * cx + cy * cy)) radius = 0;
		for (int i = 0; i < 50; ++i) {
			double a = angle + i * 6.2832 / 50;
			double r = radius * (1.0 + 0.3 * sin(i * 1.5 + angle * 2));
			int x = cx + (int)(r * cos(a));
			int y = cy + (int)(r * sin(a));
			HPEN pen = CreatePen(PS_SOLID, 2, RandGreenToBlack());
			HPEN oldPen = (HPEN)SelectObject(hdc, pen);
			Ellipse(hdc, x - 5, y - 5, x + 5, y + 5);
			SelectObject(hdc, oldPen);
			DeleteObject(pen);
		}
		Sleep(30);
	}
	ReleaseDC(0, hdc);
	return 0;
}

DWORD WINAPI NewStage10(LPVOID lpvd) {
	HDC hdc = GetDC(0);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	DWORD startTime = GetTickCount();
	int x = 0, y = 0;
	int dx = 5, dy = 3;
	int size = 100;
	while (GetTickCount() - startTime < 30000) {
		HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
		RECT rect = { 0, 0, w, h };
		FillRect(hdc, &rect, blackBrush);
		DeleteObject(blackBrush);
		x += dx;
		y += dy;
		if (x + size > w || x < 0) dx = -dx;
		if (y + size > h || y < 0) dy = -dy;
		HPEN pen = CreatePen(PS_SOLID, 3, RandGreenToBlack());
		HPEN oldPen = (HPEN)SelectObject(hdc, pen);
		HBRUSH brush = CreateSolidBrush(RandGreenToBlack());
		HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
		Rectangle(hdc, x, y, x + size, y + size);
		SelectObject(hdc, oldBrush);
		DeleteObject(brush);
		SelectObject(hdc, oldPen);
		DeleteObject(pen);
		Sleep(16);
	}
	ReleaseDC(0, hdc);
	return 0;
}

DWORD WINAPI NewStage11(LPVOID lpvd) {
	HDC hdc = GetDC(0);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	DWORD startTime = GetTickCount();
	int cx = w / 2, cy = h / 2;
	int maxRadius = (int)sqrt(cx * cx + cy * cy);
	for (int t = 0; GetTickCount() - startTime < 30000; ++t) {
		HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
		RECT rect = { 0, 0, w, h };
		FillRect(hdc, &rect, blackBrush);
		DeleteObject(blackBrush);
		for (int r = 0; r < maxRadius; r += 5) {
			double phase = (double)t / 10.0 + (double)r / 50.0;
			int brightness = (int)(255 * (1.0 - (double)r / maxRadius));
			HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, brightness, 0));
			HPEN oldPen = (HPEN)SelectObject(hdc, pen);
			for (int i = 0; i < 20; ++i) {
				double a = phase + i * 6.2832 / 20;
				int x1 = cx + (int)(r * cos(a));
				int y1 = cy + (int)(r * sin(a));
				int x2 = cx + (int)((r + 3) * cos(a + 0.05));
				int y2 = cy + (int)((r + 3) * sin(a + 0.05));
				MoveToEx(hdc, x1, y1, NULL);
				LineTo(hdc, x2, y2);
			}
			SelectObject(hdc, oldPen);
			DeleteObject(pen);
		}
		Sleep(30);
	}
	ReleaseDC(0, hdc);
	return 0;
}
DWORD WINAPI NewStage12(LPVOID lpvd) {
	HDC hdc = GetDC(0);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	DWORD startTime = GetTickCount();

	const int NUM_PARTICLES = 500;
	struct Particle {
		double x, y;
		double vx, vy;
		double radius;
		double angle;
		double speed;
	};
	Particle* particles = (Particle*)malloc(NUM_PARTICLES * sizeof(Particle));

	for (int i = 0; i < NUM_PARTICLES; i++) {
		particles[i].angle = (rand() % 360) * 3.14159 / 180.0;
		particles[i].radius = (double)(rand() % 300 + 50);
		particles[i].speed = 0.5 + (rand() % 100) / 200.0;
		particles[i].x = w / 2.0 + particles[i].radius * cos(particles[i].angle);
		particles[i].y = h / 2.0 + particles[i].radius * sin(particles[i].angle);
		particles[i].vx = -particles[i].radius * sin(particles[i].angle) * 0.01;
		particles[i].vy = particles[i].radius * cos(particles[i].angle) * 0.01;
	}

	double time = 0;
	while (GetTickCount() - startTime < 30000) {
		HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
		RECT rect = { 0, 0, w, h };
		FillRect(hdc, &rect, blackBrush);
		DeleteObject(blackBrush);

		time += 0.02;
		double cx = w / 2.0 + 100 * sin(time * 0.3);
		double cy = h / 2.0 + 80 * cos(time * 0.4);

		for (int i = 0; i < NUM_PARTICLES; i++) {
			Particle* p = &particles[i];
			p->radius += p->speed * 0.3;
			p->angle += 0.02 + p->speed * 0.005;

			double spiral = p->radius * 0.01;
			p->x = cx + p->radius * cos(p->angle + spiral);
			p->y = cy + p->radius * sin(p->angle + spiral);

			double dx = cx - p->x;
			double dy = cy - p->y;
			double dist = sqrt(dx * dx + dy * dy);
			if (dist > 5) {
				p->vx += dx / (dist * 50);
				p->vy += dy / (dist * 50);
				p->vx *= 0.99;
				p->vy *= 0.99;
			}

			p->x += p->vx;
			p->y += p->vy;

			if (p->radius > 500) p->radius = 10;
			if (p->radius < 10) p->radius = 500;

			int brightness = (int)(255 * (1.0 - p->radius / 600.0));
			int size = 1 + (int)(3 * (1.0 - p->radius / 600.0));

			HPEN pen = CreatePen(PS_SOLID, size, RGB(0, brightness, brightness / 4));
			HPEN oldPen = (HPEN)SelectObject(hdc, pen);
			HBRUSH brush = CreateSolidBrush(RGB(0, brightness, brightness / 4));
			HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);

			Ellipse(hdc, (int)p->x - size, (int)p->y - size, (int)p->x + size, (int)p->y + size);

			SelectObject(hdc, oldBrush);
			DeleteObject(brush);
			SelectObject(hdc, oldPen);
			DeleteObject(pen);
		}

		for (int r = 0; r < 50; r += 2) {
			int alpha = 255 - r * 5;
			if (alpha < 0) alpha = 0;
			HPEN glowPen = CreatePen(PS_SOLID, 1, RGB(0, alpha, 0));
			HPEN oldGlow = (HPEN)SelectObject(hdc, glowPen);
			HBRUSH glowBrush = CreateSolidBrush(RGB(0, alpha, 0));
			HBRUSH oldGlowBrush = (HBRUSH)SelectObject(hdc, glowBrush);
			Ellipse(hdc, (int)cx - r, (int)cy - r, (int)cx + r, (int)cy + r);
			SelectObject(hdc, oldGlowBrush);
			DeleteObject(glowBrush);
			SelectObject(hdc, oldGlow);
			DeleteObject(glowPen);
		}

		Sleep(16);
	}
	free(particles);
	ReleaseDC(0, hdc);
	return 0;
}

DWORD WINAPI NewStage13(LPVOID lpvd) {
	HDC hdc = GetDC(0);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	DWORD startTime = GetTickCount();

	struct Point3D { double x, y, z; };
	Point3D cube[8] = {
		{-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},
		{-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1}
	};
	int edges[12][2] = {
		{0,1},{1,2},{2,3},{3,0},
		{4,5},{5,6},{6,7},{7,4},
		{0,4},{1,5},{2,6},{3,7}
	};

	double angleX = 0, angleY = 0, angleZ = 0;
	double scale = 150;

	while (GetTickCount() - startTime < 30000) {
		HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
		RECT rect = { 0, 0, w, h };
		FillRect(hdc, &rect, blackBrush);
		DeleteObject(blackBrush);

		angleX += 0.015;
		angleY += 0.02;
		angleZ += 0.01;

		Point3D transformed[8];
		for (int i = 0; i < 8; i++) {
			double x = cube[i].x, y = cube[i].y, z = cube[i].z;

			double y1 = y * cos(angleX) - z * sin(angleX);
			double z1 = y * sin(angleX) + z * cos(angleX);
			y = y1; z = z1;

			double x1 = x * cos(angleY) + z * sin(angleY);
			z1 = -x * sin(angleY) + z * cos(angleY);
			x = x1; z = z1;

			x1 = x * cos(angleZ) - y * sin(angleZ);
			y1 = x * sin(angleZ) + y * cos(angleZ);
			x = x1; y = y1;

			transformed[i] = { x, y, z };
		}

		for (int i = 0; i < 12; i++) {
			int idx1 = edges[i][0];
			int idx2 = edges[i][1];

			int x1 = w / 2 + (int)(transformed[idx1].x * scale);
			int y1 = h / 2 + (int)(transformed[idx1].y * scale);
			int x2 = w / 2 + (int)(transformed[idx2].x * scale);
			int y2 = h / 2 + (int)(transformed[idx2].y * scale);

			double depth = (transformed[idx1].z + transformed[idx2].z) / 2;
			int brightness = (int)(128 + 127 * (depth / 2.0 + 0.5));
			if (brightness > 255) brightness = 255;
			if (brightness < 0) brightness = 0;

			HPEN pen = CreatePen(PS_SOLID, 2, RGB(0, brightness, 0));
			HPEN oldPen = (HPEN)SelectObject(hdc, pen);
			MoveToEx(hdc, x1, y1, NULL);
			LineTo(hdc, x2, y2);
			SelectObject(hdc, oldPen);
			DeleteObject(pen);
		}

		for (int i = 0; i < 8; i++) {
			int x = w / 2 + (int)(transformed[i].x * scale);
			int y = h / 2 + (int)(transformed[i].y * scale);
			int brightness = (int)(128 + 127 * (transformed[i].z / 2.0 + 0.5));

			HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, brightness, 0));
			HPEN oldPen = (HPEN)SelectObject(hdc, pen);
			HBRUSH brush = CreateSolidBrush(RGB(0, brightness, 0));
			HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
			Ellipse(hdc, x - 3, y - 3, x + 3, y + 3);
			SelectObject(hdc, oldBrush);
			DeleteObject(brush);
			SelectObject(hdc, oldPen);
			DeleteObject(pen);
		}

		Sleep(16);
	}
	ReleaseDC(0, hdc);
	return 0;
}

DWORD WINAPI NewStage14(LPVOID lpvd) {
	HDC hdc = GetDC(0);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	DWORD startTime = GetTickCount();

	const int W = 200, H = 200;
	int* fire = (int*)malloc(W * H * sizeof(int));
	memset(fire, 0, W * H * sizeof(int));

	while (GetTickCount() - startTime < 30000) {
		HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
		RECT rect = { 0, 0, w, h };
		FillRect(hdc, &rect, blackBrush);
		DeleteObject(blackBrush);

		for (int x = 0; x < W; x++) {
			fire[(H - 1) * W + x] = rand() % 200 + 55;
		}

		for (int y = H - 2; y >= 0; y--) {
			for (int x = 0; x < W; x++) {
				int sum = 0;
				int count = 0;
				for (int dy = 1; dy <= 3; dy++) {
					for (int dx = -1; dx <= 1; dx++) {
						int nx = x + dx;
						int ny = y + dy;
						if (nx >= 0 && nx < W && ny >= 0 && ny < H) {
							sum += fire[ny * W + nx];
							count++;
						}
					}
				}
				if (count > 0) {
					fire[y * W + x] = sum / count - 2;
					if (fire[y * W + x] < 0) fire[y * W + x] = 0;
					if (fire[y * W + x] > 255) fire[y * W + x] = 255;
				}
			}
		}

		int scaleX = w / W;
		int scaleY = h / H;
		if (scaleX < 1) scaleX = 1;
		if (scaleY < 1) scaleY = 1;

		for (int y = 0; y < H; y++) {
			for (int x = 0; x < W; x++) {
				int val = fire[y * W + x];
				if (val > 10) {
					int green = val / 2;
					if (green > 255) green = 255;
					COLORREF color = RGB(0, green, 0);
					for (int dy = 0; dy < scaleY; dy++) {
						for (int dx = 0; dx < scaleX; dx++) {
							int px = x * scaleX + dx + (w - W * scaleX) / 2;
							int py = y * scaleY + dy + (h - H * scaleY) / 2;
							if (px >= 0 && px < w && py >= 0 && py < h) {
								SetPixel(hdc, px, py, color);
							}
						}
					}
				}
			}
		}

		Sleep(30);
	}
	free(fire);
	ReleaseDC(0, hdc);
	return 0;
}
DWORD WINAPI NewStage15(LPVOID lpvd) {
	HDC hdc = GetDC(0);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	DWORD startTime = GetTickCount();

	const int COLS = 60;
	int* drops = (int*)malloc(COLS * sizeof(int));
	int* speeds = (int*)malloc(COLS * sizeof(int));
	int* lengths = (int*)malloc(COLS * sizeof(int));

	for (int i = 0; i < COLS; i++) {
		drops[i] = rand() % h;
		speeds[i] = rand() % 3 + 2;
		lengths[i] = rand() % 15 + 5;
	}

	HFONT hFont = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, FIXED_PITCH, "Consolas");
	HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
	SetBkMode(hdc, TRANSPARENT);

	double phase = 0;
	while (GetTickCount() - startTime < 30000) {
		phase += 0.02;
		HBRUSH fadeBrush = CreateSolidBrush(RGB(0, 0, 0));
		RECT rect = { 0, 0, w, h };
		FillRect(hdc, &rect, fadeBrush);
		DeleteObject(fadeBrush);

		for (int i = 0; i < COLS; i++) {
			int x = (i * w) / COLS;
			int y = drops[i];
			int len = lengths[i];

			for (int j = 0; j < len; j++) {
				int posY = y - j * 15;
				if (posY < 0 || posY > h) continue;

				float brightness = 1.0f - (float)j / len;
				int green = (int)(255 * brightness);

				char ch = 33 + rand() % 94;
				if (j == 0) {
					SetTextColor(hdc, RGB(0, 255, 128));
				}
				else if (j < 3) {
					SetTextColor(hdc, RGB(0, 200, 0));
				}
				else {
					SetTextColor(hdc, RGB(0, green, 0));
				}

				char str[2] = { ch, 0 };
				TextOutA(hdc, x, posY, str, 1);
			}

			drops[i] += speeds[i];
			if (drops[i] > h + 50) {
				drops[i] = -rand() % 100;
				speeds[i] = rand() % 3 + 2;
				lengths[i] = rand() % 15 + 5;
			}
		}

		Sleep(30);
	}

	SelectObject(hdc, oldFont);
	DeleteObject(hFont);
	free(drops);
	free(speeds);
	free(lengths);
	ReleaseDC(0, hdc);
	return 0;
}


VOID WINAPI RunThread(LPTHREAD_START_ROUTINE ThreadName, INT RunTime) {
	Sleep(50);
	HANDLE hT = CreateThread(0, 0, ThreadName, 0, 0, 0);
	Sleep(RunTime);
	TerminateThread(hT, 0);

	SendMessageA(FindWindowA("SysListView32", NULL), WM_ERASEBKGND, 0, 0);
	RedrawWindow(NULL, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
	Sleep(50);
}
VOID WINAPI RunThreadDouble(LPTHREAD_START_ROUTINE ThreadName1, LPTHREAD_START_ROUTINE ThreadName2, INT RunTime) {
	Sleep(50);
	HANDLE hT1 = CreateThread(0, 0, ThreadName1, 0, 0, 0);
	HANDLE hT2 = CreateThread(0, 0, ThreadName2, 0, 0, 0);
	Sleep(RunTime);
	TerminateThread(hT1, 0);
	TerminateThread(hT2, 0);

	SendMessageA(FindWindowA("SysListView32", NULL), WM_ERASEBKGND, 0, 0);
	RedrawWindow(NULL, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
	Sleep(50);
}
VOID WINAPI RunThreadThree(LPTHREAD_START_ROUTINE ThreadName1, LPTHREAD_START_ROUTINE ThreadName2, LPTHREAD_START_ROUTINE ThreadName3, INT RunTime) {
	Sleep(50);
	HANDLE hT1 = CreateThread(0, 0, ThreadName1, 0, 0, 0);
	HANDLE hT2 = CreateThread(0, 0, ThreadName2, 0, 0, 0);
	HANDLE hT3 = CreateThread(0, 0, ThreadName3, 0, 0, 0);
	Sleep(RunTime);
	TerminateThread(hT1, 0);
	TerminateThread(hT2, 0);
	TerminateThread(hT3, 0);

	SendMessageA(FindWindowA("SysListView32", NULL), WM_ERASEBKGND, 0, 0);
	RedrawWindow(NULL, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
	Sleep(50);
}
VOID WINAPI RunThreadNoSleep(LPTHREAD_START_ROUTINE ThreadName) {
	CreateThread(0, 0, ThreadName, 0, 0, 0);
}

DWORD WINAPI payload1a(LPVOID lpvd) {
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	HDC hdc = GetDC(0);
	FLOAT angle = 0.01;
	for (int i = 0;; i++) {
		FLOAT a = sin(angle) * (360 / (3.14 + i) - i) / angle;
		StretchBlt(hdc, -20, 20, w + 30, h - 30, hdc, a, 0, w, h, SRCCOPY);
		angle += 0.01;
	}
}
DWORD WINAPI payload1b(LPVOID lpvd) {
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	HDC hdc = GetDC(0);
	for (int i = 0;; i++) {
		SelectObject(hdc, CreateSolidBrush(RandGreenToBlack()));
		BitBlt(hdc, 0, 0, w, h, hdc, 0, 0, PATINVERT);
		Sleep(100);
	}
}
DWORD WINAPI shader1(LPVOID lpParam) {
	HDC hdcScreen = GetDC(0), hdcMem = CreateCompatibleDC(hdcScreen);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	BITMAPINFO bmi = { 0 };
	RGBQ rgbScreen = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFO);
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biWidth = w;
	bmi.bmiHeader.biHeight = h;
	HBITMAP hbmTemp = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, (void**)&rgbScreen, NULL, NULL);
	SelectObject(hdcMem, hbmTemp);
	for (;;) {
		hdcScreen = GetDC(0);
		BitBlt(hdcMem, 0, 0, w, h, hdcScreen, 0, 0, SRCCOPY);
		for (INT i = 0; i < w * h; i++) {
			INT x = i % w, y = i / w;
			BYTE r = rgbScreen[i].r;
			BYTE g = rgbScreen[i].g;
			BYTE b = rgbScreen[i].b;
			rgbScreen[i].rgb += (((r * i) + ((r + g) * i) + ((r + g + b) * i)) + ((x + y) ^ (w * h))) % 255;
		}
		BitBlt(hdcScreen, 0, 0, w, h, hdcMem, 0, 0, SRCCOPY);
		ReleaseDC(NULL, hdcScreen); DeleteDC(hdcScreen);
	}
	Sleep(50);
}
DWORD WINAPI payload2a(LPVOID lpParam) {
	HDC hdc = GetDC(HWND_DESKTOP);
	int X = GetSystemMetrics(SM_CXSCREEN);
	int Y = GetSystemMetrics(SM_CYSCREEN);

	while (TRUE)
	{
		HDC hdc = GetDC(HWND_DESKTOP);
		int sw = GetSystemMetrics(SM_CXSCREEN);
		int sh = GetSystemMetrics(SM_CYSCREEN);
		BitBlt(hdc, rand() % 10, rand() % 10, sw, sh, hdc, rand() % 10, rand() % 10, SRCINVERT);
		ReleaseDC(0, hdc);
		if ((rand() % 100 + 1) % 67 == 0) RedrawWindow(NULL, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
		Sleep(100);
	}
}
DWORD WINAPI shader2b(LPVOID lpParam)
{
	HDC hdc = GetDC(NULL);
	HDC hdcCopy = CreateCompatibleDC(hdc);
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	BITMAPINFO bmpi = { 0 };
	HBITMAP bmp;

	bmpi.bmiHeader.biSize = sizeof(bmpi);
	bmpi.bmiHeader.biWidth = screenWidth;
	bmpi.bmiHeader.biHeight = screenHeight;
	bmpi.bmiHeader.biPlanes = 1;
	bmpi.bmiHeader.biBitCount = 32;
	bmpi.bmiHeader.biCompression = BI_RGB;

	RGBQUAD* rgbquad = NULL;
	HSL hslcolor;

	bmp = CreateDIBSection(hdc, &bmpi, DIB_RGB_COLORS, (void**)&rgbquad, NULL, 0);
	SelectObject(hdcCopy, bmp);

	INT i = 0;

	while (1)
	{
		hdc = GetDC(NULL);
		StretchBlt(hdcCopy, 0, 0, screenWidth, screenHeight, hdc, 0, 0, screenWidth, screenHeight, SRCCOPY);

		RGBQUAD rgbquadCopy;

		for (int x = 0; x < screenWidth; x++)
		{
			for (int y = 0; y < screenHeight; y++)
			{
				int index = y * screenWidth + x;

				int fx = (int)((i ^ 4) + (i * 4) * pow(x & y, 1.0 / 3.0));

				rgbquadCopy = rgbquad[index];

				hslcolor = Colors::rgb2hsl(rgbquadCopy);
				hslcolor.h = 0.33f;
				hslcolor.s = 0.8f;
				hslcolor.l = fmod(fx / 400.f + y / screenHeight * .2f, 1.f);

				rgbquad[index] = Colors::hsl2rgb(hslcolor);
			}
		}

		i++;
		SelectObject(hdc, CreateSolidBrush(RandGreenToBlack()));
		StretchBlt(hdc, 0, 0, screenWidth, screenHeight, hdcCopy, 0, 0, screenWidth, screenHeight, PATINVERT);
		StretchBlt(hdc, 0, 0, screenWidth, screenHeight, hdcCopy, 0, 0, screenWidth, screenHeight, SRCCOPY);
		ReleaseDC(NULL, hdc);
		DeleteDC(hdc);
		Sleep(25);
	}

	return 0;
}
DWORD WINAPI payload3(LPVOID lpParam) {
	HDC hdcScreen = GetDC(NULL);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);

	BITMAPINFO bmi = { 0 };
	RGBQ prgbScreen = { 0 };
	HDC hdcTempScreen;
	HBITMAP hbmScreen;

	bmi.bmiHeader.biSize = sizeof(BITMAPINFO);
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biWidth = w;
	bmi.bmiHeader.biHeight = h;
	hdcTempScreen = CreateCompatibleDC(hdcScreen);
	hbmScreen = CreateDIBSection(hdcScreen, &bmi, 0, (void**)&prgbScreen, NULL, 0);
	SelectObject(hdcTempScreen, hbmScreen);

	for (int i = 0;; i++) {
		hdcScreen = GetDC(NULL);
		BitBlt(hdcTempScreen, 0, 0, w, h, hdcScreen, 0, 0, SRCCOPY);
		RGBQ prgbTemp = { 0 };
		prgbTemp = prgbScreen;
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				prgbScreen[i * w + j].rgb = prgbTemp[(unsigned int)((float)(j * w + i) + (float)cbrt((unsigned int)(2 * h * j - j * j))) % (w * h)].rgb;
			}
		}
		Sleep(100);
		BitBlt(hdcScreen, 0, 0, w, h, hdcTempScreen, 0, 0, SRCCOPY);
		ReleaseDC(NULL, hdcScreen);
		DeleteObject(hdcScreen);
		Sleep(10);
	}
}
DWORD WINAPI shader3b(LPVOID lpParam) {
	HDC hdcScreen = GetDC(NULL);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);

	BITMAPINFO bmi = { 0 };
	RGBQ prgbScreen = { 0 };
	HDC hdcTempScreen;
	HBITMAP hbmScreen;

	bmi.bmiHeader.biSize = sizeof(BITMAPINFO);
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biWidth = w;
	bmi.bmiHeader.biHeight = h;
	hdcTempScreen = CreateCompatibleDC(hdcScreen);
	hbmScreen = CreateDIBSection(hdcScreen, &bmi, 0, (void**)&prgbScreen, NULL, 0);
	SelectObject(hdcTempScreen, hbmScreen);

	for (int t = 0;; t++) {
		hdcScreen = GetDC(NULL);
		BitBlt(hdcTempScreen, 0, 0, w, h, hdcScreen, 0, 0, SRCCOPY);
		for (int i = 0; i < w * h; i++) {
			int r = prgbScreen[i].r;
			int g = prgbScreen[i].g;
			int b = prgbScreen[i].b;
			int brightness = ((r + g + b) / 3 + t * 2) % 256;
			prgbScreen[i].rgb = RGB(0, brightness, 0);
		}
		BitBlt(hdcScreen, 0, 0, w, h, hdcTempScreen, 0, 0, SRCCOPY);
		ReleaseDC(NULL, hdcScreen);
		DeleteObject(hdcScreen);
		Sleep(10);
	}
}
DWORD WINAPI payload5(LPVOID lpParam) {
	for (;;) {
		int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
		POINT p[3];
		HDC hdc = GetDC(0);
		p[0] = { 0,0 };
		p[1] = { w,0 };
		p[2] = { 25,h };
		PlgBlt(hdc, p, hdc, 0, 0, w, h, NULL, 0, 0);
		SelectObject(hdc, CreateSolidBrush(RandGreenToBlack()));
		switch (rand() % 3) {
		case 0:
			Pie(hdc, rand() % w, rand() % h, rand() % w, rand() % h, rand() % w, rand() % h, rand() % w, rand() % h);
			break;
		case 1:
			Ellipse(hdc, rand() % w, rand() % h, rand() % w, rand() % h);
			break;
		case 2:
			Rectangle(hdc, rand() % w, rand() % h, rand() % w, rand() % h);
			break;
		}
		Sleep(10);
		ReleaseDC(0, hdc);
		Sleep(10);
	}
}
DWORD WINAPI payload6a(LPVOID lparam) {
	int a = 2;
	for (;;) {
		HDC hdc = GetDC(0);
		int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
		POINT pos[3];
		pos[0].x = 0, pos[0].y = 0;
		pos[1].x = cos(a * (3.14 / 180)) * w, pos[1].y = sin(a * (3.14 / 180)) * h;
		pos[2].x = (sin(a * (3.14 / 180)) * w), pos[2].y = cos(a * (3.14 / 180)) * h;
		PlgBlt(hdc, pos, hdc, 0, 0, w, h, 0, 0, 0);
		ReleaseDC(0, hdc);
		Sleep(200);
	}
}
DWORD WINAPI payload6b(LPVOID lparam) {
	HWND hwnd = GetDesktopWindow();
	HDC hdc = GetWindowDC(hwnd);
	RECT rect;
	GetWindowRect(hwnd, &rect);
	while (1) {
		int rands = rand() % 10 - 5;
		StretchBlt(hdc, rect.left + 10 + rands, rect.top + 10 + rands, rect.right - 20, rect.bottom - 20, hdc, rect.left, rect.top, rect.right, rect.bottom, SRCPAINT);
		Sleep(100);
	}
}
DWORD WINAPI payload7a(LPVOID lpParam) {
	HDC hdc = GetDC(0);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);

	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFO);
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biWidth = w;
	bmi.bmiHeader.biHeight = h;

	RGBQ prgbScreen;
	HDC hcdc = CreateCompatibleDC(hdc);
	HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, 0, (void**)&prgbScreen, NULL, 0);
	SelectObject(hcdc, hBitmap);

	for (; ; ) {
		hdc = GetDC(NULL);
		BitBlt(hcdc, 0, 0, w, h, hdc, 0, 0, 13369376);
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				int rgb = (prgbScreen[x + w * y].r + prgbScreen[x + w * y].g + prgbScreen[x + w * y].b) / 3;
				prgbScreen[x + w * y].r = rgb;
				prgbScreen[x + w * y].b = rgb;
			}
		}
		BitBlt(hdc, 0, 0, w, h, hcdc, 0, 0, 13369376);
		ReleaseDC(NULL, hdc);
		DeleteObject(hdc);
		Sleep(100);
	}

	ReleaseDC(NULL, hcdc);
	DeleteObject(hcdc);
	DeleteObject(hBitmap);
	return 0;
}
DWORD WINAPI payload7b(LPVOID lparam) {
	HWND hwnd = GetDesktopWindow();
	HDC hdc = GetWindowDC(hwnd);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	while (1) {
		StretchBlt(hdc, 10, 10, w - 20, h + 20, hdc, 0, 0, w, h, SRCCOPY);
		Sleep(100);
	}
}
DWORD WINAPI payload8a(LPVOID lparam) {
	HWND hwnd = GetDesktopWindow();
	HDC hdc = GetWindowDC(hwnd);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	while (1) {
		StretchBlt(hdc, -10, +10, w + 20, h - 20, hdc, 0, 0, w, h, SRCAND);
		Sleep(100);
	}
}
DWORD WINAPI shader4b(LPVOID lparam) {
	HWND hwnd = GetDesktopWindow();
	HDC hdc = GetWindowDC(hwnd);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	while (1) {
		StretchBlt(hdc, 0, 0, w, h, hdc, 0, 0, w, h, NOTSRCCOPY);
		Sleep(1000);
		SendMessageA(FindWindowA("SysListView32", NULL), WM_ERASEBKGND, 0, 0);
		RedrawWindow(NULL, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
	}
}
VOID WINAPI ci(int x, int y, int w, int h)
{
	HDC hdc = GetDC(0);
	HRGN hrgn = CreateRectRgn(x, y, w + x, h + y);
	SelectClipRgn(hdc, hrgn);
	SelectObject(hdc, CreateSolidBrush(RandGreenToBlack()));
	BitBlt(hdc, x, y, w, h, hdc, x, y, PATINVERT);
	DeleteObject(hrgn);
	ReleaseDC(NULL, hdc);
}
DWORD WINAPI payload8c(LPVOID lpParam) {
	RECT rect;
	GetWindowRect(GetDesktopWindow(), &rect);
	int w = rect.right - rect.left - 500, h = rect.bottom - rect.top - 500;
	for (int t = 0;; t++)
	{
		const int size = 4000;
		int x = 0, y = 0;

		for (int i = 0; i < size; i += 70)
		{
			ci(x - i / 2, y - i / 2, i, i);
			Sleep(25);
		}
		Sleep(100);
		SendMessageA(FindWindowA("SysListView32", NULL), WM_ERASEBKGND, 0, 0);
		RedrawWindow(NULL, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
	}
}
DWORD WINAPI ball(LPVOID lpParam) {
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	int signX = 1;
	int signY = 1;
	int incrementor = 8;
	int x = 50;
	int y = 50;
	int radius = 60;
	DWORD startTime = GetTickCount();
	int colorState = 0;
	int direction = 1;
	int frameCount = 0;

	for (;;) {
		HDC hdc = GetDC(0);
		int top_x = x;
		int top_y = y;
		int bottom_x = radius * 2 + x;
		int bottom_y = radius * 2 + y;
		x += incrementor * signX;
		y += incrementor * signY;

		frameCount++;
		if (frameCount % 5 == 0) {
			colorState += direction * 5;
			if (colorState >= 255) {
				colorState = 255;
				direction = -1;
			}
			else if (colorState <= 0) {
				colorState = 0;
				direction = 1;
			}
		}

		int green = colorState;
		HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, green, 0));
		HBRUSH brush = CreateSolidBrush(RGB(0, green, 0));
		HPEN oldPen = (HPEN)SelectObject(hdc, pen);
		HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);

		int cx = x + radius;
		int cy = y + radius;
		Ellipse(hdc, cx - radius, cy - radius, cx + radius, cy + radius);

		SelectObject(hdc, oldBrush);
		DeleteObject(brush);
		SelectObject(hdc, oldPen);
		DeleteObject(pen);

		ReleaseDC(NULL, hdc);

		if (y + radius * 2 >= GetSystemMetrics(SM_CYSCREEN)) {
			signY = -1;
		}
		if (x + radius * 2 >= GetSystemMetrics(SM_CXSCREEN)) {
			signX = -1;
		}
		if (y <= 0) {
			signY = 1;
		}
		if (x <= 0) {
			signX = 1;
		}
		Sleep(16);
	}
}
DWORD WINAPI shader5(LPVOID lpvd) {
	HDC hdcScreen = GetDC(0), hdcMem = CreateCompatibleDC(hdcScreen);
	INT w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	BITMAPINFO bmi = { 0 };
	RGBQ rgbScreen = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFO);
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biWidth = w;
	bmi.bmiHeader.biHeight = h;
	HBITMAP hbmTemp = CreateDIBSection(hdcScreen, &bmi, NULL, (void**)&rgbScreen, NULL, NULL);
	SelectObject(hdcMem, hbmTemp);
	for (;;) {
		hdcScreen = GetDC(0);
		BitBlt(hdcMem, 0, 0, w, h, hdcScreen, 0, 0, SRCCOPY);
		for (INT i = 0; i < w * h; i++) {
			rgbScreen[i].r = 0;
			rgbScreen[i].g = rand() % 256;
			rgbScreen[i].b = 0;
		}
		BLENDFUNCTION blf = { 0 };
		blf.BlendOp = AC_SRC_OVER;
		blf.BlendFlags = 0;
		blf.SourceConstantAlpha = 78;
		blf.AlphaFormat = 0;
		AlphaBlend(hdcScreen, 0, 0, w, h, hdcMem, 0, 0, w, h, blf);
		ReleaseDC(NULL, hdcScreen); DeleteDC(hdcScreen);
	}
}
DWORD WINAPI shader6(LPVOID lpvd) {
	HDC hdcScreen = GetDC(0), hdcMem = CreateCompatibleDC(hdcScreen);
	INT w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	BITMAPINFO bmi = { 0 };
	RGBQ rgbScreen = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFO);
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biWidth = w;
	bmi.bmiHeader.biHeight = h;
	HBITMAP hbmTemp = CreateDIBSection(hdcScreen, &bmi, NULL, (void**)&rgbScreen, NULL, NULL);
	SelectObject(hdcMem, hbmTemp);
	for (int t = 0;; t++) {
		hdcScreen = GetDC(0);
		RGBQ rgbTemp = { 0 };
		rgbTemp = rgbScreen;
		BitBlt(hdcMem, 0, 0, w, h, hdcScreen, 0, 0, SRCCOPY);
		t *= 50;
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				rgbScreen[i * w + j].rgb = rgbTemp[(unsigned int)((float)(j * w + i) + (float)log((unsigned int)(2 * h * j - j * j))) % (w * h)].rgb;
				rgbScreen[i * w + j].g = rgbScreen[i * w + j].r;
				rgbScreen[i * w + j].r = 0;
				rgbScreen[i * w + j].b = 0;
			}
		}
		BitBlt(hdcScreen, 0, 0, w, h, hdcMem, 0, 0, SRCCOPY);
		ReleaseDC(NULL, hdcScreen); DeleteDC(hdcScreen);
		Sleep(50);
	}
}
DWORD WINAPI shader7(LPVOID lpvd) {
	HDC hdcScreen = GetDC(0), hdcMem = CreateCompatibleDC(hdcScreen);
	INT w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	BITMAPINFO bmi = { 0 };
	RGBQ rgbScreen = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFO);
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biWidth = w;
	bmi.bmiHeader.biHeight = h;
	HBITMAP hbmTemp = CreateDIBSection(hdcScreen, &bmi, NULL, (void**)&rgbScreen, NULL, NULL);
	SelectObject(hdcMem, hbmTemp);
	for (int t = 0;; t++) {
		hdcScreen = GetDC(0);
		RGBQ rgbTemp = { 0 };
		rgbTemp = rgbScreen;
		BitBlt(hdcMem, 0, 0, w, h, hdcScreen, 0, 0, SRCCOPY);
		t *= 50;
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				rgbScreen[i * w + j].rgb = rgbTemp[(unsigned int)((float)(j * w + i) - (float)sqrt((unsigned int)(2 * h * j - j * j))) % (w * h)].rgb;
				rgbScreen[i * w + j].g = rgbScreen[i * w + j].r;
				rgbScreen[i * w + j].r = 0;
				rgbScreen[i * w + j].b = 0;
			}
		}
		BitBlt(hdcScreen, 0, 0, w, h, hdcMem, 0, 0, SRCCOPY);
		ReleaseDC(NULL, hdcScreen); DeleteDC(hdcScreen);
		Sleep(50);
	}
}
DWORD WINAPI pay9(LPVOID lpParam) {
	FLOAT angle = 0;
	for (int t = 0;; t++) {
		int w = GetSystemMetrics(SM_CXSCREEN), h = GetSystemMetrics(SM_CYSCREEN);
		int a = (-1) * (sin(2 * (3.1415926 / 180)) * h);
		HDC hdc = GetDC(NULL);
		HDC hcdc = CreateCompatibleDC(hdc);
		HBITMAP hBitmap = CreateCompatibleBitmap(hdc, w, h);
		SelectObject(hcdc, hBitmap);
		BitBlt(hcdc, 0, 0, w / 20, h + angle, hdc, w / 20 * 19 + angle, 0, SRCCOPY);
		BitBlt(hcdc, w / 20, 0, w / 20 * 19 + angle, h + angle, hdc, 0, 0, SRCCOPY);
		BitBlt(hdc, 0, 0, w + angle, h + angle, hcdc, 0, 0, SRCINVERT);
		ReleaseDC(NULL, hdc);
		ReleaseDC(NULL, hcdc);
		DeleteObject(hdc);
		DeleteObject(hcdc);
		DeleteObject(hBitmap);
		Sleep(10);
		angle++;
	}
}
DWORD WINAPI shader8(LPVOID lpParam) {
	HDC hdc = GetDC(NULL);
	HDC hdcCopy = CreateCompatibleDC(hdc);
	int w = GetSystemMetrics(0);
	int h = GetSystemMetrics(1);
	BITMAPINFO bmpi = { 0 };
	HBITMAP bmp;
	bmpi.bmiHeader.biSize = sizeof(bmpi);
	bmpi.bmiHeader.biWidth = w;
	bmpi.bmiHeader.biHeight = h;
	bmpi.bmiHeader.biPlanes = 1;
	bmpi.bmiHeader.biBitCount = 32;
	bmpi.bmiHeader.biCompression = BI_RGB;
	RGBQUAD* rgbquad = NULL;
	HSL hslcolor;
	bmp = CreateDIBSection(hdc, &bmpi, DIB_RGB_COLORS, (void**)&rgbquad, NULL, 0);
	SelectObject(hdcCopy, bmp);
	INT i = 0;
	for (;;) {
		hdc = GetDC(NULL);
		StretchBlt(hdcCopy, 0, 0, w, h, hdc, 0, 0, w, h, SRCCOPY);
		RGBQUAD rgbquadCopy;
		for (int x = 0; x < w; x++)
		{
			for (int y = 0; y < h; y++)
			{
				int index = y * w + x;
				int fx = (int)((i ^ 4) + (i * 4) * pow(x + y & x * y & x ^ y, (1.0 / 3.0)));
				rgbquadCopy = rgbquad[index];
				hslcolor = Colors::rgb2hsl(rgbquadCopy);
				hslcolor.h = 0.33f;
				hslcolor.s = 0.8f;
				hslcolor.l = fmod(fx / 300.f + y / h * .1f + i / 1000.f, 1.f);
				rgbquad[index] = Colors::hsl2rgb(hslcolor);
			}
		}

		i++;
		StretchBlt(hdc, 0, 0, w, h, hdcCopy, 0, 0, w, h, SRCCOPY);
		ReleaseDC(NULL, hdc);
		DeleteDC(hdc);
		Sleep(20);
	}
	return 0;
}
DWORD WINAPI shader9(LPVOID lpvd) {
	HDC hdcScreen = GetDC(NULL);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);

	HDC hdcMem = CreateCompatibleDC(hdcScreen);
	HBITMAP hbmScreen = CreateCompatibleBitmap(hdcScreen, w, h);
	HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmScreen);

	HDC hdcMem2 = CreateCompatibleDC(hdcScreen);
	HBITMAP hbmScreen2 = CreateCompatibleBitmap(hdcScreen, w, h);
	HBITMAP hbmOld2 = (HBITMAP)SelectObject(hdcMem2, hbmScreen2);

	double time = 0;
	while (TRUE) {
		time += 0.01;
		hdcScreen = GetDC(NULL);

		BitBlt(hdcMem, 0, 0, w, h, hdcScreen, 0, 0, SRCCOPY);
		BitBlt(hdcMem2, 0, 0, w, h, hdcMem, 0, 0, SRCCOPY);

		for (int y = 0; y < h; y += 2) {
			for (int x = 0; x < w; x += 2) {
				double dx = sin((double)y * 0.05 + time * 3.0) * 15;
				double dy = cos((double)x * 0.05 + time * 2.0) * 15;
				int sx = (int)(x + dx);
				int sy = (int)(y + dy);

				if (sx >= 0 && sx < w && sy >= 0 && sy < h) {
					COLORREF color = GetPixel(hdcMem2, sx, sy);
					BYTE g = GetGValue(color);
					SetPixel(hdcMem, x, y, RGB(0, g, 0));
					SetPixel(hdcMem, x + 1, y, RGB(0, g, 0));
					SetPixel(hdcMem, x, y + 1, RGB(0, g, 0));
					SetPixel(hdcMem, x + 1, y + 1, RGB(0, g, 0));
				}
			}
		}

		BitBlt(hdcScreen, 0, 0, w, h, hdcMem, 0, 0, SRCCOPY);
		ReleaseDC(NULL, hdcScreen);
		Sleep(20);
	}

	SelectObject(hdcMem, hbmOld);
	SelectObject(hdcMem2, hbmOld2);
	DeleteObject(hbmScreen);
	DeleteObject(hbmScreen2);
	DeleteDC(hdcMem);
	DeleteDC(hdcMem2);
	return 0;
}

DWORD WINAPI pay10(LPVOID lpParam) {
	for (;;)
	{
		HDC hdc = GetDC(NULL);
		UINT rop[] = { SRCCOPY, SRCCOPY, SRCPAINT, SRCAND, SRCCOPY, SRCCOPY };
		int w = GetSystemMetrics(SM_CXSCREEN);
		int h = GetSystemMetrics(SM_CYSCREEN);
		HBITMAP hbm = CreateCompatibleBitmap(hdc, w, h);
		HDC hdcTemp = CreateCompatibleDC(hdc);
		HBITMAP hbmOld = (HBITMAP)SelectObject(hdcTemp, hbm);
		BitBlt(hdcTemp, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
		int numShifts = 600;
		for (int i = 0; i < numShifts; i++)
		{
			int x = rand() % w;
			int y = rand() % h;
			int dx = (rand() % 3) - 1;
			int dy = (rand() % 4) - 1;
			BitBlt(hdcTemp, x + dx, y + dy, w - x, h - y, hdcTemp, x, y, SRCCOPY);
		}
		BitBlt(hdc, 0, 0, w, h, hdcTemp, 0, 0, rop[rand() % 6]);
		SelectObject(hdcTemp, hbmOld);
		DeleteDC(hdcTemp);
		DeleteObject(hbm);
		ReleaseDC(NULL, hdc);
		Sleep(20);
	}
}
DWORD WINAPI pay11(LPVOID lpParam) {
	for (int t = 0;; t++) {
		HDC hdc = GetDC(NULL);
		int w = GetSystemMetrics(SM_CXSCREEN), h = GetSystemMetrics(SM_CYSCREEN);
		HDC hcdc = CreateCompatibleDC(hdc);
		HBITMAP hBitmap = CreateCompatibleBitmap(hdc, w, h);
		SelectObject(hcdc, hBitmap);
		BitBlt(hcdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
		for (int x = 0; x <= w; x += 5) {
			for (int y = 0; y <= h; y += 5) {
				StretchBlt(hcdc, x, y, (rand() % 10) - 3, (rand() % 10) - 3, hcdc, x, y, 1, 1, SRCCOPY);
			}
		}
		BitBlt(hdc, 0, 0, w, h, hcdc, 0, 0, SRCCOPY);
		ReleaseDC(NULL, hdc);
		ReleaseDC(NULL, hcdc);
		DeleteObject(hdc);
		DeleteObject(hcdc);
		DeleteObject(hBitmap);
		Sleep(100);
	}
}
DWORD WINAPI pay12(LPVOID l) {
	int screen_w = GetSystemMetrics(SM_CXSCREEN);
	int screen_h = GetSystemMetrics(SM_CYSCREEN);
	int cell_w = screen_w / 2;
	int cell_h = screen_h / 2;

	HDC hdcScreen = GetDC(NULL);
	HDC hdcMem = CreateCompatibleDC(hdcScreen);
	HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screen_w, screen_h);
	SelectObject(hdcMem, hBitmap);

	MSG msg;
	BOOL running = TRUE;

	while (running) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
				running = FALSE;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (!running) break;
		for (int row = 0; row < 2; row++) {
			for (int col = 0; col < 2; col++) {
				int rands = rand() % 10 - 5;
				int dst_x = col * cell_w;
				int dst_y = row * cell_h;
				int src_x = dst_x;
				int src_y = dst_y;
				int src_width = cell_w / 1;
				int src_height = cell_h / 1;
				SelectObject(hdcScreen, CreateSolidBrush(RandGreenToBlack()));
				StretchBlt(
					hdcScreen,
					dst_x + 10 + rands, dst_y + 10 + rands,
					cell_w - 20, cell_h - 20,
					hdcScreen,
					src_x, src_y,
					src_width, src_height,
					SRCCOPY
				);
				StretchBlt(
					hdcScreen,
					dst_x, dst_y,
					cell_w, cell_h,
					hdcScreen,
					src_x, src_y,
					src_width, src_height,
					PATINVERT
				);
			}
		}

		Sleep(50);
	}

	DeleteObject(hBitmap);
	DeleteDC(hdcMem);
	ReleaseDC(NULL, hdcScreen);

	RedrawWindow(NULL, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
	return 0;
}
DWORD WINAPI pay13(LPVOID l) {
	int screen_w = GetSystemMetrics(SM_CXSCREEN);
	int screen_h = GetSystemMetrics(SM_CYSCREEN);
	int cell_w = screen_w / 2;
	int cell_h = screen_h / 2;

	HDC hdcScreen = GetDC(NULL);
	HDC hdcMem = CreateCompatibleDC(hdcScreen);
	HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screen_w, screen_h);
	SelectObject(hdcMem, hBitmap);

	MSG msg;
	BOOL running = TRUE;

	while (running) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
				running = FALSE;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (!running) break;
		for (int row = 0; row < 2; row++) {
			for (int col = 0; col < 2; col++) {
				int dst_x = col * cell_w;
				int dst_y = row * cell_h;
				int src_x = dst_x;
				int src_y = dst_y;
				int src_width = cell_w / 1;
				int src_height = cell_h / 1;
				StretchBlt(
					hdcScreen,
					dst_x - 10, dst_y - 10,
					cell_w + 20, cell_h + 20,
					hdcScreen,
					src_x, src_y,
					src_width, src_height,
					SRCCOPY
				);
				StretchBlt(
					hdcScreen,
					dst_x, dst_y,
					cell_w, cell_h,
					hdcScreen,
					src_x, src_y,
					src_width, src_height,
					PATINVERT
				);
			}
		}

		Sleep(50);
	}

	DeleteObject(hBitmap);
	DeleteDC(hdcMem);
	ReleaseDC(NULL, hdcScreen);

	RedrawWindow(NULL, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
	return 0;
}
DWORD WINAPI pay14(LPVOID l) {
	int screen_w = GetSystemMetrics(SM_CXSCREEN);
	int screen_h = GetSystemMetrics(SM_CYSCREEN);
	int cell_w = screen_w / 2;
	int cell_h = screen_h / 2;

	HDC hdcScreen = GetDC(NULL);
	HDC hdcMem = CreateCompatibleDC(hdcScreen);
	HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screen_w, screen_h);
	SelectObject(hdcMem, hBitmap);

	MSG msg;
	BOOL running = TRUE;

	while (running) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
				running = FALSE;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (!running) break;
		for (int row = 0; row < 2; row++) {
			for (int col = 0; col < 2; col++) {
				int dst_x = col * cell_w;
				int dst_y = row * cell_h;
				int src_x = dst_x;
				int src_y = dst_y;
				int src_width = cell_w / 1;
				int src_height = cell_h / 1;
				StretchBlt(
					hdcScreen,
					dst_x - 20, dst_y + 20,
					cell_w + 30, cell_h - 30,
					hdcScreen,
					src_x, src_y,
					src_width, src_height,
					SRCCOPY
				);
			}
		}

		Sleep(50);
	}

	DeleteObject(hBitmap);
	DeleteDC(hdcMem);
	ReleaseDC(NULL, hdcScreen);

	RedrawWindow(NULL, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
	return 0;
}
DWORD WINAPI pay15(LPVOID lpvd) {
	HDC hdcScreen = GetDC(NULL);
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);

	HDC hdcMem = CreateCompatibleDC(hdcScreen);
	HBITMAP hbmScreen = CreateCompatibleBitmap(hdcScreen, w, h);
	HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmScreen);

	HDC hdcMem2 = CreateCompatibleDC(hdcScreen);
	HBITMAP hbmScreen2 = CreateCompatibleBitmap(hdcScreen, w, h);
	HBITMAP hbmOld2 = (HBITMAP)SelectObject(hdcMem2, hbmScreen2);

	double angle = 0;
	while (TRUE) {
		angle += 0.005;
		hdcScreen = GetDC(NULL);

		BitBlt(hdcMem, 0, 0, w, h, hdcScreen, 0, 0, SRCCOPY);
		BitBlt(hdcMem2, 0, 0, w, h, hdcMem, 0, 0, SRCCOPY);

		int cx = w / 2, cy = h / 2;
		int numSectors = 8 + (int)(4 * sin(angle * 0.5));
		double sectorAngle = 6.28318 / numSectors;

		HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
		RECT rect = { 0, 0, w, h };
		FillRect(hdcMem, &rect, blackBrush);
		DeleteObject(blackBrush);

		for (int y = 0; y < h; y += 2) {
			for (int x = 0; x < w; x += 2) {
				int dx = x - cx;
				int dy = y - cy;
				double dist = sqrt((double)(dx * dx + dy * dy));
				if (dist < 5) continue;

				double ang = atan2((double)dy, (double)dx);
				double mirroredAng = fmod(ang, sectorAngle);
				if (mirroredAng > sectorAngle / 2) mirroredAng = sectorAngle - mirroredAng;
				if (mirroredAng < 0) mirroredAng += sectorAngle;

				double distortion = 1.0 + 0.2 * sin(dist * 0.02 + angle * 2.0);
				int srcX = cx + (int)(dist * distortion * cos(mirroredAng + ang / numSectors));
				int srcY = cy + (int)(dist * distortion * sin(mirroredAng + ang / numSectors));

				if (srcX >= 0 && srcX < w && srcY >= 0 && srcY < h) {
					COLORREF color = GetPixel(hdcMem2, srcX, srcY);
					BYTE g = GetGValue(color);
					SetPixel(hdcMem, x, y, RGB(0, g, 0));
					SetPixel(hdcMem, x + 1, y, RGB(0, g, 0));
					SetPixel(hdcMem, x, y + 1, RGB(0, g, 0));
					SetPixel(hdcMem, x + 1, y + 1, RGB(0, g, 0));
				}
			}
		}

		BitBlt(hdcScreen, 0, 0, w, h, hdcMem, 0, 0, SRCCOPY);
		ReleaseDC(NULL, hdcScreen);
		Sleep(20);
	}

	SelectObject(hdcMem, hbmOld);
	SelectObject(hdcMem2, hbmOld2);
	DeleteObject(hbmScreen);
	DeleteObject(hbmScreen2);
	DeleteDC(hdcMem);
	DeleteDC(hdcMem2);
	return 0;
}
typedef struct {
	int dx, dy;
	int targetX, targetY;
	float smoothX, smoothY;
} WindowData;

LRESULT CALLBACK ExplosionWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE: {
		WindowData* pData = (WindowData*)malloc(sizeof(WindowData));
		RECT rect;
		GetWindowRect(hWnd, &rect);
		pData->smoothX = (float)rect.left;
		pData->smoothY = (float)rect.top;
		pData->dx = (rand() % 4 + 2) * (rand() % 2 ? 1 : -1);
		pData->dy = (rand() % 4 + 2) * (rand() % 2 ? 1 : -1);
		SetWindowLongPtrA(hWnd, GWLP_USERDATA, (LONG_PTR)pData);
		SetTimer(hWnd, 1, 16, NULL);
		return 0;
	}
	case WM_TIMER: {
		WindowData* pData = (WindowData*)GetWindowLongPtrA(hWnd, GWLP_USERDATA);
		if (pData) {
			RECT rect;
			GetWindowRect(hWnd, &rect);
			int screenW = GetSystemMetrics(SM_CXSCREEN);
			int screenH = GetSystemMetrics(SM_CYSCREEN);
			int winW = rect.right - rect.left;
			int winH = rect.bottom - rect.top;

			int newX = rect.left + pData->dx;
			int newY = rect.top + pData->dy;

			if (newX <= 0 || newX + winW >= screenW) pData->dx = -pData->dx;
			if (newY <= 0 || newY + winH >= screenH) pData->dy = -pData->dy;

			newX = rect.left + pData->dx;
			newY = rect.top + pData->dy;

			SetWindowPos(hWnd, NULL, newX, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}
		return 0;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			DestroyWindow(hWnd);
		}
		return 0;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		return 0;
	case WM_DESTROY: {
		WindowData* pData = (WindowData*)GetWindowLongPtrA(hWnd, GWLP_USERDATA);
		KillTimer(hWnd, 1);
		if (pData) free(pData);
		return 0;
	}
	default:
		return DefWindowProcA(hWnd, msg, wParam, lParam);
	}
}

static unsigned long long mat_n = 0, mat_r = 0;
static int mat_rndcopy() {
	mat_n = mat_r;
	mat_n ^= 0x8ebf635bee3c6d25;
	mat_n ^= mat_n << 5 | mat_n >> 26;
	mat_n *= 0xf3e05ca5c43e376b;
	mat_r = mat_n;
	return mat_n & 0x7fffffff;
}

DWORD WINAPI MATRIX1(LPVOID lpParam) {
	int time = GetTickCount();
	int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
	RGBQUAD* data = (RGBQUAD*)VirtualAlloc(0, (w * h + w) * sizeof(RGBQUAD), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	for (int i = 0;; i++, i %= 3) {
		HDC desk = GetDC(NULL);
		HDC hdcdc = CreateCompatibleDC(desk);
		HBITMAP hbm = CreateBitmap(w, h, 1, 32, data);
		SelectObject(hdcdc, hbm);
		BitBlt(hdcdc, 0, 0, w, h, desk, 0, 0, SRCCOPY);
		GetBitmapBits(hbm, w * h * 4, data);

		int v = 0;
		BYTE byte = 0;
		if ((GetTickCount() - time) > 60000)
			byte = mat_rndcopy() % 0xff;

		for (int i = 0; w * h > i; i++) {
			if (i % h == 0 && mat_rndcopy() % 110)
				v = mat_rndcopy() % 25;
			((BYTE*)(data + i))[1] = ((BYTE*)(data + i + v))[1];
			((BYTE*)(data + i))[0] = 0;
			((BYTE*)(data + i))[2] = 0;
		}

		SetBitmapBits(hbm, w * h * 4, data);
		BitBlt(desk, 0, 0, w, h, hdcdc, 0, 0, SRCCOPY);

		DeleteObject(hbm);
		DeleteObject(hdcdc);
		DeleteObject(desk);
		Sleep(20);
	}
	return 0;
}

DWORD WINAPI MATRIX2(LPVOID lpParam) {
	for (int t = 0;; t++) {
		HDC hdcScreen = GetDC(NULL);
		int w = GetSystemMetrics(SM_CXSCREEN), h = GetSystemMetrics(SM_CYSCREEN);

		HDC hcdc = CreateCompatibleDC(hdcScreen);
		HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, w, h);
		SelectObject(hcdc, hBitmap);
		BitBlt(hcdc, 0, 0, w, h, hdcScreen, 0, 0, SRCCOPY);

		POINT p = { w / 2, h / 2 };
		int nGraphicsMode = SetGraphicsMode(hdcScreen, GM_ADVANCED);
		XFORM xform;
		double fangle = 10.0 / 180.0 * 3.1415926;
		xform.eM11 = (float)cos(fangle);
		xform.eM12 = (float)sin(fangle);
		xform.eM21 = (float)-sin(fangle);
		xform.eM22 = (float)cos(fangle);
		xform.eDx = (float)(p.x - cos(fangle) * p.x + sin(fangle) * p.y);
		xform.eDy = (float)(p.y - cos(fangle) * p.y - sin(fangle) * p.x);
		SetWorldTransform(hdcScreen, &xform);

		BLENDFUNCTION blf = { 0 };
		blf.BlendOp = AC_SRC_OVER;
		blf.BlendFlags = 0;
		blf.SourceConstantAlpha = 128;
		blf.AlphaFormat = 0;

		AlphaBlend(hdcScreen, 0, t % h, w, h, hcdc, 0, 0, w, h, blf);

		xform.eM11 = 1; xform.eM12 = 0;
		xform.eM21 = 0; xform.eM22 = 1;
		xform.eDx = 0; xform.eDy = 0;
		SetWorldTransform(hdcScreen, &xform);

		DeleteObject(hcdc);
		DeleteObject(hBitmap);
		ReleaseDC(NULL, hdcScreen);
		Sleep(40);
	}
	return 0;
}

DWORD WINAPI win(LPVOID lpParam) {
	const char* messages[] = {
		"MATRIX.EXE has invaded your computer.",
		"MATRIX.EXE Made By Ghjds.",
		"MATRIX.EXE is leaking your IP.",
		"MATRIX.EXE is messing up your computer."
	};
	const int NUM_MESSAGES = sizeof(messages) / sizeof(messages[0]);

	srand((unsigned int)time(NULL) ^ GetCurrentThreadId());

	WNDCLASSA wc = { 0 };
	wc.lpfnWndProc = ExplosionWndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = "ExplosionWindow";
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIconA(GetModuleHandle(NULL), "IDI_MAIN_ICON");
	RegisterClassA(&wc);

	while (TRUE) {
		int idx = rand() % NUM_MESSAGES;

		int msgLen = strlen(messages[idx]);
		int w = 420;
		if (msgLen > 30) w = 520;
		if (msgLen > 40) w = 600;
		int h = 140;

		int x = rand() % (GetSystemMetrics(SM_CXSCREEN) - w);
		int y = rand() % (GetSystemMetrics(SM_CYSCREEN) - h);

		HWND hWnd = CreateWindowExA(
			WS_EX_TOPMOST,
			"ExplosionWindow",
			"MATRIX.EXE",
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
			x, y, w, h,
			NULL, NULL, GetModuleHandle(NULL), NULL
		);

		if (hWnd) {
			HWND hStatic = CreateWindowExA(
				0, "STATIC", messages[idx],
				WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE | SS_LEFT,
				15, 20, w - 30, h - 60,
				hWnd, NULL, GetModuleHandle(NULL), NULL
			);

			HFONT hFont = CreateFontA(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
				ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY, DEFAULT_PITCH, "Microsoft YaHei");
			SendMessageA(hStatic, WM_SETFONT, (WPARAM)hFont, TRUE);

			HWND hBtn = CreateWindowExA(
				0, "BUTTON", "OK",
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				w / 2 - 30, h - 40, 60, 28,
				hWnd, (HMENU)IDOK, GetModuleHandle(NULL), NULL
			);
			SendMessageA(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

			ShowWindow(hWnd, SW_SHOW);
			UpdateWindow(hWnd);
		}

		Sleep(200 + rand() % 150);
	}

	return 0;
}

VOID WINAPI sound1() {
	HWAVEOUT hWaveOut = 0;
	WAVEFORMATEX wfx = { WAVE_FORMAT_PCM,1,8000,8000,1,8,0 };
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
	char buffer[8000 * 30] = { 0 };
	for (DWORD t = 0; t < sizeof(buffer); ++t) {
		buffer[t] = static_cast<char>(t << (t >> (t >> 13 & t)));
	}
	WAVEHDR header = { buffer,sizeof(buffer),0,0,0,0,0,0 };
	waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutClose(hWaveOut);
}
VOID WINAPI sound2() {
	HWAVEOUT hWaveOut = 0;
	WAVEFORMATEX wfx = { WAVE_FORMAT_PCM,1,8000,8000,1,8,0 };
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
	char buffer[8000 * 30] = { 0 };
	for (DWORD t = 0; t < sizeof(buffer); ++t) {
		buffer[t] = static_cast<char>(~t + (t & t ^ t >> 6) - t * (t >> 9 & (t % 1 ? 2 : 6)));
	}
	WAVEHDR header = { buffer,sizeof(buffer),0,0,0,0,0,0 };
	waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutClose(hWaveOut);
}
VOID WINAPI sound3() {
	HWAVEOUT hWaveOut = 0;
	WAVEFORMATEX wfx = { WAVE_FORMAT_PCM,1,8000,8000,1,8,0 };
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
	char buffer[8000 * 30] = { 0 };
	for (DWORD t = 0; t < sizeof(buffer); ++t) {
		buffer[t] = static_cast<char>(t * ((t >> 12) & 63 & t >> 4) << (t * t << 5 ? 7 : 1) * t);
	}
	WAVEHDR header = { buffer,sizeof(buffer),0,0,0,0,0,0 };
	waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutClose(hWaveOut);
}
VOID WINAPI sound4() {
	HWAVEOUT hWaveOut = 0;
	WAVEFORMATEX wfx = { WAVE_FORMAT_PCM,1,8000,8000,1,8,0 };
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
	char buffer[8000 * 30] = { 0 };
	for (DWORD t = 0; t < sizeof(buffer); ++t) {
		buffer[t] = static_cast<char>((t >> 7 | t << 2 | t >> 6) * 10 + 4 * (t & t >> 13 | t >> 6));
	}
	WAVEHDR header = { buffer,sizeof(buffer),0,0,0,0,0,0 };
	waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutClose(hWaveOut);
}
VOID WINAPI sound5() {
	HWAVEOUT hWaveOut = 0;
	WAVEFORMATEX wfx = { WAVE_FORMAT_PCM,1,8000,8000,1,8,0 };
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
	char buffer[8000 * 30] = { 0 };
	for (DWORD t = 0; t < sizeof(buffer); ++t) {
		buffer[t] = static_cast<char>((5 * t & t >> 7 & 3 * t & t >> 10) * t);
	}
	WAVEHDR header = { buffer,sizeof(buffer),0,0,0,0,0,0 };
	waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutClose(hWaveOut);
}
VOID WINAPI sound6() {
	HWAVEOUT hWaveOut = 0;
	WAVEFORMATEX wfx = { WAVE_FORMAT_PCM,1,8000,8000,1,8,0 };
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
	char buffer[8000 * 30] = { 0 };
	for (DWORD t = 0; t < sizeof(buffer); ++t) {
		buffer[t] = static_cast<char>((t * (t >> 8 | t >> 9) & 46 & t >> 8 ^ (t & t >> 13 | t >> 6)) * ((0 - t)));
	}
	WAVEHDR header = { buffer,sizeof(buffer),0,0,0,0,0,0 };
	waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutClose(hWaveOut);
}
VOID WINAPI sound7() {
	HWAVEOUT hWaveOut = 0;
	WAVEFORMATEX wfx = { WAVE_FORMAT_PCM,1,8000,8000,1,8,0 };
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
	char buffer[8000 * 30] = { 0 };
	for (DWORD t = 0; t < sizeof(buffer); ++t) {
		buffer[t] = static_cast<char>((t >> 4 | t >> 8 | t >> 12) * 30 + (t & t >> 6 | t >> 9) * 5);
	}
	WAVEHDR header = { buffer,sizeof(buffer),0,0,0,0,0,0 };
	waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutClose(hWaveOut);
}
VOID WINAPI sound8() {
	HWAVEOUT hWaveOut = 0;
	WAVEFORMATEX wfx = { WAVE_FORMAT_PCM,1,8000,8000,1,8,0 };
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
	char buffer[8000 * 30] = { 0 };
	for (DWORD t = 0; t < sizeof(buffer); ++t) {
		buffer[t] = static_cast<char>((t * (t >> 6 | t >> 10) & 63 & t >> 8 ^ (t & t >> 12 | t >> 5)) * ((0 - t) / 2));
	}
	WAVEHDR header = { buffer,sizeof(buffer),0,0,0,0,0,0 };
	waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutClose(hWaveOut);
}
VOID WINAPI sound9() {
	HWAVEOUT hWaveOut = 0;
	WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 16000, 16000, 1, 8, 0 };
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
	char buffer[16000 * 30] = { 0 };
	for (DWORD t = 0; t < sizeof(buffer); ++t) {
		double freq = 50 + 30 * sin(t * 0.001);
		double phase = t * 2 * 3.14159 * freq / 16000;
		buffer[t] = static_cast<char>(64 * sin(phase) + 64);
	}
	WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
	waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
	Sleep(30000);
	waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutClose(hWaveOut);
}

VOID WINAPI sound10() {
	HWAVEOUT hWaveOut = 0;
	WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 };
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
	char buffer[8000 * 30] = { 0 };
	for (DWORD t = 0; t < sizeof(buffer); ++t) {
		int glitch = (t >> 8) & (t >> 6);
		int burst = (rand() % 100 > 95) ? (rand() % 128) : 0;
		buffer[t] = static_cast<char>((t & 255) ^ glitch ^ (burst << 1));
	}
	WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
	waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
	Sleep(30000);
	waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutClose(hWaveOut);
}

VOID WINAPI sound11() {
	HWAVEOUT hWaveOut = 0;
	WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 12000, 12000, 1, 8, 0 };
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
	char buffer[12000 * 30] = { 0 };
	for (DWORD t = 0; t < sizeof(buffer); ++t) {
		double progress = (double)t / (12000 * 30);
		double intensity = progress * progress;
		double freq = 200 + 1800 * intensity;
		double amp = 32 + 96 * intensity;
		if (amp > 127) amp = 127;
		double phase = t * 2 * 3.14159 * freq / 12000;
		double sinVal = sin(phase);
		double squareVal = (sinVal > 0) ? 1.0 : -1.0;
		double mixed = (1 - intensity) * sinVal + intensity * squareVal;
		buffer[t] = static_cast<char>(amp * mixed + 128);
	}
	WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
	waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
	Sleep(30000);
	waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutClose(hWaveOut);
}

VOID WINAPI sound12() {
	HWAVEOUT hWaveOut = 0;
	WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 22050, 22050, 1, 8, 0 };
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
	char buffer[22050 * 30] = { 0 };
	for (DWORD t = 0; t < sizeof(buffer); ++t) {
		double rotateSpeed = 0.5;
		double freqOffset = 300 * sin(t * 2 * 3.14159 * rotateSpeed / 22050);
		double baseFreq = 600 + freqOffset;
		double phase = t * 2 * 3.14159 * baseFreq / 22050;
		double tremolo = 1.0 + 0.3 * sin(t * 2 * 3.14159 * 4 / 22050);
		buffer[t] = static_cast<char>(64 * tremolo * sin(phase) + 128);
	}
	WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
	waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
	Sleep(30000);
	waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutClose(hWaveOut);
}

HWND hDlg;
HANDLE glitchmsgbox1;
int MessageBoxWidth;
int MessageBoxHeight;
HHOOK hHook = NULL;
BOOL CALLBACK EnumProc(HWND hWnd, LPARAM lParam) {
	ShowWindow(hWnd, 0);
	EnableWindow(hWnd, FALSE);
	return 1;
}
DWORD WINAPI msgboxglitch(LPVOID lpParam) {
	RGBQUAD* data = (RGBQUAD*)VirtualAlloc(0, (MessageBoxWidth * MessageBoxHeight + MessageBoxWidth) * sizeof(RGBQUAD), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	for (int i = 0;; i++, i %= 3) {
		HDC hdc = GetDC(hDlg);
		HDC hcdc = CreateCompatibleDC(hdc);
		HBITMAP hBitmap = CreateBitmap(MessageBoxWidth, MessageBoxHeight, 1, 32, data);
		SelectObject(hcdc, hBitmap);
		BitBlt(hcdc, 0, 0, MessageBoxWidth, MessageBoxHeight, hdc, 0, 0, SRCERASE);
		GetBitmapBits(hBitmap, 4 * MessageBoxHeight * MessageBoxWidth, data);
		int v = 0;
		BYTE byte = rand() % 0xff;
		for (int i = 0; MessageBoxWidth * MessageBoxHeight > i; ++i) {
			v = rand() % 114;
			*((BYTE*)data + 4 * i + v) -= 5;
		}
		SetBitmapBits(hBitmap, MessageBoxWidth * MessageBoxHeight * 4, data);
		BitBlt(hdc, 0, 0, MessageBoxWidth, MessageBoxHeight, hcdc, 0, 0, SRCERASE);
		DeleteObject(hBitmap);
		DeleteObject(hcdc);
		ReleaseDC(0, hdc);
		Sleep(5);
	}
}
VOID WINAPI MsgBoxCorruptionThread(HWND hwndMsgBox) {
	RECT rect;
	GetWindowRect(hwndMsgBox, &rect);
	int a = 2;
	for (;;) {
		int w = rect.right - rect.left, h = rect.bottom - rect.top;
		HDC hdc = GetDC(hwndMsgBox), hdcMem = CreateCompatibleDC(hdc);
		POINT pos[3];
		pos[0].x = 0, pos[0].y = 0;
		pos[1].x = cos(a * (3.14 / 180)) * w, pos[1].y = sin(a * (3.14 / 180)) * h;
		pos[2].x = (sin(a * (3.14 / 180)) * w), pos[2].y = cos(a * (3.14 / 180)) * h;
		PlgBlt(hdc, pos, hdc, 0, 0, w, h, 0, 0, 0);
		ReleaseDC(0, hdc);
		Sleep(200);
	}
}
LRESULT CALLBACK msgBoxHook(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HCBT_ACTIVATE) {
		HWND hwndMsgBox = (HWND)wParam;
		Sleep(100);
		HANDLE handle = CreateThread(NULL, 0, (PTHREAD_START_ROUTINE)MsgBoxCorruptionThread, hwndMsgBox, 0, NULL);
		return 0;
	}
	return CallNextHookEx(0, nCode, wParam, lParam);
}

VOID WINAPI RunNewStage(LPTHREAD_START_ROUTINE ThreadName, INT RunTime) {
	HANDLE hT = CreateThread(0, 0, ThreadName, 0, 0, 0);
	Sleep(RunTime);
	TerminateThread(hT, 0);
	SendMessageA(FindWindowA("SysListView32", NULL), WM_ERASEBKGND, 0, 0);
	RedrawWindow(NULL, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
	Sleep(50);
}

int main() {
	srand((unsigned int)time(NULL));
	SetProcessDPIAware();
	if (MessageBoxA(NULL, "Run the VIRUS? It's NOT a JOKE!!!",
		"Warning--MATRIX.EXE",
		MB_YESNO | MB_ICONWARNING | MB_TOPMOST | MB_RIGHT | MB_DEFBUTTON2) == IDNO) {
		return 0;
	}
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	HHOOK hMsgHook = SetWindowsHookEx(WH_CBT, msgBoxHook, 0, GetCurrentThreadId());
	HANDLE hBall = CreateThread(0, 0, ball, 0, 0, 0);

	sound1();
	RunNewStage(NewStage0, 30000);
	sound2();
	RunNewStage(NewStage1, 30000);
	sound3();
	RunNewStage(NewStage2, 30000);
	sound4();
	RunNewStage(NewStage3, 30000);
	sound5();
	RunNewStage(NewStage4, 30000);
	sound6();
	RunNewStage(NewStage5, 30000);
	sound1();
	RunNewStage(NewStage6, 30000);
	sound2();
	RunNewStage(NewStage7, 30000);
	sound3();
	RunNewStage(NewStage8, 30000);
	sound4();
	RunNewStage(NewStage9, 30000);
	sound5();
	RunNewStage(NewStage10, 30000);
	sound6();
	RunNewStage(NewStage11, 30000);
	sound1();
	RunNewStage(NewStage12, 30000);
	sound2();
	RunNewStage(NewStage13, 30000);
	sound3();
	RunNewStage(NewStage14, 30000);
	sound4();
	RunNewStage(NewStage15, 30000);
	sound1();
	RunThreadDouble(payload1a, payload1b, 30000);
	sound2();
	RunThread(shader1, 30000);
	sound3();
	RunThreadDouble(shader2b, payload2a, 30000);
	sound4();
	RunThreadDouble(payload3, shader2b, 30000);
	sound5();
	RunThread(shader3b, 30000);
	sound6();
	RunThread(payload5, 30000);
	sound7();
	RunThreadDouble(payload6a, payload6b, 30000);
	sound8();
	RunThreadDouble(payload7a, payload7b, 30000);
	sound1();
	RunThreadThree(payload8a, shader4b, payload8c, 30000);
	sound2();
	RunThread(shader5, 30000);
	sound3();
	RunThread(shader6, 30000);
	sound4();
	RunThread(shader7, 30000);
	sound5();
	RunThread(pay9, 30000);
	sound6();
	RunThread(shader8, 30000);
	sound7();
	RunThread(pay10, 30000);
	sound8();
	RunThread(pay11, 30000);
	sound1();
	RunThread(pay12, 30000);
	sound2();
	RunThread(pay13, 30000);
	sound3();
	RunThread(pay14, 30000);
	sound4();
	RunThread(shader9, 30000);
	sound7();
	RunThread(pay15, 30000);
	if (hBall != NULL) {
		TerminateThread(hBall, 0);
		CloseHandle(hBall);
		hBall = NULL;
		RedrawWindow(NULL, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
		Sleep(50);
	}
	HANDLE hM1 = CreateThread(0, 0, MATRIX1, 0, 0, 0);
	sound9();
	TerminateThread(hM1, 0);
	CloseHandle(hM1);
	RedrawWindow(NULL, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
	Sleep(50);
	HANDLE hM2 = CreateThread(0, 0, MATRIX2, 0, 0, 0);
	sound10();
	TerminateThread(hM2, 0);
	CloseHandle(hM2);
	RedrawWindow(NULL, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
	Sleep(50);
	sound8();
	RunThread(win, 30000);

	TerminateThread(hBall, 0);
	CloseHandle(hBall);

	ExitProcess(0);
}
