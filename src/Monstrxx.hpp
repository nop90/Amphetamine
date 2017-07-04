#ifndef __MONSTER_XTRAS__
#define __MONSTER_XTRAS__

#include "Monster.hpp"
#include "ObjInfo.hpp"

class CCreeper : public CMonster {
protected:
	void	OnStart();
	void	OnCollision();
	void	OnLanding();
	void	OnIdle();
	void	OnAbyss();

public:
	CCreeper(short initx, short inity, short width, short height, short number, tMonsterInfo *monsterInfo);
	~CCreeper();
};

class CWalker : public CMonster {
protected:
	long	nextShootTime;
	long	currentShootDelay;
	long	lastShoot;

	void	OnStart();
	void	OnCollision();
	void	OnIdle();
	void	OnAbyss();
	void	OnTreatment(double dirx, double diry);

public:
	CWalker(short initx, short inity, short width, short height, short number, tMonsterInfo *monsterInfo);
	~CWalker();

	void	OnShootSuccessful();
	void	OnShootNotSuccessful();

	short	Write(FILE *f);
	void	Read(FILE *f);
};

class CJumper : public CMonster {
protected:
	long	nextShootTime;
	long	currentShootDelay;
	long	lastShoot;
	short	inJump;

	void	OnStart();
	void	OnCollision();
	void	OnIdle();
	void	OnAbyss();
	void	OnTreatment(double dirx, double diry);
	void	OnLanding();

	void	Jump();

public:
	CJumper(short initx, short inity, short width, short height, short number, tMonsterInfo *monsterInfo);
	~CJumper();

	void	OnShootSuccessful();
	void	OnShootNotSuccessful();

	short	Write(FILE *f);
	void	Read(FILE *f);
};


class CFlyer : public CMonster {
protected:
	long	nextShootTime;
	long	currentShootDelay;
	long	lastShoot;
	double	rescueMeX, rescueMeY;

	void	OnIdle();
	void	OnTreatment(double dirx, double diry);

public:
	CFlyer(short initx, short inity, short width, short height, short number, tMonsterInfo *monsterInfo);
	~CFlyer();

	void	OnShootSuccessful();
	void	OnShootNotSuccessful();
	void	OnKill();

	short	Write(FILE *f);
	void	Read(FILE *f);
};

class CWarg : public CMonster {
protected:
	CWeapon	*farWeapon;
	long	nextShootTime;
	long	currentShootDelay;
	long	lastShoot;
	short	inJump;
	double	speedup;
	long	deathTime;

	void	OnStart();
	void	OnCollision();
	void	OnIdle();
	void	OnAbyss();
	void	OnTreatment(double dirx, double diry);
	void	OnLanding();

	void	Jump();

public:
	CWarg(short initx, short inity, short width, short height, short number, tMonsterInfo *monsterInfo);
	~CWarg();

	void	OnAllocate();

	void	TestForDamage(double xb, double yb, short rad, short blessure);
	void	OnDamage(short blessure);
	void	Render(short planeX, short planeY, tRect *clipRect);
	
	short	Write(FILE *f);
	void	Read(FILE *f);
};

#endif