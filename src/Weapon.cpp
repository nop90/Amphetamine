#include "Weapon.hpp"
#include "System.hpp"
#include "Bullet.hpp"
#include "Appl.hpp"
#include "Thing.hpp"
#include "ConstVal.hpp"
#include "SndSys.hpp"

extern	CSystem	*gSystem;
extern	CApplication *gApplication;
extern	tConstValues *gConst;
extern	CLevel		*gLevel;
extern	CSoundSystem	*gSoundSystem;

short	*FindDescriptor(short id);


CWeapon::CWeapon(CThing *own, tWeaponInfo *weaponInfo, CShape *attack)
{
	owner = own;
	info = weaponInfo;
	attackShape = attack;
	lastShoot = 0;
	lastShootFrame = 0; // by LL
	weaponStatus = kWeaponDoesntExist;
	munition = 0;
	repetition = (owner->typeID & kPlayer) ? info->repetition : info->repetition * gApplication->currentWeaponSF;
}

CWeapon::~CWeapon()
{}

void	CWeapon::AddMunition(short mun)
{
	munition += mun;
	munition = MIN(munition, info->munition);
	if (weaponStatus == kWeaponOutOfMunition) weaponStatus = kWeaponReady;
}

short	CWeapon::Shoot(double directionx, double directiony, double targetpos)
{
	CBullet	*temp;
	short	*descr;

	if (weaponStatus == kWeaponReady && lastShoot <= gSystem->GetTicks()) {
		gSoundSystem->Play(gSoundSystem->weaponLaunchSounds[weaponNumber], owner->xm, owner->ym);
		lastShoot = gSystem->GetTicks() + repetition;
		lastShootFrame = gSystem->GetTicks() + gConst->kShootFrameTime;
 		descr = FindDescriptor(info->projectileShapes[0]);
		temp = new CBullet((owner->lookDirection == kLookingRight ? owner->xe - descr[3] / 2 - kWeaponBodyOffset : owner->xs + kWeaponBodyOffset + descr[3] / 2), owner->ye - gConst->kWeaponCarryHeight, descr[3], descr[4], weaponNumber, info, directionx, directiony, owner, targetpos);
	
		munition --;
		if (!munition) weaponStatus = kWeaponOutOfMunition;

		return 1;
	}else return 0;
}

short	CWeapon::ShootAnimation(CShape	**currentShape)
{
	if (owner->lastTime < lastShootFrame) {
		*currentShape = attackShape;
		return 0;
	}else return 1;
}


void	CWeapon::SaveDataToNextLevel(short *mun, short *stat)
{
	*mun = munition;
	*stat = weaponStatus;
}

void	CWeapon::RestoreDataFromPrevLevel(short mun, short stat)
{
	munition = mun;
	weaponStatus = stat;
}

CHandWeapon::CHandWeapon(CThing *own, tWeaponInfo *weaponInfo, CShape *attack1, CShape *attack2, CShape *attack3) : CWeapon(own, weaponInfo, attack1)
{
	numAnimFrames = 1;
	attackShape2 = attack2; if (attackShape2) numAnimFrames = 2;
	attackShape3 = attack3; if (attackShape3) numAnimFrames = 3;

	animationFrame = 4;
	shootFrameTime = gConst->kShootFrameTime * (4 - numAnimFrames);
}

CHandWeapon::~CHandWeapon() {}

short	CHandWeapon::Shoot(double directionx, double unused, double unused2)
{
	if (weaponStatus == kWeaponReady && lastShoot <= gSystem->GetTicks()) {
		gSoundSystem->Play(gSoundSystem->weaponLaunchSounds[weaponNumber], owner->xm, owner->ym);
		lastShoot = gSystem->GetTicks() + repetition;
		lastShootFrame = gSystem->GetTicks() + gConst->kShootFrameTime;
		animationFrame = 0;

		return 1;
	}else return 0;
}

