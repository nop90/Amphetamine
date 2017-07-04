#ifndef __AMP_PLATTFORMS__
#define __AMP_PLATTFORMS__

#include "Thing.hpp"
#include "ObjInfo.hpp"

enum {
	kRunsStartToEnd = 0,
	kRunsEndToStart,
	kDelaysAtStart,
	kDelaysAtEnd,
	kStopsAtStart, // Platform stopped at start or while running start -> end
	kStopsAtEnd		// Platform stopped at end or while running end -> start
};

const short	kTimeFactor = 100;

class CPlatform : public CThing {
protected:
	tPlatformInfo	*info;
	short	isLocal;	// whether info is allocated by the platform itself (after loading) or by ObjInfo
	CShape			*shape;
	
	short	origHeight;

	short	action;
	double	startx, starty;
	double	endx, endy;
	double	maxDistance;
	long	delayTime;

public:
	CPlatform(short initx, short inity, short width, short height, short number, tPlatformInfo *platformInfo);
	~CPlatform();

	void	OnAllocate();

	void	LinkInLists();
	void	UnlinkInLists();
	short	Think();
	short	Forces();
	void	Render(short planeX, short planeY, tRect *clipRect);
	short	Collision(CObject *sender, double left, double top, double right, double bottom, double &forcex, double &forcey, double pfx, double pfy, short sourceWeight, short &collisionObject);

	void	Switch();
	CShape	*GetCurrentState(CShape *active, CShape *inactive);
	void	OnTouch(CObject *touch);

	short	Write(FILE *f);
	void	Read(FILE *f);
};

#endif
