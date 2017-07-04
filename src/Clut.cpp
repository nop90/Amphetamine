#include "Clut.hpp"
#include "System.hpp"
#include "ConstVal.hpp"
#include "Level.hpp"
#include "ShapeLd.hpp"

extern	tConstValues	*gConst;
extern	tConfigData		*gConfigData;
extern	CLevel			*gLevel;
extern	CSystem			*gSystem;
extern	CShapeManager	*gShapeManager;
extern	FILE			*logFile;

const	short	kWaterTransparence = 60; // percent of blue
const	short	kLavaTransparence = 80;  // percent of red and *3 percent of green
const	double	kFogFactor = 0.8;

short	WeightenedRandom(short anz);

CClutManager::CClutManager()
{
	long	maxRadiant, radiant;
	short	j, k, val;

	for (short n = 0; n < kMaxLuminosityLevel * 2; n ++) {
		luminosityTable[n] = new unsigned char [256];
	}

	transparentTable1 = new unsigned char * [256];
	transparentTable2 = new unsigned char * [256];
	transparentTable3 = new unsigned char * [256];
	for (j = 0; j < 256; j ++) {
		transparentTable1[j] = new unsigned char [256];
		transparentTable2[j] = new unsigned char [256];
		transparentTable3[j] = new unsigned char [256];
	}

	blueLightningTable = new unsigned char * [gConst->kLightningRadiant];
	redLightningTable = new unsigned char * [gConst->kLightningRadiant];
	greenLightningTable = new unsigned char * [gConst->kLightningRadiant];
	yellowLightningTable = new unsigned char * [gConst->kLightningRadiant];
	purpleLightningTable = new unsigned char * [gConst->kLightningRadiant];

	whiteCoronaTable = new unsigned char * [gConst->kLightningRadiant];
	yellowCoronaTable = new unsigned char * [gConst->kLightningRadiant];
	blueCoronaTable = new unsigned char * [gConst->kLightningRadiant];

	for (j = 0; j < gConst->kLightningRadiant; j ++) {
		blueLightningTable[j] = new unsigned char [256];
		redLightningTable[j] = new unsigned char [256];
		greenLightningTable[j] = new unsigned char [256];
		yellowLightningTable[j] = new unsigned char [256];
		purpleLightningTable[j] = new unsigned char [256];

		whiteCoronaTable[j] = new unsigned char [256];
		yellowCoronaTable[j] = new unsigned char [256];
		blueCoronaTable[j] = new unsigned char [256];
	}	
	radiantTable = new unsigned char [gConst->kLightningRadiant * gConst->kLightningRadiant];
	maxRadiant = gConst->kLightningRadiant * gConst->kLightningRadiant;

	for (j = 0; j < gConst->kLightningRadiant; j ++) {
		for (k = 0; k < gConst->kLightningRadiant; k ++) {
			radiant = j * j + k * k;
			if (radiant > maxRadiant) 
				radiantTable [j * gConst->kLightningRadiant + k] = gConst->kLightningRadiant -1;
			else {
				val = radiant * gConst->kLightningRadiant / maxRadiant + WeightenedRandom(5) - 1;
				radiantTable [j * gConst->kLightningRadiant + k] = MIN(val, gConst->kLightningRadiant - 1);
			}
		}
	}
	coronas[0].values = coronas[1].values = coronas[2].values = 0L;
}

CClutManager::~CClutManager()
{
	short	n;

	if (coronas[0].values) delete [] coronas[0].values;
	if (coronas[1].values) delete [] coronas[1].values;
	if (coronas[2].values) delete [] coronas[2].values;

	for (n = 0; n < kMaxLuminosityLevel * 2; n ++) {
		delete [] luminosityTable[n];
	}
	for (n = 0; n < gConst->kLightningRadiant; n ++) {
		delete [] blueLightningTable[n];
		delete [] redLightningTable[n];
		delete [] greenLightningTable[n];
		delete [] yellowLightningTable[n];
		delete [] purpleLightningTable[n];

		delete [] whiteCoronaTable[n];
		delete [] yellowCoronaTable[n];
		delete [] blueCoronaTable[n];
	}
	delete [] blueLightningTable;
	delete [] redLightningTable;
	delete [] greenLightningTable;
	delete [] yellowLightningTable;
	delete [] purpleLightningTable;

	delete [] whiteCoronaTable;
	delete [] yellowCoronaTable;
	delete [] blueCoronaTable;

	delete [] radiantTable;
}


