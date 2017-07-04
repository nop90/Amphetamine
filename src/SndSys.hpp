#ifndef _AMPH_SOUND_SYS_

#define _AMPH_SOUND_SYS_

#include "ObjInfo.hpp"
#include "SoundList.hpp"

const char kWeaponLaunchSounds[kNumWeapons][21] = {
	"sounds/swordl.wav",
	"sounds/phioll.wav",
	"sounds/sorceryl.wav",
	"sounds/bowl.wav",
	"sounds/sciel.wav",
	"sounds/handsl.wav",
	"sounds/bombl.wav",
	"sounds/staffl.wav",
	"sounds/borkl.wav",
	"sounds/aimedl.wav",
	"sounds/guidedl.wav",
	"sounds/trolll.wav",
	"sounds/otygl.wav",
	"sounds/nazgull.wav",
	"sounds/warg1l.wav",
	"sounds/warg2l.wav"
};

const char kWeaponHitSounds[kNumWeapons][21] = {
	"sounds/swordh.wav",
	"sounds/phiolh.wav",
	"sounds/sorceryh.wav",
	"sounds/bowh.wav",
	"sounds/scieh.wav",
	"sounds/handsh.wav",
	"sounds/bombh.wav",
	"sounds/staffh.wav",
	"sounds/borkh.wav",
	"sounds/aimedh.wav",
	"sounds/guidedh.wav",
	"sounds/trollh.wav",
	"sounds/otygh.wav",
	"sounds/nazgulh.wav",
	"sounds/warg1h.wav",
	"sounds/warg2h.wav"
};



const char kPlatformGoSound[21] = "sounds/platgo.wav";
const char kPlatformStopSound[21] = "sounds/platstop.wav";
const char kLightOnSound[21] = "sounds/lighton.wav";
const char kLightOffSound[21] = "sounds/lightoff.wav";

const char kSelMenuSound[21] = "sounds/selmenu.wav";
const char kEntrMenuSound[21] = "sounds/entrmenu.wav";
const char kOpenMenuSound[21] = "sounds/openmenu.wav";

const char kFirestoneDamage[21] = "sounds/stonedam.wav";

const char kPlayerJumpSound[21] = "sounds/jump.wav";
const char kPlayerDiveSound[21] = "sounds/dive.wav";
const char kPlayerPickupSound[21] = "sounds/pickup.wav";

class CSoundSystem {

private:
	long	maxDistance, minDistance;
	long	currentMaxSound;
	// List of sounds being played
	CSoundList *playList;
	
public:
	// Weapon sounds
	int		state;
	CSound	*weaponLaunchSounds[kNumWeapons];
	CSound	*weaponHitSounds[kNumWeapons];

	// Platform and light sounds
	CSound	*platformGo;
	CSound	*platformStop;
	CSound	*lightOn;
	CSound	*lightOff;

	// Item sounds
	CSound	*firestoneDamage;

	// GUI sounds
	CSound	*selMenu;
	CSound	*entrMenu;
	CSound	*openMenu;
	CSound	*playerJump;
	CSound	*playerDive;
	CSound	*playerPickup;
			
	CSoundSystem(int state);
	~CSoundSystem();

	void 	ProcessList(void*, Uint8*, int );
	void	Play(CSound *which, double x, double y);

};

#endif