#ifndef __AMP_SHAPES__
#define __AMP_SHAPES__

#include "AmpHead.hpp"
#include "Surface.hpp"

class	CShape {
protected:
	short	id;
	short	dx, dy;
	unsigned char	*shapeForward;

public:
	long	ID;
	long	savedID;

	CShape(unsigned char *, short [], long);
	~CShape();

	virtual short	RenderShape(short x, short y, tRect *clipRect, short modus, short percentage, CGraphicSurface *surface);
	unsigned char	*AllowPixelAccess(short &dx, short &dy);
};

class	CMaskedShape : public CShape {
protected:
	unsigned char	*shapeBackward;

	void	Encode(unsigned char *, unsigned char *, short [], long);
	void	RenderShapeClipped(short x, short y, tRect *clipRect, short modus, short percentage, CGraphicSurface *surface);
	void	RenderShapeUnclipped(short x, short y, short modus, short percentage, CGraphicSurface *surface);

public:
	CMaskedShape(unsigned char *, short [], long);
	~CMaskedShape();

	short	RenderShape(short x, short y, tRect *clipRect, short modus, short percentage, CGraphicSurface *surface);
};

class	CTexture : public CShape {
protected:

public:
	CTexture(unsigned char *, short [], long, short light);
	~CTexture();

	short	RenderShape(short x, short y, tRect *clipRect, short unused, short unused2, CGraphicSurface *surface);
};


class	CBackground : public CShape {
protected:
	short	shapeWidth, shapeHeight;

public:
	CBackground(unsigned char *, short [], long);
	~CBackground();

	short	RenderShape(short x, short y, tRect *clipRect, short unused, short unused2, CGraphicSurface *surface);
};

#endif