// --------------------------------------
short	WeightenedRandom(short anz)
// Returns a number which the possibility that the number is 1 is 1/2, for 2 is 1 / 3, for 3 is 1/4 etc.
{
	long	maxSum = 0, n;
	long	randValue;

	for (n = 1; n <= anz; n ++) maxSum += 100 / n;
	
	randValue = rand() * maxSum / RAND_MAX;
	maxSum = 0;
	for (n = 1; n <= anz; n ++) {
		if (randValue >= maxSum && randValue < maxSum + 100 / n) return n;
		maxSum += 100 / n;
	}
	return 1;
}

// ----------------------------------------------
void	CClutManager::LoadPalette(RGBcolor *pal)
// Writing the palette from the AmpGraf.gif file into the color manager
{
	short	j, k;

	for (short n = 0; n < 256; n ++) {
		palette[n].red = pal[n].red;
		palette[n].green = pal[n].green;
		palette[n].blue = pal[n].blue;
	}
	SWAP(palette[0].red, palette[255].red, unsigned char);
	SWAP(palette[0].green, palette[255].green, unsigned char);
	SWAP(palette[0].blue, palette[255].blue, unsigned char);
}


// --------------------------------------------------------
void	CClutManager::BuildLuminosityTable()
// Builds the luminosity and the color tables
{
	unsigned char	color;
	short	j, k;
	FILE	*fClut = fopen(gSystem->QualifyDataDir(gConst->kFileCluts), "rb");   // by LL
	short	index, index2;
	short	mode = 0;

	if (!fClut) {
		fClut = fopen(gSystem->QualifyDataDir(gConst->kFileCluts), "wb");   // by LL
		if (!fClut) MSG("!!! File not found: "); MSG(gConst->kFileCluts); MSG("\n");
		mode = 1;
	}

	fseek(fClut, 0, SEEK_SET);

	for (short n = -kMaxLuminosityLevel; n < kMaxLuminosityLevel; n ++) {
		for (short j = 0; j < 256; j ++) {
			if (mode) {
				color = ChangeColorValues(j, SIGN(n) * gConst->kBrightnessLevels[ABS(n)], 
					SIGN(n) * gConst->kBrightnessLevels[ABS(n)], 
					SIGN(n) * gConst->kBrightnessLevels[ABS(n)]);

				luminosityTable[kMaxLuminosityLevel + n][j] = color;
				fputc(color, fClut);
			}else{
				luminosityTable[kMaxLuminosityLevel + n][j] = (unsigned char)fgetc(fClut);
			}
		}
	}

	for (j = 0; j < 256; j ++) {
		for (k = 0; k < 256; k ++) {
			if (mode) {
				transparentTable1[j][k] = ChangeColorValues(j, (palette[k].red - palette[j].red) / 4 * 100 / 256, (palette[k].green - palette[j].green) / 4 * 100 / 256, (palette[k].blue - palette[j].blue) / 4 * 100 / 256);
				fputc(transparentTable1[j][k], fClut);
				transparentTable2[j][k] = ChangeColorValues(j, (palette[k].red - palette[j].red) / 2 * 100 / 256, (palette[k].green - palette[j].green) / 2 * 100 / 256, (palette[k].blue - palette[j].blue) / 2 * 100 / 256);
				fputc(transparentTable2[j][k], fClut);
				transparentTable3[j][k] = ChangeColorValues(j, (palette[k].red - palette[j].red) * 3 / 4 * 100 / 256, (palette[k].green - palette[j].green) * 3 / 4 * 100 / 256, (palette[k].blue - palette[j].blue) * 3 / 4 * 100 / 256);
				fputc(transparentTable3[j][k], fClut);
			}else{
				transparentTable1[j][k] = (unsigned char)fgetc(fClut);
				transparentTable2[j][k] = (unsigned char)fgetc(fClut);
				transparentTable3[j][k] = (unsigned char)fgetc(fClut);
			}

		}
	}
	
		// Calculating the lightning tables which are used to calculate the lightning around bullets
	for (j = 0; j < gConst->kLightningRadiant; j ++) {
		for (k = 0; k < 256; k ++) {
			if (mode) {
				blueLightningTable[j][k] = ChangeColorValues(k, 0, 0, (gConst->kLightningRadiant - j) * 100 / gConst->kLightningRadiant);
				fputc(blueLightningTable[j][k], fClut);
				
				redLightningTable[j][k] = ChangeColorValues(k, (gConst->kLightningRadiant - j) * 70 / gConst->kLightningRadiant, 0, 0);
				fputc(redLightningTable[j][k], fClut);
				
				greenLightningTable[j][k] = ChangeColorValues(k, 0, (gConst->kLightningRadiant - j) * 60 / gConst->kLightningRadiant, 0);
				fputc(greenLightningTable[j][k], fClut);
				
				yellowLightningTable[j][k] = ChangeColorValues(k, (gConst->kLightningRadiant - j) * 40 / gConst->kLightningRadiant, (gConst->kLightningRadiant - j) * 40 / gConst->kLightningRadiant, 0);
				fputc(yellowLightningTable[j][k], fClut);
				
				purpleLightningTable[j][k] = ChangeColorValues(k, (gConst->kLightningRadiant - j) * 40 / gConst->kLightningRadiant, 0, (gConst->kLightningRadiant - j) * 40 / gConst->kLightningRadiant);
				fputc(purpleLightningTable[j][k], fClut);

				whiteCoronaTable[j][k] = ChangeColorValues(k, (gConst->kLightningRadiant - j) * 100 / gConst->kLightningRadiant, (gConst->kLightningRadiant - j) * 100 / gConst->kLightningRadiant, (gConst->kLightningRadiant - j) * 100 / gConst->kLightningRadiant);
				fputc(whiteCoronaTable[j][k], fClut);
				
				yellowCoronaTable[j][k] = ChangeColorValues(k, (gConst->kLightningRadiant - j) * 100 / gConst->kLightningRadiant, (gConst->kLightningRadiant - j) * 100 / gConst->kLightningRadiant, (gConst->kLightningRadiant - j) * 80 / gConst->kLightningRadiant);
				fputc(yellowCoronaTable[j][k], fClut);

				blueCoronaTable[j][k] = ChangeColorValues(k, (gConst->kLightningRadiant - j) * 40 / gConst->kLightningRadiant, (gConst->kLightningRadiant - j) * 40 / gConst->kLightningRadiant, (gConst->kLightningRadiant - j) * 100 / gConst->kLightningRadiant);
			}else{
				blueLightningTable[j][k] = (unsigned char)fgetc(fClut);
				redLightningTable[j][k] = (unsigned char)fgetc(fClut);
				greenLightningTable[j][k] = (unsigned char)fgetc(fClut);
				yellowLightningTable[j][k] = (unsigned char)fgetc(fClut);
				purpleLightningTable[j][k] = (unsigned char)fgetc(fClut);
				whiteCoronaTable[j][k] = (unsigned char)fgetc(fClut);
				yellowCoronaTable[j][k] = (unsigned char)fgetc(fClut);
				//blueCoronaTable[j][k] = (unsigned char)fgetc(fClut);
			}
		}
	}

	for (j = 0; j < 256; j ++) {
		if (mode) {
			waterColorTable[j] = ChangeColorValues(j, 0, 0, kWaterTransparence);
			fputc(waterColorTable[j], fClut);
			lavaColorTable[j] = ChangeColorValues(j, kLavaTransparence, kLavaTransparence / 3, 0);
			fputc(lavaColorTable[j], fClut);
			shadowTable[j] = ChangeColorValues(j, -gConst->kShadowmodeDarkening, -gConst->kShadowmodeDarkening, -gConst->kShadowmodeDarkening);
			fputc(shadowTable[j], fClut);
			fogTable[j] = FindClosestColor(palette[j].red + (short)(double(128 - palette[j].red) * kFogFactor), palette[j].green + (short)(double(128 - palette[j].green) * kFogFactor), palette[j].blue + (short)(double(128 - palette[j].blue) * kFogFactor)); 
			fputc(fogTable[j], fClut);
		}else{
			waterColorTable[j] = (unsigned char)fgetc(fClut);
			lavaColorTable[j] = (unsigned char)fgetc(fClut);
			shadowTable[j] = (unsigned char)fgetc(fClut);
			fogTable[j] = (unsigned char)fgetc(fClut);
		}
	}

	fclose(fClut);
}

