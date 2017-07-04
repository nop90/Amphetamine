#include "Gui.hpp"
#include "AmpHead.hpp"
#include "ConstVal.hpp"
#include "ShapeLd.hpp"
#include "Appl.hpp"
#include "Clut.hpp"
#include "SndSys.hpp"

extern	CSystem	*gSystem;
extern	tConstValues	*gConst;
extern	CShapeManager	*gShapeManager;
extern	CLevel	*gLevel;
extern	CClutManager	*gClutManager;
extern	tConfigData	*gConfigData;
extern	CSoundSystem	*gSoundSystem;

tGUIConstants	*gGUIConst;

const short	kHealthCurveY[10] = {0, 48, -48, 15, -15, 25, -15, 0, 0, 0};
const short	kHealthCurveX = 5;
const double kOxygenCurveY = 25.0;
const double kOxygenCurveX = 2.0;

short	kMunitionStatusColors[3] = {220, 100, 220};

const	short	kKeyHitDelay = 300;

const	short	kTitleTabulator = -30;
const	short	kNameTabulator1 = 30;
const	short	kNameTabulator2 = 20;

double	XLineCut(double r0x, double r0y, double r1x, double r1y, double x);
void	SwapBlackWhite(Graphic_file *gf);

CGUI::CGUI(CGraphicSurface *curMainScreen)
{
	tRect	position;

	LoadMessages(); //

	guis = read_graphic_file(gSystem->QualifyDataDir(gConst->kFileGUIs));  // by LL
	if (!guis) gSystem->Error("Error while reading the file kFileGUIs", 0);
	SwapBlackWhite(guis);

	healthPanel = new CGraphicSurface(gGUIConst->kHealthPanelWidth, gGUIConst->kHealthPanelHeight);
	oxygenPanel = new CGraphicSurface(gGUIConst->kOxygenPanelWidth, gGUIConst->kOxygenPanelHeight);
	weaponPanel = new CGraphicSurface(gGUIConst->kWeaponPanelWidth, gGUIConst->kWeaponPanelHeight);
	backgroundPanel = new CGraphicSurface(320, 240);CGraphicSurface(kUserPlaneWidth, kGamePlaneHeight);
	if (!(healthPanel && oxygenPanel && weaponPanel && backgroundPanel)) gSystem->Error("Cannot allocate panels", 0);

	currentMainScreen = curMainScreen;
	currentMessage = 0L;
	msgDisplayTime = 0;

	position.left = kGUIsPositions[3][0];
	position.top = kGUIsPositions[3][1];
	position.right = kGUIsPositions[3][2];
	position.bottom = kGUIsPositions[3][3];
	backgroundPanel->InsertGraphic(0L, guis, &position);
	backgroundPanel->PaintGraphic(0, 0, 0, kShapemodusNormal);
//	backgroundPanel->FlipToScreen(kGamePlaneWidth, 0);
	backgroundPanel->FlipToScreen(40, 240);

	position.left = kGUIsPositions[0][0];
	position.top = kGUIsPositions[0][1];
	position.right = kGUIsPositions[0][2];
	position.bottom = kGUIsPositions[0][3];
	healthPanel->InsertGraphic(0L, guis, &position);

	position.left = kGUIsPositions[1][0];
	position.top = kGUIsPositions[1][1];
	position.right = kGUIsPositions[1][2];
	position.bottom = kGUIsPositions[1][3];
	oxygenPanel->InsertGraphic(0L, guis, &position);

	for (short n = 0; n <= 7; n ++) {
		position.left = kGUIsPositions[2][0];
		position.top = kGUIsPositions[2][1] + n * (kGUIsPositions[2][3]);
		position.right = kGUIsPositions[2][2];
		position.bottom = kGUIsPositions[2][3] + n * (kGUIsPositions[2][3]);
		weaponPanel->InsertGraphic(0L, guis, &position);
	}

	position.left = kGUIsPositions[4][0];
	position.top = kGUIsPositions[4][1];
	position.right = kGUIsPositions[4][2];
	position.bottom = kGUIsPositions[4][3];
	curMainScreen->InsertGraphic(0L, guis, &position);

	position.left = kGUIsPositions[5][0];
	position.top = kGUIsPositions[5][1];
	position.right = kGUIsPositions[5][2];
	position.bottom = kGUIsPositions[5][3];
	curMainScreen->InsertGraphic(0L, guis, &position);

	lastHealthPanelRefresh = 0;
	healthCurveStart = 0;
	lastOxygenPanelRefresh = 0;
	oxygenCurveStart = 0;
}

CGUI::~CGUI()
{
	free_graphic_file(guis);
	delete healthPanel;
	delete oxygenPanel;
	delete backgroundPanel;
	delete messages;
}

