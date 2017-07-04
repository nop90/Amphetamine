#include "Element.hpp"
#include "Appl.hpp"
#include "Gui.hpp"
#include "SndSys.hpp"

extern	CApplication	*gApplication;
extern	CShapeManager	*gShapeManager;
extern	CSystem			*gSystem;
extern	tConstValues	*gConst;
extern	CLevel			*gLevel;
extern	CGUI			*gGUI;
extern	CSoundSystem	*gSoundSystem;

CElement::CElement(short initx, short inity, short width, short height, tLevelElement *levelElement) : CObject(initx, inity, width, height)
{
	typeID |= kElement;

	if (levelElement) {
		brightness = 3 - LOBYTE(levelElement->light);
		brightness2 = 3 - HIBYTE(levelElement->light);
		iconID = levelElement->iconID;
		bitData = levelElement->bitData;
		lastState = 0;

		refNum = LOBYTE(levelElement->refNum);
		key = HIBYTE(levelElement->refNum);
		data = HIBYTE(levelElement->kind);

		OnAllocate();

		weight = SHRT_MAX;
		background = levelElement->kind;
	}
}

void	CElement::OnAllocate()
{
	if (iconID > 0) {
		shape = gShapeManager->FindShape(iconID, brightness);
		shape2 = gShapeManager->FindShape(iconID, brightness2);
	}

	// If there's something on that element, allocate the struct which holds the several shapes
	if (bitData & (kPlatformSwitchMask | kLightSwitchMask | kTeleportMask | kSaveMask | kExitMask | kPassiveLightMask | kPassivePlatformMask | kRightDriftMask | kLeftDriftMask | kInfotextMask)) {
		elementThings = new tElementThings;
		
		if (bitData & kPlatformSwitchMask) {
			elementThings->platformSwitchInactive = gShapeManager->FindShape(kPlatformSwitchInactiveID, 0); 
			elementThings->platformSwitchActive = gShapeManager->FindShape(kPlatformSwitchActiveID, 0); 
			elementThings->referencedPlatform = gApplication->platformTable[refNum];
		}else{
			elementThings->platformSwitchInactive = elementThings->platformSwitchActive = 0L;
			elementThings->referencedPlatform = 0L;
		}
		if (bitData & kPassivePlatformMask) {
			elementThings->referencedPlatform = gApplication->platformTable[refNum];
			elementThings->passiveHasChanged = 0;
		}

		if (bitData & kLightSwitchMask) {
			elementThings->lightSwitch = gShapeManager->FindShape(kLightSwitchInactiveID, 0); 
			elementThings->lightSwitch2 = gShapeManager->FindShape(kLightSwitchActiveID, 0); 
		}else{
			elementThings->lightSwitch = elementThings->lightSwitch2 = 0L;
		}

		if (bitData & kTeleportMask) {
			elementThings->teleporter = gShapeManager->FindShape(kTeleportStateOne, 0);
			elementThings->teleporter2 = gShapeManager->FindShape(kTeleportStateTwo, 0);
		}else{
			elementThings->teleporter = elementThings->teleporter2 = 0L;
		}
		elementThings->teleportBlinkTime = 0;

		if (bitData & kSaveMask) {
			elementThings->savePort = gShapeManager->FindShape(kSavePortStateOne, 0); 
			elementThings->savePort2 = gShapeManager->FindShape(kSavePortStateTwo, 0); 
		}else{
			elementThings->savePort = elementThings->savePort2 = 0L;
		}
		elementThings->savePortBlinkTime = 0;

		if (bitData & kExitMask) {
			elementThings->exitPort = gShapeManager->FindShape(kExitStateOne, 0);
			elementThings->exitPort2 = gShapeManager->FindShape(kExitStateTwo, 0); 
		}else{
			elementThings->exitPort = elementThings->exitPort2 = 0L;
		}
		elementThings->exitPortBlinkTime = 0;

		elementThings->passiveHasChanged = 0;

		elementThings->drift = 0L;
		if (bitData & kLeftDriftMask) elementThings->drift = gShapeManager->FindShape (kDriftLeftID, 0);
		if (bitData & kRightDriftMask) elementThings->drift = gShapeManager->FindShape(kDriftRightID, 0);
	
		if (bitData & kInfotextMask) {
			elementThings->infoPort = gShapeManager->FindShape(kInfotextStateOne, 0);
			elementThings->infoPort2 = gShapeManager->FindShape(kInfotextStateTwo, 0);
		}else{
			elementThings->infoPort = elementThings->infoPort2 = 0L;
		}
		elementThings->infoPortBlinkTime = 0;

	}else elementThings = 0L;
}


CElement::~CElement()
{
	if (elementThings) delete elementThings;
}


