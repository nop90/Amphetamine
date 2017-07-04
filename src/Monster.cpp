#include "Monster.hpp"
#include "Appl.hpp"
#include "ConstVal.hpp"

extern CApplication		*gApplication;
extern	CShapeManager	*gShapeManager;
extern	CObjInfo		*gObjInfo;
extern	CLevel			*gLevel;
extern	tConstValues	*gConst;

CMonster::CMonster(short initx, short inity, short width, short height, short number, tMonsterInfo *monsterInfo) : 
	CThing(initx, inity, width, height, number)
{
	typeID |= kMonster;
	background = 0;
	LinkInLists();

	if (monsterInfo) {
		info = monsterInfo;

		OnAllocate();

		lastWalkTime = 0;
		lastShootTime = 0;
		walkFrame = 0;
		flying = 0;
		teleportCounter = -1;
		lastBlessureTime = 0;
		dieFrameTime = 0;
		dieFrame = -1;
		speed = info->speed * gApplication->currentSpeedSF;
	}

	// additinal init
	lastCollisionCode = kNoCollision;
	status = kMonsterForward;
	doDarken = 0;
}

void	CMonster::OnAllocate()
{
	for (short n = 0; n < 3; n ++) {
		moveShapes[n] = gShapeManager->FindShape(info->moveShapes[n], 0);
		deathShapes[n] = gShapeManager->FindShape(info->deathShapes[n], 0);
	}
	jumpShape = gShapeManager->FindShape(info->jumpShape, 0);
	attackShape = gShapeManager->FindShape(info->attackShape, 0);

	weapon = CreateWeapon(info->weapon);
	currentShape = moveShapes[0];

	health = info->energy * gApplication->currentHealthSF;
}


void	CMonster::LinkInLists()
{
	gApplication->Enqueue(&gApplication->thingList, this);
	gApplication->Enqueue(&gApplication->collisionThingList, this);
	gApplication->Enqueue(&gApplication->renderQueue, this);
}

void	CMonster::UnlinkInLists()
{
	gApplication->Dequeue(&gApplication->thingList, this);
	gApplication->Dequeue(&gApplication->collisionThingList, this);
	gApplication->Dequeue(&gApplication->renderQueue, this);
}


CMonster::~CMonster()
{
	if (weapon) delete weapon;
}


CWeapon	*CMonster::CreateWeapon(short weapon)
{
	tWeaponInfo	*tmp = gObjInfo->FindWeapon(weapon);
	CWeapon		*tmpWeapon;

	noWeapon = 0;

	switch (tmp->art) {
	case kWeaponMultibullet:
		tmpWeapon = new CMultiBulletWeapon(this, tmp, attackShape, gConst->kFirehandAngle);
		break;
	case kWeaponInHand:
		tmpWeapon = new CWeapon(this, tmp, attackShape);
		noWeapon = 1;
		break;
	case kWeaponSine:
		tmpWeapon =  new CSineWeapon(this, tmp, attackShape, gConst->kSineWeaponRad);
		break;
	case kWeaponGuided:
		tmpWeapon =  new CGuided(this, tmp, attackShape);
		break;
	case kWeaponSorcery:
		tmpWeapon = new CSorcery(this, tmp, attackShape);
		break;
	case kWeaponNormal:
	case kWeaponHasWeight:
	default:
		tmpWeapon =  new CWeapon(this, tmp, attackShape);
		break;
	}
	if (tmpWeapon) {
		tmpWeapon->weaponStatus = kWeaponReady;
		tmpWeapon->AddMunition(SHRT_MAX);
		tmpWeapon->weaponNumber = weapon;
	}
	return tmpWeapon;
}