void	CGUI::SetFocus(CPlayer *focus)
{
	player = focus;
}


void	CGUI::Update()
{
	long	time = gSystem->GetTicks();
	if (player->typeID & kPlayer && lastHealthPanelRefresh < time) {
		DrawHealthPanel();
		lastHealthPanelRefresh = time + gGUIConst->kHealthPanelRefreshRate;
	}
	if (player->typeID & kPlayer && lastOxygenPanelRefresh < time) {
		DrawOxygenPanel();
		lastOxygenPanelRefresh = time + gGUIConst->kOxygenPanelRefreshRate;
	}
}

void	CGUI::UpdateWeapon()
{
	short	drawStart = gGUIConst->kWeaponPanelHeight - player->weapons[player->currentWeapon]->munition * gGUIConst->kWeaponPanelHeight / player->weapons[player->currentWeapon]->info->munition;
	short	drawEnd = gGUIConst->kWeaponPanelHeight;
	short	color;
	
	weaponPanel->PaintGraphic(player->currentWeapon, 0, 0, kShapemodusNormal);

	for (short n = drawStart; n < drawEnd; n ++) {
		color = gClutManager->FindClosestColor(kMunitionStatusColors[0] - kMunitionStatusColors[0] * n / 100,
			kMunitionStatusColors[1] - kMunitionStatusColors[1] * n / 100,
			kMunitionStatusColors[2] - kMunitionStatusColors[2] * n / 100);

		weaponPanel->DrawAntialiasedLine(gGUIConst->kWeaponPanelWidth - gGUIConst->kWeaponStatusWidth, n, gGUIConst->kWeaponPanelWidth -1, n, (unsigned char)color, kShapemodusNormal);
	}
	weaponPanel->FlipToScreen(gGUIConst->kWeaponPanelPosX, gGUIConst->kWeaponPanelPosY);
}

#define ADDHEALTH(curvePoint) curvePoint == 9 ? 0 : curvePoint +1
#define SUBHEALTH(curvePoint) curvePoint == 0 ? 9 : curvePoint -1

// ---------------------------------------------------
void	CGUI::DrawHealthPanel()
// Draws the health curve panel and blits it to the screen
{
	double	ptx, pty, cury1, cury2;
	short	curvePoint = 0;
	double	fak;

	healthCurveStart --;
	if (healthCurveStart < -gGUIConst->kHealthPanelWidth) healthCurveStart = 0;
	ptx = healthCurveStart;

	//healthPanel->PaintRect(0, 0, kHealthPanelWidth, kHealthPanelHeight, kBlackColor);
	healthPanel->PaintGraphic(0, 0, 0, kShapemodusNormal);

	while (ptx < gGUIConst->kHealthPanelWidth) {
		fak = player->health <= 0 ? SHRT_MAX : (double)player->info->energy / (double)player->health;
		cury1 = kHealthCurveY[curvePoint] / fak;
		cury2 = kHealthCurveY[ADDHEALTH(curvePoint)] / fak;

		if (ptx <= 0 && ptx + kHealthCurveX > 0) {
			pty = XLineCut(ptx, cury1, ptx + kHealthCurveX, cury2, 0);
			healthPanel->DrawAntialiasedLine(0, pty + gGUIConst->kHealthPanelHeight / 2, 
				ptx + kHealthCurveX, 
				cury2 + gGUIConst->kHealthPanelHeight / 2, 
				gConst->kHealthPanelColor, kShapemodusNormal);

		}else if (ptx < gGUIConst->kHealthPanelWidth && ptx + kHealthCurveX >= gGUIConst->kHealthPanelWidth) {
			pty = XLineCut(ptx, cury1, ptx + kHealthCurveX, cury2, gGUIConst->kHealthPanelWidth);
			healthPanel->DrawAntialiasedLine(ptx, cury1 + gGUIConst->kHealthPanelHeight / 2, 
				gGUIConst->kHealthPanelWidth, pty + gGUIConst->kHealthPanelHeight / 2, 
				gConst->kHealthPanelColor, kShapemodusNormal);

		}else if (ptx > 0 && ptx + kHealthCurveX < gGUIConst->kHealthPanelWidth) {
			healthPanel->DrawAntialiasedLine(ptx, cury1 + gGUIConst->kHealthPanelHeight / 2, 
				ptx + kHealthCurveX, cury2 + gGUIConst->kHealthPanelHeight / 2,
				gConst->kHealthPanelColor, kShapemodusNormal);
		}
		curvePoint = ADDHEALTH(curvePoint);
		ptx += kHealthCurveX;
	}
	healthPanel->FlipToScreen(gGUIConst->kHealthPanelPosX, gGUIConst->kHealthPanelPosY);
}


