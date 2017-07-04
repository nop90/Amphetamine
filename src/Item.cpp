#include "Item.hpp"
#include "Appl.hpp"
#include "Clut.hpp"
#include "SndSys.hpp"

extern CApplication *gApplication;
extern CShapeManager *gShapeManager;
extern CLevel		 *gLevel;
extern	tConstValues	*gConst;
extern CClutManager	*gClutManager;
extern CSoundSystem	*gSoundSystem;

CItem::CItem(short initx, short inity, short width, short height, short number, tItemInfo *itemInfo) : 
	CThing(initx, inity, width, height, number)
{
	typeID |= kItem;

	info = itemInfo;
	shape = gShapeManager->FindShape(itemInfo->iconID, 0);
	currentShape = shape;
	detonationStartTime = -1;
	isCorona = info->iconID >= kCoronaIDs[0] && info->iconID <= kCoronaIDs[kNumOfCoronas -1];
	coronaFader = 0.0;
	deltaTime = 0;
	dx -= 1;
	xe -= 1;
}

CItem::~CItem()
{}

short	CItem::Think()
{
	CObject::Think();

	if (detonationStartTime != -1) {
		long	frame = (long)((double)(lastTime - detonationStartTime) * gConst->kDetonationFrameTime);
		
		if (frame > 4) 
			return kDestroyMe;
		else 
			currentShape = gShapeManager->FindShape(gConst->kItemExplosionStartShape + frame, 0);
	}
	return kNoEvent;
}

void	CItem::Move()
{
	CThing::Move();
	environmentForceX = environmentForceY = 0;
}

void	CItem::Render(short planeX, short planeY, tRect *clipRect)
{
	CElement	*element = gLevel->GetElement(xm, ym);

	if (!isCorona && currentShape) currentShape->RenderShape(xs - planeX, ys - planeY, clipRect, 
		modus, element ? element->brightness : 0, gApplication->plane);
	else if (isCorona) 
		gClutManager->PrepareCorona(xm, ym, gApplication->plane);
}

void	CItem::PostRender(short planeX, short planeY, tRect *clipRect)
{
	if (isCorona) {
		CElement	*element = gLevel->GetElement(xm, ym);
		gClutManager->DrawCorona(xm, ym, info->iconID - kCoronaIDs[0], element->data ? element->data : 1, gApplication->plane, coronaFader, deltaTime);
	}
}

void	CItem::OnDamage(short blessure)
{
	if (blessure && info->flags & kItemExplodesMask && detonationStartTime == -1) {
		tThingList	*currentEntry = gApplication->collisionThingList;

		detonationStartTime = lastTime;

		while (currentEntry) {
			if (currentEntry->thing != this) currentEntry->thing->TestForDamage(xs, ys, gConst->kItemExplosionRad, info->data);
			currentEntry = currentEntry->next;
		}
	}
}

CStaticItem::CStaticItem(short initx, short inity, short width, short height, short number, tItemInfo *itemInfo) : 
	CItem(initx, inity, width, height, number, itemInfo)
{
	typeID |= kStaticItem;
	background = 0;
	weight = 32000;
	weightless = 1;
	LinkInLists();
}

CStaticItem::~CStaticItem()
{
}

void	CStaticItem::LinkInLists()
{
	gApplication->Enqueue(&gApplication->thingList, this);
	gApplication->Enqueue(&gApplication->collisionThingList, this);
	if (isCorona) {
		gApplication->Enqueue(&gApplication->preRenderQueue, this);
		gApplication->Enqueue(&gApplication->postRenderQueue, this);
	}else gApplication->Enqueue(&gApplication->renderQueue, this);
}

void	CStaticItem::UnlinkInLists()
{
	gApplication->Dequeue(&gApplication->thingList, this);
	gApplication->Dequeue(&gApplication->collisionThingList, this);
	if (isCorona) {
		gApplication->Dequeue(&gApplication->preRenderQueue, this);
		gApplication->Dequeue(&gApplication->postRenderQueue, this);
	}else gApplication->Dequeue(&gApplication->renderQueue, this);
}

short	CStaticItem::Collision(CObject *sender, double left, double top, double right, double bottom, double &forcex, double &forcey, double pfx, double pfy, short sourceWeight, short &collisionObject)
{
	short	returnValue = CThing::Collision(sender, left, top, right, bottom, forcex, forcey, pfx, pfy, sourceWeight, collisionObject); 
	if (returnValue && info->flags & kItemHurtMask) {
		((CThing *)sender)->OnDamage(info->data);
	}
	
	return returnValue;
}