void	CMonster::Move()
{

	CThing::Move();

	if (ys >= kLevelHeight * kElementSize) {
		health = 0;
		OnKill();
	}

	if (info->invisible) modus = kShapemodusShadow;

	currentShape = moveShapes[walkFrame];

	if (forceVectorX && lastTime > lastWalkTime ) {
		walkFrame ++;
		walkFrame %= 3;
		lastWalkTime = lastTime + gConst->kWalkFrameTime;
	}
	
	if (forceVectorY < 0) currentShape = jumpShape;

	if (dieFrame != -1) {
		if (dieFrameTime < lastTime && dieFrame < 2) {
			dieFrame ++;
			dieFrameTime = lastTime + gConst->kDieFrameTime;

			if (dieFrame == 2) 
				gApplication->Dequeue(&gApplication->collisionThingList, this);
		}
		currentShape = deathShapes[dieFrame];
	}

	if (weapon) doDarken = weapon->ShootAnimation(&currentShape);
}

short	CMonster::Forces()
{
	short	collisionObject;


	Gravitation();
	
	CObject::Forces();

	lastCollisionCode = ExertForce(resForceX, resForceY, collisionObject, 0L);
	if (lastCollisionCode & (kCollisionOnLeft | kCollisionOnRight)) forceVectorX = 0;

	return kNoEvent;
}

// --------------------------------------------------------------------
void	CMonster::Render(short planeX, short planeY, tRect *clipRect)
// Renders the current Shape of the monster
{
	CElement	*element = gLevel->GetElement(xm, ym);
	short		screenPosx = xs - planeX, screenPosy = ys - planeY; // Position of monster on the screen
	short		brightness = (element && doDarken) ? element->brightness : 0;

// Teleporting...
	if (teleportCounter != -1) {

		if (teleportCounter > dx / 2) {
			clipRect->left = MAX(screenPosx + dx - teleportCounter, clipRect->left);
			clipRect->right = MIN(screenPosx + teleportCounter, clipRect->right);
		}else{
			clipRect->left = MAX(screenPosx + teleportCounter, clipRect->left);
			clipRect->right = MIN(screenPosx + (xe - planeX) - teleportCounter, clipRect->right);
		}

		if (teleportDeltaTime < lastTime) {
			teleportCounter ++;
			teleportDeltaTime = lastTime + gConst->kTeleportTime / dx * 2;
		}

		if (teleportCounter == dx / 2) OnTeleport();
		if (teleportCounter == dx) teleportCounter = -1;

		modus = kShapemodusRandom;
	}

// Testing for blessure invulnerability
	if (lastBlessureTime > lastTime) modus = kShapemodusRandom;


	if (lookDirection == kLookingLeft) modus |= kShapemodusBackwardFlag; 

	if (currentShape) currentShape->RenderShape(screenPosx, screenPosy, clipRect, 
		modus, brightness, gApplication->plane);
}

short	CMonster::Collision(CObject *sender, double left, double top, double right, double bottom, double &forcex, double &forcey, double pfx, double pfy, short sourceWeight, short &collisionObject)
{
	short	returnValue = CThing::Collision(sender, left, top, right, bottom, forcex, forcey, pfx, pfy, sourceWeight, collisionObject); 
	if (returnValue && sender->typeID & kPlayer) {
		if (dieFrame == -1) ((CThing *)sender)->OnDamage(info->touchBlessure);
		if (info->canExplode && dieFrame == -1) OnKill();
	}

	return returnValue;
}