void	CGUI::DrawOxygenPanel()
{
	double		ptx, pty1, pty2;
	double		oxygenFactor = (double)player->oxygen / (double)gConst->kInitialOxygen;

	if (oxygenFactor > 1.0) oxygenFactor = 1.0;

	//oxygenPanel->PaintRect(0, 0, kOxygenPanelWidth, kOxygenPanelHeight, kBlackColor);
	oxygenPanel->PaintGraphic(0, 0, 0, kShapemodusNormal);

	oxygenCurveStart += 10.0 / 180.0 * 3.141;
	ptx = oxygenCurveStart;

	pty1 = kOxygenCurveY * sin(oxygenCurveStart);
	for (ptx = 0; ptx < 100; ptx += kOxygenCurveX) {

		pty2 = kOxygenCurveY * oxygenFactor * sin(ptx / 180 * 3.141 * 5.0 + oxygenCurveStart) + 
			kOxygenCurveY * oxygenFactor / 1.5 * cos(ptx / 180 * 3.141 * 15.0 + oxygenCurveStart * 3.0);
		oxygenPanel->DrawAntialiasedLine(ptx, pty1 + gGUIConst->kOxygenPanelHeight / 2, ptx + kOxygenCurveX, pty2 + gGUIConst->kOxygenPanelHeight / 2, gConst->kOxygenPanelColor, kShapemodusNormal);
		pty1 = pty2;
	}

	oxygenPanel->FlipToScreen(gGUIConst->kOxygenPanelPosX, gGUIConst->kOxygenPanelPosY);
}



/*	XLineCut
	
	Hinein: r0x, r0y: Anfangskoordinaten einer Linie
			r1x, r1y: Endkoordinaten einer Linie
			x: X-Abst einer zur Y-Achse parallelen Gerade
			
	Hinaus: Y-Abst des Schnittpunktes der Linie mit der Geraden
*/
double	XLineCut(double r0x, double r0y, double r1x, double r1y, double x)
{
	return (r0y + ((x - r0x) * (r1y - r0y)) / (r1x - r0x));
}



short	CGUI::ProcessKeyStrokes()
{
	//gSystem->ProcessEvents();
	
	if (lastKeyHit < gSystem->GetTicks() && gSystem->KeyPressed(kKeyUp) && currentNumOfItems) {
		lastKeyHit = gSystem->GetTicks() + gGUIConst->kKeyHitDelay;
		lastItem = currentItem;
		currentItem --;
		if (currentItem < 0) currentItem = currentNumOfItems;
		gSoundSystem->Play(gSoundSystem->selMenu, player->xm, player->ym); 

		return kKeyUp;
	}

	if (lastKeyHit < gSystem->GetTicks() && gSystem->KeyPressed(kKeyDown) && currentNumOfItems) {
		lastKeyHit = gSystem->GetTicks() + gGUIConst->kKeyHitDelay;
		lastItem = currentItem;
		currentItem ++;
		if (currentItem > currentNumOfItems) currentItem = 0;
		gSoundSystem->Play(gSoundSystem->selMenu, player->xm, player->ym); 

		return kKeyDown;
	}

	if (lastKeyHit < gSystem->GetTicks() && gSystem->KeyPressed(kKeyReturn) && currentNumOfItems) {
		lastKeyHit = gSystem->GetTicks() + gGUIConst->kKeyHitDelay;
		gSoundSystem->Play(gSoundSystem->openMenu, player->xm, player->ym); 
		return kKeyReturn;
	}
	if (lastKeyHit < gSystem->GetTicks() && gSystem->KeyPressed(kKeyEscape)) {
		lastKeyHit = gSystem->GetTicks() + gGUIConst->kKeyHitDelay;
		gSoundSystem->Play(gSoundSystem->openMenu, player->xm, player->ym); 
		return kKeyEscape;
	}
	return 0;
}


short	CGUI::NewGamePageEvents()
{
	switch (currentItem) {
		case 0:		// New Game
			return kCmdNewGameLevel1;
			break;

		case 1:
			return kCmdNewGameLevel2;
			break;

		case 2:
			return kCmdNewGameLevel3;
			break;

		case 3:
			return kCmdNewGameLevel4;
			break;

		default:
			return kNoCmdEvent;
			break;
	}
}

short	CGUI::QuitPageEvents()
{
	switch (currentItem) {
		case 0:
			return kCmdQuit;
			break;
		case 1:
			OnOpenMainPage();
			return kNoCmdEvent;
		default:
			return kNoCmdEvent;
	}
}

