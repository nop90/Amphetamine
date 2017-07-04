#include "Appl.hpp"
#include "System.hpp"
#include "Clut.hpp"
#include "Gui.hpp"
#include "SndSys.hpp"
#include <stdio.h>

extern	CSystem		*gSystem;
extern	CLevel		*gLevel;
extern	CObjInfo	*gObjInfo;
extern	CShapeManager	*gShapeManager;
extern	CClutManager	*gClutManager;
extern	tConstValues	*gConst;
extern	CGUI			*gGUI;
extern	tGUIConstants	*gGUIConst;
extern	tConfigData		*gConfigData;
extern	CSoundSystem	*gSoundSystem;

FILE	*logFile;

void	LoadParameters();
void	LoadGUIParameters(); // in GUI.cpp
void	StartupSoundSystem();
void	ShutdownSoundSystem();

CApplication::CApplication()
{
	MSG("new CSystem\n");
	gSystem = new CSystem("Amphetamine");

	MSG("LoadParameters()\n");
	LoadParameters();

	gSystem->NewWindow(0, 0, gConfigData->screenWidth, gConfigData->screenHeight);
	
	MSG("LoadGUIParameters\n");
	LoadGUIParameters();

	MSG("new CSoundSystem\n");
	gSoundSystem = new CSoundSystem(gSystem->workingSound);
}

CApplication::~CApplication()
{
	delete gSoundSystem;

	delete gConst;
	delete gGUIConst;
	delete gConfigData;

	gSystem->DisposeWindow();
	delete gSystem;

	fclose(logFile);
}


void	CApplication::InitGraphics()
{
	MSG("gSystem->AllocateScreen\n");
	gSystem->AllocateScreen(gConfigData->screenWidth, gConfigData->screenHeight, kScreenDepth);
	
	MSG("gSystem->LoadPalette\n");
	gSystem->LoadPalette(gConst->kFilePalette);
}

void	CApplication::LoadData()
{
	long	startupTime;

	MSG("new CClutManager\n");
	gClutManager = new CClutManager();
	MSG("Loading Palette\n");
	gClutManager->LoadPalette(gSystem->palColors);
	MSG("Building Luminosity Table\n");
	gClutManager->BuildLuminosityTable();

	MSG("new CGraphicsSurface (startup)\n");
//	startup = new CGraphicSurface(kGamePlaneWidth + kUserPlaneWidth, kGamePlaneHeight);
	startup = new CGraphicSurface(kGamePlaneWidth , kGamePlaneHeight);
	MSG("startup->Insert Graphic\n");
	startup->InsertGraphic(gSystem->QualifyDataDir(gConst->kFileStartup), 0L, 0L);  // by LL
	MSG("startup->PaintGraphic\n");
	startup->PaintGraphic(0, 0, 0, kShapemodusNormal);
	MSG("startup->FlipToScreen\n");
	startup->FlipToScreen(0, 0);
	MSG("set startupTime\n");
	startupTime = gSystem->GetTicks() + gConst->kStartupTime;



	MSG("new CShapeManager\n");
	gShapeManager = new CShapeManager();
	MSG("new gShapeManager->LoadShapes\n");
	gShapeManager->LoadShapes();
	MSG("new gShapeManager->LoadBackground\n");
	gShapeManager->LoadBackground(gConst->kFileBackground1);
	lastBackground = 1;

	MSG("creating plane (new CGraphicSurface\n");
	plane = new CGraphicSurface(kGamePlaneWidth, kGamePlaneHeight);

	while (gSystem->GetTicks() < startupTime && !gSystem->KeyPressed(kKeySpace)) { gSystem->ProcessEvents(); }
	delete startup;

	MSG("new CGUI\n");
	gGUI = new CGUI(plane);

	MSG("new CObjInfo\n");
	gObjInfo = new CObjInfo(0);

	command = kCmdNewGameLevel2;


}

