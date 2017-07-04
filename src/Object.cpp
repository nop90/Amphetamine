#include "Object.hpp"
#include "Appl.hpp"
#include "ConstVal.hpp"

#include <math.h>

enum {
	kLeftTopCorner = 0,
	kLeftBottomCorner = 1,
	kRightTopCorner = 2,
	kRightBottomCorner = 3
};

const double	kCollisionThingDetectError = 3.0;

extern	CApplication	*gApplication;
extern	CSystem			*gSystem;
extern	CLevel			*gLevel;
extern	tConstValues	*gConst;

CObject::CObject(short initx, short inity, short width, short height)
{
	typeID = 0;
	typeID |= kObject;

	xm = initx;
	ym = inity;
	xs = xm - width / 2;
	ys = ym - height / 2;
	xe = xs + width;
	ye = ys + height;


	forceVectorX = forceVectorY = 0;
	gravitation = 0;
	environmentForceX = environmentForceY = 0;
	lastTime = 0;

	// additinal initializations
	weight = 0;
	resForceX = resForceY = 0;
	deltaTime = 0;
	background = 0;	
}

CObject::~CObject()
{}


short	CObject::Think()
{
	deltaTime = gApplication->deltaTime;
	lastTime = gApplication->time;
	pusher = 0L;
	environmentForceX = environmentForceY = 0;

	return kNoEvent;
}

short	CObject::Forces() 
{
	resForceX = forceVectorX;
	resForceY = forceVectorY + gravitation;

	return kNoEvent;
}


void	CalcSteps(double forcex, double forcey, double &sx, double &sy)
{
	if (ABS(forcex) > ABS(forcey)) {
		sx += SIGN(forcex);
		sy += forcey / ABS(forcex);
	}else{
		sy += SIGN(forcey);
		sx += forcex / ABS(forcey);
	}
}

