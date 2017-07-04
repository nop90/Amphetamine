#include "Monstrxx.hpp"
#include "Level.hpp"
#include "ConstVal.hpp"
#include "Appl.hpp"
#include "System.hpp"

extern CLevel *gLevel;
extern tConstValues	*gConst;
extern CApplication	*gApplication;
extern CSystem		*gSystem;

/* ###########################################
		CREEPER CLASS EVENTS
		############################################### */
CCreeper::CCreeper(short initx, short inity, short width, short height, short number, tMonsterInfo *monsterInfo) : CMonster(initx, inity, width, height, number, monsterInfo)
{
}

CCreeper::~CCreeper()
{}

void	CCreeper::OnStart()
{
	OnLanding();
}

void	CCreeper::OnLanding()
{

	status = (FindPlayerX() < 0 ? kMonsterBackward : kMonsterForward);
}

void	CCreeper::OnCollision()
{
	status = (status == kMonsterForward ? kMonsterBackward : kMonsterForward);
}

void	CCreeper::OnAbyss()
{
	if (FindPlayerY() < 0) status = (status == kMonsterForward ? kMonsterBackward : kMonsterForward);
}

void	CCreeper::OnIdle()
{
	lookDirection = (status == kMonsterBackward ? kLookingLeft : kLookingRight);
	forceVectorX = speed * deltaTime * gConst->kVelocityUnit * (status == kMonsterForward ? 1.0 : -1.0);
}


/* ###########################################
		WALKER CLASS EVENTS
		############################################### */

CWalker::CWalker(short initx, short inity, short width, short height, short number, tMonsterInfo *monsterInfo) : CMonster(initx, inity, width, height, number, monsterInfo)
{
	lastShoot = 0;
	currentShootDelay = 0;
	nextShootTime = 0;
}

CWalker::~CWalker()
{}

void	CWalker::OnStart()
{
	status = (FindPlayerX() < 0 ? kMonsterBackward : kMonsterForward);
}

void	CWalker::OnCollision()
{
	status = (status == kMonsterForward ? kMonsterBackward : kMonsterForward);
}

void	CWalker::OnAbyss()
{
	status = (status == kMonsterForward ? kMonsterBackward : kMonsterForward);
}

void	CWalker::OnIdle()
{
	double	px = FindPlayerX();

	if (!noWeapon && lastTime > nextShootTime) {
		lookDirection = (FindPlayerX() < 0 ? kLookingLeft : kLookingRight);

		if (weapon->Shoot(px, FindPlayerY(), (px > 0 ? px + xs : px + xe))) {
			nextShootTime = lastTime + currentShootDelay;
			lastShoot = lastTime + kWalkerWaitAfterShooting;
			status = (FindPlayerX() < 0 ? kMonsterBackward : kMonsterForward);
		};
	}

	lookDirection = (status == kMonsterBackward ? kLookingLeft : kLookingRight);

	if (lastTime > lastShoot && lastTime > lastBlessureTime) forceVectorX = speed * deltaTime * gConst->kVelocityUnit * (status == kMonsterForward ? 1.0 : -1.0);
}


void	CWalker::OnTreatment(double directionx, double directiony)
{
	if (directiony > 0) {
		if (directionx > 0) status = kMonsterForward; else status = kMonsterBackward;
	}
}


void	CWalker::OnShootSuccessful()
{
	currentShootDelay = 0;
}

void	CWalker::OnShootNotSuccessful()
{
	 if (currentShootDelay < kMonsterMaxShootDelay) currentShootDelay += kMonsterMaxShootDelay / 4;
}


/* ###########################################
		JUMPER CLASS EVENTS
		############################################### */


CJumper::CJumper(short initx, short inity, short width, short height, short number, tMonsterInfo *monsterInfo) : CMonster(initx, inity, width, height, number, monsterInfo)
{
	inJump = 0;
	nextShootTime = 0;
	currentShootDelay = 0;
	lastShoot = 0;
}

CJumper::~CJumper()
{}

void	CJumper::OnStart()
{
	status = (FindPlayerX() < 0 ? kMonsterBackward : kMonsterForward);
}

