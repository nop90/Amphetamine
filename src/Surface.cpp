#include "Surface.hpp"
#include "Shape.hpp"
#include "ShapeLd.hpp"
#include "Clut.hpp"
#include "ConstVal.hpp"
#include <memory.h>

const	short	kNoCharSpace = 20;
const	char	kNoCharChar = '_';

extern	CSystem	*gSystem;
extern	CShapeManager *gShapeManager;
extern	CClutManager	*gClutManager;
extern	tConstValues	*gConst;

CGraphicSurface::CGraphicSurface(short dx, short dy)
{
	buffer = gSystem->AllocateBuffer(dx, dy);
	gSystem->SetBufferPalette(buffer);

	width = dx;
	height = dy;

	numGraphics = 0;
}

CGraphicSurface::~CGraphicSurface()
{
	for (short n = 0; n < numGraphics; n ++) {
		if (graphicTypes[n] == kGraphicEmbedded) free_graphic_file(graphics[n]);
	}
	gSystem->DisposeBuffer(buffer);
}

unsigned char	*CGraphicSurface::GetSurfacePtr(short *pitch)
{
	unsigned char	*tmp = gSystem->GetBufferPtr(buffer, pitch);

	return tmp;
}

void	CGraphicSurface::ReleaseSurface()
{
	gSystem->ReleaseBufferPtr(buffer);
}

void	CGraphicSurface::InsertGraphic(char *filename, Graphic_file *graphic, tRect *position)
{
	if (filename) {
		graphics[numGraphics] = read_graphic_file(filename);

		SwapBlackWhite(graphics[numGraphics]);

		graphicsPositions[numGraphics].left = 0;
		graphicsPositions[numGraphics].top = 0;
		graphicsPositions[numGraphics].right = graphics[numGraphics]->width;
		graphicsPositions[numGraphics].bottom = graphics[numGraphics]->height;
		graphicTypes[numGraphics] = kGraphicEmbedded;

		numGraphics ++;
	}
	if (graphic && position) {
		graphics[numGraphics] = graphic;

		graphicsPositions[numGraphics].left = position->left;
		graphicsPositions[numGraphics].top = position->top;
		graphicsPositions[numGraphics].right = position->right;
		graphicsPositions[numGraphics].bottom = position->bottom;
		graphicTypes[numGraphics] = kGraphicReferenced;

		numGraphics ++;
	}
}

void	CGraphicSurface::PaintGraphic(short num, short left, short top, short modus)
{
	short	j, k;
	short	bottom, right;
	unsigned char	*baseAddr, *linePtr, *sourcePtr, *sourceLinePtr;
	short	pitch;
	
	baseAddr = gSystem->GetBufferPtr(buffer, &pitch);

// Luke hat hier das -1 entfernt
	bottom = MIN(top + graphicsPositions[num].bottom - graphicsPositions[num].top, height);
	right = MIN(left + graphicsPositions[num].right - graphicsPositions[num].left, pitch);

	baseAddr += top * pitch + left;
	sourcePtr = graphics[num]->bitmap + graphicsPositions[num].top * graphics[num]->width + graphicsPositions[num].left;

	for (j = top; j < bottom; j ++) {
		k = right - left;
		
		sourceLinePtr = sourcePtr;
		linePtr = baseAddr;
		
		if (modus == kShapemodusNormal) {
			memcpy(linePtr, sourcePtr, k);
		} else {
			while (k) {
				gClutManager->SetPixel((unsigned char *)sourceLinePtr, (unsigned char *)linePtr, modus, 0);
				linePtr++;
				sourceLinePtr++;
				k--;
			}
		}
		baseAddr += pitch;
		sourcePtr += graphics[num]->width;
	}

	gSystem->ReleaseBufferPtr(buffer);
}



#define INT_TO_FIXED(i) ((i) << 16)
#define FIXED_TO_INT(f) ((f) >> 16)
#define FIXED_TO_FLOAT(f) (((double) (f)) * 1.52587890625e-5)
#define FLOAT_TO_FIXED(f) ((long) ((f) * 65536.0))