void	CClutManager::CalculateCoronas(unsigned char *bmp, short width)
{
	unsigned char	*tmp;
	short	coronaColor;
	short	value;
	unsigned char	*bitmap;

	for (short n = 0; n < kNumOfCoronas; n ++) {
		coronas[n].dx = kCoronaDescriptor[n][3];
		coronas[n].dy = kCoronaDescriptor[n][4];

		coronas[n].values = new unsigned char [coronas[n].dx * coronas[n].dy];

		coronas[n].table = 0L;
		
		bitmap = bmp + kCoronaDescriptor[n][2] * width + kCoronaDescriptor[n][1];
		for (short j = 0; j < coronas[n].dy; j ++) {
			for (short k = 0; k < coronas[n].dx; k ++) {
				if (bitmap[k] == 255) bitmap[k] = kBlackColor;
				else if (bitmap[k] == 0) bitmap[k] = kWhiteColor;

				value = (palette[bitmap[k]].red + palette[bitmap[k]].green + palette[bitmap[k]].blue) / 3;
				
				coronas[n].values[j * coronas[n].dx + k] = (unsigned char)(value * gConst->kLightningRadiant / 256);		
			}
			bitmap += width;
		}
	}
}


// --------------------------------------------------
short	CClutManager::FindClosestColor(unsigned char red, unsigned char green, unsigned char blue)
/*	In: an exact color value
	Out: color index to a color in the palette which matches the color value the exactest
*/
{
	long	distance, minDistance = LONG_MAX;
	short	n, minn = -1;

	for (n = 0; n < 256; n ++) {
		distance = (long)(palette[n].red - red) * (long)(palette[n].red - red) + 
			(long)(palette[n].green - green) * (long)(palette[n].green - green) +
			(long)(palette[n].blue - blue) * (long)(palette[n].blue - blue);
		if (distance < minDistance) {
			minDistance = distance;
			minn = n;
		}
	}

	return minn;
}