void	CJumper::OnCollision()
{
	short	elemx, elemy;
	short	condition;
	
	// Is there only a step, so the jumper can jump onto the step (but only when the player
	// is in this direction)
	if (status == kMonsterForward) {
		elemx = (short)((xs + (xe - xs) * 0.5) / kElementSize) +1;
		condition = FindPlayerX() > 0 && elemx < kLevelWidth;
	}else{
		elemx = (short)((xs + (xe - xs) * 0.5) / kElementSize) -1;
		condition = FindPlayerX() < 0 && elemx > 0;
	}
	elemy = (short)((ys + (ye - ys) * 0.5) / kElementSize) -1;

	if (condition && elemy >= 0 && gLevel->level[elemy][elemx]->background) 
		Jump();
	else if (!inJump)
		status = (status == kMonsterForward ? kMonsterBackward : kMonsterForward);
}

void	CJumper::OnAbyss()
{
	short	condition;

	// If there's an abyss, the jumper just jumps (only if the player is in the right direction)
	if (status == kMonsterForward) {
		condition = FindPlayerX() > 0;
	}else{
		condition = FindPlayerX() < 0;
	}

	if (condition)
		Jump();
	else
		status = (status == kMonsterForward ? kMonsterBackward : kMonsterForward);
}

void	CJumper::OnIdle()
{
	double	px = FindPlayerX();
	short	oldStatus = status;

	if (!noWeapon && lastTime > nextShootTime) {
		lookDirection = (FindPlayerX() < 0 ? kLookingLeft : kLookingRight);
		if (weapon->Shoot(px, FindPlayerY(), (px > 0 ? px + xs : px + xe))) {
			nextShootTime = lastTime + currentShootDelay;
			lastShoot = lastTime + kWalkerWaitAfterShooting;
		}

	
	}
	lookDirection = (status == kMonsterBackward ? kLookingLeft : kLookingRight);

	if (lastTime > lastShoot && lastTime > lastBlessureTime) forceVectorX = speed * deltaTime * gConst->kVelocityUnit * (status == kMonsterForward ? 1.0 : -1.0);
}


void	CJumper::OnTreatment(double directionx, double directiony)
{
	if (ABS(directiony / directionx) > 1.0) { // the bullet comes vertical
		if (directiony > 0) {
			if (directionx > 0) status = kMonsterForward; else status = kMonsterBackward;
		}
	}else{
		Jump();
	}
}

void	CJumper::OnLanding()
{
	status = (FindPlayerX() < 0 ? kMonsterBackward : kMonsterForward);
	inJump = 0;
}

void	CJumper::OnShootSuccessful()
{
	currentShootDelay = 0;
}

void	CJumper::OnShootNotSuccessful()
{
	 if (currentShootDelay < kMonsterMaxShootDelay) currentShootDelay += kMonsterMaxShootDelay / 4;
}

void	CJumper::Jump()
{
	if (!inJump) {
		forceVectorY = -gConst->kJumperJumpAcceleration * gConst->kVelocityUnit * deltaTime;
		inJump = 1;
	}
}


/* ###########################################
		FLYER CLASS EVENTS
		############################################### */

CFlyer::CFlyer(short initx, short inity, short width, short height, short number, tMonsterInfo *monsterInfo) : CMonster(initx, inity, width, height, number, monsterInfo)
{
	weight = 10;
	rescueMeX = 0;
	rescueMeY = 0;
	weightless = 1;

	nextShootTime = 0;
	currentShootDelay = 0;
	lastShoot = 0;
}

CFlyer::~CFlyer()
{}

