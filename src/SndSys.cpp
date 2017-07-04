#include "SndSys.hpp"
#include "Level.hpp"
#include "ConstVal.hpp"

extern	CSoundSystem *gSoundSystem;
extern	CLevel		*gLevel;
extern	tConstValues	*gConst;
extern	tConfigData	*gConfigData;

const	short	kMaxUserSound = 10;

void Mix_Audio(void *udata, Uint8 *stream, int len)
{
	gSoundSystem->ProcessList(udata, stream, len);
}

int InitializeSoundSystem()
{	
		SDL_AudioSpec wanted, obtained;
        
        /* Set the audio format */
        // Not all sounds are of the same sampling rate which is why
        // some of them sound a little crappy. Some conversion should be performed...        
        wanted.freq = 11025;
#ifdef _3DS
        wanted.format = (AUDIO_S16);
        wanted.channels = 1;
        wanted.samples = 1024; 
#else
        wanted.format = (AUDIO_U8);
        wanted.channels = 1;
        wanted.samples = 512;  /* Good low-latency value for callback */
#endif
         wanted.callback = Mix_Audio;
        wanted.userdata = NULL;

        /* Open the audio device, forcing the desired format */
        if ( SDL_OpenAudio(&wanted, &obtained) < 0 ) {
                fprintf(stderr, "InitSoundSystem: Couldn't open audio: %s\n", SDL_GetError());
                fprintf(stderr, "Sound was disabled. \n");
                return(-1);
        }
        return(0);
}

CSoundSystem::CSoundSystem(int status)
{
	state=status;	

	for (short n = 0; n < kNumWeapons; n ++) {
		weaponLaunchSounds[n] = new CSound(kWeaponLaunchSounds[n]);
		weaponHitSounds[n] = new CSound(kWeaponHitSounds[n]);
	}

	platformGo = new CSound(kPlatformGoSound);
	platformStop = new CSound(kPlatformStopSound);
	lightOn = new CSound(kLightOnSound);
	lightOff = new CSound(kLightOffSound);

	firestoneDamage = new CSound(kFirestoneDamage);

	selMenu = new CSound(kSelMenuSound);
	entrMenu = new CSound(kEntrMenuSound);
	openMenu = new CSound(kOpenMenuSound);

	playerJump = new CSound(kPlayerJumpSound);
	playerDive = new CSound(kPlayerDiveSound);
	playerPickup = new CSound(kPlayerPickupSound);

	minDistance = gConst->kSoundMinDistance * gConst->kSoundMinDistance;
	maxDistance = gConst->kSoundMaxDistance * gConst->kSoundMaxDistance;

	playList = new CSoundList();
	
	currentMaxSound = (SDL_MIX_MAXVOLUME) * gConfigData->soundVolume / kMaxUserSound; 
	
	if (state) {
		SDL_PauseAudio(0);  // turn the music on...
	}		
	
}

CSoundSystem::~CSoundSystem()
{
	if (state) {
		SDL_PauseAudio(1);
		SDL_CloseAudio();
	}
	delete playList;
	for (short n = 0; n < kNumWeapons; n ++) {
		delete weaponLaunchSounds[n];
		delete weaponHitSounds[n];
	}

	delete platformGo;
	delete platformStop;
	delete lightOn;
	delete lightOff;

	delete playerJump;
	delete playerDive;
	delete playerPickup;
}

void	CSoundSystem::Play(CSound *which, double x, double y)
{
	if (!state) return;
	long	distance = (long)((x - gLevel->focus->xm) * (x - gLevel->focus->xm) + 	(y - gLevel->focus->ym) * (y - gLevel->focus->ym));
	double	factor;
	
	SoundState *newstate = new SoundState;
	
	if (distance < minDistance)	{
		which->SetVol(currentMaxSound); 
	} else if (distance < maxDistance) {
		factor = double(distance - minDistance) / double(maxDistance - minDistance);
		which->SetVol(currentMaxSound - currentMaxSound * factor);
	} else return;

	which->Play(newstate);
	playList->Insert(newstate);
	
}

void	CSoundSystem::ProcessList(void *udata, Uint8 *stream, int len)
{
		SoundState *theSound;
		int mixlen;

		playList->Reset();
		// traverse the list of sounds being played
        while (theSound = playList->GetNext()) {
			mixlen = MIN(len, (int) theSound->bytesleft);
        	SDL_MixAudio(stream, theSound->soundpos, mixlen, theSound->volume);
        	theSound->soundpos += mixlen;
        	theSound->bytesleft -= mixlen;
        	if (!theSound->bytesleft) { // sound finished playing
        		playList->RemoveCurrent();
        	}
        }
}