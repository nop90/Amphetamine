#ifndef __AMP_ITEM__
#define __AMP_ITEM__

#include "Thing.hpp"
#include "ObjInfo.hpp"
#include "Shape.hpp"

class CItem : public CThing {
protected:
	CShape		*shape;
	CShape		*currentShape;
	tItemInfo	*info;

	long	detonationStartTime;
	short	isCorona;
	double	coronaFader;

public:
	CItem(short initx, short inity, short width, short height, short number, tItemInfo *itemInfo);
	~CItem();

	short	Think();
	void	Render(short planeX, short planeY, tRect *clipRect);
	void	PostRender(short planeX, short planeY, tRect *clipRect);
	void	Move();

	void	OnDamage(short blessure);

	short	Write(FILE *f);
	void	Read(FILE *f);
};

class CStaticItem : public CItem {
protected:

public:
	CStaticItem(short initx, short inity, short width, short height, short number, tItemInfo *itemInfo);
	~CStaticItem();

	void	LinkInLists();
	void	UnlinkInLists();

	short	Collision(CObject *sender, double left, double top, double right, double bottom, double &forcex, double &forcey, double pfx, double pfy, short sourceWeight, short &collisionObject);
};

class CBackgroundItem : public CItem {
protected:

public:
	CBackgroundItem(short initx, short inity, short width, short height, short number, tItemInfo *itemInfo);
	~CBackgroundItem();

	void	LinkInLists();
	void	UnlinkInLists();
};

class CMovableItem : public CItem {
protected:

public:
	CMovableItem(short initx, short inity, short width, short height, short number, tItemInfo *itemInfo);
	~CMovableItem();

	void	LinkInLists();
	void	UnlinkInLists();
};

class CPortableItem : public CItem {
protected:
	short	pickedUp;
	short	type;

public:
	CPortableItem(short initx, short inity, short width, short height, short number, tItemInfo *itemInfo, short type);
	~CPortableItem();

	void	LinkInLists();
	void	UnlinkInLists();

	short	Think();
	short	PickMeUp(short &value);

	short	Write(FILE *f);
	void	Read(FILE *f);
};

#endif