short	CHandWeapon::ShootAnimation(CShape **currentShape)
{
	if (animationFrame < numAnimFrames) {
		switch (animationFrame) {
			case 0: *currentShape = attackShape; break;
			case 1: *currentShape = attackShape2; break;
			case 2: *currentShape = attackShape3; break;
		}
		if (owner->lastTime > lastShootFrame) {
			animationFrame ++;
			lastShootFrame = owner->lastTime + shootFrameTime;
		}
		return 0;
	}else if (animationFrame == numAnimFrames) {

		tThingList	*currentEntry = gApplication->collisionThingList;

		while (currentEntry) {
			if (currentEntry->thing != owner) currentEntry->thing->TestForDamage(owner->lookDirection == kLookingRight ? owner->xe : owner->xs, owner->ye - gConst->kWeaponCarryHeight, info->rad, info->damage);
			currentEntry = currentEntry->next;
		}
		animationFrame = SHRT_MAX;
	}
	return 1;
}

CSorcery::CSorcery(CThing *own, tWeaponInfo *weaponInfo, CShape *attack) : CWeapon(own, weaponInfo, attack)
{}

CSorcery::~CSorcery() {}

short	CSorcery::Shoot(double directionx, double directiony, double unused)
{
	CBullet	*temp;
	short	*descr;

	if (weaponStatus == kWeaponReady && lastShoot <= gSystem->GetTicks()) {
		gSoundSystem->Play(gSoundSystem->weaponLaunchSounds[weaponNumber], owner->xm, owner->ym);
		lastShoot = gSystem->GetTicks() + repetition;
		lastShootFrame = gSystem->GetTicks() + gConst->kShootFrameTime;
		descr = FindDescriptor(info->projectileShapes[0]);
		temp = new CSorceryBullet((owner->lookDirection == kLookingRight ? owner->xe - descr[3] / 2 - kWeaponBodyOffset : owner->xs + kWeaponBodyOffset + descr[3] / 2), owner->ye - gConst->kWeaponCarryHeight, descr[3], descr[4], weaponNumber, info, directionx, ABS(directionx), owner, 0);
		
		munition --;
		if (!munition) weaponStatus = kWeaponOutOfMunition;

		return 1;
	}else return 0;
}

CBomb::CBomb(CThing *own, tWeaponInfo *weaponInfo, CShape *attack, CShape *det) : CWeapon(own, weaponInfo, attack)
{
	status = kReady;
	detonator = det;
}

CBomb::~CBomb() {}

short	CBomb::Shoot(double directionx, double directiony, double unused)
{
	CBombBullet	*temp;
	short		*descr;

	if (weaponStatus == kWeaponReady && lastShoot <= gSystem->GetTicks()) {
		lastShoot = gSystem->GetTicks() + repetition;
		lastShootFrame = gSystem->GetTicks() + gConst->kShootFrameTime;

		if (status == kReady) {
			gSoundSystem->Play(gSoundSystem->weaponLaunchSounds[weaponNumber], owner->xm, owner->ym);
			status = kDetonation;
			descr = FindDescriptor(info->projectileShapes[0]);
			temp = new CBombBullet((owner->lookDirection == kLookingRight ? owner->xe - descr[3] / 2 - kWeaponBodyOffset : owner->xs + kWeaponBodyOffset + descr[3] / 2), owner->ye - gConst->kWeaponCarryHeight, descr[3], descr[4], weaponNumber, info, directionx, directiony, owner, 0);
			bomb = temp;

			munition --;
		}else{
			gSoundSystem->Play(gSoundSystem->weaponHitSounds[weaponNumber], bomb->xm, bomb->ym);
			if (bomb) ((CBombBullet *)bomb)->detonate = 1;
			bomb = 0L;
			status = kReady;
			if (!munition) weaponStatus = kWeaponOutOfMunition;
		}
		return 1;
	}else return 0;
}

short	CBomb::ShootAnimation(CShape **currentShape)
{
	if (status == kDetonation) *currentShape = detonator;
	if (owner->lastTime < lastShootFrame && status == kDetonation) *currentShape = attackShape;
	return 1;
}


CMultiBulletWeapon::CMultiBulletWeapon(CThing *own, tWeaponInfo *weaponInfo, CShape *attack, double angle) : CWeapon(own, weaponInfo, attack)
{
	numOfBullets = weaponInfo->error;
	alpha = angle;
	initAlpha = (double)(numOfBullets - 1) / 2.0 * alpha;
}