void	CApplication::LoadLevelData(short levelNumber)
{
	thingList = 0L;
	collisionThingList = 0L;
	bulletList = 0L;
	preRenderQueue = 0L;
	postRenderQueue = 0L;
	renderQueue = 0L;
	
	
	gObjInfo->LoadPlatforms(levelNumber);

	MSG("new CLevel\n");
	if (command >= kLoadGameSlot0 && command <= kLoadGameSlot5) {
		char *fileName = gSystem->QualifyHomeDir(gGUIConst->kSavedGames[command - kLoadGameSlot0]);
		gLevel = new CLevel(levelNumber, fileName); // by LL
		delete [] fileName;
	} else 
		gLevel = new CLevel(levelNumber, 0L);

	gGUI->SetFocus((CPlayer *)gLevel->focus);
}

void	CApplication::Run() 
{
	levelNumber = 0;

	while (command != kCmdQuit) {

		if (command >= kCmdNewGameLevel1 && command <= kCmdNewGameLevel4) {
			difficulty = command - kCmdNewGameLevel1;
			levelNumber = 0;
		}
			
		currentWeaponSF = gConst->kWeaponSF[difficulty];
		currentHealthSF = gConst->kHealthSF[difficulty];
		currentSpeedSF = gConst->kSpeedSF[difficulty];

		LoadLevelData(levelNumber);

		if (command == kCmdNextLevel) {
			if (gLevel->player->typeID & kPlayer) ((CPlayer *)gLevel->player)->RestoreDataFromNextLevel(&savedData);
			gClutManager->FadeToColor(kFadeFromColor, kWhiteCoronaTable, plane);
		}
		
		MSG("--- RunLevel() --- \n");
		RunLevel();

		if (command == kCmdNextLevel) {
			levelNumber ++; 
			gClutManager->FadeToColor(kFadeToColor, kWhiteCoronaTable, plane);
		}else if (command == kCmdPrevLevel) levelNumber --;	
		
		UnloadLevelData();

	}
}

void	CApplication::RunLevel()
{
	char	fps[5];
	long	savedTicks = 0, ticks = 0, ttmp;
	long	frameCount = 0, saveFrameCount = 0;
	tThingList	*currentEntry, *tmp;
	short	message;
	CThing	*tmpThing;
	char* fileName;

	firstPlayRound = 1;
	command = kCmdNoCommand;
	syncTime = gSystem->GetTicks();
	aveTime = 20;

	while (command == kCmdNoCommand) {
		frameCount ++;
		//usleep(1000);

		time = gSystem->GetTicks();
		deltaTime = time - syncTime;
		aveTime = (aveTime + deltaTime) / 2;
		syncTime = time;
		
		currentEntry = thingList;
		while (currentEntry) {
			message = kNoEvent;

			message |= currentEntry->thing->Think();
							
			if (message == kDestroyMe) {
				tmp = currentEntry->next;
				tmpThing = currentEntry->thing;
				tmpThing->UnlinkInLists();
				delete tmpThing;
				currentEntry = tmp;
			} else
				currentEntry = currentEntry->next;
		}

		currentEntry = thingList; 
		while (currentEntry) {
			message = kNoEvent;

			message |= currentEntry->thing->Forces();
				
			if (message == kDestroyMe) {
				tmp = currentEntry->next;
				tmpThing = currentEntry->thing;
				tmpThing->UnlinkInLists();
				delete tmpThing;
				currentEntry = tmp;
			} else
				currentEntry = currentEntry->next;
		}

		currentEntry = thingList;
		while (currentEntry) {
			currentEntry->thing->Move();
			currentEntry = currentEntry->next;
		}


		gLevel->PaintLevel();
		gGUI->DisplayMessages();

		plane->FlipToScreen(0, 0);
		firstPlayRound = 0;

		gGUI->Update();
		
		ticks = gSystem->GetTicks();
		if (gConst->kShowFPS && ticks - savedTicks > kTicksPerSecond) {
			sprintf(fps, "%02d", saveFrameCount);
			gSystem->PaintString(fps, 500, 20, 0);

			sprintf(fps, "%02d", frameCount);
			gSystem->PaintString(fps, 500, 20, 255);
			saveFrameCount = frameCount;
			frameCount = 0;
			savedTicks = ticks;
		}

		if (gSystem->KeyPressed(kKeyEscape)) {
			command = gGUI->RunUserInterface(kMainPage);
			syncTime = gSystem->GetTicks();
		}
		
		if (command >= kSaveGameSlot0 && command <= kSaveGameSlot5) {
			fileName = gSystem->QualifyHomeDir(gGUIConst->kSavedGames[command - kSaveGameSlot0]);
			gLevel->WriteLevel(fileName);  // by LL
			delete [] fileName;
			command = kCmdNoCommand;
		}
		if ((command == kCmdNextLevel) && (gLevel->player->typeID & kPlayer)) 
			((CPlayer *)(gLevel->player))->SaveDataToNextLevel(&savedData);
	}
}

