#ifndef __AMP_APPL__
#define __AMP_APPL__

#include "AmpHead.hpp"
#include "System.hpp"
#include "Level.hpp"
#include "ObjInfo.hpp"
#include "Bullet.hpp"
#include "ConstVal.hpp"

enum {	// commands
	kCmdNoCommand,
	kCmdNextLevel,
	kCmdPrevLevel,
	kCmdQuit,
	kCmdNewGameLevel1,
	kCmdNewGameLevel2,
	kCmdNewGameLevel3,
	kCmdNewGameLevel4,
	kSaveGameSlot0,
	kSaveGameSlot1,
	kSaveGameSlot2,
	kSaveGameSlot3,
	kSaveGameSlot4,
	kSaveGameSlot5,

	kLoadGameSlot0,
	kLoadGameSlot1,
	kLoadGameSlot2,
	kLoadGameSlot3,
	kLoadGameSlot4,
	kLoadGameSlot5
};

struct tThingList {
	tThingList	*next, *prev;
	CThing		*thing;
};

struct tPlayerData {
	short	munition[8];
	short	weaponStatus[8];

	short	currentWeapon;
	short	oxygen;
	short	health;
};

class CApplication {
protected:
	tPlayerData	savedData;
	CGraphicSurface	*startup;

public:
	tThingList	*thingList;
	tThingList	*collisionThingList;
	tThingList	*bulletList;
	tThingList	*preRenderQueue;
	tThingList	*postRenderQueue;
	tThingList	*renderQueue;

	CGraphicSurface	*plane;

	CPlatform	*platformTable[kNumPlatforms];
	short	lastBackground;

	short	firstPlayRound;
	long	syncTime;	// current time in ticks, but only once each play round measured
	short	command;
	short	difficulty;
	double	currentWeaponSF, currentHealthSF, currentSpeedSF;
	short	levelNumber;
	long	time;
	long	deltaTime;
	long	aveTime;

	CApplication();
	~CApplication();
	
	void	InitGraphics();
	void	LoadData();
	void	LoadLevelData(short levelNumber);
	void	Run();
	void	RunLevel();
	void	UnloadLevelData();
	void	UnloadData();
	void	Quit();

	void	Enqueue(tThingList **list, CThing *newThing);
	void	Dequeue(tThingList **list, CThing *remove);

	/*void	InsertThing(CThing *newThing);
	void	RemoveThing(CThing *remove);
	void	InsertCollisionThing(CThing *newThing);
	void	RemoveCollisionThing(CThing *remove);
	void	InsertBullet(CBullet *newBullet);
	void	RemoveBullet(CBullet *remove);
	void	InsertPreRenderThing(CThing *newThing);
	void	RemovePreRenderThing(CThing *remove);
	void	InsertPostRenderThing(CThing *newThing);
	void	RemovePostRenderThing(CThing *remove);*/

};

#endif