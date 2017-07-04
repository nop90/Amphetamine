#include <stdio.h>
#include <string.h>
#include <time.h>

#include "Level.hpp"
#include "Appl.hpp"
#include "ConstVal.hpp"
#include "Gui.hpp"
#include "ShapeLd.hpp"
 
extern	CApplication *gApplication;
extern	CObjInfo	*gObjInfo;
extern	tConstValues	*gConst;
extern	CSystem		*gSystem;
extern	CShapeManager	*gShapeManager;
extern	tGUIConstants	*gGUIConst;
extern	CGUI		*gGUI;
extern	FILE		*logFile;

//----------------------------------------------------
CLevel::CLevel(short levelNumber, char *fileName)
/*	In: levelNumber: number of level to be loaded

	First loads the level data from file, then generating each level element object, and if there are monsters
	or items on the element, generate them
	Then create platforms
*/
{
	if (fileName) {

		ReadLevel(fileName);

	}else{
		short	j, k;
		tLevelElement	*levelElement = new tLevelElement;
		CThing	*thing;

		LoadBackground(levelNumber);
		
		gObjInfo->CreatePlatforms();

		if (!OpenDataFile(gSystem->QualifyDataDir(gConst->kFileLevel))) gSystem->Error("File kFileLevel not found or not able to open it", 0);   // by LL

		SetFilePos(levelNumber * kLevelWidth * kLevelHeight * sizeof(tLevelElement));

		MSG("Creating level\n");
		for (j = 0; j < kLevelHeight; j ++) {
			for (k = 0; k < kLevelWidth; k ++) {
				ReadData(levelElement, sizeof(tLevelElement));

				TRANS_PUT_NUM(levelElement->kind);
				TRANS_PUT_NUM(levelElement->iconID);
				TRANS_PUT_NUM(levelElement->light);
				TRANS_PUT_NUM(levelElement->refNum);
				TRANS_PUT_NUM(levelElement->bitData);
				TRANS_PUT_NUM(levelElement->monsterRef);
				TRANS_PUT_NUM(levelElement->itemRef);

				if (levelElement->iconID > 0) levelElement->iconID += gConst->kShapeSets[levelNumber] * 20;

				if (levelElement->iconID)
					level[j][k] = new CElement(k * kElementSize + kElementSize / 2, j * kElementSize + kElementSize / 2, kElementSize, kElementSize, levelElement);
				else level[j][k] = new CBackgroundElement(k * kElementSize + kElementSize / 2, j * kElementSize + kElementSize / 2, kElementSize, kElementSize, levelElement);
				
				level[j][k]->LinkPlatforms();

				if (levelElement->monsterRef != -1) {
					if (levelElement->monsterRef == kCameraNo)
						thing = new CCamera(k * kElementSize, j * kElementSize, 2, 2, 0);
					else 
						thing = gObjInfo->CreateMonster(levelElement->monsterRef, k, j);


					if (thing->typeID & (kPlayer | kCamera)) {
						focus = thing;
						player = thing;
					}
				}

				if (levelElement->itemRef != -1) {
					thing = gObjInfo->CreateItem(levelElement->itemRef, k, j);
				}
				
			}
		}

		delete levelElement;

		CloseDataFile();

		gSystem->ResetTicks(0);
		gGUI->ResetTicks(0);
	}
}


CLevel::~CLevel()
{
	short	j, k;

	for (j = 0; j < kLevelHeight; j ++) {
		for (k = 0; k < kLevelWidth; k ++) {
			delete level[j][k];
		}
	}
}

void	CLevel::LoadBackground(short levelNumber)
{	
	if (gConst->kBackgrounds[levelNumber] != gApplication->lastBackground) {
		gShapeManager->UnloadBackground();

		if (gConst->kBackgrounds[levelNumber] == 1)
			gShapeManager->LoadBackground(gConst->kFileBackground1);
		else
			gShapeManager->LoadBackground(gConst->kFileBackground2);
		gApplication->lastBackground = gConst->kBackgrounds[levelNumber];
	}
}

