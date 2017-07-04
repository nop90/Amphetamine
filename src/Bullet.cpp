#include "Bullet.hpp"
#include "Monster.hpp"
#include <math.h>
#include "ShapeLd.hpp"
#include "Appl.hpp"
#include "Clut.hpp"
#include "SndSys.hpp"

extern	CShapeManager	*gShapeManager;
extern	CApplication	*gApplication;
extern	CSystem			*gSystem;
extern	tConstValues	*gConst;
extern	CLevel			*gLevel;
extern	CClutManager	*gClutManager;
extern	CObjInfo		*gObjInfo;
extern	CSoundSystem	*gSoundSystem;

CBullet::CBullet(short initx, short inity, short width, short height, short number, tWeaponInfo *weaponInfo, double directionx, double directiony, CThing *source, double targetpos) : CThing(initx, inity, width, height, number)
{
	typeID |= kBullet;
	LinkInLists();

	if (weaponInfo) {
		info = weaponInfo;
		weight = info->art == kWeaponHasWeight ? 10 : 0;
		weightless = 0;
		background = 0;
		/*xs = xm - gConst->kBulletWidth / 2;
		ys = ym - gConst->kBulletWidth / 2;
		xe = xm + gConst->kBulletWidth / 2;
		ye = ym + gConst->kBulletWidth / 2;*/

		OnAllocate();

		shooter = source;
		targetposition = targetpos;

		if (projectile2) {
			for (short n = 0; n < gConst->kBulletTailLength; n ++) {
				tailX[n] = -1;
				tailY[n] = -1;
			}
			tailStep = 0;
		}
		
		directionUnitx = directionx * gConst->kVelocityUnit / sqrt(directionx * directionx + directiony * directiony);
		directionUnity = directiony * gConst->kVelocityUnit / sqrt(directionx * directionx + directiony * directiony);

		action = kInFlight;

		forceVectorX = 0;
		forceVectorY = 0;
		resForceX = resForceY = 0;

		lastTime = gSystem->GetTicks();
		deltaTime = 0;
	}
}

void	CBullet::OnAllocate()
{
	projectile1 = gShapeManager->FindShape(info->projectileShapes[0], 0);
	projectile2 = gShapeManager->FindShape(info->projectileShapes[1], 0);
	
	if (projectile2) {
		tailX = new short [gConst->kBulletTailLength];
		tailY = new short [gConst->kBulletTailLength];
	}else{
		tailX = tailY = 0L;
	}
			
	currentDetonationShape = 0L;

	for (short n = 0; n < 5; n ++) detonation[n] = gShapeManager->FindShape(info->detonationShapes[n], 0);
}

CBullet::~CBullet()
{
	if (projectile2) {
		delete [] tailX;
		delete [] tailY;
	}
}

void	CBullet::LinkInLists()
{
	gApplication->Enqueue(&gApplication->thingList, this);
	gApplication->Enqueue(&gApplication->bulletList, this);
	gApplication->Enqueue(&gApplication->renderQueue, this);
}

void	CBullet::UnlinkInLists()
{
	gApplication->Dequeue(&gApplication->thingList, this);
	gApplication->Dequeue(&gApplication->bulletList, this);
	gApplication->Dequeue(&gApplication->renderQueue, this);
}

short	CBullet::Think()
{
	CObject::Think();

	Gravitation();
	
	if (directionUnitx) forceVectorX = directionUnitx * info->speed * deltaTime;
	if (directionUnity) forceVectorY = directionUnity * info->speed * deltaTime;

	return kNoEvent;
}

