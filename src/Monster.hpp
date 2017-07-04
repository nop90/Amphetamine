#ifndef __AMP_MONSTER__
#define __AMP_MONSTER__

#include "Thing.hpp"
#include "ObjInfo.hpp"
#include "Weapon.hpp"

enum { // stati
	kMonsterForward,
	kMonsterBackward,
	kMonsterWaiting,
	kMonsterFastForward,	// for Warg
	kMonsterFastBackward
}; 

// if no shoots are successful, so shoot at least every 4 seconds
const short	kMonsterMaxShootDelay = 4000;
const short kWalkerWaitAfterShooting = 800;

class CMonster : public CThing {
protected:
	CShape	*moveShapes[3];
	CShape	*jumpShape;
	CShape	*attackShape;
	CShape	*deathShapes[3];

	CWeapon	*weapon;
	short	noWeapon;

	tMonsterInfo	*info;
	long	lastWalkTime, lastShootTime;
	short	walkFrame;
	CShape	*currentShape;
	double	speed;

	short	status;
	short	lastCollisionCode;
	short	flying;
	long	teleportDeltaTime;	// actually only used by the player
	short	teleportCounter;
	long	lastBlessureTime;
	long	dieFrameTime;
	short	dieFrame;
	short	doDarken;

	short	health;

	// Events
	virtual void	OnStart();
	virtual void	OnAbyss();
	virtual void	OnCollision();
	virtual void	OnLanding();
	virtual void	OnIdle();
	virtual void	OnTreatment(double directionx, double directiony);
	virtual void	OnTeleport();	// actually only used by the player

	double	FindPlayerX();
	double	FindPlayerY();
	CWeapon	*CreateWeapon(short weapon);

public:
	CMonster(short initx, short inity, short width, short height, short number, tMonsterInfo *monsterInfo);
	~CMonster();

	void	OnAllocate();

	void	LinkInLists();
	void	UnlinkInLists();

	// public events
	virtual void	OnShootSuccessful();
	virtual void	OnShootNotSuccessful();
	virtual void	OnDamage(short blessure);
	virtual void	OnKill();
	virtual void	OnTouch(CObject *touch);

	virtual void	Render(short planeX, short planeY, tRect *clipRect);
	short	Collision(CObject *sender, double left, double top, double right, double bottom, double &forcex, double &forcey, double pfx, double pfy, short sourceWeight, short &collisionObject);
	short	Think();
	void	Move();
	short	Forces();

	virtual	short	Write(FILE *f);
	virtual void	Read(FILE *f);
};



#endif