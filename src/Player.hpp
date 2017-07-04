#ifndef __AMP_PLAYER__
#define __AMP_PLAYER__

#include "Monster.hpp"
#include "SoundList.hpp"

enum {
	kJobTeleporting,
	kJobExiting
};

class CGUI;
struct tPlayerData;

class CPlayer : public CMonster {
	friend class CGUI;

protected:
	CShape	*playerMoveShapes[8][3];
	CShape	*jumpShapes[8];
	CShape	*attackShapes[8][3];

	CWeapon	*weapons[8];
	CWeapon	*unused;

	CSound	*jumpSound;

	long	jumpTime;
	short	jumped;
	short	currentWeapon;
	long	lastWeaponChanging;
	long	lastPickupTime;
	long	lastActionTime;
	short	teleportRefNum;
	short	currentJob;

	void	PerformAction();
	void	OnTeleport();

	double	oxygen;
	long	lastOxygenDecTime;
	
	void	OnStart();

public:
	CPlayer(short initx, short inity, short width, short height, short number, tMonsterInfo *monsterInfo);
	~CPlayer();

	short	Forces();
	void	Move();
	short	Think();

	void	OnTouch(CObject *touch);
	void	OnDamage(short blessure);

	void	SaveDataToNextLevel(tPlayerData *);
	void	RestoreDataFromNextLevel(tPlayerData *);

	short	Write(FILE *f);
	void	Read(FILE *f);

	// cheat
	void	GetItAll();

};


class CCamera : public CThing {
private:
	char	*currentMessage;
	double	msgY;

public:
	CCamera(short initx, short inity, short width, short height, short number);
	~CCamera();

	short	Forces();
	void	Move();
	short	Think();
	void	Render(short planeX, short planeY, tRect *clipRect);

};

#endif