#ifndef __AMP_ELEMENT__
#define __AMP_ELEMENT__

#include "Object.hpp"
#include "File.hpp"
#include "Pltform.hpp"

// Added by Luke
#define LOBYTE(x) ((unsigned char)((x) & 0xff))
#define HIBYTE(x) ((unsigned char)((unsigned short)(x) >> 8))

enum {
	kPlatformSwitchInactiveID = 200,
	kPlatformSwitchActiveID = 201,
	kLightSwitchInactiveID = 208,
	kLightSwitchActiveID = 209,
	kInfotextStateOne = 210,
	kInfotextStateTwo = 211,
	kSavePortStateOne = 202,
	kSavePortStateTwo = 203,
	kExitStateOne = 204,
	kExitStateTwo = 205,
	kTeleportStateOne = 206,
	kTeleportStateTwo = 207,
	kDriftLeftID = 4015,
	kDriftRightID = 4016
};

struct tElementThings {
	CShape	*lightSwitch;
	CShape	*lightSwitch2;
	CShape	*platformSwitchActive;
	CShape	*platformSwitchInactive;
	CPlatform	*referencedPlatform;
	CShape	*drift;

	CShape	*teleporter;
	CShape	*teleporter2;
	long	teleportBlinkTime;
	CShape	*savePort;
	CShape	*savePort2;
	long	savePortBlinkTime;
	CShape	*exitPort;
	CShape	*exitPort2;
	long	exitPortBlinkTime;
	CShape	*infoPort;
	CShape	*infoPort2;
	long	infoPortBlinkTime;

	short	passiveHasChanged;
};

class CPlayer;

class	CElement : public CObject {
	friend class CPlayer;
protected:
	CShape	*shape;
	CShape	*shape2;
	short	bitData;
	short	lastState;
	short	iconID;

	tElementThings	*elementThings;

public:
	short	brightness;
	short	brightness2;
	short	key;
	short	refNum;
	short	data;

	CElement(short initx, short inity, short width, short height, tLevelElement *element);
	~CElement();

	void	OnAllocate();
	void	LinkPlatforms();

	virtual void	PaintElement(short planeLeft, short planeTop, tRect *clipRect);
	void			PaintElementThings(short planeLeft, short planeTop, tRect *clipRect);
	virtual short	Collision(CObject *sender, double left, double top, double right, double bottom, double &forcex, double &forcey, double pfx, double pfy, short sourceWeight, short &collisionObject);

	short	Action();
	void	PassiveAction();
	void	SwapLights();
	void	Teleport(double &xs, double &ys);
	short	GetElementLiquid();

	virtual short	Write(FILE *f);
	virtual void	Read(FILE *f);
};

class	CBackgroundElement : public CElement {
public:
	CBackgroundElement(short initx, short inity, short width, short height, tLevelElement *element);
	~CBackgroundElement();

	void	OnAllocate();

	void	PaintElement(short planeLeft, short planeTop, tRect *clipRect);

	void	Read(FILE *f);
};

#endif