short	CGUI::MainPageEvents()
{
	switch (currentItem) {
		case 0:		// New Game
			OnOpenNewGamePage();
			return kNoCmdEvent;
			break;

		case 1:
			OnOpenLoadPage(kLoadGamePage);
			return kNoCmdEvent;
			break;

		case 2:
			OnOpenHelpPage();
			return kNoCmdEvent;
			break;
		case 3:
			OnOpenCreditsPage();
			return kNoCmdEvent;
			break;
		
		case 4:
			OnOpenQuitPage();
			return kNoCmdEvent;
			break;

		default:
			return kNoCmdEvent;
			break;
	}
}


short	CGUI::LoadGamePageEvents()
{
	return (page == kLoadGamePage ? kLoadGameSlot0 + currentItem : kSaveGameSlot0 + currentItem);
}


short	CGUI::RunUserInterface(short whichPage)
{
	short	command = kNoCmdEvent;

	if (lastKeyHit > gSystem->GetTicks()) return kCmdNoCommand;
	lastKeyHit = gSystem->GetTicks() + gGUIConst->kKeyHitDelay;
				
	gSoundSystem->Play(gSoundSystem->entrMenu, player->xm, player->ym); 

	switch (whichPage) {
		case kMainPage:
			OnOpenMainPage();
			break;
		case kNewGamePage:
			OnOpenNewGamePage();
			break;
		case kLoadGamePage:
		case kSaveGamePage:
			OnOpenLoadPage(whichPage);
			break;
		case kHelpPage:
			OnOpenHelpPage();
			break;
		case kCreditPage:
			OnOpenCreditsPage();
			break;
		case kQuitPage:
			OnOpenQuitPage();
			break;
	}

	while (command == kNoCmdEvent) {
		
		switch (ProcessKeyStrokes()) {
			case kKeyEscape:
				if (page != kMainPage) OnOpenMainPage(); else command = kCmdNoCommand;
				break;

			case kKeyReturn:
				switch (page) {
					case kMainPage:
						command = MainPageEvents();
						break;
					case kNewGamePage:
						command = NewGamePageEvents();
						break;
					case kLoadGamePage:
					case kSaveGamePage:
						command = LoadGamePageEvents();
						break;
					case kQuitPage:
						command = QuitPageEvents();
						break;
				}
				break;
		}
		

		SelectMenuItem();
		currentMainScreen->FlipToScreen(0, 0);
	}
	
	return command;
}					
void	CGUI::OnOpenMainPage()
{
	gLevel->PaintLevel();

	currentMainScreen->PaintRect(gGUIConst->kUserMenuPosX, gGUIConst->kUserMenuPosY, gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuWidth, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuHeight, kBlackColor, kShapemodusTransparent2);
	
	currentMainScreen->DrawString(gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuEntryPosX + kTitleTabulator, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuEntryPosY, gGUIConst->kUserMainPageTitle, kShapemodusLava);

	currentMainScreen->DrawString(gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuEntryPosX, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuEntryPosY + 1 * gGUIConst->kUserMenuEntryDY, gGUIConst->kUserMenuEntry1, kShapemodusNormal);
	currentMainScreen->DrawString(gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuEntryPosX, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuEntryPosY + 2 * gGUIConst->kUserMenuEntryDY, gGUIConst->kUserMenuEntry2, kShapemodusNormal);
	currentMainScreen->DrawString(gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuEntryPosX, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuEntryPosY + 3 * gGUIConst->kUserMenuEntryDY, gGUIConst->kUserMenuEntry3, kShapemodusNormal);
	currentMainScreen->DrawString(gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuEntryPosX, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuEntryPosY + 4 * gGUIConst->kUserMenuEntryDY, gGUIConst->kUserMenuEntry4, kShapemodusNormal);
	currentMainScreen->DrawString(gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuEntryPosX, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuEntryPosY + 5 * gGUIConst->kUserMenuEntryDY, gGUIConst->kUserMenuEntry5, kShapemodusNormal);
	
	currentItem = lastItem = 0;
	currentNumOfItems = 4;
	page = kMainPage;
	currentItemStart = gGUIConst->kUserMenuEntryPosY + gGUIConst->kUserMenuEntryDY;
	currentEntryDY = gGUIConst->kUserMenuEntryDY;

	SelectMenuItem();
}

