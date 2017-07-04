#include "Pltform.hpp"
#include "Appl.hpp"
#include "Player.hpp"
#include "ConstVal.hpp"
#include "SndSys.hpp"

extern	CApplication	*gApplication;
extern	CShapeManager	*gShapeManager;
extern	tConstValues	*gConst;
extern	CLevel			*gLevel;
extern	CSoundSystem	*gSoundSystem;

const	short	kPlatformCollisionHeight = 30;

CPlatform::CPlatform(short initx, short inity, short width, short height, short number, tPlatformInfo *platformInfo) :
	CThing(initx, inity, width, height, number)
{
	typeID |= kPlatform;
	LinkInLists();

	if (platformInfo) {
		info = platformInfo;
		isLocal = 0;
		weight = SHRT_MAX;
		background = 0;

		origHeight = dy;
		ys = ym - kPlatformCollisionHeight / 2;
		ye = ym + kPlatformCollisionHeight / 2;
		dy = kPlatformCollisionHeight;
		dx -= 1;
		ye -= 1;

		OnAllocate();

		startx = info->startx * kElementSize;
		starty = info->starty * kElementSize + (kElementSize - kPlatformCollisionHeight) / 2;
		endx = info->endx * kElementSize;
		endy = info->endy * kElementSize + (kElementSize - kPlatformCollisionHeight) / 2;

		maxDistance = sqrt((endx - startx) * (endx - startx) + (endy - starty) * (endy - starty));

		if (info->activ) action = kRunsStartToEnd; else action = kStopsAtStart;
	}else info = 0L;
}

void	CPlatform::OnAllocate()
{
	shape = gShapeManager->FindShape(info->iconID, 0);
}


CPlatform::~CPlatform()
{
	if (isLocal) delete info;
}

void	CPlatform::LinkInLists()
{
	gApplication->Enqueue(&gApplication->thingList, this);
	gApplication->Enqueue(&gApplication->collisionThingList, this);
	gApplication->Enqueue(&gApplication->renderQueue, this);
}

void	CPlatform::UnlinkInLists()
{
	gApplication->Dequeue(&gApplication->thingList, this);
	gApplication->Dequeue(&gApplication->collisionThingList, this);
	gApplication->Dequeue(&gApplication->renderQueue, this);
}

short	CPlatform::Think()
{
	double	speedVector, distance;

	CObject::Think();

	switch (action) {

	case kRunsStartToEnd:
		distance = sqrt((xs - startx) * (xs - startx) + (ys - starty) * (ys - starty));

		if (distance >= maxDistance) {
			if (info->stopsAtEnd) action = kStopsAtEnd; else action = kDelaysAtEnd;
			xs = endx; ys = endy;
			xe = xs + kElementSize -1; ye = ys + kElementSize -1;
			delayTime = lastTime;
			gSoundSystem->Play(gSoundSystem->platformStop, xm, ym);
		}else{
			speedVector = info->speed * gConst->kVelocityUnit * deltaTime;
			forceVectorX = (endx - startx) * speedVector / maxDistance;
			forceVectorY = (endy - starty) * speedVector / maxDistance;
		}
		break;

	case kRunsEndToStart:
		distance = sqrt((xs - endx) * (xs - endx) + (ys - endy) * (ys - endy));

		if (distance >= maxDistance) {
			if (info->stopsAtStart) action = kStopsAtStart; else action = kDelaysAtStart;
			xs = startx; ys = starty;
			xe = xs + kElementSize -1; ye = ys + kElementSize -1;
			delayTime = lastTime;
			gSoundSystem->Play(gSoundSystem->platformStop, xm, ym);
		}else{
			speedVector = info->speed * gConst->kVelocityUnit * deltaTime;
			forceVectorX = (startx - endx) * speedVector / maxDistance;
			forceVectorY = (starty - endy) * speedVector / maxDistance;
		}
		break;

	case kDelaysAtStart:
		if (lastTime - delayTime > info->delay * kTimeFactor) {
			action = kRunsStartToEnd;
			gSoundSystem->Play(gSoundSystem->platformGo, xm, ym);
		}
		forceVectorX = forceVectorY = 0;
		break;

	case kDelaysAtEnd:
		if (lastTime - delayTime > info->delay * kTimeFactor) {
			action = kRunsEndToStart;
			gSoundSystem->Play(gSoundSystem->platformGo, xm, ym);
		}
		forceVectorX = forceVectorY = 0;
		break;

	case kStopsAtStart:
	case kStopsAtEnd:
		forceVectorX = forceVectorY = 0;
		break;
	}
	
	return kNoEvent;
}