short	CBullet::Forces()
{
	short	collisionCode, collisionObject;
	tThingList	*entry;
	CElement	*element;

	if (action == kInFlight) {
		CObject::Forces();
		//collisionCode = ExertForce(resForceX, resForceY, collisionObject, &obstacle);

		if (xm < 0 || ym < 0 || xm > kLevelWidth * kElementSize + kElementSize || ym > kLevelHeight * kElementSize + kElementSize) {
			Detonate(kCollisionWithLevelBorders, 0L);
			return kNoEvent;
		}

		element = gLevel->GetElement(xm, ym);
		if (element && !element->background) {
			Detonate(0, 0L);
			return kNoEvent;
		}

		entry = gApplication->collisionThingList;
		while (entry) {
			if (entry->thing != this && entry->thing != shooter && !(entry->thing->typeID & (kBullet | kStaticItem | kPortableItem)) && entry->thing->CollisionPossible(xm, ym)) {
				Detonate(0, entry->thing);
				entry = 0L;
			}else entry = entry->next;
		}			

		return kNoEvent;

	}else if (action == kInDetonation) {
		return DetonationAnimation();
	}

	return kNoEvent;
}


void	CBullet::Detonate(short collisionCode, CObject *victim)
{
    action = kInDetonation;
	detonationStartTime = lastTime;

	noDetonation = collisionCode == kCollisionWithLevelBorders;

	Damage(victim);

	gSoundSystem->Play(gSoundSystem->weaponHitSounds[thingNumber], xm, ym);

	if (shooter && !(shooter->typeID & kPlayer)) {
		if ((directionUnitx > 0 && xe >= targetposition) || (directionUnitx < 0 && xs <= targetposition)) 
			((CMonster *)shooter)->OnShootSuccessful();
		else ((CMonster *)shooter)->OnShootNotSuccessful();
	}
}


short	CBullet::DetonationAnimation()
{
	long	frame = (long)((double)(lastTime - detonationStartTime) * gConst->kDetonationFrameTime);
		
	if (frame > 4 || noDetonation) {
		currentDetonationShape = 0L;
		return kDestroyMe;

	}else{
		currentDetonationShape = detonation[frame];
		return kNoEvent;
	}
}

// ---------------------------------------
void	CBullet::Damage(CObject *victim)
// called from CBullet::DetonationAnimation
// sends a TestForDamage-Event to all collision objects
{
	if (info->rad) {
		tThingList	*currentEntry = gApplication->collisionThingList;

		while (currentEntry) {
			if (currentEntry->thing != this) currentEntry->thing->TestForDamage(xs, ys, info->rad, info->damage);
			currentEntry = currentEntry->next;
		}
	}else{
		if (victim && victim != shooter && victim->typeID & kThing) ((CThing *)victim)->OnDamage(info->damage);
	}
}


void	CBullet::Move()
{
	if (action == kInFlight) CThing::Move();

	if (projectile2) {
		if (tailStep > gConst->kBulletTailDistance) {
			for (short n = gConst->kBulletTailLength -2; n >= 0; n --) {
				tailX[n + 1] = tailX[n];
				tailY[n + 1] = tailY[n];
			}
			tailX[0] = (short)xs;
			tailY[0] = (short)ys;
			tailStep = 0;
		}else tailStep += gConst->kVelocityUnit * info->speed * deltaTime;
	}
}

void	CBullet::Render(short planeX, short planeY, tRect *clipRect)
{
	short	tailMode;
	if (info->effect != kLightningNoEffect) gClutManager->DrawLightning(xm, ym, info->effect, gApplication->plane);

	if (action == kInFlight) {
		if (resForceX < 0) modus |= kShapemodusBackwardFlag;
		if (projectile1) projectile1->RenderShape(xs - planeX, ys - planeY, clipRect, 
			modus, 0, gApplication->plane);
	}else{
		if (currentDetonationShape) currentDetonationShape->RenderShape(xs - planeX, ys - planeY, clipRect, 
			modus, 0, gApplication->plane);
	}
	if (projectile2) {
		for (short n = 0; n < gConst->kBulletTailLength; n ++) {
			if (n < gConst->kBulletTailLength / 3) tailMode = kShapemodusTransparent1; 
			else if (n > gConst->kBulletTailLength * 2 / 3) tailMode = kShapemodusTransparent3;
			else tailMode = kShapemodusTransparent2;

			if (tailX[n] != -1) projectile2->RenderShape(tailX[n] + (short)xs - tailX[0] - planeX, tailY[n] + (short)ys - tailY[0] - planeY, clipRect, tailMode, 0, gApplication->plane);
		}
	}
}

