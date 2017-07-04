#ifndef __AMP_CLUTMANAGER__
#define __AMP_CLUTMANAGER__

#include "AmpHead.hpp"
#include "Surface.hpp"

const	short	kMaxLuminosityLevel = 4;
const	short	kNumOfCoronas = 3;
const	short	kCoronaIDs[kNumOfCoronas] = {10000, 10001, 10002};

static short kCoronaDescriptor[kNumOfCoronas][5] = {
	{10000, 317, 543, 94, 92},
	{10001, 382, 806, 126, 126},
	{10002, 322, 802, 40, 40}
};

enum {
	kLightningNoEffect,
	kLightningBlueEffect,
	kLightningRedEffect,
	kLightningGreenEffect,
	kLightningYellowEffect,
	kLightningPurpleEffect,
	kYellowCoronaTable,
	kWhiteCoronaTable,
	kBlueCoronaTable
};

enum {
	kFadeToColor,
	kFadeFromColor
};

struct tCorona {
	unsigned char	*values;
	unsigned char	**table;
	short	dx, dy;
};

class CClutManager {
protected:
	RGBcolor	palette[256];
	unsigned char	*luminosityTable[256];
	unsigned char	waterColorTable[256];
	unsigned char	lavaColorTable[256];
	unsigned char	shadowTable[256];
	unsigned char	fogTable[256];

	unsigned char	**transparentTable1;
	unsigned char	**transparentTable2;
	unsigned char	**transparentTable3;

	unsigned char	**blueLightningTable;
	unsigned char	**redLightningTable;
	unsigned char	**greenLightningTable;
	unsigned char	**yellowLightningTable;
	unsigned char	**purpleLightningTable;

	unsigned char	**whiteCoronaTable;
	unsigned char	**yellowCoronaTable;
	unsigned char	**blueCoronaTable;

	unsigned char	*radiantTable;

	tCorona		coronas[kNumOfCoronas];

	unsigned char	**EffectToTable(short effect);

public:
	CClutManager();
	~CClutManager();

	short		FindClosestColor(unsigned char red, unsigned char green, unsigned char blue);
	void	LoadPalette(RGBcolor *pal);
	void	BuildLuminosityTable();
	void	CalculateCoronas(unsigned char *, short);
	unsigned char	ChangeColorValues(unsigned char color, short percentageRed, short percentageGreen, short percentageBlue);
	void	SetPixel(unsigned char *source, unsigned char *dest, short modus, short luminosity);
	void	DrawLightning(double xm, double ym, short effect, CGraphicSurface *surface);
	void	PrepareCorona(double xm, double ym, CGraphicSurface *surface);
	void	DrawCorona(double xm, double ym, short corona, short effect, CGraphicSurface *surface, double &coronaFade, long deltaTime);
	void	FadeToColor(short direction, short effect, CGraphicSurface *surface);
};

void	SwapBlackWhite(Graphic_file *gf);

#endif