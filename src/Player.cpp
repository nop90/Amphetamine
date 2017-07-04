#include "Player.hpp"
#include "ShapeDes.hpp"
#include "Appl.hpp"
#include "ConstVal.hpp"
#include "Gui.hpp"
#include "Item.hpp"
#include "SndSys.hpp"

extern	CApplication *gApplication;
extern	CSystem			*gSystem;
extern	CShapeManager	*gShapeManager;
extern	CObjInfo		*gObjInfo;
extern	CLevel			*gLevel;
extern	tConstValues	*gConst;
extern	CGUI			*gGUI;
extern	tConfigData		*gConfigData;
extern	CSoundSystem	*gSoundSystem;

CPlayer::CPlayer(short initx, short inity, short width, short height, short number, tMonsterInfo *monsterInfo) : 
	CMonster(initx, inity, width, height, number, monsterInfo)
{
	short	n;

	typeID |= kPlayer;

	xs += (dx - gConst->kPlayerWidth) / 2;
	xe -= (dx - gConst->kPlayerWidth) / 2;
	ys += 2;

	weight = 2;
	jumpTime = LONG_MAX;
	jumped = 0;

	for (short m = 0; m < 3; m ++) {
		playerMoveShapes[0][m] = moveShapes[m];
		
	}
	attackShapes[0][0] = gShapeManager->FindShape(504, 0);
	attackShapes[0][1] = attackShapes[0][2] = 0L;
	jumpShapes[0] = jumpShape;

	for (n = 1; n < 8; n ++) {
		for (short m = 0; m < 3; m ++) {
			playerMoveShapes[n][m] = gShapeManager->FindShape(500 + n * 10 + m, 0);
			attackShapes[n][m] = gShapeManager->FindShape(504 + n * 10 + m, 0);
		}
		jumpShapes[n] = gShapeManager->FindShape(503 + n * 10, 0);
	}
	
	unused = weapon;
	weapons[0] = new CHandWeapon(this, gObjInfo->FindWeapon(0), attackShapes[0][0], attackShapes[0][1], attackShapes[0][2]);
	weapons[1] = new CWeapon(this, gObjInfo->FindWeapon(1), attackShapes[1][0]);
	weapons[2] = new CSorcery(this, gObjInfo->FindWeapon(2), attackShapes[2][0]);
	weapons[3] = new CWeapon(this, gObjInfo->FindWeapon(3), attackShapes[3][0]);
	weapons[4] = new CHandWeapon(this, gObjInfo->FindWeapon(4), attackShapes[4][0], attackShapes[4][1], attackShapes[4][2]);
	weapons[5] = new CMultiBulletWeapon(this, gObjInfo->FindWeapon(5), attackShapes[5][0], gConst->kFirehandAngle);
	weapons[6] = new CBomb(this, gObjInfo->FindWeapon(6), attackShapes[6][0], attackShapes[3][1]);
	weapons[7] = new CStaff(this, gObjInfo->FindWeapon(7), attackShapes[7][0], attackShapes[7][1]);
	weapon = weapons[0];

	for (n = 0; n <= 7; n ++) weapons[n]->weaponNumber = n;

	teleportCounter = -1;
	lastOxygenDecTime = 0;
	
	lastWeaponChanging = 0;  // by LL
	lastActionTime = 0; // by LL
	// teleportRefNum = 0;   ? by LL
	lastOxygenDecTime = 0; // by LL
	
	lastPickupTime = 0;

	currentWeapon = 0;
	weapons[0]->weaponStatus = kWeaponReady;
	oxygen = gConst->kInitialOxygen;
}


CPlayer::~CPlayer()
{
	for (short n = 1; n < 8; n ++) delete weapons[n];
}