short	CBullet::Collision(CObject *sender, double left, double top, double right, double bottom, double &forcex, double &forcey, double pfx, double pfy, short sourceWeight, short &collisionObject)
{
	if (action != kInDetonation && 
		((top >= ys && top <= ye) || (bottom >= ys && bottom <= ye)) && 
		sender != shooter && !(sender->typeID & kBullet)) {
		if (left >= xs && left <= xe) Detonate(kCollisionOnRight, sender); 
		if (right >= xs && right <= xe) Detonate(kCollisionOnLeft, sender);
	}
	return kNoCollision;
}


short	CBullet::AmIATreatment(double victimxs, double victimys, double victimxe, double victimye, double &fx, double &fy)
{
	double	coll;

	if ((xm - victimxs) * (xm - victimxs) + (ym - victimys) * (ym - victimys) < gConst->kTreatDistance * gConst->kTreatDistance && 
		SIGN(victimxs - xs) == SIGN(resForceX)) {
 	 	fx = resForceX; fy = resForceY;

		if (ABS(resForceX) > ABS(resForceY)) {
			coll = ym + resForceY * (victimxs - xm) / resForceX;
			if (coll > victimys - (ye - ys) && coll < victimye + (ye - ys)) return 1; else return 0;
		}else{
			coll = xm + resForceX * (victimys - ym) / resForceY;
			if (coll > victimxs - (xe - xs) && coll < victimxe + (xe - xs)) return 1; else return 0;
		}
	}else return 0;
}


CSorceryBullet::CSorceryBullet(short initx, short inity, short width, short height, short number, tWeaponInfo *weaponInfo, double directionx, double directiony, CThing *source, double targetpos) : CBullet(initx, inity, width, height, number, weaponInfo, directionx, directiony, source, targetpos)
{
	typeID |= kSorceryBullet;
	numBounces = 0;
}

CSorceryBullet::~CSorceryBullet() {}

short	CSorceryBullet::Forces()
{
	short	collisionCode, collisionObject;
	double	savedForcex = forceVectorX, savedForcey = forceVectorY;
	CObject	*collObj;

	if (action == kInFlight) {
		CObject::Forces();

		collisionCode = ExertForce(resForceX, resForceY, collisionObject, &collObj);
		if (collisionCode && collObj != shooter) {
			if (collisionObject & (kElement | kItem | kPlatform) || collObj == shooter) {
			
				if (numBounces < gConst->kNumOfBounces) {

					numBounces ++;

					if (collisionCode & kCollisionOnTop || collisionCode & kCollisionOnBottom) {
						forceVectorY = -savedForcey;
						directionUnity *= -1.0;
					}
					if (collisionCode & kCollisionOnLeft || collisionCode & kCollisionOnRight) {
						forceVectorX = -savedForcex;
						directionUnitx *= -1.0;
					}
				}else{
					Detonate(collisionCode, collObj);
				}
			}else{
				Detonate(collisionCode, collObj);
			}
		}
	}else if (action == kInDetonation) {
		return DetonationAnimation();
	}

	return kNoEvent;
}


CBombBullet::CBombBullet(short initx, short inity, short width, short height, short number, tWeaponInfo *weaponInfo, double directionx, double directiony, CThing *source, double targetpos) : CBullet(initx, inity, width, height, number, weaponInfo, directionx, directiony, source, targetpos)
{
	weight = 5;
	action = kInFlight;
	detonate = 0;
}

CBombBullet::~CBombBullet() {}

short	CBombBullet::Forces()
{
	short	collisionObject, returnCode, collisionCode;
	CObject	*obstacle;
	short	dx, dy;

	if (action != kInDetonation) {
		CObject::Forces();

		collisionCode = ExertForce(resForceX, resForceY, collisionObject, &obstacle);
		if (collisionCode) {
			if (collisionObject == kMonster) detonate = 1;
		}
		if (detonate == 1) {
			Detonate(collisionCode, obstacle);
			if (detonation[0]) detonation[0]->AllowPixelAccess(dx, dy);
			ys = ye - dy; ye = ys + dy; ym = (ye - ys) / 2;
			detonate = 2;
		}
	}else return DetonationAnimation();

	return kNoEvent;
}