void	CGUI::OnOpenNewGamePage()
{
	gLevel->PaintLevel();

	currentMainScreen->PaintRect(gGUIConst->kUserMenuPosX, gGUIConst->kUserMenuPosY, gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuWidth, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuHeight, kBlackColor, kShapemodusTransparent2);
	
	currentMainScreen->DrawString(gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuEntryPosX + kTitleTabulator, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuEntryPosY, gGUIConst->kNewGamePageTitle, kShapemodusLava);

	currentMainScreen->DrawString(gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuEntryPosX, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuEntryPosY + 1 * gGUIConst->kUserMenuEntryDY, gGUIConst->kNewGameEntry1, kShapemodusNormal);
	currentMainScreen->DrawString(gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuEntryPosX, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuEntryPosY + 2 * gGUIConst->kUserMenuEntryDY, gGUIConst->kNewGameEntry2, kShapemodusNormal);
	currentMainScreen->DrawString(gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuEntryPosX, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuEntryPosY + 3 * gGUIConst->kUserMenuEntryDY, gGUIConst->kNewGameEntry3, kShapemodusNormal);
	currentMainScreen->DrawString(gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuEntryPosX, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuEntryPosY + 4 * gGUIConst->kUserMenuEntryDY, gGUIConst->kNewGameEntry4, kShapemodusNormal);

	currentItem = 1;
	lastItem = 0;
	currentNumOfItems = 3;
	page = kNewGamePage;
	currentItemStart = gGUIConst->kUserMenuEntryPosY + gGUIConst->kUserMenuEntryDY;
	currentEntryDY = gGUIConst->kUserMenuEntryDY;

	SelectMenuItem();
}


void	CGUI::OnOpenLoadPage(short whichPage)
{
	short	n;

	gLevel->PaintLevel();
	currentMainScreen->PaintRect(gGUIConst->kUserMenuPosX, gGUIConst->kUserMenuPosY, gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuWidth, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuHeight, kBlackColor, kShapemodusTransparent2);

	for (n = 0; n < kNumOfSaveGameSlots; n ++) {
		char *filename = gSystem->QualifyHomeDir(gGUIConst->kSavedGames[n]);
		gLevel->GetSavedGameTitle(filename, sgTitles[n]);  // by LL
		delete [] filename;
	}

	if (whichPage == kSaveGamePage) 
		currentMainScreen->DrawString(gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuEntryPosX + kTitleTabulator, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuEntryPosY + 0 * gGUIConst->kUserMenuEntryDY, gGUIConst->kSavePageTitle, kShapemodusLava);
	else
		currentMainScreen->DrawString(gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuEntryPosX + kTitleTabulator, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuEntryPosY + 0 * gGUIConst->kUserMenuEntryDY, gGUIConst->kLoadPageTitle, kShapemodusLava);
	
	for (n = 0; n < kNumOfSaveGameSlots; n ++) {
		currentMainScreen->DrawString(gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuEntryPosX, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuEntryPosY + (n + 1) * gGUIConst->kUserMenuEntryDY, sgTitles[n], kShapemodusNormal);
	}
	currentItem = lastItem = 0;
	currentNumOfItems = 5;
	page = whichPage;
	currentItemStart = gGUIConst->kUserMenuEntryPosY + gGUIConst->kUserMenuEntryDY;
	currentEntryDY = gGUIConst->kUserMenuEntryDY;

	SelectMenuItem();
}

void	CGUI::OnOpenHelpPage()
{
	gLevel->PaintLevel();
	currentMainScreen->PaintRect(gGUIConst->kUserMenuPosX, gGUIConst->kUserMenuPosY, gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuWidth, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuHeight, kBlackColor, kShapemodusTransparent2);
	
	currentMainScreen->DrawString(gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuEntryPosX + kTitleTabulator, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuEntryPosY + 0 * gGUIConst->kUserMenuEntryDY, gGUIConst->kHelpPageTitle, kShapemodusLava);
	
	currentMainScreen->PaintGraphic(1, gGUIConst->kUserMenuPosX + 1.5 * gGUIConst->kUserMenuEntryPosX, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuEntryPosY + 2 * gGUIConst->kCreditsEntryDY, kShapemodusTransparent2);

	currentItem = lastItem = -1;
	currentNumOfItems = 0;
	currentItemStart = gGUIConst->kUserMenuPosY;
	currentEntryDY = gGUIConst->kHelpMenuEntryDY;

	page = kHelpPage;
}

void	CGUI::OnOpenCreditsPage()
{
	gLevel->PaintLevel();
		
	currentMainScreen->PaintRect(gGUIConst->kUserMenuPosX, gGUIConst->kUserMenuPosY, gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuWidth, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuHeight, kBlackColor, kShapemodusTransparent2);
	
	currentMainScreen->DrawString(gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuEntryPosX + kTitleTabulator, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuEntryPosY + 0 * gGUIConst->kCreditsEntryDY, gGUIConst->kCreditsPageTitle, kShapemodusLava);
	
	currentMainScreen->PaintGraphic(0, gGUIConst->kUserMenuPosX + 1.5 * gGUIConst->kUserMenuEntryPosX, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuEntryPosY + 2 * gGUIConst->kCreditsEntryDY, kShapemodusTransparent2);

	currentItem = lastItem = 1;
	currentNumOfItems = 0;
	currentItemStart = gGUIConst->kUserMenuEntryPosY + 7 * gGUIConst->kCreditsEntryDY;
	currentEntryDY = gGUIConst->kCreditsEntryDY;
	page = kCreditPage;
}