short	CPlayer::Think()
{
	CElement	*element;
	short		action;
	short		liquid;
	double		currentSpeed;

	CObject::Think();
	if (gApplication->firstPlayRound) OnStart();

	Gravitation();

	element = gLevel->GetElement(xm, ym);
	if (element) liquid = element->GetElementLiquid(); else liquid = kShapemodusNormal;

	if (dieFrame == -1) {
		currentSpeed = (gSystem->KeyPressed(gConfigData->runKey) && lastCollisionCode & kCollisionOnBottom ? info->speed * gConst->kRunScaleFactor : info->speed) * gConst->kVelocityUnit * deltaTime;

		if (gSystem->KeyPressed(gConfigData->leftKey)) {
			if (forceVectorX > -currentSpeed) forceVectorX -= gConst->kPlayerAcceleration * deltaTime;
			lookDirection = kLookingLeft;
		}
		if (gSystem->KeyPressed(gConfigData->rightKey)) {
			if (forceVectorX < currentSpeed) forceVectorX += gConst->kPlayerAcceleration * deltaTime;
			lookDirection = kLookingRight;
		}
		if (gSystem->KeyPressed(gConfigData->jumpKey)) {
			if (liquid == kShapemodusWater || liquid == kShapemodusLava) {
				if (forceVectorY > -info->speed * gConst->kVelocityUnit * deltaTime) forceVectorY = -gConst->kPlayerLiquidAccel * deltaTime;
				if (!gSoundSystem->playerDive->isPlaying()) gSoundSystem->Play(gSoundSystem->playerDive, xm, ym);
			}else if (jumpTime > lastTime){
				forceVectorY = -gConst->kJumpVelocity * gConst->kVelocityUnit * deltaTime;
				if (!jumped) gSoundSystem->Play(gSoundSystem->playerJump, xm, ym);
				jumped = 1;
			}
		}else jumpTime = 0;

		if (gSystem->KeyPressed(gConfigData->shootKey)) {
			weapons[currentWeapon]->Shoot(lookDirection == kLookingRight ? 1 : -1, 0, 0);
			gGUI->UpdateWeapon();
		}
		if (gSystem->KeyPressed(gConfigData->prevWeaponKey) && lastTime > lastWeaponChanging) {
			do {
				currentWeapon ++;
				if (currentWeapon > 7) currentWeapon = 0;
			} while (weapons[currentWeapon]->weaponStatus != kWeaponReady);

			lastWeaponChanging = lastTime + gConst->kWeaponChangeTime;
			gGUI->UpdateWeapon();
		}
		
		if (gSystem->KeyPressed(gConfigData->nextWeaponKey) && lastTime > lastWeaponChanging) {
			do { 
				currentWeapon --;
				if (currentWeapon < 0) currentWeapon = 7;
			} while (weapons[currentWeapon]->weaponStatus != kWeaponReady);
			
			lastWeaponChanging = lastTime + gConst->kWeaponChangeTime;
			gGUI->UpdateWeapon();
		}

		if (gSystem->KeyPressed(gConfigData->activateKey) && lastActionTime < lastTime) {
			PerformAction();
			lastActionTime = lastTime + gConst->kActionDelayTime;
		}


		if (gSystem->KeyPressed(gConfigData->weapon1Key) && weapons[0]->weaponStatus == kWeaponReady && lastTime > lastWeaponChanging) {
			currentWeapon = 0;
			gGUI->UpdateWeapon();
			lastWeaponChanging = lastTime + gConst->kWeaponChangeTime;
		}
		if (gSystem->KeyPressed(gConfigData->weapon2Key) && weapons[1]->weaponStatus == kWeaponReady && lastTime > lastWeaponChanging) {
			currentWeapon = 1;
			gGUI->UpdateWeapon();
			lastWeaponChanging = lastTime + gConst->kWeaponChangeTime;
		}
		if (gSystem->KeyPressed(gConfigData->weapon3Key) && weapons[2]->weaponStatus == kWeaponReady && lastTime > lastWeaponChanging) {
			currentWeapon = 2;
			gGUI->UpdateWeapon();
			lastWeaponChanging = lastTime + gConst->kWeaponChangeTime;
		}
		if (gSystem->KeyPressed(gConfigData->weapon4Key) && weapons[3]->weaponStatus == kWeaponReady && lastTime > lastWeaponChanging) {
			currentWeapon = 3;
			gGUI->UpdateWeapon();
			lastWeaponChanging = lastTime + gConst->kWeaponChangeTime;
		}
		if (gSystem->KeyPressed(gConfigData->weapon5Key) && weapons[4]->weaponStatus == kWeaponReady && lastTime > lastWeaponChanging) {
			currentWeapon = 4;
			gGUI->UpdateWeapon();
			lastWeaponChanging = lastTime + gConst->kWeaponChangeTime;
		}
		if (gSystem->KeyPressed(gConfigData->weapon6Key) && weapons[5]->weaponStatus == kWeaponReady && lastTime > lastWeaponChanging) {
			currentWeapon = 5;
			gGUI->UpdateWeapon();
			lastWeaponChanging = lastTime + gConst->kWeaponChangeTime;
		}
		if (gSystem->KeyPressed(gConfigData->weapon7Key) && weapons[6]->weaponStatus == kWeaponReady && lastTime > lastWeaponChanging) {
			currentWeapon = 6;
			gGUI->UpdateWeapon();
			lastWeaponChanging = lastTime + gConst->kWeaponChangeTime;
		}
		if (gSystem->KeyPressed(gConfigData->weapon8Key) && weapons[7]->weaponStatus == kWeaponReady && lastTime > lastWeaponChanging) {
			currentWeapon = 7;
			gGUI->UpdateWeapon();
			lastWeaponChanging = lastTime + gConst->kWeaponChangeTime;
		}

		if (liquid == kShapemodusWater || liquid == kShapemodusLava) {	
			oxygen -= gConst->kOxygenDecrease * deltaTime;
		}
		if (liquid == kShapemodusLava) OnDamage(gConst->kLavaDamage);
	}
	
	if (liquid == kShapemodusWater || liquid == kShapemodusLava) {
		forceVectorX *= gConst->kLiquidFriction;
		forceVectorY *= gConst->kLiquidFriction;
	}

	if (element) element->PassiveAction();

	return kNoEvent;
}

