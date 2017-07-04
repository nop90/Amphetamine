#ifndef __AMP_SYSTEM__
#define __AMP_SYSTEM__


#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <stdio.h>
#include "AmpHead.hpp"

extern "C" {
#include <SDL/SDL.h>
}

#ifndef INSTALL_DIR
#ifdef _3DS
#define INSTALL_DIR  "romfs:"
#else
#define INSTALL_DIR  "/usr/local/games/amph"
#endif
#endif

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#undef __BIG_ENDIAN__
#else
#define __BIG_ENDIAN__ 1
#endif

typedef bool boolVar;

const char 	kHomeName[] = ".amph";

enum {
	kKeyLeft = SDLK_LEFT,
	kKeyRight = SDLK_RIGHT,
	kKeyUp = SDLK_UP,
	kKeyDown = SDLK_DOWN,
#ifdef _3DS
	kKeyD = SDLK_d, // dummy
	kKeyC = SDLK_b,
	kKeyY = SDLK_l,
	kKeyX = SDLK_r,
	kKeyScrLock = SDLK_SCROLLOCK, // dummy
	kKeyPrscreen = SDLK_NUMLOCK,  // dummy
	kKeyEscape = SDLK_ESCAPE,
	kKeyPlus = SDLK_KP_PLUS, // dummy
	kKeyMinus = SDLK_KP_MINUS, // dummy
	kKeyControl = SDLK_x,
	kKeyReturn = SDLK_a,//SDLK_RETURN,
	kKeyTab = SDLK_y,
	kKeySpace = SDLK_a
#else
	kKeyD = SDLK_d,
	kKeyC = SDLK_c,
	kKeyY = SDLK_y,
	kKeyX = SDLK_x,
	kKeyScrLock = SDLK_SCROLLOCK,
	kKeyPrscreen = SDLK_NUMLOCK,
	kKeyEscape = SDLK_ESCAPE,
	kKeyPlus = SDLK_KP_PLUS,
	kKeyMinus = SDLK_KP_MINUS,
	kKeyControl = SDLK_LCTRL,
	kKeyReturn = SDLK_RETURN,
	kKeyTab = SDLK_TAB,
	kKeySpace = SDLK_SPACE
#endif
};

const unsigned char	kASCIISpace = 32;
const unsigned char	kASCIIEqual = 61;

const	long	kTicksPerSecond = 1000;
// Windows can't change the color indexes for black and white
const unsigned char	kWhiteColor = 255;
const unsigned char	kBlackColor = 0;

typedef SDL_Surface	tGraphicBuffer;

//************************* class CSystem ********************

class	CSystem {
protected:
//"""""""""""""""" Misc stuff
	char	name[16];
	long	startTicks;
	bool	textout;  
	
//"""""""""""""""" Graphics stuff 
	tGraphicBuffer	*screenPort; 
	SDL_Color	*palette;	
	#ifdef _USE_LIB_XPM
	SDL_Surface *XPM2Surface(char *pixmap);
	#endif
  
public:
	RGBcolor	*palColors;
	char		*homeDir;
	char		*dataDir;
	int			workingSound;

//"""""""""""""""" System Stuff
	CSystem(char *);
	~CSystem();

	void	NewWindow(short, short, short, short);
	void	DisposeWindow();
	
//"""""""""""""""" SDL Stuff
	void	AllocateScreen(short rx, short ry, short depth);
	tGraphicBuffer	*AllocateBuffer(short rx, short ry);
	void	DisposeBuffer(tGraphicBuffer *);
	void	DisposeScreen();
	void	LoadPalette(char *name);
	void	SetBufferPalette(tGraphicBuffer *);
	unsigned char *GetBufferPtr(tGraphicBuffer *, short *width);
	void	ReleaseBufferPtr(tGraphicBuffer *);
	void	FlipSurfaces(tGraphicBuffer *, short width, short height, short posx, short posy);
	
//"""""""""""""""" User stuff
	void	Error(char *message, short errorNo);
	boolVar	KeyPressed(short key);
	long	GetTicks();
	long	GetTickCount();
	void	ResetTicks(long startTickOffset);
	void	PaintString(char *, short x, short y, unsigned long color);
	void	ProcessEvents();
	void	GetHomeDir();
	char	*QualifyDataDir(const char *fname);
	char	*QualifyHomeDir(const char *fname);
	void 	ScreenShot();
	FILE 	*FindFile(const char *fname);
};

#endif