CMultiBulletWeapon::~CMultiBulletWeapon() {}


short	CMultiBulletWeapon::Shoot(double directionx, double directiony, double targetpos)
{
	CBullet	*temp;
	short	*descr;
	double	dx, dy, tmp;

	if (weaponStatus == kWeaponReady && lastShoot <= gSystem->GetTicks()) {
		gSoundSystem->Play(gSoundSystem->weaponLaunchSounds[weaponNumber], owner->xm, owner->ym);
		lastShoot = gSystem->GetTicks() + repetition;
		lastShootFrame = gSystem->GetTicks() + gConst->kShootFrameTime;
 		descr = FindDescriptor(info->projectileShapes[0]);

		dx = directionx * cos(initAlpha) - directiony * sin(initAlpha);
		dy = directionx * sin(initAlpha) + directiony * cos(initAlpha);
		for (short n = 0; n < numOfBullets && weaponStatus == kWeaponReady; n ++) {
			temp = new CBullet((owner->lookDirection == kLookingRight ? owner->xe - descr[3] / 2 - kWeaponBodyOffset : owner->xs + kWeaponBodyOffset + descr[3] / 2), owner->ye - gConst->kWeaponCarryHeight, descr[3], descr[4], weaponNumber, info, dx, dy, owner, targetpos);

			munition --;
			if (!munition) weaponStatus = kWeaponOutOfMunition;

			tmp = dx * cos(alpha) + dy * sin(alpha);
			dy = -dx * sin(alpha) + dy * cos(alpha);
			dx = tmp;
		}

		return 1;
	}else return 0;
}


CStaff::CStaff(CThing *own, tWeaponInfo *weaponInfo, CShape *attack, CShape *load) : CWeapon(own, weaponInfo, attack)
{
	loaded = load;
	inLoad = 0;
	lastLoad = 0;
}

CStaff::~CStaff()
{}


short	CStaff::Shoot(double directionx, double directiony, double targetpos)
{
	long	time = gSystem->GetTicks();

	dx = directionx;
	dy = directiony;

	if (time - lastLoad < 100) {
		lastLoad = time;
		if (time - staffLoadTime >= gConst->kStaffLoadTime) inLoad = 1;
	}else if (lastShoot <= time && weaponStatus == kWeaponReady) {
		lastLoad = time;
		staffLoadTime = time;
	}

	return 0;
}


short	CStaff::ShootAnimation(CShape **currentShape)
{
	long	time = gSystem->GetTicks();

	if (time - lastLoad < 100) {	// Weapon is in load
		if (time - staffLoadTime < gConst->kStaffLoadTime) 
			*currentShape = attackShape;
		else *currentShape = loaded;
		return 0;
	}else if (inLoad) {	// Weapon is released
		CBullet	*temp;
		short	*descr;

		gSoundSystem->Play(gSoundSystem->weaponLaunchSounds[weaponNumber], owner->xm, owner->ym);
		lastShoot = gSystem->GetTicks() + repetition;
		lastShootFrame = gSystem->GetTicks() + gConst->kShootFrameTime;
 		descr = FindDescriptor(info->projectileShapes[0]);
		temp = new CSineBullet((owner->lookDirection == kLookingRight ? owner->xe - descr[3] / 2 - kWeaponBodyOffset : owner->xs + kWeaponBodyOffset + descr[3] / 2), owner->ye - gConst->kWeaponCarryHeight, descr[3], descr[4], weaponNumber, info, dx, 0, owner, 0, gConst->kSineWeaponRad, info->error);
			
		munition --;
		if (!munition) weaponStatus = kWeaponOutOfMunition;

		inLoad = 0;
	}

	if (time < lastShootFrame) {
		*currentShape = loaded;
		return 0;
	}
	return 1;
}


CSineWeapon::CSineWeapon(CThing *own, tWeaponInfo *weaponInfo, CShape *attack, short rad) : CWeapon(own, weaponInfo, attack)
{
	radius = rad;
}

CSineWeapon::~CSineWeapon()
{}

