#ifndef __AMP_GRAPHSURFACE__
#define __AMP_GRAPHSURFACE__

#include "System.hpp"
#include "Graphfil.hpp"

const short	kMaxGraphics = 32;
const short	kNumChars = 40;

enum {
	kGraphicEmbedded,
	kGraphicReferenced
}; // graphicType

class CShape;

class CGraphicSurface {
protected:
	tGraphicBuffer	*buffer;
	
	Graphic_file	*graphics[kMaxGraphics];
	short			graphicTypes[kMaxGraphics];
	short			numGraphics;
	tRect			graphicsPositions[kMaxGraphics];
	
public:
	short	width, height;

	CGraphicSurface(short dx, short dy);
	~CGraphicSurface();
	
	void	InsertGraphic(char *filename, Graphic_file *graphic, tRect *position);
	void	PaintGraphic(short num, short left, short top, short modus);
	void	DrawAntialiasedLine(short x1, short y1, short x2, short y2, unsigned char color, short modus);
	void	PaintRect(short left, short top, short right, short bottom, unsigned char color, short modus);
	void	DrawString(short left, short top, char *text, short modus);
	
	void	FlipToScreen(short left, short top);
	unsigned char	*GetSurfacePtr(short *pitch);
	void	ReleaseSurface();
};

#endif	