CSineBullet::CSineBullet(short initx, short inity, short width, short height, short number, tWeaponInfo *weaponInfo, double directionx, double directiony, CThing *source, double targetpos, short rad, short per) : CBullet(initx, inity, width, height, number, weaponInfo, directionx, directiony, source, targetpos)
{
	typeID |= kSineBullet;

	distance = 0;
	radius = rad;
	period = 2.0 * 3.141 / (double)per;
}

CSineBullet::~CSineBullet() {}

short	CSineBullet::Think()
{
	CObject::Think();

	Gravitation();

	forceVectorX = gConst->kVelocityUnit * info->speed * deltaTime * SIGN(directionUnitx);
	distance += forceVectorX;
	forceVectorY = -radius * period * cos(distance * period);

	return kNoEvent;
}


CGuidedBullet::CGuidedBullet(short initx, short inity, short width, short height, short number, tWeaponInfo *weaponInfo, double directionx, double directiony, CThing *source, double targetpos, CThing *tg) : CBullet(initx, inity, width, height, number, weaponInfo, directionx, directiony, source, targetpos)
{
	typeID |= kGuidedBullet;

	target = tg;

	if (target) {
		initAbstSign = SIGN(target->xs - xs);

		lastfx = target->xs - xs; 
		lastfy = target->ys - ys;
		lastf = sqrt(lastfx * lastfx + lastfy * lastfy);
	}
}

CGuidedBullet::~CGuidedBullet() {}

short	CGuidedBullet::Think()
{
	double	abstx, absty, abst;
	double	alpha, tmp;
	double	a, b, phi1, phi2, dPhi;

	CObject::Think();

	if (target) {
		abstx = target->xs - xs; 
		absty = target->ys - ys;
	
		a = sqrt(lastfx * lastfx + lastfy * lastfy);
		b = sqrt(abstx*abstx+absty*absty);
		phi1=asin(lastfy / a); if (lastfx<0) phi1=phi1*(-1.0);
		phi2=asin(absty/b); if (abstx<0) phi2=phi2*(-1.0);
		dPhi=phi2-phi1;
		if (fabs(dPhi)>gConst->kMaxTurnAngle) dPhi=SIGN(dPhi)*gConst->kMaxTurnAngle;

		tmp=cos(dPhi)*lastfx-sin(dPhi)*lastfy;
		lastfy=sin(dPhi)*lastfx+cos(dPhi)*lastfy;
		lastfx = tmp;
	}

	forceVectorX = lastfx * gConst->kVelocityUnit * info->speed * deltaTime / lastf;
	forceVectorY = lastfy * gConst->kVelocityUnit * info->speed * deltaTime / lastf;

	return kNoEvent;
}

CStaffBullet::CStaffBullet(short initx, short inity, short width, short height, short number, tWeaponInfo *weaponInfo, double directionx, double directiony, CThing *source, double targetpos, CThing *tg) : CGuidedBullet(initx, inity, width, height, number, weaponInfo, directionx, directiony, source, targetpos, tg)
{
	lastMonsterScanTime = 0;
}

const double	kStaffBulletRad = 40.0;
const long		kMonsterScanTime = 200;

short	CStaffBullet::Think()
{
	tThingList	*currentEntry;

	if (!target && lastMonsterScanTime < lastTime) {

		currentEntry = gApplication->collisionThingList;
		while (currentEntry) {
			if (currentEntry->thing->typeID & kMonster && currentEntry->thing != shooter) {
				if ((currentEntry->thing->xm - xm) * (currentEntry->thing->xm - xm) * 0.5 +
					(currentEntry->thing->ym - ym) * (currentEntry->thing->ym - ym) < kStaffBulletRad * kStaffBulletRad) {
					target = currentEntry->thing;
				}
			}
			currentEntry = currentEntry->next;
		}
		lastMonsterScanTime = lastTime + kMonsterScanTime;

		return CBullet::Think();
	} else if (target)
		return CGuidedBullet::Think();
	else 
		return kNoEvent;
}


