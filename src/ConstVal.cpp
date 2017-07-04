#include "ConstVal.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <System.hpp>

extern	tConstValues	*gConst;
extern	tConfigData		*gConfigData;
extern	FILE			*logFile;
extern  CSystem			*gSystem;

long	GetLongConstant(FILE *f, char *constName);
double	GetDoubleConstant(FILE *f, char *constName);
void	GetStringConstant(FILE *f, char *constName, char *buffer);

// ------------------------------------------
void	LoadParameters()
// Loads the general constants from the *.par file
{
	char	levelSetString[11] = "xxLevelSet";
	char	levelBkgndString[13] = "xxLevelBkgnd";
	FILE	*paramFile = fopen(gSystem->QualifyDataDir(kParFileName), "r");  // by LL
	gConst = new tConstValues;
	
	gConst->kVelocityUnit = GetDoubleConstant(paramFile, "kVelocityUnit");
	gConst->kPlayerAcceleration = GetDoubleConstant(paramFile, "kPlayerAcceleration");
	gConst->kPlayerLiquidAccel = GetDoubleConstant(paramFile, "kPlayerLiquidAccel");
	gConst->kRunScaleFactor = GetDoubleConstant(paramFile, "kRunScaleFactor");
	gConst->kJumpVelocity = GetDoubleConstant(paramFile, "kJumpVelocity");
	gConst->kJumpAccelerationTime = GetLongConstant(paramFile, "kJumpAccelerationTime");
	gConst->kTeleportTime = GetLongConstant(paramFile, "kTeleportTime");
	gConst->kPlayerWidth = GetLongConstant(paramFile, "kPlayerWidth");
	gConst->kBulletWidth = GetLongConstant(paramFile, "kBulletWidth");
	gConst->kFirehandNumOfBullets = GetLongConstant(paramFile, "kFirehandNumOfBullets");
	gConst->kFirehandAngle = (double)GetLongConstant(paramFile, "kFirehandAngle") * 3.141 / 180;
	gConst->kStaffLoadTime = GetLongConstant(paramFile, "kStaffLoadTime");
	gConst->kInitialOxygen = GetLongConstant(paramFile, "kInitialOxygen");
	gConst->kOxygenDecrease = GetDoubleConstant(paramFile, "kOxygenDecrease");
	gConst->kLavaDamage = GetLongConstant(paramFile, "kLavaDamage");
	gConst->kPlayersFirstWeaponShape = GetLongConstant(paramFile, "kPlayersFirstWeaponShape");
	gConst->kPickupTime = GetLongConstant(paramFile, "kPickupTime");

	gConst->kBulletTailLength = GetLongConstant(paramFile, "kBulletTailLength");
	gConst->kBulletTailDistance = GetLongConstant(paramFile, "kBulletTailDistance");
	gConst->kDetonationFrameTime = GetDoubleConstant(paramFile, "kDetonationFrameTime");
	gConst->kWalkFrameTime = GetLongConstant(paramFile, "kWalkFrameTime");
	gConst->kShootFrameTime = GetLongConstant(paramFile, "kShootFrameTime");
	gConst->kWeaponChangeTime = GetLongConstant(paramFile, "kWeaponChangeTime");
	gConst->kActionDelayTime = GetLongConstant(paramFile, "kActionDelayTime");
	gConst->kDieFrameTime = GetLongConstant(paramFile, "kDieFrameTime");
	gConst->kDriftSpeed = GetLongConstant(paramFile, "kDriftSpeed");
	gConst->kWeaponCarryHeight = GetLongConstant(paramFile, "kWeaponCarryHeight");

	gConst->kTeleportBlinkTime = GetLongConstant(paramFile, "kTeleportBlinkTime");
	gConst->kSavePortBlinkTime = GetLongConstant(paramFile, "kSavePortBlinkTime");
	gConst->kExitPortBlinkTime = GetLongConstant(paramFile, "kExitPortBlinkTime");
	
	gConst->kGravitation = GetDoubleConstant(paramFile, "kGravitation");
	gConst->kNormalFriction = GetDoubleConstant(paramFile, "kNormalFriction");
	gConst->kLiquidFriction = GetDoubleConstant(paramFile, "kLiquidFriction");
	gConst->kMaxFallingSpeed = GetDoubleConstant(paramFile, "kMaxFallingSpeed");

	gConst->kJumperJumpAcceleration = GetDoubleConstant(paramFile, "kJumperJumpAcceleration");

	gConst->kNumOfBounces = GetLongConstant(paramFile, "kNumOfBounces");
	gConst->kSineWeaponRad = GetLongConstant(paramFile, "kSineWeaponRad");
	gConst->kMaxTurnAngle = (double)GetLongConstant(paramFile, "kMaxTurnAngle") * 3.141 / 180.0;
	gConst->kWargFastSpeedup = (double)GetLongConstant(paramFile, "kWargFastSpeedup");
	gConst->kWargNearWeaponRadix = GetLongConstant(paramFile, "kWargNearWeaponRadix");
	gConst->kWargJumpAcceleration = GetDoubleConstant(paramFile, "kWargJumpAcceleration");
	gConst->kDelayAfterWargDeath = GetLongConstant(paramFile, "kDelayAfterWargDeath");

	gConst->kActivateDistance = GetLongConstant(paramFile, "kActivateDistance");
	gConst->kTreatDistance = GetLongConstant(paramFile, "kTreatDistance");
	gConst->kBrightnessLevels[0] = GetLongConstant(paramFile, "kLight");
	gConst->kBrightnessLevels[1] = GetLongConstant(paramFile, "kMedium");
	gConst->kBrightnessLevels[2] = GetLongConstant(paramFile, "kDark");
	gConst->kBrightnessLevels[3] = GetLongConstant(paramFile, "kVeryDark");

	gConst->kWeaponSF[0] = GetDoubleConstant(paramFile, "kWeaponSF1");
	gConst->kWeaponSF[1] = GetDoubleConstant(paramFile, "kWeaponSF2");
	gConst->kWeaponSF[2] = GetDoubleConstant(paramFile, "kWeaponSF3");
	gConst->kWeaponSF[3] = GetDoubleConstant(paramFile, "kWeaponSF4");
	gConst->kHealthSF[0] = GetDoubleConstant(paramFile, "kHealthSF1");
	gConst->kHealthSF[1] = GetDoubleConstant(paramFile, "kHealthSF2");
	gConst->kHealthSF[2] = GetDoubleConstant(paramFile, "kHealthSF3");
	gConst->kHealthSF[3] = GetDoubleConstant(paramFile, "kHealthSF4");
	gConst->kSpeedSF[0] = GetDoubleConstant(paramFile, "kSpeedSF1");
	gConst->kSpeedSF[1] = GetDoubleConstant(paramFile, "kSpeedSF2");
	gConst->kSpeedSF[2] = GetDoubleConstant(paramFile, "kSpeedSF3");
	gConst->kSpeedSF[3] = GetDoubleConstant(paramFile, "kSpeedSF4");

	gConst->kFlickeringPeriod = GetLongConstant(paramFile, "kFlickeringPeriod");
	gConst->kLightningRadiant = GetLongConstant(paramFile, "kLightningRadiant");
	gConst->kBkgndScrollFactor = GetDoubleConstant(paramFile, "kBkgndScrollFactor");
	gConst->kCoronaFadeSpeed = GetDoubleConstant(paramFile, "kCoronaFadeSpeed");

	gConst->kBlessureInvulnerabilityTime = GetLongConstant(paramFile, "kBlessureInvulnerabilityTime");
	gConst->kMonsterTouchBlessure = GetLongConstant(paramFile, "kMonsterTouchBlessure");
	gConst->kItemExplosionRad = GetLongConstant(paramFile, "kItemExplosionRad");
	gConst->kItemExplosionStartShape = GetLongConstant(paramFile, "kItemExplosionStartShape");
	
	gConst->kHealthPanelColor = GetLongConstant(paramFile, "kHealthPanelColor");
	gConst->kOxygenPanelColor = GetLongConstant(paramFile, "kOxygenPanelColor");
	gConst->kShadowmodeDarkening = GetLongConstant(paramFile, "kShadowmodeDarkening");
	gConst->kFadeTime = GetLongConstant(paramFile, "kFadeTime");
	gConst->kStartupTime = GetLongConstant(paramFile, "kStartupTime");

	gConst->kTextYDistance = GetLongConstant(paramFile, "kTextYDistance");
	gConst->kCameraSpeed = GetLongConstant(paramFile, "kCameraSpeed");

	gConst->kSoundMaxDistance = GetLongConstant(paramFile, "kSoundMaxDistance");
	gConst->kSoundMinDistance = GetLongConstant(paramFile, "kSoundMinDistance");

	GetStringConstant(paramFile, "kFilePalette", gConst->kFilePalette);
	GetStringConstant(paramFile, "kFileLevel", gConst->kFileLevel);
	GetStringConstant(paramFile, "kFileMonster", gConst->kFileMonster);
	GetStringConstant(paramFile, "kFileWeapon", gConst->kFileWeapon);
	GetStringConstant(paramFile, "kFileInfo", gConst->kFileInfo);
	GetStringConstant(paramFile, "kFilePlatform", gConst->kFilePlatform);
	GetStringConstant(paramFile, "kFileObjects", gConst->kFileObjects);
	GetStringConstant(paramFile, "kFileShapes", gConst->kFileShapes);
	GetStringConstant(paramFile, "kFileBackground1", gConst->kFileBackground1);
	GetStringConstant(paramFile, "kFileBackground2", gConst->kFileBackground2);
	GetStringConstant(paramFile, "kFileCluts", gConst->kFileCluts);
	GetStringConstant(paramFile, "kFileGUIs", gConst->kFileGUIs);
	GetStringConstant(paramFile, "kFileConfig", gConst->kFileConfig);
	GetStringConstant(paramFile, "kFileStartup", gConst->kFileStartup);

	
	GetStringConstant(paramFile, "kMonsterName1", gConst->kMonsterNames[0]);
	GetStringConstant(paramFile, "kMonsterName2", gConst->kMonsterNames[1]);
	GetStringConstant(paramFile, "kMonsterName3", gConst->kMonsterNames[2]);
	GetStringConstant(paramFile, "kMonsterName4", gConst->kMonsterNames[3]);
	GetStringConstant(paramFile, "kMonsterName5", gConst->kMonsterNames[4]);
	GetStringConstant(paramFile, "kMonsterName6", gConst->kMonsterNames[5]);
	GetStringConstant(paramFile, "kMonsterName7", gConst->kMonsterNames[6]);
	GetStringConstant(paramFile, "kMonsterName8", gConst->kMonsterNames[7]);
	GetStringConstant(paramFile, "kMonsterName9", gConst->kMonsterNames[8]);
	GetStringConstant(paramFile, "kMonsterName10", gConst->kMonsterNames[9]);
	GetStringConstant(paramFile, "kMonsterName11", gConst->kMonsterNames[10]);

	gConst->kMonsterNameSpeed = GetLongConstant(paramFile, "kMonsterNameSpeed");
	gConst->kMonsterNameX = GetLongConstant(paramFile, "kMonsterNameX");


	gConst->kShowFPS = GetLongConstant(paramFile, "kShowFPS");

	for (short n = 1; n <= kNumOfLevels; n ++) {
		levelSetString[0] = (char)(n / 10) + '0';
		levelSetString[1] = (char)(n % 10) + '0';
		levelBkgndString[0] = (char)(n / 10) + '0';
		levelBkgndString[1] = (char)(n % 10) + '0';
		gConst->kShapeSets[n -1] = GetLongConstant(paramFile, levelSetString) -1;
		gConst->kBackgrounds[n -1] = GetLongConstant(paramFile, levelBkgndString);
	}

	fclose(paramFile);

	// Hier sollte auch im Home Directory gesucht werden...
	paramFile = gSystem->FindFile(gConst->kFileConfig);   // by LL
	
	gConfigData->leftKey = GetLongConstant(paramFile, "LeftKey");
	GetStringConstant(paramFile, "LeftKeyText", gConfigData->leftKeyText);
	gConfigData->rightKey = GetLongConstant(paramFile, "RightKey");
	GetStringConstant(paramFile, "RightKeyText", gConfigData->rightKeyText);
	gConfigData->jumpKey = GetLongConstant(paramFile, "JumpKey");
	GetStringConstant(paramFile, "JumpKeyText", gConfigData->jumpKeyText);
	gConfigData->runKey = GetLongConstant(paramFile, "RunKey");
	GetStringConstant(paramFile, "RunKeyText", gConfigData->runKeyText);
	gConfigData->shootKey = GetLongConstant(paramFile, "ShootKey");
	GetStringConstant(paramFile, "ShootKeyText", gConfigData->shootKeyText);
	gConfigData->nextWeaponKey = GetLongConstant(paramFile, "NextWeaponKey");
	GetStringConstant(paramFile, "NextWeaponKeyText", gConfigData->nextWeaponKeyText);
	gConfigData->prevWeaponKey = GetLongConstant(paramFile, "PrevWeaponKey");
	GetStringConstant(paramFile, "PrevWeaponKeyText", gConfigData->prevWeaponKeyText);
	gConfigData->activateKey = GetLongConstant(paramFile, "ActivateKey");
	GetStringConstant(paramFile, "ActivateKeyText", gConfigData->activateKeyText);

	gConfigData->weapon1Key = GetLongConstant(paramFile, "Weapon1Key");
	gConfigData->weapon2Key = GetLongConstant(paramFile, "Weapon2Key");
	gConfigData->weapon3Key = GetLongConstant(paramFile, "Weapon3Key");
	gConfigData->weapon4Key = GetLongConstant(paramFile, "Weapon4Key");
	gConfigData->weapon5Key = GetLongConstant(paramFile, "Weapon5Key");
	gConfigData->weapon6Key = GetLongConstant(paramFile, "Weapon6Key");
	gConfigData->weapon7Key = GetLongConstant(paramFile, "Weapon7Key");
	gConfigData->weapon8Key = GetLongConstant(paramFile, "Weapon8Key");

	gConfigData->soundVolume = GetLongConstant(paramFile, "SoundVolume");

	gConfigData->screenWidth = GetLongConstant(paramFile, "ScreenWidth");
	gConfigData->screenHeight = GetLongConstant(paramFile, "ScreenHeight");

	gConfigData->disableCoronas = GetLongConstant(paramFile, "DisableCoronas");
	gConfigData->disableLightning = GetLongConstant(paramFile, "DisableLightning");
	gConfigData->disableShapeModes = GetLongConstant(paramFile, "DisableShapeModes");

	fclose(paramFile);
}






