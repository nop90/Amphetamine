/*************************************
	System.cpp

	(c) 1999 Jonas Spillmann

	Contains system-depending routines such as creating windows and video buffers
**************************************/

#include "System.hpp"
#include "Graphfil.hpp"
#include <sys/time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "Appl.hpp"
#include "Level.hpp"
#include "Player.hpp"

#ifdef _3DS
#include "3ds.h"
#endif

#ifdef _USE_LIB_XPM
extern "C" {
#include <X11/xpm.h>
}
#endif

extern	FILE	*logFile;
extern  CSystem *gSystem;
extern  CApplication	*gApplication;
extern  CLevel			*gLevel;
extern  tConfigData		*gConfigData;

int		InitializeSoundSystem();

/* Loads the icon to a surface */

inline static unsigned char Hex2Dec(char digit)
{
	if (digit > 64) 
		return (digit - 55); 
	else 
		return (digit - 48);
}

static void ExtractColors(char *gstring, Uint8 *red, Uint8 *green, Uint8 *blue) {

	*red = Hex2Dec(gstring[1])*16+Hex2Dec(gstring[2]);
	*green = Hex2Dec(gstring[3])*16+Hex2Dec(gstring[4]);
	*blue = Hex2Dec(gstring[5])*16+Hex2Dec(gstring[6]);
}

#ifdef _USE_LIB_XPM

SDL_Surface *CSystem::XPM2Surface(char *filename) 
{
	int x, ret;
	unsigned int i, y;
	unsigned char *buffer;
	unsigned int *data;
	short pitch;
	SDL_Surface *icon;
	XpmImage xpmimage;
	SDL_Color *palette;
	
	ret = XpmReadFileToXpmImage(filename, &xpmimage, NULL);
	
	if (ret != XpmSuccess || xpmimage.cpp > 1) {   // only 8bit pixmaps will work
		return NULL;
	}
	
	icon = AllocateBuffer(xpmimage.width,xpmimage.height);
	palette = new SDL_Color[xpmimage.ncolors];

	// Get color palette
	for (i=0; i < xpmimage.ncolors; ++i) {
		ExtractColors(xpmimage.colorTable[i].c_color, &(palette[i].r),&(palette[i].g),&(palette[i].b));
	}
	
	buffer = GetBufferPtr(icon, &pitch);
	data = xpmimage.data;

	// Copy data
	for (y = 0; y < xpmimage.width; y++ ) {
		for (x = 0; x < xpmimage.height; x++) {
			*buffer = (unsigned char)*data;
			buffer++; data++;
		}
	}
	
	ReleaseBufferPtr(icon);
	SDL_SetColors(icon, palette, 0, xpmimage.ncolors);
	
	return icon;
}

#endif

/* This function may run in a separate event thread */
static int FilterEvents(const SDL_Event *event) {
        
        /* This quit event signals the closing of the window */
        if ( (event->type == SDL_QUIT) ) {
                exit(0);
                return(0);
        }
        if ( event->type == SDL_KEYDOWN ) {
               if (event->key.keysym.mod == KMOD_RALT || event->key.keysym.mod == KMOD_LALT){
               		switch (event->key.keysym.sym) {
               		case (SDLK_s): 
               		    gSystem->ScreenShot(); 
               		    break;
               		case (SDLK_n):
               			gApplication->command = kCmdNextLevel;
               			break;
               		case (SDLK_p):
               			gApplication->command = kCmdPrevLevel;
               			break;
               		case (SDLK_w):
               			((CPlayer *)gLevel->player)->GetItAll();
               			break;
               		default:
               			break;
               		}	
               		return(0); 
               }
        }
        return(1);
}

char *CSystem::QualifyHomeDir(const char *fname)
{
	char *tmp = new char[strlen(homeDir)+strlen(fname)+2];
	sprintf(tmp, "%s/%s", homeDir, fname);
	
	return tmp;
}

char *CSystem::QualifyDataDir(const char *fname)
{
	char *tmp = new char[strlen(dataDir)+strlen(fname)+2];
	sprintf(tmp, "%s/%s", dataDir, fname);
	
	return tmp;
}

