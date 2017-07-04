// SoundList.hpp: Schnittstelle für die Klasse CSoundList.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_AMPH_SOUND_LIST_)
#define _AMPH_SOUND_LIST_

extern "C" {
#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>
#include <SDL/SDL_types.h>
}

class CSound;

struct SoundState {
	Uint8 *soundpos;
	Uint32 bytesleft;
	int volume;
	CSound *sound;
	SoundState *next;
};

class CSound
{
private:
	short	state; // 0 if sound doesn't exist, 1 if ok
public:
	Uint8 *data;
	Uint32 soundlen, playing;
	int volume;
	
	SDL_AudioSpec specs;
	CSound(const char* wav);
	~CSound();
	void Play(SoundState *newstate);
	void Reset();
	bool isPlaying();
	void SetVol(long vol);
};


class CSoundList  
{
public:
	CSoundList();
	~CSoundList();
	
	void Reset();
	void Insert(SoundState *s);
	SoundState *GetNext();
	void RemoveCurrent();
	
private:
	SoundState *first, *prev, *current;
};

#endif 