void	CGUI::OnOpenQuitPage()
{
	gLevel->PaintLevel();
		
	currentMainScreen->PaintRect(gGUIConst->kUserMenuPosX, gGUIConst->kUserMenuPosY, gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuWidth, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuHeight, kBlackColor, kShapemodusTransparent2);
	
	currentMainScreen->DrawString(gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuEntryPosX + kTitleTabulator, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuEntryPosY + 0 * gGUIConst->kCreditsEntryDY, gGUIConst->kCreditsPageTitle, kShapemodusLava);
	
	currentMainScreen->PaintGraphic(0, gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuEntryPosX, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuEntryPosY + 2 * gGUIConst->kCreditsEntryDY, kShapemodusTransparent2);

	currentMainScreen->DrawString(gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuEntryPosX, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuEntryPosY + 8 * gGUIConst->kCreditsEntryDY, gGUIConst->kCreditsEntry4, kShapemodusNormal);
	currentMainScreen->DrawString(gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuEntryPosX, gGUIConst->kUserMenuPosY + gGUIConst->kUserMenuEntryPosY + 9 * gGUIConst->kCreditsEntryDY, gGUIConst->kCreditsEntry5, kShapemodusNormal);

	currentItem = lastItem = 0;
	currentNumOfItems = 1;
	currentItemStart = gGUIConst->kUserMenuEntryPosY + 8 * gGUIConst->kCreditsEntryDY;
	currentEntryDY = gGUIConst->kCreditsEntryDY;
	page = kQuitPage;
}

void	CGUI::SelectMenuItem()
{
	char	*curItem;

	curItem = ItemToItem(lastItem);
	currentMainScreen->DrawString(gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuEntryPosX, gGUIConst->kUserMenuPosY + currentItemStart + lastItem * currentEntryDY, curItem, kShapemodusNormal);
	
	curItem = ItemToItem(currentItem);
	currentMainScreen->DrawString(gGUIConst->kUserMenuPosX + gGUIConst->kUserMenuEntryPosX, gGUIConst->kUserMenuPosY + currentItemStart + currentItem * currentEntryDY, curItem, kShapemodusRandom);
}


char	*CGUI::ItemToItem(short item)
{
	switch (page) {
		case kMainPage:
			switch (item) {
				case 0: return gGUIConst->kUserMenuEntry1; break;
				case 1: return gGUIConst->kUserMenuEntry2; break;
				case 2: return gGUIConst->kUserMenuEntry3; break;
				case 3: return gGUIConst->kUserMenuEntry4; break;		
				case 4: return gGUIConst->kUserMenuEntry5; break;
				default: return gGUIConst->kUserMenuEntry1; break;
			}
			break;

		case kNewGamePage:
			switch (item) {
				case 0: return gGUIConst->kNewGameEntry1; break;
				case 1: return gGUIConst->kNewGameEntry2; break;
				case 2: return gGUIConst->kNewGameEntry3; break;
				case 3: return gGUIConst->kNewGameEntry4; break;		
				default: return gGUIConst->kNewGameEntry1; break;
			}
			break;

		case kLoadGamePage:
		case kSaveGamePage:
			return sgTitles[item];
			break;

		case kQuitPage:
			switch (item) {
				case 0: return gGUIConst->kCreditsEntry4; break;
				case 1: return gGUIConst->kCreditsEntry5; break;
				default: return gGUIConst->kCreditsEntry5; break;
			}
			break;
			
		default:
			return 0L;
			break;
	}
}

void	CGUI::DisplayInfotext(short num)
{
	currentMessage = messages->generalMessages[num];
	msgDisplayTime = gSystem->GetTicks() + gGUIConst->kInfoTextTime;
}


