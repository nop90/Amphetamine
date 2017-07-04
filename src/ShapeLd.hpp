#ifndef __AMP_SHAPEMANAGER__
#define __AMP_SHAPEMANAGER__


#include "System.hpp"
#include "Shape.hpp"
#include "Graphfil.hpp"


enum {
	kTotalShapes = 336,
	kTextureStart = 3000,
	kTextureEnd = 3054,
	kNumTextures = 45,
	kNumShapes = kTotalShapes - kNumTextures,
	kAddPlayerStart = 510,
	kAddPlayerEnd = 574
};


class CShapeManager {
protected:
	CShape			*shapes[kNumShapes];
	CShape			*textures[kNumTextures][4];
	unsigned char	*coronas;
	Graphic_file	*backgroundPicture;

public:
	CShapeManager();
	~CShapeManager();

	void	LoadShapes();
	void	LoadBackground(char *name);
	void	UnloadShapes();
	void	UnloadBackground();

	CShape	*FindShape(short id, short light);
	unsigned char	*GetBackground(short &width, short &height);
};

#endif