short	CObject::ExertForce(double &forcex, double &forcey, short &collisionObject, CObject **obstacle)
{
	double	sx = 0, sy = 0;
	short	oldElem1x = -1, elem1x, oldElem1y = -1, elem1y;
	short	oldElem2x = -1, elem2x, oldElem2y = -1, elem2y;
	short	oldElem3x = -1, elem3x, oldElem3y = -1, elem3y;
	short	oldElem4x = -1, elem4x, oldElem4y = -1, elem4y;
	short	resultCode = 0, oldResultCode;
	double	oldForcex = forcex, oldForcey = forcey;
	tThingList	*currentEntry;
	double	postForcex, postForcey;
	short	tmpResultCode;

	if (obstacle) *obstacle = 0L;
	if (!forcex && !forcey) return kNoCollision;


	do {
		if (xs + sx <= 0) {
			resultCode |= kCollisionOnLeft;
			resultCode |= kCollisionWithLevelBorders;
		}
		if (xs + sx >= kLevelWidth * kElementSize -1) {
			resultCode |= kCollisionOnRight;
			resultCode |= kCollisionWithLevelBorders;
		}
		if (ys + sy <= 0) {
			resultCode |= kCollisionOnTop;
			resultCode |= kCollisionWithLevelBorders;
		}

		postForcex = (ABS(sx) > ABS(forcex) ? forcex : forcex - sx);
		postForcey = (ABS(sy) > ABS(forcey) ? forcey : forcey - sy);

		elem1x = (xs + sx) / kElementSize;
		elem1y = (ys + sy) / kElementSize;
		if ((elem1x != oldElem1x || elem1y != oldElem1y) && elem1x >= 0 && elem1y >= 0 && elem1x < kLevelWidth && elem1y < kLevelHeight) {
			resultCode |= gLevel->level[elem1y][elem1x]->Collision(this, xs + sx, ys + sy, xe + sx, ye + sy, forcex, forcey, postForcex, postForcey, weight, collisionObject);
			oldElem1x = elem1x;
			oldElem1y = elem1y;
		}
	
		elem2x = (xe + sx) / kElementSize;
		elem2y = (ys + sy) / kElementSize;
		if ((elem2x != oldElem2x || elem2y != oldElem2y) && elem2x >= 0 && elem2y >= 0 && elem2x < kLevelWidth && elem2y < kLevelHeight) {
			resultCode |= gLevel->level[elem2y][elem2x]->Collision(this, xs + sx, ys + sy, xe + sx, ye + sy, forcex, forcey, postForcex, postForcey, weight, collisionObject);
			oldElem2x = elem2x;
			oldElem2y = elem2y;
		}
		
		elem3x = (xs + sx) / kElementSize;
		elem3y = (ye + sy) / kElementSize;
		if ((elem3x != oldElem3x || elem3y != oldElem3y) && elem3x >= 0 && elem3y >= 0 && elem3x < kLevelWidth && elem3y < kLevelHeight) {
			resultCode |= gLevel->level[elem3y][elem3x]->Collision(this, xs + sx, ys + sy, xe + sx, ye + sy, forcex, forcey, postForcex, postForcey, weight, collisionObject);
			oldElem3x = elem3x;
			oldElem3y = elem3y;
		}

		elem4x = (xe + sx) / kElementSize;
		elem4y = (ye + sy) / kElementSize;
		if ((elem4x != oldElem4x || elem4y != oldElem4y) && elem4x >= 0 && elem4y >= 0 && elem4x < kLevelWidth && elem4y < kLevelHeight) {
			resultCode |= gLevel->level[elem4y][elem4x]->Collision(this, xs + sx, ys + sy, xe + sx, ye + sy, forcex, forcey, postForcex, postForcey, weight, collisionObject);
			oldElem4x = elem4x;
			oldElem4y = elem4y;
		}

		currentEntry = gApplication->collisionThingList;
		while (currentEntry) {
			oldResultCode = resultCode;
			tmpResultCode = 0;
			if (currentEntry->thing != this) tmpResultCode = currentEntry->thing->Collision(this, xs + sx, ys + sy, xe + sx, ye + sy, forcex, forcey, postForcex, postForcey, weight, collisionObject);
			if (obstacle && tmpResultCode) 
				*obstacle = currentEntry->thing;
			resultCode |= tmpResultCode;
			currentEntry = currentEntry->next;
		}

		if ((resultCode & kCollisionOnTop) || (resultCode & kCollisionOnBottom)) {
			if ((resultCode & kCollisionOnLeft) || (resultCode & kCollisionOnRight)) forcex = forcey = 0;
		}

		CalcSteps(forcex, forcey, sx, sy);

	} while ((forcex || forcey) && (ABS(sx) < ABS(forcex) || !forcex) && (ABS(sy) < ABS(forcey) || !forcey));

	if (ABS(forcex) > 1.0 && ((resultCode & kCollisionOnLeft) || (resultCode & kCollisionOnRight))) forcex = sx;
	if (ABS(forcey) > 1.0 &&  ((resultCode & kCollisionOnTop) || (resultCode & kCollisionOnBottom))) forcey = sy;

	if ((resultCode & kCollisionOnTop) && forcey < 0) forceVectorY = forcey = 0;
	if (resultCode & kCollisionOnBottom) forceVectorY = forcey = 0;

	return resultCode;
}

void	CObject::OnTouch(CObject *touch) {}

// Determines if the line a1-a2 intersects with the line b1-b2 where equality does not count
short	PointIntersection(double a1, double a2, double b1, double b2)
{
	return !((a1 >= b1 && a2 >= b1 && a1 >= b2 && a2 >= b2) || (a1 <= b1 && a2 <= b1 && a1 <= b2 && a2 <= b2));
}

// Determines if the line a1-a2 intersects with the line b1-b2 where equality does count
short	PointTouch(double a1, double a2, double b1, double b2)
{
	return !((a1 > b1 && a2 > b1 && a1 > b2 && a2 > b2) || (a1 < b1 && a2 < b1 && a1 < b2 && a2 < b2));
}


short	CObject::CollisionPossible(double ptx, double pty)
{
	return (!background && ptx >= xs && ptx < xe && pty >= ys && pty < ye);
}