void	CLevel::PaintLevel()
{
	short	j, k;
	short	planeX, planeY;
	short	startElementX, startElementY, numElementsX, numElementsY;
	tRect	clipRect;
	tThingList	*currentEntry;

	focus->CalcPlaneOffsets(planeX, planeY);

	clipRect.left = 0; 
	clipRect.top = 0;
	clipRect.right = kGamePlaneWidth;
	clipRect.bottom = kGamePlaneHeight;

	startElementX = planeX / kElementSize;
	startElementY = planeY / kElementSize;
	numElementsX = kGamePlaneWidth / kElementSize;
	numElementsY = kGamePlaneHeight / kElementSize;

	for (j = startElementY; j < startElementY + numElementsY + 1 && j < kLevelHeight; j ++) {
		for (k = startElementX; k < startElementX + numElementsX + 1 && k < kLevelWidth; k ++) {
			level[j][k]->PaintElement(planeX, planeY, &clipRect);
		}
	}
	currentEntry = gApplication->preRenderQueue;
	while (currentEntry) {
		currentEntry->thing->Render(planeX, planeY, &clipRect);
		currentEntry = currentEntry->next;
	}

	currentEntry = gApplication->renderQueue; 
	while (currentEntry) {
		currentEntry->thing->Render(planeX, planeY, &clipRect);
		currentEntry = currentEntry->next;
	}

	currentEntry = gApplication->postRenderQueue; 
	while (currentEntry) {
		currentEntry->thing->PostRender(planeX, planeY, &clipRect);
		currentEntry = currentEntry->next;
	}
}


CElement	*CLevel::GetElement(double x, double y)
{
	short	elemx, elemy;

	elemx = (short)(x / kElementSize);
	elemy = (short)(y / kElementSize);

	if (elemx >= 0 && elemy >= 0 && elemx < kLevelWidth && elemy < kLevelHeight)
		return level[elemy][elemx];
	else return 0L;
}

// -----------------------------------------------
void	CLevel::SwitchLight(short lightID)
// Searches all elements with key equal to key, and then change their
// lights. Don't call this routine too often, it's slow
{
	short	j, k;
													
	for (j = 0; j < kLevelHeight; j ++) {
		for (k = 0; k < kLevelWidth; k ++) {
			if (level[j][k]->key == lightID) level[j][k]->SwapLights();
		}
	}
}

// -----------------------------------------------
CElement	*CLevel::FindRefnum(short refnum)
// Finds the first element with key == refnum and returns it
{
	short	j, k;
													
	for (j = 0; j < kLevelHeight; j ++) {
		for (k = 0; k < kLevelWidth; k ++) {
			if (level[j][k]->key == refnum) return level[j][k];
		}
	}
	return 0L;
}


void	CLevel::WriteLevel(char *fileName)
{
	char	tmpName[12] = "tmp.sg0";
	long	versionNumber = kVersionNumber;
	long	key = kVersionKey;
	time_t	theTime;
	long	ticks;
	long	size;
	tThingList	*currentEntry;
	char *tmpFileName;

	long	typeID;
	short	thingNumber;

	tmpFileName = gSystem->QualifyHomeDir(tmpName);
	remove(tmpFileName);
	FILE *f = fopen(tmpFileName, "wb");  // by LL	
	if (!f) {
		gSystem->Error("Cannot open the saving file", 0);
	}
	
	MSG("Writing save file header\n");
	fwrite(&versionNumber, sizeof(versionNumber), 1, f);
	fwrite(&key, sizeof(key), 1, f);
	theTime = time(&theTime);
	fwrite(&theTime, sizeof(theTime), 1, f);
	ticks = gSystem->GetTicks();
	fwrite(&ticks, sizeof(ticks), 1, f);
	fwrite(&gApplication->difficulty, sizeof(gApplication->difficulty), 1, f);
	size = 0;
	fwrite(&size, sizeof(size), 1, f);
	fwrite(&gApplication->levelNumber, sizeof(gApplication->levelNumber), 1, f);

	MSG("Writing level elements\n");
	for (short j = 0; j < kLevelHeight; j ++) {
		for (short k = 0; k < kLevelWidth; k ++) {
			typeID = level[j][k]->typeID;
			fwrite(&typeID, sizeof(typeID), 1, f);
			level[j][k]->Write(f);
		}
	}

	MSG("Writing objects\n");
	currentEntry = gApplication->thingList;
	while (currentEntry) {
		typeID = currentEntry->thing->typeID;
		fwrite(&typeID, sizeof(typeID), 1, f);
		thingNumber = currentEntry->thing->thingNumber;
		fwrite(&thingNumber, sizeof(thingNumber), 1, f);
		currentEntry->thing->Write(f);
		currentEntry = currentEntry->next;
	}

	size = ftell(f);
	fseek(f, sizeof(versionNumber) + sizeof(key) + sizeof(theTime) + sizeof(ticks) + sizeof(gApplication->difficulty), SEEK_SET);
	fwrite(&size, sizeof(size), 1, f);

	MSG("Finishing up\n");
	fflush(f);
	fclose(f);

	remove(fileName);
	rename(tmpFileName, fileName);
	delete [] tmpFileName;
}