short	CBullet::Write(FILE *f)
{
	long	size = 0;

	WRITEDATA(size);
	WRITEDATA(typeID);
	WRITEDATA(thingNumber);

	size += CThing::Write(f);

	if (tailX) {
		for (short n = 0; n < gConst->kBulletTailLength; n ++) {
			WRITEDATA(tailX[n]);
			WRITEDATA(tailY[n]);
		}
	}

	WRITEDATA(tailStep);
	WRITEDATA(targetposition);
	WRITEDATA(directionUnitx);
	WRITEDATA(directionUnity);
	WRITEDATA(action);
	WRITEDATA(detonationStartTime);
	WRITEDATA(noDetonation);

	FINISHWRITE;

	return size;
}

void	CBullet::Read(FILE *f)
{
	long	size = 0;

	READDATA(size);
	READDATA(typeID);
	READDATA(thingNumber);

	CThing::Read(f);

	info = gObjInfo->FindWeapon(thingNumber);
	shooter = 0L;

	OnAllocate();

	if (projectile2) {
		for (short n = 0; n < gConst->kBulletTailLength; n ++) {
			READDATA(tailX[n]);
			READDATA(tailY[n]);
		}
	}

	READDATA(tailStep);
	READDATA(targetposition);
	READDATA(directionUnitx);
	READDATA(directionUnity);
	READDATA(action);
	READDATA(detonationStartTime);
	READDATA(noDetonation);
}


short	CSorceryBullet::Write(FILE *f)
{
	long	size = 0;

	WRITEDATA(size);
	WRITEDATA(typeID);
	WRITEDATA(thingNumber);

	size += CBullet::Write(f);

	WRITEDATA(numBounces);

	FINISHWRITE;

	return size;
}

void	CSorceryBullet::Read(FILE *f)
{
	long	size = 0;

	READDATA(size);
	READDATA(typeID);
	READDATA(thingNumber);

	CBullet::Read(f);

	READDATA(numBounces);
}

short	CBombBullet::Write(FILE *f)
{
	long	size = 0;

	WRITEDATA(size);
	WRITEDATA(typeID);
	WRITEDATA(thingNumber);

	size += CBullet::Write(f);

	WRITEDATA(detonate);

	FINISHWRITE;

	return size;
}

void	CBombBullet::Read(FILE *f)
{
	long	size = 0;

	READDATA(size);
	READDATA(typeID);
	READDATA(thingNumber);

	CBullet::Read(f);

	READDATA(detonate);
}

short	CSineBullet::Write(FILE *f)
{
	long	size = 0;

	WRITEDATA(size);
	WRITEDATA(typeID);
	WRITEDATA(thingNumber);

	size += CBullet::Write(f);

	WRITEDATA(distance);
	WRITEDATA(radius);
	WRITEDATA(period);

	FINISHWRITE;

	return size;
}

void	CSineBullet::Read(FILE *f)
{
	long	size = 0;

	READDATA(size);
	READDATA(typeID);
	READDATA(thingNumber);

	CBullet::Read(f);

	READDATA(distance);
	READDATA(radius);
	READDATA(period);
}

short	CGuidedBullet::Write(FILE *f)
{
	long	size = 0;

	WRITEDATA(size);
	WRITEDATA(typeID);
	WRITEDATA(thingNumber);

	size += CBullet::Write(f);

	WRITEDATA(initAbstSign);
	WRITEDATA(lastfx);
	WRITEDATA(lastfy);
	WRITEDATA(lastf);

	FINISHWRITE;

	return size;
}

void	CGuidedBullet::Read(FILE *f)
{
	long	size = 0;

	READDATA(size);
	READDATA(typeID);
	READDATA(thingNumber);

	CBullet::Read(f);

	READDATA(initAbstSign);
	READDATA(lastfx);
	READDATA(lastfy);
	READDATA(lastf);
}