CBackgroundItem::CBackgroundItem(short initx, short inity, short width, short height, short number, tItemInfo *itemInfo) : 
	CItem(initx, inity, width, height, number, itemInfo)
{
	typeID |= kBackgroundItem;
	weight = 0;	// Background items are hanging in the air, they don't fall, so they have no weight
	background = 1;
	weightless = 1;

	LinkInLists();
}

CBackgroundItem::~CBackgroundItem()
{
}

void	CBackgroundItem::LinkInLists()
{
	gApplication->Enqueue(&gApplication->thingList, this);
	gApplication->Enqueue(&gApplication->preRenderQueue, this);
	if (isCorona) {
		gApplication->Enqueue(&gApplication->postRenderQueue, this);
	}
}

void	CBackgroundItem::UnlinkInLists()
{
	gApplication->Dequeue(&gApplication->thingList, this);
	gApplication->Dequeue(&gApplication->preRenderQueue, this);
	if (isCorona) {
		gApplication->Dequeue(&gApplication->postRenderQueue, this);
	}
}

CMovableItem::CMovableItem(short initx, short inity, short width, short height, short number, tItemInfo *itemInfo) : 
	CItem(initx, inity, width, height, number, itemInfo)
{
	typeID |= kMovableItem;
	weight = 1;
	background = 0;
	LinkInLists();
}

CMovableItem::~CMovableItem()
{
}

void	CMovableItem::LinkInLists()
{
	gApplication->Enqueue(&gApplication->thingList, this);
	gApplication->Enqueue(&gApplication->collisionThingList, this);
	if (isCorona) {
		gApplication->Enqueue(&gApplication->preRenderQueue, this);
		gApplication->Enqueue(&gApplication->postRenderQueue, this);
	}else gApplication->Enqueue(&gApplication->renderQueue, this);
}

void	CMovableItem::UnlinkInLists()
{
	gApplication->Dequeue(&gApplication->thingList, this);
	gApplication->Dequeue(&gApplication->collisionThingList, this);
	if (isCorona) {
		gApplication->Dequeue(&gApplication->preRenderQueue, this);
		gApplication->Dequeue(&gApplication->postRenderQueue, this);
	}else gApplication->Dequeue(&gApplication->renderQueue, this);
}


CPortableItem::CPortableItem(short initx, short inity, short width, short height, short number, tItemInfo *itemInfo, short itemNo) :
	CItem(initx, inity, width, height, number, itemInfo)
{
	typeID |= kPortableItem;
	type = itemNo;
	background = 0;
	weight = 1;
	pickedUp = 0;
	LinkInLists();
}

CPortableItem::~CPortableItem()
{
}

void	CPortableItem::LinkInLists()
{
	gApplication->Enqueue(&gApplication->thingList, this);
	gApplication->Enqueue(&gApplication->collisionThingList, this);
	if (isCorona) {
		gApplication->Enqueue(&gApplication->preRenderQueue, this);
		gApplication->Enqueue(&gApplication->postRenderQueue, this);
	}else gApplication->Enqueue(&gApplication->renderQueue, this);
}

void	CPortableItem::UnlinkInLists()
{
	gApplication->Dequeue(&gApplication->thingList, this);
	gApplication->Dequeue(&gApplication->collisionThingList, this);
	if (isCorona) {
		gApplication->Dequeue(&gApplication->preRenderQueue, this);
		gApplication->Dequeue(&gApplication->postRenderQueue, this);
	}else gApplication->Dequeue(&gApplication->renderQueue, this);
}


short	CPortableItem::Think()
{
	CObject::Think();

	if (pickedUp) return kDestroyMe; else return kNoEvent;
}

short	CPortableItem::PickMeUp(short &value)
{
	value = info->data;
	pickedUp = 1;

	return type;
}

short	CItem::Write(FILE *f)
{
	long	size = 0;

	WRITEDATA(size);
	WRITEDATA(typeID);
	WRITEDATA(thingNumber);

	size += CThing::Write(f);

	WRITEDATA(isCorona);
	WRITEDATA(detonationStartTime);

	FINISHWRITE;

	return size;
}


void	CItem::Read(FILE *f)
{
	long	size = 0;

	READDATA(size);
	READDATA(typeID);
	READDATA(thingNumber);

	CThing::Read(f);

	READDATA(isCorona);
	READDATA(detonationStartTime);
}

short	CPortableItem::Write(FILE *f)
{
	long	size = 0;

	WRITEDATA(size);
	WRITEDATA(typeID);
	WRITEDATA(thingNumber);

	size += CItem::Write(f);

	WRITEDATA(pickedUp);
	WRITEDATA(type);

	FINISHWRITE;

	return size;
}


void	CPortableItem::Read(FILE *f)
{
	long	size = 0;

	READDATA(size);
	READDATA(typeID);
	READDATA(thingNumber);

	CItem::Read(f);

	READDATA(pickedUp);
	READDATA(type);
}