void	CLevel::GetSavedGameTitle(char *fileName, char *title)
{


	long	versionNumber, key;
	time_t	time;
	FILE	*f = fopen(fileName, "rb");
	tm		*tp;

	if (f) fread(&versionNumber, sizeof(versionNumber), 1, f);
	if (f) fread(&key, sizeof(key), 1, f);

	if (!f || versionNumber != kVersionNumber || key != kVersionKey) 
		title = strcpy(title, gGUIConst->kNoSavedGame);
	else {
		fread(&time, sizeof(time), 1, f);
		tp = localtime(&time);
		strftime(title, 20, "%d.%m.%Y, %H.%M", tp);
	}

	if (f) fclose(f);
}

short	CLevel::ReadLevel(char *fileName)
{
	FILE	*f = fopen(fileName, "rb");
	long	versionNumber, key;
	time_t	theTime;
	long	ticks;
	long	size, fileSize;
	long	typeID = -1;
	CThing	*thing;
	short	thingNumber;
	short	j, k;

	if (!f) gSystem->Error("Cannot open the saving file", 0);

	MSG("Reading level header\n");
	fread(&versionNumber, sizeof(versionNumber), 1, f); if (versionNumber != kVersionNumber) return 0;
	fread(&key, sizeof(key), 1, f); if (key != kVersionKey) return 0;
	fread(&theTime, sizeof(theTime), 1, f);
	fread(&ticks, sizeof(ticks), 1, f);
	fread(&gApplication->difficulty, sizeof(gApplication->difficulty), 1, f);
	fread(&fileSize, sizeof(fileSize), 1, f);
	fread(&gApplication->levelNumber, sizeof(gApplication->levelNumber), 1, f);

	LoadBackground(gApplication->levelNumber);

	MSG("Reading level elements\n");
	for (j = 0; j < kLevelHeight; j ++) {
		for (k = 0; k < kLevelWidth; k ++) {
//			fread(&size, sizeof(size), 1, f);
			fread(&typeID, sizeof(typeID), 1, f);
//			fseek(f, - (sizeof(size) + sizeof(typeID)), SEEK_CUR);

			if (typeID & kBackgroundElement)
				level[j][k] = new CBackgroundElement(k * kElementSize, j * kElementSize, kElementSize, kElementSize, 0L);
			else
				level[j][k] = new CElement(k * kElementSize, j * kElementSize, kElementSize, kElementSize, 0L);
			
			level[j][k]->Read(f);
		}
	}


	MSG("Reading objects\n");
	while (!feof(f) && ftell(f) < fileSize) {


//		fread(&size, sizeof(size), 1, f);
		fread(&typeID, sizeof(typeID), 1, f);
		fread(&thingNumber, sizeof(thingNumber), 1, f);
//		fseek(f, - (sizeof(size) + sizeof(typeID) + sizeof(thingNumber)), SEEK_CUR);

		
		if (typeID & kMonster) {
			thing = gObjInfo->CreateMonster(thingNumber, 0, 0);

			if (typeID & kPlayer) {
				focus = thing;
				player = thing;
			}
			
			thing->Read(f);
		}

		if (typeID & kItem) {
			thing = gObjInfo->CreateItem(thingNumber, 0, 0);

			thing->Read(f);
		}

		if (typeID & kBullet) {

			if (typeID & kSorceryBullet) 
				thing = new CSorceryBullet(0, 0, 0, 0, 0, 0L, 0, 0, 0L, 0);
			else if (typeID & kBombBullet) 
				thing = new CBombBullet(0, 0, 0, 0, 0, 0L, 0, 0, 0L, 0);
			else if (typeID & kSineBullet)
				thing = new CSineBullet(0, 0, 0, 0, 0, 0L, 0, 0, 0L, 0, 0, 0);
			else if (typeID & kGuidedBullet)
				thing = new CGuidedBullet(0, 0, 0, 0, 0, 0L, 0, 0, 0L, 0, 0L);
			else 
				thing = new CBullet(0, 0, 0, 0, 0, 0L, 0, 0, 0L, 0);


			
			thing->Read(f);
		}

		if (typeID & kPlatform) {
			thing = new CPlatform(0, 0, 0, 0, 0, 0L);

			thing->Read(f);

			gApplication->platformTable[thingNumber] = (CPlatform *)thing;
		}
	}

	MSG("Finishing up\n");
	fflush(f);
	fclose(f);

	for (j = 0; j < kLevelHeight; j ++) {
		for (k = 0; k < kLevelWidth; k ++) {
			level[j][k]->LinkPlatforms();
		}
	}


	gSystem->ResetTicks(ticks);
	gGUI->ResetTicks(ticks);

	return 1;
}