void	CGUI::OnPickUpSomething(short what)
{
	switch (what) {
			case kItemPhiol:
				currentMessage = messages->msgPhiol;
				break;
			case kItemSorcery:
				currentMessage = messages->msgBM;
				break;
			case kItemBow:
				currentMessage = messages->msgBow;
				break;
			case kItemScie:
				currentMessage = messages->msgScie;
				break;
			case kItemHands:
				currentMessage = messages->msgHands;
				break;
			case kItemBomb:
				currentMessage = messages->msgBombs;
				break;
			case kItemStaff:
				currentMessage = messages->msgStaff;
				break;
			case kItemPhiolmun:
				currentMessage = messages->msgPhiolMun;
				break;
			case kItemSorcerymun:
				currentMessage = messages->msgBMMun;
				break;
			case kItemBowmun:
				currentMessage = messages->msgBowMun;
				break;
			case kItemHandsmun:
				currentMessage = messages->msgHandsMun;
				break;
			case kItemBombmun:
				currentMessage = messages->msgBombsMun;
				break;
			case kItemStaffmun:
				currentMessage = messages->msgStaffMun;
				break;
			case kItemOxygen:
				currentMessage = messages->msgOxygen;
				break;
			case kItemHelppacket:
				currentMessage = messages->msgHealth;
				break;
			default:
				currentMessage = 0L;
				break;
	}
	msgDisplayTime = gSystem->GetTicks() + gGUIConst->kInfoTextTime;
}


void	CGUI::DisplayMessages()
{
	if (msgDisplayTime > gSystem->GetTicks() && currentMessage)
		currentMainScreen->DrawString(gGUIConst->kInfoTextLeft, gGUIConst->kInfoTextTop, currentMessage, kShapemodusTransparent1);
}


void	CGUI::ResetTicks(long ticks)
{
	lastKeyHit = ticks;

	lastHealthPanelRefresh = 0;
	healthCurveStart = 0;
	lastOxygenPanelRefresh = 0;
	oxygenCurveStart = 0;
	msgDisplayTime = 0;
}

long	GetLongConstant(FILE *f, char *constName);
double	GetDoubleConstant(FILE *f, char *constName);
void	GetStringConstant(FILE *f, char *constName, char *buffer);

void	CGUI::LoadMessages()
{
	FILE	*f = fopen(gSystem->QualifyDataDir(kParFileName), "r");   // by LL
	char	msgString[6] = "msgxx";
	short	n;

	messages = new tMessages;

	GetStringConstant(f, "msgPickedUpHealth", messages->msgHealth);
	GetStringConstant(f, "msgPickedUpOxygen", messages->msgOxygen);

	GetStringConstant(f, "msgPickedUpPhiol", messages->msgPhiol);
	GetStringConstant(f, "msgPickedUpBM", messages->msgBM);
	GetStringConstant(f, "msgPickedUpBow", messages->msgBow);
	GetStringConstant(f, "msgPickedUpScie", messages->msgScie);
	GetStringConstant(f, "msgPickedUpHands", messages->msgHands);
	GetStringConstant(f, "msgPickedUpBombs", messages->msgBombs);
	GetStringConstant(f, "msgPickedUpStaff", messages->msgStaff);

	GetStringConstant(f, "msgPickedUpPhiolMun", messages->msgPhiolMun);
	GetStringConstant(f, "msgPickedUpBMMun", messages->msgBMMun);
	GetStringConstant(f, "msgPickedUpBowMun", messages->msgBowMun);
	GetStringConstant(f, "msgPickedUpHandMun", messages->msgHandsMun);
	GetStringConstant(f, "msgPickedUpBombMun", messages->msgBombsMun);
	GetStringConstant(f, "msgPickedUpStaffMun", messages->msgStaffMun);

	for (n = 0; n < kMaxInfotexts -1; n ++) {
		msgString[3] = (char)(n / 10) + '0';
		msgString[4] = (char)(n % 10) + '0';
		GetStringConstant(f, msgString, messages->generalMessages[n]);
	}

	fclose(f);
}