short	CObject::Collision(CObject *sender, double left, double top, double right, double bottom, double &forcex, double &forcey, double postForcex, double postForcey, short sourceWeight, short &collisionObject)
{						   
	if (background || (!forcex && !forcey)) return kNoCollision;
		

	if (weight < sourceWeight) {
		short	carryx = 0, carryy = 0, collCode = kCollisionWithPushing;
		double	tmpForcex = forcex, tmpForcey = forcey;
                      
		pusher = sender;
		sender->OnTouch(this);

		if (((short)left >= (short)xe - kCollisionThingDetectError && (short)(left + forcex) <= (short)xe) || 
			((short)right <= (short)xs + kCollisionThingDetectError && (short)(right + forcex + kCollisionThingDetectError) >= (short)xs)) {
			carryy = 1; forcey = 0;
		}
		if ((short)bottom <= (short)ys + kCollisionThingDetectError && (short)(bottom + forcey) >= (short)ys) {
			carryx = 1; forcex = 0;
			collCode |= kCollisionOnBottom;
		}
		collCode |= ExertForce(forcex, forcey, collisionObject, 0L);
		environmentForceX = forcex;
		environmentForceY = forcey;

		if (carryx) forcex = tmpForcex;
		if (carryy) forcey = tmpForcey;

		return collCode;

	} else {
		if ((short)right == (short)xs && (PointIntersection(floor(top), floor(bottom), floor(ys), floor(ye) -1) || 
			((short)top == (short)ys && (short)bottom == (short)ye))) {
			if (forcex > 0) {
				forcex = 0;
				//sender->OnTouch(this);
				collisionObject = typeID;
				return kCollisionOnRight; 
			}
		}

		if ((short)left == (short)xe -1 && (PointIntersection(floor(top), floor(bottom), floor(ys), floor(ye) -1) || 
			((short)top == (short)ys && (short)bottom == (short)ye))) {
			if (forcex < 0) {
				forcex = 0;
				//sender->OnTouch(this);
				collisionObject = typeID;
				return kCollisionOnLeft; 
			}
		}

		if (((short)bottom >= (short)ys && (short)bottom <= (short)ys + 3) && PointIntersection(floor(left), floor(right), floor(xs), floor(xe) -1)) {
			if (forcey > 0) {
				forcey = 0;
				//sender->OnTouch(this);
				collisionObject = typeID;
				return kCollisionOnBottom;
			}
		}

		if ((short)top == (short)ye -1 && PointIntersection(floor(left), floor(right), floor(xs), floor(xe) -1)) {
			if (forcey < 0) {
				forcey = 0;
				//sender->OnTouch(this);
				collisionObject = typeID;
				return kCollisionOnTop;
			}
		}

		return kNoCollision;
	}
}

void	CObject::CollisionEvent(double friction, double externForce)
{
	double	theorForce;

	if (forceVectorX) {
		theorForce = forceVectorX - SIGN(forceVectorX) * friction * deltaTime;
		if (SIGN(theorForce) != SIGN(forceVectorX))
			forceVectorX = 0;
		else forceVectorX = theorForce;
	}
	if (externForce) environmentForceX += externForce * deltaTime * gConst->kVelocityUnit;
}



short	CObject::Write(FILE *f)
{
	long	size = 0;
	
	WRITEDATA(size);
	WRITEDATA(typeID);

	WRITEDATA(xs); 
	WRITEDATA(ys);
	WRITEDATA(xe);
	WRITEDATA(ye);
	WRITEDATA(xm);
	WRITEDATA(ym);
	WRITEDATA(weight);
	WRITEDATA(forceVectorX);
	WRITEDATA(forceVectorY);
	WRITEDATA(environmentForceX);
	WRITEDATA(environmentForceY);
	WRITEDATA(gravitation);
	WRITEDATA(resForceX);
	WRITEDATA(resForceY);

	WRITEDATA(background);

	FINISHWRITE;

	return size;
}

void	CObject::Read(FILE *f)
{
	long	size;

	READDATA(size);
	READDATA(typeID);

	READDATA(xs); 
	READDATA(ys);
	READDATA(xe);
	READDATA(ye);
	READDATA(xm);
	READDATA(ym);
	READDATA(weight);
	READDATA(forceVectorX);
	READDATA(forceVectorY);
	READDATA(environmentForceX);
	READDATA(environmentForceY);
	READDATA(gravitation);
	READDATA(resForceX);
	READDATA(resForceY);
	READDATA(background);
}
