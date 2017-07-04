#ifndef __AMP_OBJECT__
#define __AMP_OBJECT__

#include "Shape.hpp"

class CMonster;
class CGuidedBullet;
class CSoundSystem;

class	CObject {
friend class CMonster;
friend class CGuidedBullet;
friend class CSoundSystem;

protected:
	short	weight;
	double	forceVectorX, forceVectorY;
	double	environmentForceX, environmentForceY;
	double	gravitation;
	double	resForceX, resForceY;
	long	lastTime, deltaTime;
	CObject	*pusher;

public:
	unsigned long	typeID;
	short	background;
	double	xs, ys, xe, ye, xm, ym;

	CObject(short initx, short inity, short width, short height);
	~CObject();

	short	ExertForce(double &forcex, double &forcey, short &collisionObject, CObject **obstacle);
	virtual short	Collision(CObject *sender, double left, double top, double right, double bottom, double &forcex, double &forcey, double pfx, double pfy, short sourceWeight, short &collisionObject);
	virtual void	CollisionEvent(double friction, double externForce);
	virtual short	CollisionPossible(double ptx, double pty);
	virtual short	Think();
	virtual short	Forces();

	virtual void	OnTouch(CObject *touch);

	virtual short	Write(FILE *f);
	virtual void	Read(FILE *f);
};


#define WRITEDATA(data) fwrite(&data, sizeof(data), 1, f); size += sizeof(data)
#define READDATA(data) fread(&data, sizeof(data), 1, f);

#define FINISHWRITE  fseek(f, -size, SEEK_CUR); fwrite(&size, sizeof(size), 1, f); fseek(f, size - sizeof(size), SEEK_CUR);

#endif
