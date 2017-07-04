#ifndef __AMP_THING__
#define __AMP_THING__

#include "Object.hpp"

class CWeapon;
class CHandWeapon;
class CSorcery;
class CBomb;
class CMultiBulletWeapon;
class CStaff;
class CSineWeapon;
class CGuided;

enum {	// Looking values
	kLookingRight = 1,
	kLookingLeft = -1
};	

class CThing : public CObject {
	friend class CWeapon;
	friend class CHandWeapon;
	friend class CSorcery;
	friend class CBomb;
	friend class CMultiBulletWeapon;
	friend class CStaff;
	friend class CSineWeapon;
	friend class CGuided;

protected:
	short	lookDirection;	// in which direction the thing looks
	short	weightless;
	short	dx, dy;
	short	modus;

public:
	CThing	*next, *prev, *nextCollisionThing, *prevCollisionThing;
	CThing	*nextPreRenderThing, *prevPreRenderThing;
	CThing	*nextPostRenderThing, *prevPostRenderThing;

	short	thingNumber;


	CThing(short initx, short inity, short width, short height, short number);
	~CThing();

	void			Gravitation();
	virtual void	LinkInLists();
	virtual void	UnlinkInLists();

	virtual void	Render(short planeX, short planeY, tRect *clipRect);
	virtual void	PostRender(short planeX, short planeY, tRect *clipRect);
	virtual void	Move();
	virtual short	Forces();
	void			CalcPlaneOffsets(short &planex, short &planey);
	short			Collision(CObject *sender, double left, double top, double right, double bottom, double &forcex, double &forcey, double pfx, double pfy, short sourceWeight, short &collisionObject);
	virtual void	TestForDamage(double xb, double yb, short rad, short blessure);

	virtual	void	OnDamage(short blessure);

	virtual short	Write(FILE *f);
	virtual void	Read(FILE *f);

	

};

#endif