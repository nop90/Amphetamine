#ifndef __AMP_WEAPON__
#define __AMP_WEAPON__

#include "ObjInfo.hpp"
#include "Thing.hpp"
#include "SoundList.hpp"

const short	kWeaponBodyOffset = 3; // At which direction of the body the shooted bullet is placed

class CGUI;

class CWeapon {
friend class CGUI;

protected:
	CThing	*owner;
	tWeaponInfo	*info;
	CShape	*attackShape;
	short	munition;

	long	lastShoot, lastShootFrame;
	short	repetition;

	CSound	*launchSound, *hitSound;


public:
	long			ID;
	long			savedID;
	short			weaponStatus;
	short			weaponNumber;

	CWeapon(CThing *own, tWeaponInfo *weaponInfo, CShape *attack);
	~CWeapon();

	void			SetID(long);

	void			AddMunition(short munition);
	virtual short	Shoot(double directionx, double directiony, double targetpos);
	virtual short	ShootAnimation(CShape **);

	void	SaveDataToNextLevel(short *, short *);
	void	RestoreDataFromPrevLevel(short, short);

	virtual short	Write(FILE *f);
	virtual void	Read(FILE *f);
};


class CHandWeapon : public CWeapon {
protected:
	CShape	*attackShape2, *attackShape3;
	short	animationFrame;
	short	numAnimFrames;
	long	shootFrameTime;

public:
	CHandWeapon(CThing *own, tWeaponInfo *weaponInfo, CShape *attack1, CShape *attack2, CShape *attack3);
	~CHandWeapon();

	short	Shoot(double directionx, double directiony, double targetpos);
	short	ShootAnimation(CShape **);

	short	Write(FILE *f);
	void	Read(FILE *f);
};


class CSorcery : public CWeapon {
protected:

public:
	CSorcery(CThing *own, tWeaponInfo *weaponInfo, CShape *attack);
	~CSorcery();

	short	Shoot(double directionx, double directiony, double unused);
};

enum {	// stati
	kReady,
	kDetonation
};

class CBomb : public CWeapon {
protected:
	short	status;
	CThing	*bomb;
	CShape	*detonator;

public:
	CBomb(CThing *own, tWeaponInfo *info, CShape *attack, CShape *det);
	~CBomb();

	short	Shoot(double directionx, double directiony, double unused);
	short	ShootAnimation(CShape **);

	short	Write(FILE *f);
	void	Read(FILE *f);
};

class CMultiBulletWeapon : public CWeapon {
protected:
	short	numOfBullets;
	double	alpha, initAlpha;

public:
	CMultiBulletWeapon(CThing *own, tWeaponInfo *info, CShape *attack, double angle);
	~CMultiBulletWeapon();

	short	Shoot(double directionx, double directiony, double targetpos);
};


class CStaff : public CWeapon {
protected:
	long	staffLoadTime;
	long	lastLoad;
	short	inLoad;
	CShape	*loaded;
	double	dx, dy;

public:
	CStaff(CThing *own, tWeaponInfo *info, CShape *attack, CShape *load);
	~CStaff();

	short	Shoot(double directionx, double directiony, double unused);
	short	ShootAnimation(CShape **);

	short	Write(FILE *f);
	void	Read(FILE *f);
};


class CSineWeapon : public CWeapon {
protected:
	short	radius;

public:
	CSineWeapon(CThing *own, tWeaponInfo *info, CShape *attack, short rad);
	~CSineWeapon();

	short	Shoot(double directionx, double directiony, double targetpos);
};


class CGuided : public CWeapon  
{
public:
	CGuided(CThing *own, tWeaponInfo *info, CShape *attack);
	~CGuided();

	short	Shoot(double directionx, double directiony, double targetpos);
};

#endif