// --------------------------------------------------------------
unsigned char	CClutManager::ChangeColorValues(unsigned char color, short percentageRed, short percentageGreen, short percentageBlue)
/*	In: color which red, green and blue parts are to change
		How many percent each color has to change 100% <= x <= -100%
	Out: changed color
*/
{
	if (color == 0) color = kWhiteColor;
	if (color == 255) color = kBlackColor;

	short	red = palette[color].red, green = palette[color].green, blue = palette[color].blue;

	if (percentageRed > 0)
		red += (unsigned char)((long)(255 - (long)palette[color].red) * percentageRed / 100);
	else red -= (unsigned char)((long)(palette[color].red) * ABS(percentageRed) / 100);
	red = MAX(0, red); red = MIN(255, red);

	if (percentageGreen > 0) 
		green += (unsigned char)((long)(255 - (long)palette[color].green) * percentageGreen / 100);
	else green -= (unsigned char)((long)(palette[color].green) * ABS(percentageGreen) / 100);
	green = MAX(0, green); green = MIN(255, green);

	if (percentageBlue > 0) 
		blue += (unsigned char)((long)(255 - (long)palette[color].blue) * percentageBlue / 100);
	else blue -= (unsigned char)((long)(palette[color].blue) * ABS(percentageBlue) / 100);
	blue = MAX(0, blue); blue = MIN(255, blue);

	return (unsigned char)FindClosestColor((unsigned char)red, (unsigned char)green, (unsigned char)blue);
}