void CSystem::GetHomeDir()
{

	int rcode;
#ifdef _3DS
	homeDir = new char[strlen("/3ds/amph")+2];
	sprintf(homeDir, "/3ds/amph");
	mkdir("/3ds", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#else	
	char *tmp;
	if (tmp = getenv("HOME")) {
		homeDir = new char[strlen(tmp)+strlen(kHomeName)+2];
		sprintf(homeDir, "%s/%s", tmp, kHomeName);
	} else {
		homeDir = strdup(".");
	}
#endif
	rcode = mkdir(homeDir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
}

//---------------------------------------------------------
CSystem::CSystem(char *theName)
/*	In: theName: Name of the application

	Register the application class and allocates space for the palette

	WP: -
*/
{
#ifdef _3DS
	romfsInit();
#endif
	GetHomeDir();
	dataDir = new char[strlen(INSTALL_DIR)+1];
	strcpy(dataDir, INSTALL_DIR);
	
	char	*logFileName = QualifyHomeDir(kLogFileName);

	logFile = fopen(logFileName, "w");  // by LL

	strcpy(name, theName);

	/* Initialize SDL */
    if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0 ) {
       MSG("SDL_Init_Failed.");
       MSG(SDL_GetError());
 
       exit(1);
    }
    atexit(SDL_Quit);

	// Ignore most events
	SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
	SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
	SDL_EventState(SDL_ACTIVEEVENT, SDL_IGNORE);
	/* Filter quit and mouse motion events */
    SDL_SetEventFilter(FilterEvents);

#ifdef _3DS
	SDL_N3DSKeyBind(KEY_CPAD_UP|KEY_CSTICK_UP, SDLK_UP);
	SDL_N3DSKeyBind(KEY_CPAD_DOWN|KEY_CSTICK_DOWN, SDLK_DOWN);
	SDL_N3DSKeyBind(KEY_CPAD_LEFT|KEY_CSTICK_LEFT, SDLK_LEFT);
	SDL_N3DSKeyBind(KEY_CPAD_RIGHT|KEY_CSTICK_RIGHT, SDLK_RIGHT);
#endif

	#ifdef _USE_LIB_XPM
	
	SDL_Surface *icon;
	icon = XPM2Surface(QualifyDataDir("amph.xpm"));
	if (icon) { 
		SDL_WM_SetIcon(icon, 0);
		DisposeBuffer(icon);
	}

	#endif
	
	textout = false;

	palColors = new RGBcolor[256];
	palette = new SDL_Color[256];
	startTicks = 0;
	
}


//------------------------------------
CSystem::~CSystem()
/*
	Releases SDL
*/
{

#ifdef _3DS
	romfsExit();
#endif
}

//-----------------------------------------------------------------------------
void CSystem::NewWindow(short left, short top, short width, short height)
/*	In: left, top, width, height: position and size of windows being created
		Usually left and top are set to 0 and width, height are equal to the screen size

	Creates a new window
	Initializes DirectX
	Calls sound initialization

	WP:	* Application class registered
*/
{
	MSG("InitializeSoundSystem\n");
	workingSound = gConfigData->haveSound && !(InitializeSoundSystem());
}


//----------------------------------------------------------
void CSystem::DisposeWindow()
/*	
	Destroys window
*/
{
	SDL_Quit();
}


//-----------------------------------------------------------------------
void CSystem::AllocateScreen(short rx, short ry, short depth)
/*	In: rx, ry: Width and height of the videobuffer being allocated
		depth: Pixeldepth of video buffer. Must be set to 8

	Out: -

	Locks the screen
	Sets the screen resolution to rx x ry
	Creates the primary video surface (which is actually the visible screen)

	WP:	* DirectDraw - object allocated (done in NewWindow)
		* Window created 
*/
{
	Uint32 sdl_vid_options;
	
	MSG("Allocating screens\n");
#ifdef _3DS
	sdl_vid_options = SDL_INIT_TIMER | SDL_DUALSCR;//SDL_FULLSCREEN | SDL_CONSOLEBOTTOM;
#else	
	sdl_vid_options = SDL_INIT_TIMER | SDL_HWSURFACE|SDL_HWPALETTE;
	if (gConfigData->tryFullScreen) {
		sdl_vid_options = sdl_vid_options | SDL_FULLSCREEN;
	} 
#endif	
//	screenPort=SDL_SetVideoMode(640, 480, 8, sdl_vid_options);
	screenPort=SDL_SetVideoMode(400, 480, 8, sdl_vid_options);
	if (!screenPort) {
		fprintf(stderr, "Opening a window failed: %s \n", SDL_GetError());
		exit(1);
	}
	PaintString(name,0,0,0);
	SDL_ShowCursor(0);
}


// -------------------------------------------------
tGraphicBuffer	*CSystem::AllocateBuffer(short rx, short ry)
/*	In: rx, ry: Size of buffer being allocated
	Out: Reference to new graphic buffer
	
	Allocates a new graphics buffer. The buffer is allocated in main memory
	because read access is faster on main memory than on video memory (perhaps not
	true for AGP video cards)

	WP:	* Directdraw object created
*/
{
	tGraphicBuffer	*port;

	MSG("Allocate a buffer\n");
	port = SDL_CreateRGBSurface(SDL_SWSURFACE,rx,ry,kScreenDepth,0,0,0,0);
	if (port == NULL) Error("Error while creating an SDL Surface", 0);
	
	return port; 
}

//-------------------------------------------------------
void	CSystem::DisposeScreen()
/* Disposes the screen buffer
*/
{
	if (screenPort) SDL_FreeSurface(screenPort);
}

//------------------------------------------------------
void	CSystem::DisposeBuffer(tGraphicBuffer *buffer)
/*	In: buffer: buffer being disposed
	
	Disposes the indicated graphic buffer

	WP: * buffer exists and is not locked
*/
{	
	if (buffer) SDL_FreeSurface(buffer);
}


// --------------------------------------------------------
void	CSystem::LoadPalette(char *name)
/*	In: name: Name of palette file. The palette file must be a *.gif image file
				with a saved palette. The image itself is ignored

	Reads the palette file and stores the color information in the allocated palette array
	Transforms the palette to a DirectDraw - palette and loads it
	Make this palette active for the screen buffer

	WP: * directDraw - object allocated
		* palette allocated (in CSystem::CSystem)
		* The file name must exist
*/
{
	Graphic_file	*gf = read_graphic_file(QualifyDataDir(name));
	if (!gf) Error("Cannot find or open the palette file (*.pal)", 0);
	
	for (short n = 0; n < 256; n ++) {
		palColors[n].red = gf->palette[n].red;
		palColors[n].green = gf->palette[n].green;
		palColors[n].blue = gf->palette[n].blue;
	}

	free_graphic_file(gf);
	
	// Alle Farben der Reihe nach aus dem File lesen
    for (short i=0; i<256; i++) {
      	palette[i].r   = palColors[i].red;
       	palette[i].g = palColors[i].green;
        palette[i].b  = palColors[i].blue;
	}

	// Perform funny color switch
	palette[0].r = 0;
	palette[0].g = 0;
	palette[0].b = 0;
	palette[255].r = 255;
	palette[255].g = 255;
	palette[255].b = 255;
	
	SDL_SetColors(screenPort, palette, 0, 256);
}

// --------------------------------------------------------
void	CSystem::SetBufferPalette(tGraphicBuffer *buffer)
/*	In: buffer: The buffer the palette should be made active
	
	Makes the palette active for the indicated buffer. For each buffer this routine
	must be called

	WP:	* buffer exists
		* The palette is loaded (done in LoadPalette)
*/
{
	SDL_SetColors(buffer, palette, 0, 256);
}

// -------------------------------------------------------------------------
unsigned char *CSystem::GetBufferPtr(tGraphicBuffer *buffer, short *width)
// In: buffer: The buffer being locked for pixel access
//		
// Out:	width: Actual width of pixel plane of this buffer
//
// Prepares the graphic buffer indicated for drawing and returns the actual width
// and the adress of the plane
// if buffer = 0L, then the standard screen buffer will be locked
// 
// WP:	* buffer exists
//		* width points to a valid container
{
	if ( SDL_MUSTLOCK(buffer) ) {
         if ( SDL_LockSurface(buffer) < 0 )
         return NULL;
    }
    
	*width = buffer->pitch;

	return (unsigned char *)buffer->pixels;
}

// -----------------------------------------------------------
void	CSystem::ReleaseBufferPtr(tGraphicBuffer *buffer)
// Unlocks the graphic buffer indicated. If buffer = 0L, then the standard 
// screen buffer will be unlocked
{
	if ( SDL_MUSTLOCK(buffer) ) {
    	SDL_UnlockSurface(buffer);
    }
}

// ----------------------------------------------------------------
void CSystem::FlipSurfaces(tGraphicBuffer *buffer, short width, short height, short posx, short posy)
/*	In: buffer: The buffer being flipped to screen
		width, height: Width and height of the plane being flipped. Usually equal to
			the size of the buffer
		posx, posy: Position of the lefttop corner of the flipped plane on the screen
*/
// Blittes the content of buffer to the screen, assuming the buffer to have width X height, 
// and blitting it on the screen to position posx / posy
// If buffer = 0L, then the standard screen buffer will be blitted using standard coordinates

//	WP:	* The screenPort is allocated (in AllocateScreen)
//		* buffer exists
{	
	int hRet;
	SDL_Rect rcRect, destRect;

	if (buffer) {
		rcRect.x = 0;
		rcRect.y = 0;
		rcRect.w = width;
		rcRect.h = height;

		destRect.x = posx;
		destRect.y = posy;
		destRect.w = width;
		destRect.h = height;
       
		hRet = SDL_BlitSurface( buffer, &rcRect,  screenPort,&destRect);
		SDL_UpdateRects(screenPort, 1, &destRect);
	}
}

void	CSystem::ProcessEvents() 
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) { 
			
	}   // Luke
}