// ------------------------------------------------
void	CPlayer::PerformAction()
// Performs the action which is returned by CElement::Action
{
	short	action = 0;
	CElement	*element = gLevel->GetElement(xm, ym);

	if (element) action = element->Action();

	if (action & kTeleportMask) {	// Player used a teleporter
		teleportCounter = 0;
		teleportRefNum = element->refNum;
		teleportDeltaTime = lastTime;
		currentJob = kJobTeleporting;
	}
	if (action & kExitMask) gApplication->command = kCmdNextLevel;
}

short	CPlayer::Forces()
{
	short	collisionObject, collisionCode;

	CObject::Forces();

	lastCollisionCode = ExertForce(resForceX, resForceY, collisionObject, 0L);
	if (lastCollisionCode & kCollisionOnBottom) {
		jumpTime = lastTime + gConst->kJumpAccelerationTime;
		jumped = 0;
	}
	if (lastCollisionCode & kCollisionOnTop) {
		jumpTime = 0;
		forceVectorY = 0;
	}
	
	if (lastCollisionCode & (kCollisionOnLeft | kCollisionOnRight)) forceVectorX = 0;

	return kNoEvent;
}

void	CPlayer::Move()
{
	moveShapes[0] = playerMoveShapes[currentWeapon][0];
	moveShapes[1] = playerMoveShapes[currentWeapon][1];
	moveShapes[2] = playerMoveShapes[currentWeapon][2];
	jumpShape = jumpShapes[currentWeapon];
	weapon = weapons[currentWeapon];

	CMonster::Move();
}

void	CPlayer::OnStart()
{
	gGUI->UpdateWeapon();
}

// ------------------------------------
void	CPlayer::OnTeleport()
// Teleport event, sent by CMonster::Render
{
	if (currentJob == kJobTeleporting) {
		CElement	*element = gLevel->FindRefnum(teleportRefNum);

		if (element) {
			element->Teleport(xs, ys);
			xe = xs + dx; ye = ys + dy;
		}
	}
}