short	CPlatform::Forces()
{
	short	collisionObject, collisionCode;
	CObject	*collObj;

	CObject::Forces();
	double	oldfx = resForceX, oldfy = resForceY;

	collisionCode = ExertForce(resForceX, resForceY, collisionObject, &collObj);
	if (!(collisionCode & kCollisionWithPushing)) {
		if (collisionObject & (kElement | kItem | kPlatform)) {	// Platforms can go through level elements
			resForceX = oldfx; resForceY = oldfy;
		}
	}else if (collisionCode & ~kCollisionWithPushing) {
		action = (action == kRunsStartToEnd ? kRunsEndToStart : kRunsStartToEnd);
		if (info->returnsOnHit && collObj->typeID & kThing) ((CThing *)collObj)->OnDamage(info->returnsOnHit);
	
	}
	return kNoEvent;
}


void	CPlatform::Render(short planeX, short planeY, tRect *clipRect)
{
	CElement	*element = gLevel->GetElement(xm, ym);

	if (shape) shape->RenderShape(xs - planeX, ym - planeY - origHeight / 2, clipRect,
		modus, element ? element->brightness : 0, gApplication->plane);
}

short	CPlatform::Collision(CObject *sender, double left, double top, double right, double bottom, double &forcex, double &forcey, double pfx, double pfy, short sourceWeight, short &collisionObject)
{
	 if (right + 2 < xs || left - 2 > xe || bottom + 2 < ys || top - 2 > ye) return kNoCollision;

	short	returnValue;

	if ((returnValue = CObject::Collision(sender, left, top, right, bottom, forcex, forcey, pfx, pfy, sourceWeight, collisionObject)) & kCollisionOnBottom) {
		sender->CollisionEvent(gConst->kNormalFriction, 0);
		if (info->hurts) ((CThing *)sender)->OnDamage(info->hurts);

		if ((sender->typeID & kPlayer) && info->playerControls) {
			if (action == kStopsAtStart) {
				action = kRunsStartToEnd;
				gSoundSystem->Play(gSoundSystem->platformGo, xm, ym);
			}
			if (action == kStopsAtEnd) {
				action = kRunsEndToStart;
				gSoundSystem->Play(gSoundSystem->platformGo, xm, ym);
			}
		}
	}
	return returnValue;
}

void	CPlatform::OnTouch(CObject *touch)
{
   	if ((touch->typeID & kThing) && info->hurts) ((CThing *)touch)->OnDamage(info->hurts);
}

void	CPlatform::Switch()
{
	switch (action) {
		case kRunsStartToEnd:
		case kDelaysAtStart:
			action = kStopsAtStart;
			gSoundSystem->Play(gSoundSystem->platformStop, xm, ym);
			break;
		case kRunsEndToStart:
		case kDelaysAtEnd:
			action = kStopsAtEnd;
			gSoundSystem->Play(gSoundSystem->platformStop, xm, ym);
			break;
		case kStopsAtStart:
			action = kRunsStartToEnd;
			gSoundSystem->Play(gSoundSystem->platformGo, xm, ym);
			if (info->refNum > 0) gApplication->platformTable[info->refNum]->Switch();
			break;
		case kStopsAtEnd:
			action = kRunsEndToStart;
			gSoundSystem->Play(gSoundSystem->platformGo, xm, ym);
			if (info->refNum > 0) gApplication->platformTable[info->refNum]->Switch();
			break;
	}
}


CShape	*CPlatform::GetCurrentState(CShape *active, CShape *inactive)
{
	if (action == kStopsAtStart || action == kStopsAtEnd) return inactive; else return active;
}



short	CPlatform::Write(FILE *f)
{
	long	size = 0;

	WRITEDATA(size);
	WRITEDATA(typeID);
	WRITEDATA(thingNumber);

	size += CThing::Write(f);

	WRITEDATA(origHeight);
	WRITEDATA(action);
	WRITEDATA(startx);
	WRITEDATA(starty);
	WRITEDATA(endx);
	WRITEDATA(endy);
	WRITEDATA(maxDistance);
	WRITEDATA(delayTime);
	fwrite(info, sizeof(tPlatformInfo), 1, f); size += sizeof(tPlatformInfo);

	FINISHWRITE;

	return size;
}

void	CPlatform::Read(FILE *f)
{
	long	size = 0;

	READDATA(size);
	READDATA(typeID);
	READDATA(thingNumber);

	CThing::Read(f);

	READDATA(origHeight);
	READDATA(action);
	READDATA(startx);
	READDATA(starty);
	READDATA(endx);
	READDATA(endy);
	READDATA(maxDistance);
	READDATA(delayTime);

	isLocal = 1;
	info = new tPlatformInfo;
	fread(info, sizeof(tPlatformInfo), 1, f);

	OnAllocate();
}