void	CApplication::UnloadLevelData()
{
	CThing	*tmp; 

	while (thingList) {
		tmp = thingList->thing;
		tmp->UnlinkInLists();
		delete tmp;
	}

	delete gLevel;
}

void	CApplication::UnloadData()
{
	delete gObjInfo;
	delete gGUI;
	gShapeManager->UnloadBackground();
	gShapeManager->UnloadShapes();
	delete gShapeManager;
	delete gClutManager;
}

void	CApplication::Quit()
{
	delete plane;
	gSystem->DisposeScreen();
}

void	CApplication::Enqueue(tThingList **list, CThing *newThing)
{
	tThingList	*tmp;

	if (!newThing) return;

	tmp = new tThingList;
	tmp->next = *list;
	tmp->prev = 0L;
	tmp->thing = newThing;

	if (*list) (*list)->prev = tmp;

	*list = tmp;
}

// INVARIANT: *list->prev is always 0L, i.e. *list points to the first valid entry in the list
void	CApplication::Dequeue(tThingList **list, CThing *remove)
{
	tThingList	*tmp = *list;

	if (remove) {
		while (tmp) {
			if (tmp->thing == remove) {
				if (tmp == *list) *list = tmp->next;
				if (tmp->next) tmp->next->prev = tmp->prev;
				if (tmp->prev) tmp->prev->next = tmp->next;
				delete tmp;
				
				return;
			}
			tmp = tmp->next;
		}
	}
}


/*void	CApplication::InsertCollisionThing(CThing *newThing)
{
	if (!newThing) return;

	if (!collisionThingList) {
		newThing->nextCollisionThing = 0L;
		newThing->prevCollisionThing = 0L;
		collisionThingList = newThing; 
	} else {
		newThing->prevCollisionThing = 0L;
		newThing->nextCollisionThing = collisionThingList;
		collisionThingList->prevCollisionThing = newThing;
		collisionThingList = newThing;
	}
}

void	CApplication::RemoveCollisionThing(CThing *remove)
{
	if (collisionThingList == remove) collisionThingList = collisionThingList->nextCollisionThing;
	if (remove->nextCollisionThing) remove->nextCollisionThing->prevCollisionThing = remove->prevCollisionThing;
	if (remove->prevCollisionThing) remove->prevCollisionThing->nextCollisionThing = remove->nextCollisionThing;
}

void	CApplication::InsertBullet(CBullet *newBullet)
{
	if (!newBullet) return;

	if (!bulletList) {
		newBullet->nextBullet = 0L;
		newBullet->prevBullet = 0L;
		bulletList = newBullet; 
	} else {
		newBullet->prevBullet = 0L;
		newBullet->nextBullet = bulletList;
		bulletList->prevBullet = newBullet;
		bulletList = newBullet;
	}
}

void	CApplication::RemoveBullet(CBullet *remove)
{
	if (bulletList == remove) bulletList = bulletList->nextBullet;
	if (remove->nextBullet) remove->nextBullet->prevBullet = remove->prevBullet;
	if (remove->prevBullet) remove->prevBullet->nextBullet = remove->nextBullet;
}


void	CApplication::InsertPreRenderThing(CThing *newThing)
{
	if (!newThing) return;

	if (!preRenderQueue) {
		newThing->nextPreRenderThing = 0L;
		newThing->prevPreRenderThing = 0L;
		collisionThingList = newThing; 
	} else {
		newThing->prevPreRenderThing = 0L;
		newThing->nextPreRenderThing = preRenderQueue;
		preRenderQueue->prevPreRenderThing = newThing;
		preRenderQueue = newThing;
	}
}

void	CApplication::RemovePreRenderThing(CThing *remove)
{
	if (preRenderQueue == remove) bulletList = bulletList->nextBullet;
	if (remove->nextBullet) remove->nextBullet->prevBullet = remove->prevBullet;
	if (remove->prevBullet) remove->prevBullet->nextBullet = remove->nextBullet;
}*/