// SoundList.cpp: Implementierung der Klasse CSoundList.
//
//////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <stdio.h>
#include "SoundList.hpp"
#include "System.hpp"

extern	CSystem	*gSystem;

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

extern FILE *logFile;

CSound::CSound(const char* wav)
{
	if (wav) {
		// MSG("Loading ");
		// MSG(wav);
		if (!SDL_LoadWAV(gSystem->QualifyDataDir(wav),&specs, &data, &soundlen)) { 
			// MSG(" failed");
		} else {
			// MSG(" succeeded\n");
		}
		state = 1;
	}
	Reset();
}

void CSound::Reset()
{
	playing = 0;
}

CSound::~CSound()
{
	delete [] data;
}

void CSound::Play(SoundState *newstate)
{
	newstate->sound=this;
	newstate->bytesleft=soundlen;
	newstate->volume=volume;
	newstate->soundpos=data;
	newstate->next=NULL;		
}

bool CSound::isPlaying()
{
	if (state) {
		return (bool)playing;
	}else return 0;
}

void CSound::SetVol(long vol)
{
	volume=vol;
}


// --------------------------------------------------------

void CSoundList::Reset() {
	current = first;
	prev = first;
}

CSoundList::CSoundList()
{
	first = new SoundState;
	first->sound=NULL;
	first->next=NULL;
	Reset();
}

CSoundList::~CSoundList()
{
	delete first;
}

SoundState *CSoundList::GetNext()
{	
	if (current) {
		prev = current;
		current = current->next;
	}
	return current;
}

void CSoundList::Insert(SoundState *s)
{
	s->sound->playing ++;
	s->next=first->next;
	first->next=s;   
}

void CSoundList::RemoveCurrent()
{	
	SoundState *tmp;
	
	tmp =current;
	
	if (current->sound) {
		current->sound->playing --;
	}	
	prev->next = current->next;
	current = current->next;
	
	delete tmp;
}