void	CPlayer::GetItAll()
// cheat
{
	int i;

	for (i = 0; i < 7; ++i) {
		//if (weapons[i]->weaponStatus != kWeaponReady) currentWeapon = i;
		weapons[i]->weaponStatus = kWeaponReady;
		weapons[i]->AddMunition(100);

		oxygen += 100;
		oxygen = MIN(oxygen, gConst->kInitialOxygen);

		health += 100;
		health = MIN(health, info->energy);
	}
}

void	CPlayer::OnTouch(CObject *touch)
{
	short	value;
	short	what;

	if (touch->typeID & kPortableItem) {
		what = ((CPortableItem *)touch)->PickMeUp(value);
		gSoundSystem->Play(gSoundSystem->playerPickup, xm, ym);
		switch (what) {
			case kItemSword:
				if (weapons[0]->weaponStatus != kWeaponReady) currentWeapon = 0;
				weapons[0]->weaponStatus = kWeaponReady;
				weapons[0]->AddMunition(0);
				break;

			case kItemPhiol:
				if (weapons[1]->weaponStatus != kWeaponReady) currentWeapon = 1;
				weapons[1]->weaponStatus = kWeaponReady;
				weapons[1]->AddMunition(gObjInfo->GetItemInfo(kItemPhiolmun)->data);
				break;

			case kItemSorcery:
				if (weapons[2]->weaponStatus != kWeaponReady) currentWeapon = 2;
				weapons[2]->weaponStatus = kWeaponReady;
				weapons[2]->AddMunition(gObjInfo->GetItemInfo(kItemSorcerymun)->data);
				break;

			case kItemBow:
				if (weapons[3]->weaponStatus != kWeaponReady) currentWeapon = 3;
				weapons[3]->weaponStatus = kWeaponReady;
				weapons[3]->AddMunition(gObjInfo->GetItemInfo(kItemBowmun)->data);
				break;

			case kItemScie:
				if (weapons[4]->weaponStatus != kWeaponReady) currentWeapon = 4;
				weapons[4]->weaponStatus = kWeaponReady;
				weapons[4]->AddMunition(0);
				break;

			case kItemHands:
				if (weapons[5]->weaponStatus != kWeaponReady) currentWeapon = 5;
				weapons[5]->weaponStatus = kWeaponReady;
				weapons[5]->AddMunition(gObjInfo->GetItemInfo(kItemHandsmun)->data);
				break;

			case kItemBomb:
				if (weapons[6]->weaponStatus != kWeaponReady) currentWeapon = 6;
				weapons[6]->weaponStatus = kWeaponReady;
				weapons[6]->AddMunition(gObjInfo->GetItemInfo(kItemBombmun)->data);
				break;

			case kItemStaff:
				if (weapons[7]->weaponStatus != kWeaponReady) currentWeapon = 7;
				weapons[7]->weaponStatus = kWeaponReady;
				weapons[7]->AddMunition(gObjInfo->GetItemInfo(kItemStaffmun)->data);
				break;

			case kItemPhiolmun:
				weapons[1]->AddMunition(value);
				break;

			case kItemSorcerymun:
				weapons[2]->AddMunition(value);
				break;
	
			case kItemBowmun:
				weapons[3]->AddMunition(value);
				break;

			case kItemSciemun:
				weapons[4]->AddMunition(value);
				break;

			case kItemHandsmun:
				weapons[5]->AddMunition(value);
				break;

			case kItemBombmun:
				weapons[6]->AddMunition(value);
				break;

			case kItemStaffmun:
				weapons[7]->AddMunition(value);
				break;

			case kItemOxygen:
				oxygen += value;
				oxygen = MIN(oxygen, gConst->kInitialOxygen);
				break;

			case kItemHelppacket:
				health += value;
				health = MIN(health, info->energy);
				break;
		}
		lastPickupTime = lastTime + gConst->kPickupTime;
		gGUI->UpdateWeapon();
		gGUI->OnPickUpSomething(what);
	}
}

