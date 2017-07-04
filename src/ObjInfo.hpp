#ifndef __AMP_THINGINFO__
#define __AMP_THINGINFO__

#include "File.hpp"
#include "Thing.hpp"

const short	kNumMonster = 12;
const short kNumItems = 42;
const short kNumWeapons = 16;
const short kNumPlatforms = 10;

struct tMonsterInfo {
	Var2Bytes	canExplode;
	Var2Bytes	moveShapes[3];
	Var2Bytes	touchBlessure;
	Var2Bytes	additionalData[2];
	Var2Bytes	jumpShape;
	Var2Bytes	attackShape;
	Var1Byte	unused;
	Var2Bytes	deathShapes[3];
			
	Var2Bytes	kind;
	Var2Bytes	speed;
	Var2Bytes	energy;
	Var2Bytes	weapon;
	Var2Bytes	aggression;
	Var2Bytes	invisible;
};

struct tItemInfo {
	Var2Bytes	iconID;
	Var2Bytes	data;
	Var2Bytes	flags;
};

struct tWeaponInfo {
	Var2Bytes	art;
	Var2Bytes	projectileShapes[2];
	Var2Bytes	detonationShapes[5];
	Var2Bytes	effect;
	Var2Bytes	speed;
	Var2Bytes	error;
	Var2Bytes	repetition;
	Var2Bytes	munition;
	Var2Bytes	rad;
	Var2Bytes	damage;
};

struct tPlatformInfo {
	Var1Byte	exists;
	Var1Byte	isElevator;
	Var2Bytes	ID;
	Var2Bytes	starty;
	Var2Bytes	startx;
	Var2Bytes	endy;
	Var2Bytes	endx;
	Var2Bytes	speed;
	Var2Bytes	delay;
	Var2Bytes	iconID;
	Var2Bytes	refNum;
	Var1Byte	playerControls;
	Var1Byte	stopsAtStart;
	Var1Byte	stopsAtEnd;
	Var1Byte	delaysBeforeStart;
	Var1Byte	hurts;
	Var1Byte	returnsOnHit;
	Var1Byte	activ;
	Var1Byte	unused;
};


class CObjInfo : public CFile {
protected:
	void	LoadMonsters(short levNum);
	void	LoadItems(short levNum);
	void	LoadWeapons(short levNum);

public:

	tMonsterInfo*	monsterInfo[kNumMonster];
	tItemInfo*		itemInfo[kNumItems];
	tWeaponInfo*	weaponInfo[kNumWeapons];

	tPlatformInfo*	platformInfo[kNumPlatforms];
	
	CObjInfo(short levNum);
	~CObjInfo();

	void	LoadPlatforms(short levNum);

	CThing	*CreateMonster(short monsterNum, short j, short k);
	CThing	*CreateItem(short itemNum, short j, short k);

	void	CreatePlatforms();
	short	GetMonsterType(short ref);
	tItemInfo	*GetItemInfo(short ref);
	tWeaponInfo	*FindWeapon(short ref);
};

#endif