short	CSineWeapon::Shoot(double directionx, double directiony, double targetpos)
{
	CBullet	*temp;
	short	*descr;

	if (lastShoot <= gSystem->GetTicks()) {
		gSoundSystem->Play(gSoundSystem->weaponLaunchSounds[weaponNumber], owner->xm, owner->ym);
		lastShoot = gSystem->GetTicks() + repetition;
		lastShootFrame = gSystem->GetTicks() + gConst->kShootFrameTime;
 		descr = FindDescriptor(info->projectileShapes[0]);
		temp = new CSineBullet((owner->lookDirection == kLookingRight ? owner->xe - descr[3] / 2 - kWeaponBodyOffset : owner->xs + kWeaponBodyOffset + descr[3] / 2), owner->ye - gConst->kWeaponCarryHeight, descr[3], descr[4], weaponNumber, info, directionx, directiony, owner, targetpos, radius, info->error);
		return 1;
	}else return 0;
}

CGuided::CGuided(CThing *own, tWeaponInfo *weaponInfo, CShape *attack) : CWeapon(own, weaponInfo, attack)
{
}

CGuided::~CGuided()
{}

short	CGuided::Shoot(double directionx, double directiony, double targetpos)
{
	CBullet	*temp;
	short	*descr;

	if (lastShoot <= gSystem->GetTicks()) {
		gSoundSystem->Play(gSoundSystem->weaponLaunchSounds[weaponNumber], owner->xm, owner->ym);
		lastShoot = gSystem->GetTicks() + repetition;
		lastShootFrame = gSystem->GetTicks() + gConst->kShootFrameTime;
 		descr = FindDescriptor(info->projectileShapes[0]);
		temp = new CGuidedBullet((owner->lookDirection == kLookingRight ? owner->xe - descr[3] / 2 - kWeaponBodyOffset : owner->xs + kWeaponBodyOffset + descr[3] / 2), owner->ye - gConst->kWeaponCarryHeight, descr[3], descr[4], weaponNumber, info, directionx, directiony, owner, targetpos, gLevel->player);

		return 1;
	}else return 0;
}



short	CWeapon::Write(FILE *f)
{
	long	size = 0;

	WRITEDATA(size);

	WRITEDATA(munition);
	WRITEDATA(lastShoot);
	WRITEDATA(lastShootFrame);
	WRITEDATA(weaponStatus);
	WRITEDATA(repetition);

	FINISHWRITE;

	return size;
}

void	CWeapon::Read(FILE *f)
{
	long	size = 0;

	READDATA(size);

	READDATA(munition);
	READDATA(lastShoot);
	READDATA(lastShootFrame);
	READDATA(weaponStatus);
	READDATA(repetition);
}

short	CHandWeapon::Write(FILE *f)
{
	long	size = 0;

	WRITEDATA(size);

	size += CWeapon::Write(f);
	
	WRITEDATA(animationFrame);
	WRITEDATA(numAnimFrames);
	WRITEDATA(shootFrameTime);
	
	FINISHWRITE;

	return size;
}

void	CHandWeapon::Read(FILE *f)
{
	long	size = 0;

	READDATA(size);

	CWeapon::Read(f);
	
	READDATA(animationFrame);
	READDATA(numAnimFrames);
	READDATA(shootFrameTime);
	
}

short	CBomb::Write(FILE *f)
{
	long	size = 0;

	WRITEDATA(size);

	size += CWeapon::Write(f);

	WRITEDATA(status);

	FINISHWRITE;

	return size;
}

void	CBomb::Read(FILE *f)
{
	long	size = 0;

	READDATA(size);

	CWeapon::Read(f);

	READDATA(status);
}


short	CStaff::Write(FILE *f)
{
	long	size = 0;

	WRITEDATA(size);

	size += CWeapon::Write(f);

	WRITEDATA(staffLoadTime);
	WRITEDATA(lastLoad);
	WRITEDATA(inLoad);
	WRITEDATA(dx);
	WRITEDATA(dy);

	FINISHWRITE;

	return size;
}

void	CStaff::Read(FILE *f)
{
	long	size = 0;

	READDATA(size);

	CWeapon::Read(f);

	READDATA(staffLoadTime);
	READDATA(lastLoad);
	READDATA(inLoad);
	READDATA(dx);
	READDATA(dy);

}