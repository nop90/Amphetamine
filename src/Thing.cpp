#include "Thing.hpp"
#include "Appl.hpp"
#include "ConstVal.hpp"

extern CApplication *gApplication;
extern tConstValues	*gConst;
extern CLevel		*gLevel;


CThing::CThing(short initx, short inity, short width, short height, short number) : CObject(initx, inity, width, height)
{
	typeID |= kThing;

	dx = xe - xs;
	dy = ye - ys;

	weight = 10;
	next = prev = nextCollisionThing = prevCollisionThing = 0L;
	lookDirection = kLookingRight;
	weightless = 0;
	thingNumber = number;

	// additinal initializations
	modus = 0;	
}

CThing::~CThing()
{}

void	CThing::Gravitation()
{
	if (weight && !weightless && forceVectorY < gConst->kMaxFallingSpeed * deltaTime * gConst->kVelocityUnit) forceVectorY += gConst->kGravitation * deltaTime; 
}


void	CThing::LinkInLists()
{
	gApplication->Enqueue(&gApplication->thingList, this);
}

void	CThing::UnlinkInLists()
{
	gApplication->Dequeue(&gApplication->thingList, this);
}


void	CThing::Render(short planeX, short planeY, tRect *clipRect)
{}

void	CThing::PostRender(short planeX, short planeY, tRect *clipRect)
{}

short	CThing::Forces()
{
	short	collisionObject;

	Gravitation();
	
	CObject::Forces();
	ExertForce(resForceX, resForceY, collisionObject, 0L);

	return kNoEvent;
}

void	CThing::Move()
{
	CElement	*element;

	//CObject::Forces();

	if (ys < kLevelHeight * kElementSize) {
		xs += resForceX + environmentForceX;
		xe += resForceX + environmentForceX;
		ys += resForceY + environmentForceY;
		ye += resForceY + environmentForceY;
		xm = xs + (xe - xs) * 0.5;
		ym = ys + (ye - ys) * 0.5;
	}
	element = gLevel->GetElement(xm, ym);
	modus = (element ? element->GetElementLiquid() : kShapemodusNormal);
}

void	CThing::CalcPlaneOffsets(short &planex, short &planey)
{
	short	midx = xm;
	short	midy = ym;

	if (midx <= kGamePlaneWidth / 2) planex = 0;
	else if (midx >= kLevelWidth * kElementSize - kGamePlaneWidth / 2) planex = kLevelWidth * kElementSize - kGamePlaneWidth;
	else planex = midx - kGamePlaneWidth / 2;

	if (midy <= kGamePlaneHeight / 2) planey = 0;
	else if (midy >= kLevelHeight * kElementSize - kGamePlaneHeight / 2) planey = kLevelHeight * kElementSize - kGamePlaneHeight;
	else planey = midy - kGamePlaneHeight / 2;
}

short	CThing::Collision(CObject *sender, double left, double top, double right, double bottom, double &forcex, double &forcey, double pfx, double pfy, short sourceWeight, short &collisionObject)
{
	if (right + forcex < xs || left + forcex > xe || 
		bottom + forcey < ys || top + forcey > ye) return kNoCollision;

	short	returnValue = CObject::Collision(sender, left, top, right, bottom, forcex, forcey, pfx, pfy, sourceWeight, collisionObject);

	if (returnValue & (kCollisionOnBottom | kCollisionWithPushing)) {
		sender->CollisionEvent(gConst->kNormalFriction, 0);
	}
	return returnValue;
}

// ------------------------------------------------------------------------------
void	CThing::TestForDamage(double xb, double yb, short rad, short blessure)
// Sending a damage event to the thing if the distance to the bullet is less than rad
{
	if ((xm - xb) * (xm - xb) + (ym - yb) * (ym - yb) <= rad * rad) OnDamage(blessure);
}


void	CThing::OnDamage(short blessure)
{
}


short	CThing::Write(FILE *f)
{
	long	size = 0;

	WRITEDATA(size);
	WRITEDATA(typeID);
	WRITEDATA(thingNumber);

	size += CObject::Write(f);

	WRITEDATA(lookDirection);
	WRITEDATA(weightless);
	WRITEDATA(dx);
	WRITEDATA(dy);
	WRITEDATA(modus);

	FINISHWRITE;

	return size;
}

void	CThing::Read(FILE *f)
{
	long	size = 0;

	READDATA(size);
	READDATA(typeID);
	READDATA(thingNumber);

	CObject::Read(f);

	READDATA(lookDirection);
	READDATA(weightless);
	READDATA(dx);
	READDATA(dy);
	READDATA(modus);
}