void	CElement::LinkPlatforms()
{
	if (elementThings) {
		if (bitData & kPlatformSwitchMask) {
			elementThings->referencedPlatform = gApplication->platformTable[refNum];
		}else{
			elementThings->referencedPlatform = 0L;
		}
		if (bitData & kPassivePlatformMask) {
			elementThings->referencedPlatform = gApplication->platformTable[refNum];
			elementThings->passiveHasChanged = 0;
		}
	}
}


void	CElement::PaintElementThings(short planeX, short planeY, tRect *clipRect)
{
	CShape	*tmp;

	if (bitData & kPlatformSwitchMask && elementThings->referencedPlatform) 
		elementThings->referencedPlatform->GetCurrentState(elementThings->platformSwitchActive, elementThings->platformSwitchInactive)->RenderShape(xs - planeX, ys - planeY, clipRect, kShapemodusNormal, 0, gApplication->plane);
		
	if (elementThings->lightSwitch) elementThings->lightSwitch->RenderShape(xs - planeX, ys - planeY, clipRect, kShapemodusNormal, 0, gApplication->plane);
		
	if (elementThings->teleporter) {
			elementThings->teleporter->RenderShape(xs - planeX, ys - planeY, clipRect, kShapemodusNormal, 0, gApplication->plane);
			if (elementThings->teleportBlinkTime < gSystem->GetTicks()) {
				tmp = elementThings->teleporter; elementThings->teleporter = elementThings->teleporter2; elementThings->teleporter2 = tmp;
				elementThings->teleportBlinkTime = gSystem->GetTicks() + gConst->kTeleportBlinkTime;
			}
	}
	if (elementThings->savePort) {
			elementThings->savePort->RenderShape(xs - planeX, ys - planeY, clipRect, kShapemodusNormal, 0, gApplication->plane);
			if (elementThings->savePortBlinkTime < gSystem->GetTicks()) {
				tmp = elementThings->savePort; elementThings->savePort = elementThings->savePort2; elementThings->savePort2 = tmp;
				elementThings->savePortBlinkTime = gSystem->GetTicks() + gConst->kSavePortBlinkTime;
			}
	}

	if (elementThings->exitPort) {
			elementThings->exitPort->RenderShape(xs - planeX, ys - planeY, clipRect, kShapemodusNormal, 0, gApplication->plane);
			if (elementThings->exitPortBlinkTime < gSystem->GetTicks()) {
				tmp = elementThings->exitPort; elementThings->exitPort = elementThings->exitPort2; elementThings->exitPort2 = tmp;
				elementThings->exitPortBlinkTime = gSystem->GetTicks() + gConst->kExitPortBlinkTime;
			}
	}	
	if (elementThings->drift) elementThings->drift->RenderShape(xs - planeX, ys - planeY, clipRect, kShapemodusNormal, 0, gApplication->plane);

	if (elementThings->infoPort) {
			elementThings->infoPort->RenderShape(xs - planeX, ys - planeY, clipRect, kShapemodusNormal, 0, gApplication->plane);
			if (elementThings->infoPortBlinkTime < gSystem->GetTicks()) {
				tmp = elementThings->infoPort; elementThings->infoPort = elementThings->infoPort2; elementThings->infoPort2 = tmp;
				elementThings->infoPortBlinkTime = gSystem->GetTicks() + gConst->kExitPortBlinkTime;
			}
	}	
}

void	CElement::PaintElement(short planeX, short planeY, tRect *clipRect)
{
	long	timeStamp;

	if (shape) shape->RenderShape(xs - planeX, ys - planeY, clipRect, 0, 0, gApplication->plane);

	if (bitData & kFlickeringMask) {
		timeStamp = (gApplication->syncTime / gConst->kFlickeringPeriod) & 1;
		if ((long)(timeStamp) != lastState) {
			SwapLights();
			lastState = timeStamp;
		}
	}

	if (elementThings) PaintElementThings(planeX, planeY, clipRect);
}


short	CElement::Collision(CObject *sender, double left, double top, double right, double bottom, double &forcex, double &forcey, double pfx, double pfy, short sourceWeight, short &collisionObject)
{
	short returnValue;
	double	driftSpeed = 0;
	double	friction = bitData & kIceMask ? 0 : gConst->kNormalFriction;

	if ((returnValue = CObject::Collision(sender, left, top, right, bottom, forcex, forcey, pfx, pfy, sourceWeight, collisionObject)) & kCollisionOnBottom) {
		if (bitData & kRightDriftMask) driftSpeed = gConst->kDriftSpeed;
		if (bitData & kLeftDriftMask) driftSpeed = -gConst->kDriftSpeed;
		sender->CollisionEvent(friction, driftSpeed);
		
	}
	return returnValue;
}

