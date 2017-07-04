#ifndef __AMP_BULLET__
#define __AMP_BULLET__

#include "Thing.hpp"
#include "ObjInfo.hpp"
#include "Shape.hpp"

enum {
	kInFlight,
	kInDetonation,
	kWaitForDetonation	// only for bombs
};

class CBullet : public CThing {
protected:
	CShape	*projectile1;
	CShape	*projectile2;
	CShape	*detonation[5];

	short	*tailX, *tailY;
	double	tailStep;

	CThing	*shooter;
	double	targetposition;

	tWeaponInfo *info;
	double	directionUnitx;
	double	directionUnity;

	short	action;
	CShape	*currentDetonationShape;
	long	detonationStartTime;
	short	noDetonation;

	void	Detonate(short collisionCode, CObject *);
	short	DetonationAnimation();
	void	Damage(CObject *);

public:
	CBullet	*nextBullet, *prevBullet;

	CBullet(short initx, short inity, short width, short height, short number, tWeaponInfo *weaponInfo, double directionx, double directiony, CThing *shooter, double targetpos);
	~CBullet();

	void	OnAllocate();
	void	LinkInLists();
	void	UnlinkInLists();

	virtual short	Think();
	void	Move();
	short	Forces();
	void	Render(short planeX, short planeY, tRect *clipRect);
	short	AmIATreatment(double victimxs, double victimys, double victimxe, double victimye, double &fx, double &fy);
	short	Collision(CObject *sender, double left, double top, double right, double bottom, double &forcex, double &forcey, double pfx, double pfy, short sourceWeight, short &collisionObject);

	virtual short	Write(FILE *f);
	virtual void	Read(FILE *f);
};


class CSorceryBullet : public CBullet {
protected:
	short	numBounces;

public:
	CSorceryBullet(short initx, short inity, short width, short height, short number, tWeaponInfo *weaponInfo, double directionx, double directiony, CThing *shooter, double targetpos);
	~CSorceryBullet();

	short	Forces();
	short	Write(FILE *f);
	void	Read(FILE *f);
};

class CBomb;

class CBombBullet : public CBullet {
	friend class CBomb;

protected:
	short	detonate;

public:
	CBombBullet(short initx, short inity, short width, short height, short number, tWeaponInfo *weaponInfo, double directionx, double directiony, CThing *shooter, double targetpos);
	~CBombBullet();

	short	Forces();
	short	Write(FILE *f);
	void	Read(FILE *f);
};

class CSineBullet : public CBullet {
protected:
	double	distance;
	double	radius;
	double	period;

public:
	CSineBullet(short initx, short inity, short width, short height, short number, tWeaponInfo *weaponInfo, double directionx, double directiony, CThing *shooter, double targetpos, short rad, short period);
	~CSineBullet();

	short	Think();
	short	Write(FILE *f);
	void	Read(FILE *f);
};


class CGuidedBullet : public CBullet {
protected:
	CThing	*target;
	double	initAbstSign;
	double	lastfx, lastfy, lastf;

public:
	CGuidedBullet(short initx, short inity, short width, short height, short number, tWeaponInfo *weaponInfo, double directionx, double directiony, CThing *shooter, double targetpos, CThing *tg);
	~CGuidedBullet();

	virtual short	Think();
	short	Write(FILE *f);
	void	Read(FILE *f);
};

class CStaffBullet : public CGuidedBullet {
protected:
	long	lastMonsterScanTime;

public:
	CStaffBullet(short initx, short inity, short width, short height, short number, tWeaponInfo *weaponInfo, double directionx, double directiony, CThing *shooter, double targetpos, CThing *tg);
	~CStaffBullet();

	short	Think();
};


#endif
