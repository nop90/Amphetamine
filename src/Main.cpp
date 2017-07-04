#include "System.hpp"
#include "Appl.hpp"
#include "Clut.hpp"
#include "Gui.hpp"
#include "SndSys.hpp"
#include "string.h"

// Services
CApplication	*gApplication;
CSystem			*gSystem;
CLevel			*gLevel;
CObjInfo		*gObjInfo;
CShapeManager	*gShapeManager;
CClutManager	*gClutManager;
tConstValues	*gConst;
tConfigData		*gConfigData;
CGUI			*gGUI;
CSoundSystem	*gSoundSystem;

static char **my_argv;
static int my_argc;

int checkParam(const char *s)
{
	int i;

	for (i=1; i<my_argc; i++) {
		if (!strcasecmp(s, my_argv[i])) {
			// printf("Detected option %s as %d\n", s, i);
			return i;
		}
	}
	return 0;
}

void showUsage()
{
	
	printf("-ns, --nosound                  disable sound\n");
	printf("-fs, --fullscreen               try DGA fullscreen mode\n");
	printf("     --version                  display version information and exit\n");
	printf("     --help                     display this help and exit\n");
}

void showVersion()
{
	printf("Amphetamine %s \n", kVersionString);
	printf("Contact <loehrerl@bigfoot.com> for comments.\n");
}


int checkCommandLine()
{
	if (checkParam("--help")) {
		showUsage();
		return -1;
	}

	if (checkParam("--version")) {
		showVersion();
		return -1;
	}
	
	gConfigData = new tConfigData;
#ifdef _3DS
	gConfigData->haveSound= true;
	gConfigData->tryFullScreen= true;	
#else
	gConfigData->haveSound= !(checkParam("--nosound") || checkParam("-ns")) ;
	gConfigData->tryFullScreen= checkParam("--fullscreen") || checkParam("-fs");	
#endif
	return 0;
}

int main(int argc, char **argv)
{
	my_argc = argc;
	my_argv = argv;
	if (checkCommandLine()) return 0;
	
	gApplication = new CApplication();
	gApplication->InitGraphics();
	gApplication->LoadData();
	gApplication->Run();
	gApplication->UnloadData();
	gApplication->Quit();
	delete gApplication;

	return 0;
}