void	LoadGUIParameters()
{
	FILE	*f = fopen(gSystem->QualifyDataDir(kParFileName), "r");  // by LL
	gGUIConst = new tGUIConstants;

	gGUIConst->kHealthPanelHeight = GetLongConstant(f, "kHealthPanelHeight");
	gGUIConst->kHealthPanelPosX = GetLongConstant(f, "kHealthPanelPosX");
	gGUIConst->kHealthPanelPosY = GetLongConstant(f, "kHealthPanelPosY");
	gGUIConst->kHealthPanelRefreshRate = GetLongConstant(f, "kHealthPanelRefreshRate");
	gGUIConst->kHealthPanelWidth = GetLongConstant(f, "kHealthPanelWidth");
	gGUIConst->kOxygenPanelHeight = GetLongConstant(f, "kOxygenPanelHeight");
	gGUIConst->kOxygenPanelPosX = GetLongConstant(f, "kOxygenPanelPosX");
	gGUIConst->kOxygenPanelPosY = GetLongConstant(f, "kOxygenPanelPosY");
	gGUIConst->kOxygenPanelRefreshRate = GetLongConstant(f, "kOxygenPanelRefreshRate");
	gGUIConst->kOxygenPanelWidth = GetLongConstant(f, "kOxygenPanelWidth");
	gGUIConst->kWeaponPanelHeight = GetLongConstant(f, "kWeaponPanelHeight");
	gGUIConst->kWeaponPanelPosX = GetLongConstant(f, "kWeaponPanelPosX");
	gGUIConst->kWeaponPanelPosY = GetLongConstant(f, "kWeaponPanelPosY");
	gGUIConst->kWeaponPanelWidth = GetLongConstant(f, "kWeaponPanelWidth");
	gGUIConst->kWeaponStatusWidth = GetLongConstant(f, "kWeaponStatusWidth");

	gGUIConst->kUserMenuEntryDY = GetLongConstant(f, "kUserMenuEntryDY");
	gGUIConst->kUserMenuEntryPosX = GetLongConstant(f, "kUserMenuEntryPosX");
	gGUIConst->kUserMenuEntryPosY = GetLongConstant(f, "kUserMenuEntryPosY");
	gGUIConst->kUserMenuHeight = GetLongConstant(f, "kUserMenuHeight");
	gGUIConst->kUserMenuPosX = GetLongConstant(f, "kUserMenuPosX");
	gGUIConst->kUserMenuPosY = GetLongConstant(f, "kUserMenuPosY");
	gGUIConst->kUserMenuWidth = GetLongConstant(f, "kUserMenuWidth");
	gGUIConst->kHelpMenuEntryDY = GetLongConstant(f, "kHelpMenuEntryDY");

	gGUIConst->kInfoTextLeft = GetLongConstant(f, "kInfoTextLeft");
	gGUIConst->kInfoTextTop = GetLongConstant(f, "kInfoTextTop");
	gGUIConst->kInfoTextTime = GetLongConstant(f, "kInfoTextTime");
	
	GetStringConstant(f, "kUserMainPageTitle", gGUIConst->kUserMainPageTitle);
	GetStringConstant(f, "kUserMenuEntry1", gGUIConst->kUserMenuEntry1);
	GetStringConstant(f, "kUserMenuEntry2", gGUIConst->kUserMenuEntry2);
	GetStringConstant(f, "kUserMenuEntry3", gGUIConst->kUserMenuEntry3);
	GetStringConstant(f, "kUserMenuEntry4", gGUIConst->kUserMenuEntry4);
	GetStringConstant(f, "kUserMenuEntry5", gGUIConst->kUserMenuEntry5);

	GetStringConstant(f, "kNewGamePageTitle", gGUIConst->kNewGamePageTitle);
	GetStringConstant(f, "kNewGameEntry1", gGUIConst->kNewGameEntry1);
	GetStringConstant(f, "kNewGameEntry2", gGUIConst->kNewGameEntry2);
	GetStringConstant(f, "kNewGameEntry3", gGUIConst->kNewGameEntry3);
	GetStringConstant(f, "kNewGameEntry4", gGUIConst->kNewGameEntry4);

	GetStringConstant(f, "kNoSavedGame", gGUIConst->kNoSavedGame);
	GetStringConstant(f, "kSavePageTitle", gGUIConst->kSavePageTitle);
	GetStringConstant(f, "kLoadPageTitle", gGUIConst->kLoadPageTitle);
	GetStringConstant(f, "kHelpPageTitle", gGUIConst->kHelpPageTitle);

	GetStringConstant(f, "kSavedGame0", gGUIConst->kSavedGames[0]);
	GetStringConstant(f, "kSavedGame1", gGUIConst->kSavedGames[1]);
	GetStringConstant(f, "kSavedGame2", gGUIConst->kSavedGames[2]);
	GetStringConstant(f, "kSavedGame3", gGUIConst->kSavedGames[3]);
	GetStringConstant(f, "kSavedGame4", gGUIConst->kSavedGames[4]);
	GetStringConstant(f, "kSavedGame5", gGUIConst->kSavedGames[5]);

	gGUIConst->kKeyHitDelay = GetLongConstant(f, "kKeyHitDelay");

	gGUIConst->kCreditsEntryDY = GetLongConstant(f, "kCreditsEntryDY");
	GetStringConstant(f, "kCreditsPageTitle", gGUIConst->kCreditsPageTitle);
	GetStringConstant(f, "kCreditsEntry1", gGUIConst->kCreditsEntry1);
	GetStringConstant(f, "kCreditsEntry2", gGUIConst->kCreditsEntry2);
	GetStringConstant(f, "kCreditsEntry3", gGUIConst->kCreditsEntry3);
	GetStringConstant(f, "kCreditsEntry4", gGUIConst->kCreditsEntry4);
	GetStringConstant(f, "kCreditsEntry5", gGUIConst->kCreditsEntry5);


	fclose(f);
}

	