void	CClutManager::SetPixel(unsigned char *source, unsigned char *dest, short modus, short luminosity)
{
	switch (modus) {
		case kShapemodusNormal:
			*dest = luminosityTable[kMaxLuminosityLevel + luminosity][*source];
			break;
		case kShapemodusRandom:
			*dest = rand() & 255;
			break;
		case kShapemodusWater:
			*dest = waterColorTable[luminosityTable[kMaxLuminosityLevel + luminosity][*source]];
			break;
		case kShapemodusLava:
			*dest = lavaColorTable[luminosityTable[kMaxLuminosityLevel + luminosity][*source]];
			break;
		case kShapemodusShadow:
			*dest = shadowTable[luminosityTable[kMaxLuminosityLevel + luminosity][*dest]];
			break;
		case kShapemodusFog:
			*dest = fogTable[luminosityTable[kMaxLuminosityLevel + luminosity][*source]];
			break;
		case kShapemodusTransparent1:
			*dest = transparentTable1[*source][*dest];
			break;
		case kShapemodusTransparent2:
			*dest = transparentTable2[*source][*dest];
			break;
		case kShapemodusTransparent3:
			*dest = transparentTable3[*source][*dest];
			break;
		default:
			*dest = *source;
	}
}


unsigned char	**CClutManager::EffectToTable(short effect)
{
	switch (effect) {
		case kLightningBlueEffect:
			return blueLightningTable;
			break;
		case kLightningRedEffect:
			return redLightningTable;
			break;
		case kLightningGreenEffect:
			return greenLightningTable;
			break;
		case kLightningYellowEffect:
			return yellowLightningTable;
			break;
		case kLightningPurpleEffect:
			return purpleLightningTable;
			break;
		case kWhiteCoronaTable:
			return whiteCoronaTable;
			break;
		case kYellowCoronaTable:
			return yellowCoronaTable;
			break;
		case kBlueCoronaTable:
			return blueCoronaTable;
			break;
		default:
			return 0L;
	}
}


// ------------------------------------------------------------------------
void	CClutManager::DrawLightning(double xm, double ym, short effect, CGraphicSurface *surface)
// Draws a gConst->kLightningRadinant * 2 - sqare on the screen which illuminates
// the background in the color effect
{
	short	planeX, planeY;
	long	startx, starty, endx, endy, midx, midy;
	unsigned char	*baseAddr;
	short	pitch;
	short	j, k;
	CElement	*element;
	unsigned char	**table;

	if (gConfigData->disableLightning) return;

	table = EffectToTable(effect);

	gLevel->focus->CalcPlaneOffsets(planeX, planeY);
	midx = xm - planeX;
	midy = ym - planeY;

	startx = midx - gConst->kLightningRadiant; 
	startx = MAX(startx, 0);
	startx = MIN(startx, kGamePlaneWidth);

	starty = midy - gConst->kLightningRadiant;
	starty = MAX(starty, 0);
	starty = MIN(starty, kGamePlaneHeight);

	endx = midx + gConst->kLightningRadiant;
	endx = MAX(endx, 0);
	endx = MIN(endx, kGamePlaneWidth);

	endy = midy + gConst->kLightningRadiant;
	endy = MAX(endy, 0);
	endy = MIN(endy, kGamePlaneHeight);

	baseAddr = surface->GetSurfacePtr(&pitch);
	baseAddr += starty * pitch;

	for (j = starty + 1; j < endy; j ++) {
		for (k = startx + 1; k < endx; k ++) {
			element = gLevel->level[(j + planeY) / kElementSize][(k + planeX) / kElementSize];
			if (element->background && !(element->typeID & kBackgroundElement))
				baseAddr[k] = table[radiantTable[ABS(j - midy) * gConst->kLightningRadiant + ABS(k - midx)]][baseAddr[k]];
		}
		baseAddr += pitch;
	}

	surface->ReleaseSurface();
}

void	CClutManager::PrepareCorona(double xm, double ym, CGraphicSurface *surface)
{
	short	planeX, planeY;
	short	midx, midy;
	short	pitch;
	unsigned char	*baseAddr;

	gLevel->focus->CalcPlaneOffsets(planeX, planeY);
	midx = xm - planeX;
	midy = ym - planeY;

	if (midx >= 0 && midx < kGamePlaneWidth && midy >= 0 && midy < kGamePlaneHeight) {
		baseAddr = surface->GetSurfacePtr(&pitch);
		baseAddr += midy * pitch + midx;

		*baseAddr = (unsigned char)kWhiteColor;

		surface->ReleaseSurface();
	}
}