void	CFlyer::OnIdle()
{
	double	px = FindPlayerX();

	if (!noWeapon && lastTime > nextShootTime) {
		lookDirection = (FindPlayerX() < 0 ? kLookingLeft : kLookingRight);
		if (weapon->Shoot(px, FindPlayerY(), (px > 0 ? px + xs : px + xe))) {
			nextShootTime = lastTime + currentShootDelay;
			lastShoot = lastTime + kWalkerWaitAfterShooting;
		}
	}

	if (lastTime > lastShoot) {
		if (!rescueMeX && !rescueMeY) {
 			forceVectorX = speed * deltaTime * gConst->kVelocityUnit * SIGN(FindPlayerX());
			forceVectorY = speed * deltaTime * gConst->kVelocityUnit * SIGN(FindPlayerY());
		}else{
			forceVectorX = rescueMeX * deltaTime;
			forceVectorY = rescueMeY * deltaTime;
			rescueMeX = 0;
			rescueMeY = 0;
		}
	}
	lookDirection = SIGN(forceVectorX);
}

void	CFlyer::OnTreatment(double directionx, double directiony)
{
	double	len = sqrt(directionx * directionx + directiony * directiony);

 	rescueMeX = SIGN(directionx) * directiony * speed * gConst->kVelocityUnit / len * 2.0;
	rescueMeY = SIGN(directiony) * -1.0 * directionx * speed * gConst->kVelocityUnit / len * 2.0;
}


void	CFlyer::OnShootSuccessful()
{
	currentShootDelay = 0;
}

void	CFlyer::OnShootNotSuccessful()
{
	 if (currentShootDelay < kMonsterMaxShootDelay) currentShootDelay += kMonsterMaxShootDelay / 4;
}

void	CFlyer::OnKill()
{
	weightless = 0;
	CMonster::OnKill();
}


/* ###########################################
		WARG CLASS EVENTS
		############################################### */

CWarg::CWarg(short initx, short inity, short width, short height, short number, tMonsterInfo *monsterInfo) : CMonster(initx, inity, width, height, number, monsterInfo)
{
	inJump = 0;
	if (monsterInfo) OnAllocate();
	deathTime = 0;

	nextShootTime = 0;
	currentShootDelay = 0;
	lastShoot = 0;
	speedup = 1;
}

void	CWarg::OnAllocate()
{
	farWeapon = CreateWeapon(info->weapon + 1);
}

CWarg::~CWarg()
{
	if (farWeapon) delete farWeapon;
}

void	CWarg::OnStart()
{
	status = (FindPlayerX() < 0 ? kMonsterBackward : kMonsterForward);
}

void	CWarg::OnCollision()
{
	status = (lastCollisionCode & kCollisionOnRight ? kMonsterBackward : kMonsterForward);
	speedup = 1;
}

void	CWarg::OnAbyss()
{
	status = (status == kMonsterForward ? kMonsterBackward : kMonsterForward);
	speedup = 1.0;
}

void	CWarg::OnIdle()
{
	double	px = FindPlayerX();
	CWeapon	*currentWeapon;

	if (!noWeapon && lastTime > nextShootTime) {
		lookDirection = (px < 0 ? kLookingLeft : kLookingRight);

		currentWeapon = (ABS(px) > gConst->kWargNearWeaponRadix ? farWeapon : weapon);

		if (currentWeapon->Shoot(px, FindPlayerY(), (px > 0 ? px + xs : px + xe))) {
			nextShootTime = lastTime + currentShootDelay;
			lastShoot = lastTime + kWalkerWaitAfterShooting;
			status = (FindPlayerX() < 0 ? kMonsterBackward : kMonsterForward);
		};
	}

	lookDirection = (status == kMonsterBackward ? kLookingLeft : kLookingRight);

	if (lastTime > lastShoot) forceVectorX = speed * deltaTime * lookDirection * gConst->kVelocityUnit * speedup;
}


void	CWarg::OnTreatment(double directionx, double directiony)
{
	if (ABS(directiony / directionx) > 1.0) { // the bullet comes vertical
		if (directiony > 0) {
			if (directionx > 0) status = kMonsterForward; else status = kMonsterBackward;
		}
	}else{
		Jump();
		status = (FindPlayerX() < 0 ? kMonsterBackward : kMonsterForward);
	}
}

void	CWarg::OnLanding()
{
	inJump = 0;
}

void	CWarg::Jump()
{
	if (!inJump) {
		forceVectorY = -gConst->kWargJumpAcceleration;
		inJump = 1;
	}
}

