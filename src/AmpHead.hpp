#ifndef __AMP_HEADER__
#define __AMP_HEADER__

#include "limits.h"
#include "float.h"

const long	kVersionNumber = 0x0090;
const char  kVersionString[] = "0.8.10"; // by LL
const long	kVersionKey = 10835;
const char	kMyName1[6] = "JONAS";
const char	kMyName2[10] = "SPILLMANN";
const char	kParFileName[] = "amph.conf";  // by LL
const char	kLogFileName[] = "logfile";

enum {
	// Gameplane
	kGamePlaneWidth = 400,//480,
	kGamePlaneHeight = 240,//480,
	kUserPlaneWidth = 160,

	// Num and size of level elements
	kElementSize = 32,
	kLevelWidth = 100,
	kLevelHeight = 20,
	
	// Screen resolution
	kScreenWidth = 400,//1280,
	kScreenHeight = 240,//1024,
	kScreenDepth = 8,

	kNumOfLevels = 15,


	// Monster classes
	kClassPlayer = 0,
	kClassCreeper = 1,
	kClassJumper = 2,
	kClassFlyer = 3,
	kClassWalker = 4,
	kClassWarg = 5,

	// Item classes
	kClassBackgroundItem = 0,
	kClassUnpassableItem = 1,
	kClassMovableItem = 2,
	kClassPortableItem = 3,

	// Flags for flags field in item info
	kItemUnpassableMask = 32768,
	kItemPortableMask = 16384,
	kItemHurtMask = 8192,
	kItemExplodesMask = 4096,

	
	// Flags for data field
	kPassiveLightMask = 1,
	kPassivePlatformMask = 2,
	kRightDriftMask = 4,
	kLeftDriftMask = 8,
	kIceMask = 16,
	kFlickeringMask = 32,
	kInfotextMask = 64,
	kHurtMask = 128,
	kFogMask = 256,
	kWaterMask = 512,
	kLavaMask = 1024,
	kTeleportMask = 2048,
	kExitMask = 4096,
	kSaveMask = 8192,
	kLightSwitchMask = 16384,
	kPlatformSwitchMask = 32768,

	// Collision codes
	kNoCollision = 0,
	kCollisionOnTop = 1,
	kCollisionOnLeft = 2,
	kCollisionOnRight = 4,
	kCollisionOnBottom = 8,
	kCollisionWithPushing = 16,
	kCollisionWithLevelBorders = 32,

	// Return codes for ::Think and ::Move
	kNoEvent = 0,
	kDestroyMe = 1,

	kCameraNo = 12
};

// Type IDs
enum {
	kObject =			1,
	kElement =			1 << 1,
	kBackgroundElement =1 << 2,
	kThing =			1 << 3,
	kMonster =			1 << 4,
	kPlayer =			1 << 5,
	kItem =				1 << 6,
	kStaticItem =		1 << 7,
	kBackgroundItem =	1 << 8,
	kMovableItem =		1 << 9,
	kPortableItem =		1 << 10,
	kPlatform =			1 << 11,
	kBullet =			1 << 12,
	kSorceryBullet =	1 << 13,
	kBombBullet =		1 << 14,
	kSineBullet =		1 << 15,
	kGuidedBullet =		1 << 16,
	kCreeper =			1 << 17,
	kWalker =			1 << 18,
	kJumper =			1 << 19,
	kFlyer =			1 << 20,
	kWarg =				1 << 21,
	kCamera =			1 << 22
};

// Modi for drawing a shape
enum {
	kShapemodusNormal = 0,
	kShapemodusTransparent1,
	kShapemodusTransparent2,
	kShapemodusTransparent3,
	kShapemodusRandom,
	kShapemodusWater,
	kShapemodusLava,
	kShapemodusFog,
	kShapemodusShadow,
	kShapemodusBackwardFlag = 32768	// when set, the shape is drawn horizontally inverted
};

enum {
	kWeaponNormal = 0,
	kWeaponSorcery,
	kWeaponMultibullet,
	kWeaponBomb,
	kWeaponStaff,
	kWeaponInHand,
	kWeaponSine,
	kWeaponGuided,
	kWeaponHasWeight
};

// Weapon stati
enum {
	kWeaponDoesntExist,
	kWeaponOutOfMunition,
	kWeaponReady
};


enum {	// Portable Items
	kItemSword = 0,
	kItemPhiol,
	kItemSorcery,
	kItemBow,
	kItemScie,
	kItemHands,
	kItemBomb,
	kItemStaff,
	kItemPhiolmun,
	kItemSorcerymun,
	kItemBowmun,
	kItemSciemun,  // not used
	kItemHandsmun,
	kItemBombmun,
	kItemStaffmun,
	kItemOxygen,
	kItemHelppacket
};

struct tRect {
	short	left, top, right, bottom;
};

typedef struct {
     unsigned char red, green, blue;
} RGBcolor;


#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ABS(a) ((a) > 0 ? (a) : (-(a)))
#define SIGN(a) ((a) >= 0 ? 1 : -1)
#define SWAP(a, b, _t) (a) = (_t)((long)a ^ (long)(b)); (b) = (_t)((long)(a) ^ (long)(b)); (a) = (_t)((long)(a) ^ (long)(b));
#define NZ(a, b) ((a) == 0 ? (b) : (a))

#define MSG(message) if (logFile) fprintf(logFile, message); fflush(logFile)

#endif