//----------------------------------------------
boolVar	CSystem::KeyPressed(short key)
// In: key: Virtual key code of the tested key
// Out: True if the key is being pressed
//
// WP: -
{
	Uint8 *keyboard_state;

	ProcessEvents();
	keyboard_state = SDL_GetKeyState(0);
	return (boolVar)(keyboard_state[key] == SDL_PRESSED);
}

//--------------------------------------------------------
void	CSystem::Error(char *message, short errorNo)
/*	In: message: Error text
		errorNo: Error number

	Writes the message followed by the error number in the log file

	WP: * logFile exists
*/
{
	MSG(message); fprintf(logFile, " %d\n", errorNo); fflush(logFile);
}


long 	CSystem::GetTickCount()
{
	// Unused
	struct timeval now;
	long  ticks;

	gettimeofday(&now, NULL);
	ticks=now.tv_sec*1000+now.tv_usec/1000;
	return ticks;
}
	
//---------------------------------------------
long	CSystem::GetTicks()
/*	Out: Ticks
	
	WP: startTicks is valid (done in ResetTicks)
*/
{	
	return (SDL_GetTicks() + startTicks);
}

//---------------------------------------------------
void	CSystem::ResetTicks(long startTickOffset)
/*	In: startTickOffset: Reset value for ticks
	
*/
{
	startTicks = -SDL_GetTicks() + startTickOffset;
}

//-----------------------------------------------------------------------------------
void	CSystem::PaintString(char *text, short x, short y, unsigned long foreColor)
/*	In: text: Text being drawn
		x, y: Position on screen
		foreColor: color of text

	Draws the text on the screen. Usually you do not call this routine

	WP: * screenPort is allocated
*/
{
	SDL_WM_SetCaption(text,NULL);
}

void 	CSystem::ScreenShot()
{
	if (screenPort) {
		char *fileName = QualifyHomeDir("screenshot.bmp");
		SDL_SaveBMP(screenPort, fileName);
		delete [] fileName;
	}
}

FILE	*CSystem::FindFile(const char *fname) 
{
	FILE *tmp;
	
	if (tmp=fopen(gSystem->QualifyHomeDir(fname), "r")) 
		return tmp;   // check home directory
	else
		delete [] tmp;
		
	if (tmp=fopen(gSystem->QualifyDataDir(fname), "r")) 
		return tmp;   // check data directory
	else
		delete [] tmp;
		
	return NULL;  // failed
}