// ---------------------------------------------
void	CWarg::OnDamage(short blessure)
// Event occurs, when a monster is hit by something (bullet etc.)
{
	if (dieFrame == -1 && lastBlessureTime < lastTime) {
		health -= blessure;
		if (health < 0) {
			OnKill();
			deathTime = lastTime + gConst->kDelayAfterWargDeath;
		} else {
			lastBlessureTime = lastTime + gConst->kBlessureInvulnerabilityTime;
			speedup = gConst->kWargFastSpeedup;
		}
	}
}

void	CWarg::TestForDamage(double xb, double yb, short rad, short blessure)
{
	CThing::TestForDamage(xb, yb, rad + dx / 2, blessure);
}

void	CWarg::Render(short planeX, short planeY, tRect *clipRect)
{
	CMonster::Render(planeX, planeY, clipRect);
	if (deathTime && deathTime < gSystem->GetTicks()) gApplication->command = kCmdNextLevel;
}


short	CWalker::Write(FILE *f)
{
	long	size = 0;

	WRITEDATA(size);
	WRITEDATA(typeID);
	WRITEDATA(thingNumber);

	size += CMonster::Write(f);

	WRITEDATA(nextShootTime);
	WRITEDATA(currentShootDelay);
	WRITEDATA(lastShoot);

	FINISHWRITE;

	return size;
}

void	CWalker::Read(FILE *f)
{
	long	size = 0;

	READDATA(size);
	READDATA(typeID);
	READDATA(thingNumber);

	CMonster::Read(f);

	READDATA(nextShootTime);
	READDATA(currentShootDelay);
	READDATA(lastShoot);
}


short	CJumper::Write(FILE *f)
{
	long	size = 0;

	WRITEDATA(size);
	WRITEDATA(typeID);
	WRITEDATA(thingNumber);

	size += CMonster::Write(f);

	WRITEDATA(nextShootTime);
	WRITEDATA(currentShootDelay);
	WRITEDATA(lastShoot);
	WRITEDATA(inJump);

	FINISHWRITE;

	return size;
}

void	CJumper::Read(FILE *f)
{
	long	size = 0;

	READDATA(size);
	READDATA(typeID);
	READDATA(thingNumber);

	CMonster::Read(f);

	READDATA(nextShootTime);
	READDATA(currentShootDelay);
	READDATA(lastShoot);
	READDATA(inJump);
}

short	CFlyer::Write(FILE *f)
{
	long	size = 0;

	WRITEDATA(size);
	WRITEDATA(typeID);
	WRITEDATA(thingNumber);

	size += CMonster::Write(f);

	WRITEDATA(nextShootTime);
	WRITEDATA(currentShootDelay);
	WRITEDATA(lastShoot);
	WRITEDATA(rescueMeX);
	WRITEDATA(rescueMeY);

	FINISHWRITE;

	return size;
}

void	CFlyer::Read(FILE *f)
{
	long	size = 0;

	READDATA(size);
	READDATA(typeID);
	READDATA(thingNumber);

	CMonster::Read(f);

	READDATA(nextShootTime);
	READDATA(currentShootDelay);
	READDATA(lastShoot);
	READDATA(rescueMeX);
	READDATA(rescueMeY);
}


short	CWarg::Write(FILE *f)
{
	long	size = 0;

	WRITEDATA(size);
	WRITEDATA(typeID);
	WRITEDATA(thingNumber);

	size += CMonster::Write(f);
	if (farWeapon) size += farWeapon->Write(f);

	WRITEDATA(nextShootTime);
	WRITEDATA(currentShootDelay);
	WRITEDATA(lastShoot);
	WRITEDATA(inJump);
	WRITEDATA(speedup);

	FINISHWRITE;

	return size;
}


void	CWarg::Read(FILE *f)
{
	long	size = 0;

	READDATA(size);
	READDATA(typeID);
	READDATA(thingNumber);

	CMonster::Read(f);

	OnAllocate();

	if (farWeapon) farWeapon->Read(f);

	READDATA(nextShootTime);
	READDATA(currentShootDelay);
	READDATA(lastShoot);
	READDATA(inJump);
	READDATA(speedup);
}
