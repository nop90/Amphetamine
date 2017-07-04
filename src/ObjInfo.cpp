#include "ObjInfo.hpp"
#include "Monster.hpp"
#include "Monstrxx.hpp"
#include "Player.hpp"
#include "Item.hpp"
#include "ShapeDes.hpp"
#include "Appl.hpp"
#include "Pltform.hpp"
#include "ConstVal.hpp"
#include "System.hpp"
#include <stdio.h>

extern	CApplication	*gApplication;
extern	tConstValues	*gConst;
extern	CSystem			*gSystem;
extern	FILE			*logFile;

short	*FindDescriptor(short id);

CObjInfo::CObjInfo(short levelNumber)
{
	MSG("LoadMonsters\n");
	LoadMonsters(levelNumber);
	MSG("LoadItems\n");
	LoadItems(levelNumber);
	MSG("LoadWeapons\n");
	LoadWeapons(levelNumber);
	for (short n = 0; n < kNumPlatforms; n ++) platformInfo[n] = new tPlatformInfo;
}

CObjInfo::~CObjInfo()
{
	short	n;


	for (n = 0; n < kNumMonster; n ++) delete monsterInfo[n];
	for (n = 0; n < kNumItems; n ++) delete itemInfo[n];
	for (n = 0; n < kNumWeapons; n ++) delete weaponInfo[n];
	for (n = 0; n < kNumPlatforms; n ++) delete platformInfo[n];
}


void	CObjInfo::LoadMonsters(short levelNumber)
{
	if (!OpenDataFile(gSystem->QualifyDataDir(gConst->kFileMonster))) gSystem->Error("Cannot find file kFileMonster or unable to open it", 0);  // by LL
	SetFilePos(levelNumber * kNumMonster * sizeof(tMonsterInfo));

	for (short n = 0; n < kNumMonster; n ++) {
		monsterInfo[n] = new tMonsterInfo;
		ReadData(monsterInfo[n], sizeof(tMonsterInfo));

		TRANS_PUT_NUM(monsterInfo[n]->canExplode);
		for (short m = 0; m < 3; m ++) {
			TRANS_PUT_NUM(monsterInfo[n]->moveShapes[m]);
			TRANS_PUT_NUM(monsterInfo[n]->additionalData[m]);
			TRANS_PUT_NUM(monsterInfo[n]->deathShapes[m]);
		}
		TRANS_PUT_NUM(monsterInfo[n]->touchBlessure);
		TRANS_PUT_NUM(monsterInfo[n]->additionalData[0]);
		TRANS_PUT_NUM(monsterInfo[n]->additionalData[1]);
		//TRANS_PUT_NUM(monsterInfo[n]->jumpShape);
		TRANS_PUT_NUM(monsterInfo[n]->attackShape);
		TRANS_PUT_NUM(monsterInfo[n]->unused);
		TRANS_PUT_NUM(monsterInfo[n]->kind);
		TRANS_PUT_NUM(monsterInfo[n]->speed);
		TRANS_PUT_NUM(monsterInfo[n]->energy);
		TRANS_PUT_NUM(monsterInfo[n]->weapon);
		TRANS_PUT_NUM(monsterInfo[n]->aggression);
		TRANS_PUT_NUM(monsterInfo[n]->invisible);
	}

	CloseDataFile();
}


void	CObjInfo::LoadItems(short levelNumber)
{
	if (!OpenDataFile(gSystem->QualifyDataDir(gConst->kFileObjects))) gSystem->Error("Cannot find file kFileObjects or uable to open it", 0);  // by LL
	SetFilePos(levelNumber * kNumItems * sizeof(tItemInfo));

	for (short n = 0; n < kNumItems; n ++) {
		itemInfo[n] = new tItemInfo;
		ReadData(itemInfo[n], sizeof(tItemInfo));

		TRANS_PUT_NUM(itemInfo[n]->iconID);
		TRANS_PUT_NUM(itemInfo[n]->data);
		TRANS_PUT_NUM(itemInfo[n]->flags);
	}
	
	CloseDataFile();
}

void	CObjInfo::LoadWeapons(short levelNumber)
{
	if (!OpenDataFile(gSystem->QualifyDataDir(gConst->kFileWeapon))) gSystem->Error("Cannot find file kFileWeapon or unable to open it", 0);  // by LL

	SetFilePos(levelNumber * kNumWeapons * sizeof(tWeaponInfo));

	for (short n = 0; n < kNumWeapons; n ++) {
		weaponInfo[n] = new tWeaponInfo;
		ReadData(weaponInfo[n], sizeof(tWeaponInfo));

		TRANS_PUT_NUM(weaponInfo[n]->art);
		TRANS_PUT_NUM(weaponInfo[n]->projectileShapes[0]);
		TRANS_PUT_NUM(weaponInfo[n]->projectileShapes[1]);
		for (short m = 0; m < 5; m ++) {
			TRANS_PUT_NUM(weaponInfo[n]->detonationShapes[m]);
		}
		TRANS_PUT_NUM(weaponInfo[n]->effect);
		TRANS_PUT_NUM(weaponInfo[n]->speed);
		TRANS_PUT_NUM(weaponInfo[n]->error);
		TRANS_PUT_NUM(weaponInfo[n]->repetition);
		TRANS_PUT_NUM(weaponInfo[n]->munition);
		TRANS_PUT_NUM(weaponInfo[n]->rad);
		TRANS_PUT_NUM(weaponInfo[n]->damage);
	}

	CloseDataFile();
}