#define CATCHSPACES while (c == (unsigned char)' ') {c = fgetc(f); }

short	GetValue(FILE	*f, char *constName, char *value)
{
	char	key[30];
	char	c = 0;
	short	n;

	fseek(f, 0, SEEK_SET);

	while (strcmp(key, constName)) {
		c = fgetc(f);
		n = 1;
		if (c == (unsigned char)'#') {	// comment
			while (fgetc(f) != (unsigned char)'\n') {}
		}else if (c == (unsigned char)'@') {
			value[0] = '\0';
			return 0;
		}else{
			key[0] = c;
			while (c != (unsigned char)' ' && c != (unsigned char)'=') {
				c = fgetc(f);
				key[n] = c; n ++;
			}
			key[n - 1] = '\0';
			CATCHSPACES
			if (c != (unsigned char)'=') return 0;
			c = fgetc(f); n = 1;
			CATCHSPACES
			value[0] = c;
			while (c != (unsigned char)' ' && c != (unsigned char)'\n') {
				c = fgetc(f); 
				value[n] = c; n ++;
			}
			value[n - 1] = '\0';
			while (c != (unsigned char)'\n') {c = fgetc(f); }
		}
	}
	if (key[0] == '@') return 0; 
	else return 1;
}

long	GetLongConstant(FILE *f, char *constName)
{
	char	val[30];

	if (GetValue(f, constName, val)) 
		return atoi(val); 
	else{
		return 0;
	}
}

double	GetDoubleConstant(FILE *f, char *constName)
{
	char	val[30];

	if (GetValue(f, constName, val))
		return atof(val);
	else{
		return 0;
	}
}

void	GetStringConstant(FILE *f, char *constName, char *buffer)
{
	GetValue(f, constName, buffer);
}