void	CClutManager::DrawCorona(double xm, double ym, short coronaNum, short effect, CGraphicSurface *surface, double &coronaFader, long deltaTime)
{
// Draws a gConst->kLightningRadinant * 2 - sqare on the screen which illuminates
// the background in the color effect
	short	planeX, planeY;
	long	startx, starty, endx, endy, midx, midy;
	short	smidx, smidy;
	unsigned char	*baseAddr, *origBaseAddr;
	short	pitch;
	short	j, k;

	if (gConfigData->disableCoronas) return;

	tCorona	*corona = &coronas[coronaNum];
	unsigned char	**table = EffectToTable(effect);

	gLevel->focus->CalcPlaneOffsets(planeX, planeY);
	midx = xm - planeX;
	midy = ym - planeY;

	smidx = corona->dx / 2;
	startx = midx - smidx; 
	startx = MAX(startx, 0);
	startx = MIN(startx, kGamePlaneWidth);

	smidy = corona->dy / 2;
	starty = midy - smidy;
	starty = MAX(starty, 0);
	starty = MIN(starty, kGamePlaneHeight);

	endx = midx + corona->dx / 2;
	endx = MAX(endx, 0);
	endx = MIN(endx, kGamePlaneWidth);

	endy = midy + corona->dy / 2;
	endy = MAX(endy, 0);
	endy = MIN(endy, kGamePlaneHeight);

	origBaseAddr = surface->GetSurfacePtr(&pitch);
	baseAddr = origBaseAddr + midy * pitch + midx;

	if (midx >= 0 && midy >= 0 && midx < kGamePlaneWidth && midy < kGamePlaneHeight && *(unsigned char *)baseAddr == kWhiteColor) {
		coronaFader += gConst->kCoronaFadeSpeed * deltaTime;
		coronaFader = MIN(1.0, coronaFader);
	}else{
		coronaFader -= gConst->kCoronaFadeSpeed * deltaTime;
		coronaFader = MAX(0, coronaFader);
	}
	
	baseAddr = origBaseAddr + starty * pitch;

	for (j = starty + 1; j < endy; j ++) {
		for (k = startx + 1; k < endx; k ++) {
			baseAddr[k] = table[gConst->kLightningRadiant - 1 - (short)((double)corona->values[(smidy + (midy - j)) * corona->dx + smidx + midx - k] * coronaFader)][baseAddr[k]];
		}
		baseAddr += pitch;
	}

	surface->ReleaseSurface();
}


void	SwapBlackWhite(Graphic_file *gf)
{
	for (long n = 0; n < gf->width * gf->height; n ++) 
		if (gf->bitmap[n] == 0) gf->bitmap[n] = kWhiteColor; else if (gf->bitmap[n] == 255) gf->bitmap[n] = kBlackColor;
}

void	CClutManager::FadeToColor(short direction, short effect, CGraphicSurface *surface)
{
	short	fadeCounter = 0;
	long	lastFadeTime = 0;
	short	pitch, height;
	unsigned char	*baseAddr;
	long	size;
	unsigned char	**table = EffectToTable(effect);
	short	j, k;
	short	border, step;

	if (direction == kFadeFromColor) {
		border = gConst->kLightningRadiant; 
		fadeCounter = 0;
		step = 1;
	}else{
		border = -1;
		fadeCounter = gConst->kLightningRadiant -1;
		step = -1;
	}
	
	while (fadeCounter != border) {
		if (lastFadeTime < gSystem->GetTicks()) {
			lastFadeTime = gSystem->GetTicks() + gConst->kFadeTime / gConst->kLightningRadiant;

			gLevel->PaintLevel();

			baseAddr = surface->GetSurfacePtr(&pitch);

			size = surface->width * surface->height;
			for (j = 0; j < surface->height; j ++) {
				for (k = 0; k < surface->width; k ++) {
					baseAddr[k] = table[fadeCounter][baseAddr[k]];
				}
				baseAddr += pitch;
			}
			surface->ReleaseSurface();
			surface->FlipToScreen(0, 0);

			fadeCounter += step;
		}
	}
}
			