void	CPlayer::OnDamage(short blessure)
{
	gSoundSystem->Play(gSoundSystem->firestoneDamage, xm, ym);
	CMonster::OnDamage(blessure);
}

void	CPlayer::SaveDataToNextLevel(tPlayerData *playerData)
{
	for (short n = 0; n <= 7; n ++) weapons[n]->SaveDataToNextLevel(&(playerData->munition[n]), &(playerData->weaponStatus[n]));

	playerData->currentWeapon = currentWeapon;
	playerData->health = health;
	playerData->oxygen = oxygen;
}

void	CPlayer::RestoreDataFromNextLevel(tPlayerData *savedData)
{
	currentWeapon = savedData->currentWeapon;
	oxygen = savedData->oxygen;
	health = savedData->health;
	for (short n = 0; n <= 7; n ++) {
		weapons[n]->RestoreDataFromPrevLevel(savedData->munition[n], savedData->weaponStatus[n]);
	}
}

CCamera::CCamera(short initx, short inity, short width, short height, short thingNumber) : CThing(initx, inity, width, height, thingNumber)
{
	typeID |= kCamera;

	weightless = 1;

	gApplication->Enqueue(&gApplication->thingList, this);
	gApplication->Enqueue(&gApplication->renderQueue, this);
}

CCamera::~CCamera()
{
	gApplication->Dequeue(&gApplication->thingList, this);
	gApplication->Dequeue(&gApplication->renderQueue, this);
}

short	CCamera::Forces()
{
	if (xm < kLevelWidth * kElementSize - kGamePlaneWidth / 2)
		forceVectorX = gConst->kCameraSpeed * deltaTime * gConst->kVelocityUnit;
	else forceVectorX = 0;

	return kNoEvent;
}

short	CCamera::Think()
{
	CElement	*element;

	CObject::Think();

	element = gLevel->GetElement(xm, ym);
	if (element && element->GetElementLiquid() == kShapemodusFog) {
		currentMessage = gConst->kMonsterNames[element->data -1];
		msgY = kGamePlaneHeight;
	}

	msgY -= gConst->kMonsterNameSpeed * deltaTime * gConst->kVelocityUnit;

	return kNoEvent;
}

void	CCamera::Move()
{
	xm += forceVectorX;
}

void	CCamera::Render(short planeX, short planeY, tRect *clipRect)
{
	if (msgY > -kElementSize) gApplication->plane->DrawString(gConst->kMonsterNameX, (short)msgY, currentMessage, kShapemodusNormal);
}

short	CPlayer::Write(FILE *f)
{
	long	size = 0;

	WRITEDATA(size);
	WRITEDATA(typeID);
	WRITEDATA(thingNumber);

	weapon = unused;
	size += CMonster::Write(f);
	weapon = unused;
	
	for (short n = 0; n < 8; n ++) {
		size += weapons[n]->Write(f);
	}

	WRITEDATA(jumpTime);
	WRITEDATA(currentWeapon);
	WRITEDATA(lastWeaponChanging);
	WRITEDATA(lastPickupTime);
	WRITEDATA(lastActionTime);
	WRITEDATA(teleportRefNum);
	WRITEDATA(currentJob);

	WRITEDATA(oxygen);
	WRITEDATA(lastOxygenDecTime);

	FINISHWRITE;

	return size;
}

void	CPlayer::Read(FILE *f)
{
	long	size = 0;


	READDATA(size);
	READDATA(typeID);
	READDATA(thingNumber);

	CMonster::Read(f);
	
	for (short n = 0; n < 8; n ++) {
		weapons[n]->Read(f);
	}

	READDATA(jumpTime);
	READDATA(currentWeapon);
	READDATA(lastWeaponChanging);
	READDATA(lastPickupTime);
	READDATA(lastActionTime);
	READDATA(teleportRefNum);
	READDATA(currentJob);

	READDATA(oxygen);
	READDATA(lastOxygenDecTime);
}