short	CMonster::Think()
{
	tThingList	*bulletEntry;
	short	elemx, elemy;
	double	directionx, directiony;

	CObject::Think();

	if (dieFrame != -1) return kNoEvent;
	if (gApplication->firstPlayRound) OnStart();
	
	if (ABS(FindPlayerX()) > gConst->kActivateDistance) return kNoEvent;

	// Checking for a treatment event
	bulletEntry = gApplication->bulletList;
	while (bulletEntry) {
		if (((CBullet *)(bulletEntry->thing))->AmIATreatment(xs, ys, xe, ye, directionx, directiony)) {
			OnTreatment(directionx, directiony);
			bulletEntry = 0L;
		}else bulletEntry = bulletEntry->next;
	}	

	// Checking for an abyss event
	if (forceVectorX > 0) {
		elemx = (short)(xe + forceVectorX) / kElementSize;
		elemy = (short)ym / kElementSize;
	}else{
		elemx = (short)(xs + forceVectorX) / kElementSize;
		elemy = (short)ym / kElementSize;
	}
	if ((lastCollisionCode & kCollisionOnBottom) && elemx > 0 && elemy > 0 && elemx < kLevelWidth && elemy + 1 < kLevelHeight) {
		if (gLevel->level[elemy][elemx]->background &&
			gLevel->level[elemy + 1][elemx]->background) OnAbyss();
	}

	// Checking for a collision event
	if (lastCollisionCode & ~kCollisionOnBottom) OnCollision();

	// Checking for a landing event
	if (flying && lastCollisionCode & kCollisionOnBottom) OnLanding();
	flying = !(lastCollisionCode & kCollisionOnBottom);

	// Sending an idle event
	OnIdle();

	return kNoEvent;
}

void	CMonster::OnStart() {}
void	CMonster::OnAbyss() {}
void	CMonster::OnCollision() {}
void	CMonster::OnLanding() {}
void	CMonster::OnIdle() {}
void	CMonster::OnTreatment(double directionx, double directiony) {}
void	CMonster::OnShootSuccessful() {}
void	CMonster::OnShootNotSuccessful() {}
void	CMonster::OnTeleport() {}


// ---------------------------------------------
void	CMonster::OnDamage(short blessure)
// Event occurs, when a monster is hit by something (bullet etc.)
{
	if (dieFrame == -1 && lastBlessureTime < lastTime) {
		health -= blessure;
		if (health < 0) 
			OnKill();
		else
			lastBlessureTime = lastTime + gConst->kBlessureInvulnerabilityTime;
	}
}


void	CMonster::OnKill()
{
	dieFrameTime = lastTime + gConst->kDieFrameTime;

	dieFrame = 0;
}

void	CMonster::OnTouch(CObject *touch)
{
	if (dieFrame == -1) ((CThing *)touch)->OnDamage(info->touchBlessure);
}


double	CMonster::FindPlayerX()
{
	return gLevel->player->xm - xm;
}

double	CMonster::FindPlayerY()
{
	return gLevel->player->ym - ym;
}


short	CMonster::Write(FILE *f)
{
	long	size = 0;

	WRITEDATA(size);
	WRITEDATA(typeID);
	WRITEDATA(thingNumber);

	size += CThing::Write(f);
	if (weapon) size += weapon->Write(f);

	WRITEDATA(speed);
	WRITEDATA(lastWalkTime);
	WRITEDATA(lastShootTime);
	WRITEDATA(walkFrame);
	WRITEDATA(status);
	WRITEDATA(lastCollisionCode);
	WRITEDATA(flying);
	WRITEDATA(teleportDeltaTime);
	WRITEDATA(teleportCounter);
	WRITEDATA(lastBlessureTime);
	WRITEDATA(dieFrameTime);
	WRITEDATA(dieFrame);
	WRITEDATA(doDarken);
	WRITEDATA(health);

	FINISHWRITE;

	return size;
}

void	CMonster::Read(FILE *f)
{
	long	size = 0;

	READDATA(size);
	READDATA(typeID);
	READDATA(thingNumber);

	CThing::Read(f);

	OnAllocate();

	if (weapon) weapon->Read(f);

	READDATA(speed);
	READDATA(lastWalkTime);
	READDATA(lastShootTime);
	READDATA(walkFrame);
	READDATA(status);
	READDATA(lastCollisionCode);
	READDATA(flying);
	READDATA(teleportDeltaTime);
	READDATA(teleportCounter);
	READDATA(lastBlessureTime);
	READDATA(dieFrameTime);
	READDATA(dieFrame);
	READDATA(doDarken);
	READDATA(health);

	if (dieFrame != -1) gApplication->Dequeue(&gApplication->collisionThingList, this);
}