void	CGraphicSurface::DrawAntialiasedLine(short x1, short y1, short x2, short y2, unsigned char color, short modus)
{	
	unsigned char	*baseAddr;
	short	pitch;

	baseAddr = gSystem->GetBufferPtr(buffer, &pitch);

	x1 = MAX(0, x1);
	x1 = MIN(pitch, x1);
	x2 = MAX(0, x2);
	x2 = MIN(pitch, x2);
	y1 = MAX(0, y1);
	y1 = MIN(height -1, y1);
	y2 = MAX(0, y2);
	y2 = MIN(height -1, y2);

	short	dx = x2 - x1, dy = y2 - y1, dmax;
	long	ex, ey, curx = 0, cury = 0;
	long	lastx, lasty;
	short	counter;

	if (abs(dx) > abs(dy)) {
		ex = FLOAT_TO_FIXED(SIGN(dx) * 1.0);
		ey = FLOAT_TO_FIXED((double)dy / (double)abs(dx));
		counter = SIGN(dx) * dx;
	}else{
		ex = FLOAT_TO_FIXED((double)dx / (double)abs(dy));
		ey = FLOAT_TO_FIXED(SIGN(dy) * 1.0);
		counter = SIGN(dy) * dy;
	}
	curx = INT_TO_FIXED(x1);
	cury = INT_TO_FIXED(y1);

	lastx = INT_TO_FIXED(x1);
	lasty = INT_TO_FIXED(y1);



	baseAddr += (FIXED_TO_INT(lasty) * pitch + FIXED_TO_INT(lastx));

	while (counter) {
		gClutManager->SetPixel((unsigned char *)&color, baseAddr, modus, 0);

		curx += ex;
		cury += ey;
		counter --;

		if (lasty >> 16 != cury >> 16) {
			baseAddr += (SIGN(cury - lasty)) * pitch;
			lasty = cury;
		}
		if (lastx >> 16 != curx >> 16) {
			baseAddr += SIGN(curx - lastx);
			lastx = curx;
		}
	}
	gSystem->ReleaseBufferPtr(buffer);
}


void	CGraphicSurface::PaintRect(short left, short top, short right, short bottom, unsigned char color, short modus)
{
	short	j, k;
	unsigned char	*baseAddr;
	short	pitch;

	baseAddr = gSystem->GetBufferPtr(buffer, &pitch);

	left = MAX(0, left);
	top = MAX(0, top);
	left = MIN(pitch, left);
	top = MIN(height, top);

	right = MAX(0, right);
	bottom = MAX(0, bottom);
	right = MIN(pitch, right);
	bottom = MIN(height, bottom);


	baseAddr += top * pitch;

	for (j = top; j < bottom; j ++) {
		for (k = left; k < right; k ++) {
			gClutManager->SetPixel((unsigned char *)&color, baseAddr + k, modus, 0);
		}
		baseAddr += pitch;
	}

	gSystem->ReleaseBufferPtr(buffer);
}

void	CGraphicSurface::DrawString(short left, short top, char *text, short modus)
{
	short	n = 0, x, startWord, startShapePos;
	short	shapePos = left;
	short	dx, unused;
	CShape	*shape;
	unsigned char	*unused2;

	tRect	clipRect;

	clipRect.left = clipRect.top = 0;
	clipRect.right = width; clipRect.bottom = height;

	while (text && text[n] != '\0') {
		startWord = n;
		startShapePos = shapePos;
		x = n;
		while (text[x] != kNoCharChar && text[x] != '\0') {
			shape = gShapeManager->FindShape((short)text[x], 0);
			if (shape) unused2 = shape->AllowPixelAccess(dx, unused); else dx = 0;
			shapePos += dx;
			x ++;
		}
		if (shapePos >= width) {
			top += gConst->kTextYDistance;
			shapePos = left;
		}else shapePos = startShapePos;

		x = startWord;
		while (text[x] != kNoCharChar && text[x] != '\0') {
			shape = gShapeManager->FindShape((short)text[x], 0);
			if (shape) shapePos += shape->RenderShape(shapePos, top, &clipRect, modus, 0, this);
			x ++;
		}

		if (text[x] == kNoCharChar) {
			shapePos += kNoCharSpace;
			x ++;
		}
		n = x;
	}
}

void	CGraphicSurface::FlipToScreen(short left, short top)
{
	gSystem->FlipSurfaces(buffer, width, height, left, top);
}
