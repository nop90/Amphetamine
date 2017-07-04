#ifndef __AMP_CONSTVAL__
#define __AMP_CONSTVAL__

#include "AmpHead.hpp"

const	short	kFilenameLength = 12;
const	short	kKeyDescriptionLength = 20;

struct tConstValues {
// The accelerations and velocities are measured
	// in pixel / (second * 1000)
	double	kVelocityUnit;
	double	kPlayerAcceleration;
	double	kPlayerLiquidAccel;
	double	kJumpVelocity;
	double	kRunScaleFactor; 
	short	kJumpAccelerationTime;
	short	kTeleportTime;			// Time for a teleport process
	short	kPlayerWidth;			// Width of player used for collisions
	short	kBulletWidth;			// Width of bullets used for collisions
	short	kFirehandNumOfBullets;	// Num of bullets shots when used firehands
	double	kFirehandAngle;			// angle between two shoot directions
	short	kStaffLoadTime;			// Time to load the staff weapon
	short	kInitialOxygen;			// Inital amound of oxygen
	double	kOxygenDecrease;		// Decrease of oxygen in one second
	short	kLavaDamage;			// Damage caused by lava per tick
	short	kPlayersFirstWeaponShape; // Players first weapon shape
	long	kPickupTime;			// Time the pickup shape is drawn

	short	kBulletTailLength;		// How many shapes are to be drawn at the tail of a bullet
	short	kBulletTailDistance;    // Distance of two shapes
	double	kDetonationFrameTime;	// Every 1/2 second a new detonation frame
	short	kWalkFrameTime;			// in ticks
	short	kShootFrameTime;			// Time for one shoot animation frame
	short	kWeaponChangeTime;		// Time between changing a weapon (Pressing x or y)
	short	kActionDelayTime;		// Time between two actions the player performs
	short	kDieFrameTime;			// Time between two die frames

	short	kWeaponCarryHeight;		// Height of carrying a weapon

	double	kDriftSpeed;
// Blinking times for player ports
	short	kTeleportBlinkTime;
	short	kSavePortBlinkTime;
	short	kExitPortBlinkTime;

// general physics constants
	double	kGravitation;
	double	kNormalFriction;
	double	kLiquidFriction;
	double	kMaxFallingSpeed;		// max. velocity when falling

// Monster movement constants
	double	kJumperJumpAcceleration;
		// How often the sorcery bullet bounces off
	short	kNumOfBounces;
	short	kSineWeaponRad;
	double	kMaxTurnAngle;	// max angle a guided bullet can turn
	short	kWargNearWeaponRadix;
	double	kWargFastSpeedup;
	double	kWargJumpAcceleration;
	
	long	kDelayAfterWargDeath; // Delay before level switch after killed warg

	// At which distance a bullet becomes a treat for a monster
	short	kTreatDistance;
	short	kBrightnessLevels[4];
	long	kFlickeringPeriod;

	short	kActivateDistance; // Distance from the player a monster starts to move

	double	kWeaponSF[4];
	double	kHealthSF[4];
	double	kSpeedSF[4];

	short	kLightningRadiant;	// Radius of lightning of a bullet
	double	kBkgndScrollFactor; // Background scrolling speed in percent of foreground scrolling speed
	double	kCoronaFadeSpeed; // How many percent of 1 the corona fades every tick

	short	kBlessureInvulnerabilityTime;	// How long is a monster invulnerable after a hurt
	short	kMonsterTouchBlessure;	// Blessure when touching a monster
	
	short	kItemExplosionRad;
	short	kItemExplosionStartShape;

	short	kHealthPanelColor;
	short	kOxygenPanelColor;
	short	kShadowmodeDarkening; // Percent of darkness of shaded bodies
	short	kFadeTime;	// Time for entirely fading the screen
	short	kStartupTime; // Time for displaying the startup screen

	short	kSoundMaxDistance; // At which distance a sound can be heard
	short	kSoundMinDistance; // At which distance a sound is maximal loud

	short	kTextYDistance; // Distance between two lines of text displayed
	short	kCameraSpeed;  // Speed of camera

	short	kShowFPS;	// Indicates whether the FPS is displayed

	char	kFilePalette[kFilenameLength];
	char	kFileLevel[kFilenameLength];
	char	kFileMonster[kFilenameLength];
	char	kFileWeapon[kFilenameLength];
	char	kFileInfo[kFilenameLength];
	char	kFilePlatform[kFilenameLength];
	char	kFileObjects[kFilenameLength];
	char	kFileShapes[kFilenameLength];
	char	kFileBackground1[kFilenameLength];
	char	kFileBackground2[kFilenameLength];
	char	kFileCluts[kFilenameLength];
	char	kFileGUIs[kFilenameLength];
	char	kFileConfig[kFilenameLength];
	char	kFileStartup[kFilenameLength];

	char	kMonsterNames[11][kFilenameLength];
	short	kMonsterNameSpeed; // Scrollspeed of monstername
	short	kMonsterNameX;  // x-pos on screen

	short	kShapeSets[kNumOfLevels];
	short	kBackgrounds[kNumOfLevels];
};


struct tConfigData {
	short	leftKey;
	char	leftKeyText[kKeyDescriptionLength];
	short	rightKey;
	char	rightKeyText[kKeyDescriptionLength];
	short	jumpKey;
	char	jumpKeyText[kKeyDescriptionLength];
	short	runKey;
	char	runKeyText[kKeyDescriptionLength];
	short	shootKey;
	char	shootKeyText[kKeyDescriptionLength];
	short	nextWeaponKey;
	char	nextWeaponKeyText[kKeyDescriptionLength];
	short	prevWeaponKey;
	char	prevWeaponKeyText[kKeyDescriptionLength];
	short	activateKey;
	char	activateKeyText[kKeyDescriptionLength];

	short	weapon1Key;
	short	weapon2Key;
	short	weapon3Key;
	short	weapon4Key;
	short	weapon5Key;
	short	weapon6Key;
	short	weapon7Key;
	short	weapon8Key;

	short	soundVolume;
	short	haveSound;		// by LL
	
	short	screenWidth;
	short	screenHeight;
	short 	tryFullScreen;  // by LL

	short	disableCoronas;
	short	disableLightning;
	short	disableShapeModes;
};

#endif