// -----------------------------------------
short	CElement::Action()
// Is called from CPlayer::Think, when the player performs an action on that element
// Looks if there is some action possible, and do it
{
	if (elementThings) {	// is there any action possible?
 		if(elementThings->referencedPlatform) 	{
	  		elementThings->referencedPlatform->Switch();
		}
		
		if (bitData & kLightSwitchMask) {
			SWAP(elementThings->lightSwitch, elementThings->lightSwitch2, CShape *)
			gLevel->SwitchLight(refNum);
		}

		if (bitData & kSaveMask) gApplication->command = gGUI->RunUserInterface(kSaveGamePage);
	
		if (bitData & kInfotextMask) gGUI->DisplayInfotext(data);
	}

	return bitData;
}

void	CElement::PassiveAction()
{
	if (elementThings) {
		if ((bitData & kPassiveLightMask) && !elementThings->passiveHasChanged) {
			gLevel->SwitchLight(refNum);
			elementThings->passiveHasChanged = 1;
		}
		if ((bitData & kPassivePlatformMask) && !elementThings->passiveHasChanged && elementThings->referencedPlatform) {
			elementThings->referencedPlatform->Switch();		
			elementThings->passiveHasChanged = 1;
		}
	}
}

// --------------------------------
void	CElement::SwapLights()
// Swaps the light textures for that element
{
	SWAP(shape, shape2, CShape *);
	SWAP(brightness, brightness2, short);
}

// --------------------------------
void	CElement::Teleport(double &xp, double &yp)
// Moves the coordinates to this element
{
	if (background) {
		xp = xs;
		yp = ys;
	}
}


// ---------------------------------------------
short	CElement::GetElementLiquid()
// Returns the liquid on this element, expressed by the shape drawing modus
{
	if (bitData & kLavaMask) return kShapemodusLava;
	if (bitData & kWaterMask) return kShapemodusWater;
	if (bitData & kFogMask) return kShapemodusFog;
	return kShapemodusNormal;
}

CBackgroundElement::CBackgroundElement(short initx, short inity, short width, short height, tLevelElement *levelElement) : CElement(initx, inity, width, height, levelElement)
{
	typeID |= kBackgroundElement;

	if (levelElement) OnAllocate();
	brightness = 0;
}

void	CBackgroundElement::OnAllocate()
{
	short	params[5] = {-1, xs, ys, xe - xs, ye - ys};
	unsigned char *tmpBmp;

	tmpBmp = gShapeManager->GetBackground(params[1], params[2]);
	shape = new CBackground(tmpBmp, params, 0);
}


CBackgroundElement::~CBackgroundElement()
{
	delete shape;
}

void	CBackgroundElement::PaintElement(short planeX, short planeY, tRect *clipRect)
{
	long	timeStamp;

	if (shape) shape->RenderShape(xs - planeX, ys - planeY, clipRect, planeX * gConst->kBkgndScrollFactor, 0, gApplication->plane);

	if (elementThings) PaintElementThings(planeX, planeY, clipRect);
}



short	CElement::Write(FILE *f)
{
	long	size = 0;

	WRITEDATA(size);
	WRITEDATA(typeID);

	size += CObject::Write(f);

	WRITEDATA(bitData);
	WRITEDATA(lastState);
	WRITEDATA(iconID);
	WRITEDATA(brightness);
	WRITEDATA(brightness2);
	WRITEDATA(key);
	WRITEDATA(refNum);
	WRITEDATA(data);

	WRITEDATA(elementThings);
	if (elementThings) {
		WRITEDATA(elementThings->teleportBlinkTime);
		WRITEDATA(elementThings->savePortBlinkTime);
		WRITEDATA(elementThings->exitPortBlinkTime);
		WRITEDATA(elementThings->passiveHasChanged);
	}


	
	FINISHWRITE;

	return size;
}

void	CElement::Read(FILE *f)
{
	long	size = 0;
	long	elemThings;

	READDATA(size);
	READDATA(typeID);

	CObject::Read(f);

	READDATA(bitData);
	READDATA(lastState);
	READDATA(iconID);
	READDATA(brightness);
	READDATA(brightness2);
	READDATA(key);
	READDATA(refNum);
	READDATA(data);

	OnAllocate();

	READDATA(elemThings);
	if (elementThings) {
		READDATA(elementThings->teleportBlinkTime);
		READDATA(elementThings->savePortBlinkTime);
		READDATA(elementThings->exitPortBlinkTime);
		READDATA(elementThings->passiveHasChanged);
	}


}

void	CBackgroundElement::Read(FILE *f)
{
	CElement::Read(f);

	OnAllocate();
}