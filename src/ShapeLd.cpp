#include "ShapeLd.hpp"
#include "Graphfil.hpp"
#include "Appl.hpp"
#include "Clut.hpp"
#include "ShapeDes.hpp"
#include "ConstVal.hpp"
#include <stdio.h>

extern	CApplication *gApplication;
extern	CSystem		*gSystem;
extern	CClutManager	*gClutManager;
extern	tConstValues	*gConst;
extern	CClutManager	*gClutManager;
extern	FILE			*logFile;

CShapeManager::CShapeManager()
{
}

CShapeManager::~CShapeManager()
{
}

void	CShapeManager::LoadShapes()
{
	short	n;
	Graphic_file	*plane;

	MSG("Opening shape file\n");
	plane = read_graphic_file(gSystem->QualifyDataDir(gConst->kFileShapes));  // by LL
	if (!plane) gSystem->Error("Cannot find kFileShapes or cannot open it. Perhaps lack of memory?", 0);

	MSG("Creating shapes and textures\n");
	for (n = 0; n < kNumShapes; n ++) {
		shapes[n] = new CMaskedShape(plane->bitmap, &kShapeDescriptor[n][0], plane->width);
	}
	for (n = 0; n < kNumTextures; n ++) {
		for (short m = 0; m < 4; m ++) {
			textures[n][m] = new CTexture(plane->bitmap, &kTextureDescriptor[n][0], plane->width, m);
		}
	}

	gClutManager->CalculateCoronas(plane->bitmap, plane->width);

	free_graphic_file(plane);
}

void	CShapeManager::UnloadShapes()
{
	short	n;

	for (n = 0; n < kNumShapes; n ++) {
		delete shapes[n];
	}
	for (n = 0; n < kNumTextures; n ++) {
		for (short m = 0; m < 4; m ++) {
			delete textures[n][m];
		}
	}
}

void	CShapeManager::LoadBackground(char *background)
{
	MSG("Opening background image\n");
	backgroundPicture = read_graphic_file(gSystem->QualifyDataDir(background));
	if (!backgroundPicture) gSystem->Error("Cannot open background file. Lack of memory?", 0);
	SwapBlackWhite(backgroundPicture);
}

void	CShapeManager::UnloadBackground()
{
	free_graphic_file(backgroundPicture);
}

CShape*	CShapeManager::FindShape(short id, short light)
{
	short n;
	if (id == -1) return NULL; else {
		if (id >= kTextureStart && id <= kTextureEnd) {
			for (n = 0; n < kNumTextures && kTextureDescriptor[n][0] != id; n ++) {}
			if (n != kNumTextures) return textures[n][light]; else return 0L;

		}else{ 
			for (n = 0; n < kNumShapes && kShapeDescriptor[n][0] != id; n ++) {}
			if (n != kNumShapes) return shapes[n]; else return 0L;
		}
	}
}

unsigned char *CShapeManager::GetBackground(short &width, short &height)
{
	width = backgroundPicture->width;
	height = backgroundPicture->height;

	return backgroundPicture->bitmap;
}