void	CObjInfo::LoadPlatforms(short levelNumber)
{
	if (!OpenDataFile(gSystem->QualifyDataDir(gConst->kFilePlatform))) gSystem->Error("Cannot find file kFilePlatform or unable to open it", 0);  // by LL
	SetFilePos(levelNumber * kNumPlatforms * sizeof(tPlatformInfo));

	for (short n = 0; n < kNumPlatforms; n ++) {
		ReadData(platformInfo[n], sizeof(tPlatformInfo));

		TRANS_PUT_NUM2(platformInfo[n]->ID);
		TRANS_PUT_NUM2(platformInfo[n]->startx);
		TRANS_PUT_NUM2(platformInfo[n]->starty);
		TRANS_PUT_NUM2(platformInfo[n]->endx);
		TRANS_PUT_NUM2(platformInfo[n]->endy);
		TRANS_PUT_NUM2(platformInfo[n]->speed);
		TRANS_PUT_NUM2(platformInfo[n]->delay);
		TRANS_PUT_NUM2(platformInfo[n]->iconID);
		TRANS_PUT_NUM2(platformInfo[n]->refNum);
		
		//for (short m = 0; m < 8; m ++) TRANS_PUT_NUM2(platformInfo[n]->vars[m]);
	}
}


short	*FindDescriptor(short id)
{
	short n;
	for (n = 0; n < kNumShapes && id != kShapeDescriptor[n][0]; n++) {}
	if (n < kNumShapes) return kShapeDescriptor[n];
	else{
		for (n = 0; n < kNumTextures && id != kTextureDescriptor[n][0]; n++) {}
		if (n < kNumTextures) return kTextureDescriptor[n]; else return 0L;
	}
}

//-----------------------------------------------------------------
CThing *CObjInfo::CreateMonster(short monsterNo, short j, short k)
/*	In: monsterNo: Number of monster in monster info table
		j, k: position in level elements
	Out: new monster object

  Creates a new monster object at position j, k (in level elements)
*/
{
	short	*descriptor;

	if (monsterNo != -1) {
		descriptor = FindDescriptor(monsterInfo[monsterNo]->moveShapes[0]);

		switch (monsterInfo[monsterNo]->kind) {
			case kClassPlayer: return new CPlayer(j * kElementSize + kElementSize / 2, k * kElementSize + kElementSize / 2, descriptor[3], descriptor[4], monsterNo, monsterInfo[monsterNo]); break;
			case kClassCreeper: return new CCreeper(j * kElementSize + kElementSize / 2, k * kElementSize + kElementSize / 2, descriptor[3], descriptor[4], monsterNo, monsterInfo[monsterNo]); break;
			case kClassWalker: return new CWalker(j * kElementSize + kElementSize / 2, k * kElementSize + kElementSize / 2, descriptor[3], descriptor[4], monsterNo, monsterInfo[monsterNo]); break;
			case kClassJumper: return new CJumper(j * kElementSize + kElementSize / 2, k * kElementSize + kElementSize / 2, descriptor[3], descriptor[4], monsterNo, monsterInfo[monsterNo]); break;
			case kClassFlyer: return new CFlyer(j * kElementSize + kElementSize / 2, k * kElementSize + kElementSize / 2, descriptor[3], descriptor[4], monsterNo, monsterInfo[monsterNo]); break;
			case kClassWarg: return new CWarg(j * kElementSize + kElementSize / 2, k * kElementSize + kElementSize / 2, descriptor[3], descriptor[4], monsterNo, monsterInfo[monsterNo]); break;
		}
	}
	return 0L;
}

//------------------------------------------------------------------
CThing *CObjInfo::CreateItem(short itemNo, short j, short k)
/*	In: itemNo: Number of item in item info record
		j, k: position in level elements
	Out: new item object
*/
{
	short	*descriptor;

	if (itemNo >= 0 && itemNo < kNumItems) {
		descriptor = FindDescriptor(itemInfo[itemNo]->iconID);
		switch (itemInfo[itemNo]->flags & 3) {
			case kClassBackgroundItem: return new CBackgroundItem(j * kElementSize + kElementSize / 2, k * kElementSize + kElementSize / 2, descriptor[3], descriptor[4], itemNo, itemInfo[itemNo]); break;
			case kClassUnpassableItem: return new CStaticItem(j * kElementSize + kElementSize / 2, k * kElementSize + kElementSize / 2, descriptor[3], descriptor[4], itemNo, itemInfo[itemNo]); break;
			case kClassMovableItem: return new CMovableItem(j * kElementSize + kElementSize / 2, k * kElementSize + kElementSize / 2, descriptor[3], descriptor[4], itemNo, itemInfo[itemNo]); break;
			case kClassPortableItem: return new CPortableItem(j * kElementSize + kElementSize / 2, k * kElementSize + kElementSize / 2, descriptor[3], descriptor[4], itemNo, itemInfo[itemNo], itemNo); break;
		}
	}
	return 0L;
}

void	CObjInfo::CreatePlatforms()
{
	short	*descriptor;
	CThing	*temp;

	for (short n = 0; n < kNumPlatforms; n ++) {
		if (platformInfo[n]->exists) {
			descriptor = FindDescriptor(platformInfo[n]->iconID);
			temp = new CPlatform(platformInfo[n]->startx * kElementSize + kElementSize / 2, platformInfo[n]->starty * kElementSize + kElementSize / 2, descriptor[3], descriptor[4], n, platformInfo[n]);
			gApplication->platformTable[n] = (CPlatform *)temp;
		}else gApplication->platformTable[n] = 0L;
	}
}

short	CObjInfo::GetMonsterType(short ref)
{
	return (ref == -1 ? -1 : monsterInfo[ref]->kind);
}

tItemInfo	*CObjInfo::GetItemInfo(short ref)
{
	return (ref == -1 ? 0L : itemInfo[ref]);
}

tWeaponInfo	*CObjInfo::FindWeapon(short ref)
{
	return weaponInfo[ref];
}
