#ifndef __AMP_GUI__
#define __AMP_GUI__

#include "System.hpp"
#include "Player.hpp"
#include "Surface.hpp"

const short	kMaxEntryLength = 20;
const short	kNoCmdEvent = -1;
const	short	kNumOfSaveGameSlots = 6;
const	short	kMaxInfotexts = 45;

enum {
	kMainPage,
	kNewGamePage,
	kLoadGamePage,
	kSaveGamePage,
	kCreditPage,
	kHelpPage,
	kQuitPage
};

const short kGUIsPositions[6][4] = {
	{120, 0, 220, 100}, // health
	{120, 100, 220, 200}, // oxygen
	{0, 0, 120, 100},     // weapons
//	{120, 200, 280, 680},  // board
	{120, 200, 440, 440},  // board
	{0, 801, 210, 944},  // about box
	{0, 943, 210, 1091} // key config
};

typedef struct {
	short	kHealthPanelWidth,
		kHealthPanelHeight,
		kHealthPanelPosX,
		kHealthPanelPosY,
		kHealthPanelRefreshRate,

		kOxygenPanelWidth,
		kOxygenPanelHeight,
		kOxygenPanelPosX,
		kOxygenPanelPosY,
		kOxygenPanelRefreshRate,

		kWeaponPanelWidth,
		kWeaponPanelHeight,
		kWeaponPanelPosX,
		kWeaponPanelPosY,
		kWeaponStatusWidth,

		kUserMenuPosX,
		kUserMenuPosY,
		kUserMenuWidth,
		kUserMenuHeight,
		kUserMenuEntryPosX,
		kUserMenuEntryPosY,
		kUserMenuEntryDY;

	short	kInfoTextLeft;
	short	kInfoTextTop;
	short	kInfoTextTime;

	short	kHelpMenuEntryDY;

	short	kKeyHitDelay;

	char kUserMainPageTitle[kMaxEntryLength];
	char kUserMenuEntry1[kMaxEntryLength];
	char kUserMenuEntry2[kMaxEntryLength];
	char kUserMenuEntry3[kMaxEntryLength];
	char kUserMenuEntry4[kMaxEntryLength];
	char kUserMenuEntry5[kMaxEntryLength];

	char kNewGamePageTitle[kMaxEntryLength];
	char kNewGameEntry1[kMaxEntryLength];
	char kNewGameEntry2[kMaxEntryLength];
	char kNewGameEntry3[kMaxEntryLength];
	char kNewGameEntry4[kMaxEntryLength];

	char kNoSavedGame[kMaxEntryLength];
	char kSavePageTitle[kMaxEntryLength];
	char kLoadPageTitle[kMaxEntryLength];
	char kHelpPageTitle[kMaxEntryLength];
	char kSavedGames[kNumOfSaveGameSlots][kMaxEntryLength];

	short kCreditsEntryDY;
	char kCreditsPageTitle[kMaxEntryLength];
	char kCreditsEntry1[kMaxEntryLength];
	char kCreditsEntry2[kMaxEntryLength];
	char kCreditsEntry3[kMaxEntryLength];
	char kCreditsEntry4[kMaxEntryLength];
	char kCreditsEntry5[kMaxEntryLength];
} tGUIConstants;


const short	kMessageLength = 30;

typedef struct {
	char msgHealth[kMessageLength];
	char msgOxygen[kMessageLength];

	char msgPhiol[kMessageLength];
	char msgBM[kMessageLength];
	char msgBow[kMessageLength];
	char msgScie[kMessageLength];
	char msgHands[kMessageLength];
	char msgBombs[kMessageLength];
	char msgStaff[kMessageLength];

	char msgPhiolMun[kMessageLength];
	char msgBMMun[kMessageLength];
	char msgBowMun[kMessageLength];
	char msgHandsMun[kMessageLength];
	char msgBombsMun[kMessageLength];
	char msgStaffMun[kMessageLength];

	char generalMessages[kMaxInfotexts][kMessageLength];

} tMessages;


class CGUI {
protected:
	Graphic_file	*guis;
	CGraphicSurface	*backgroundPanel;
	CGraphicSurface	*healthPanel;
	CGraphicSurface	*oxygenPanel;
	CGraphicSurface *weaponPanel;
	CPlayer			*player;
	tMessages		*messages;

	long			lastKeyHit;

	long			lastHealthPanelRefresh;
	short			healthCurveStart;
	long			lastOxygenPanelRefresh;
	double			oxygenCurveStart;

	CGraphicSurface	*currentMainScreen;
	short			currentItem, lastItem;
	short			currentNumOfItems;
	short			currentItemStart;
	short			currentEntryDY;
	short			page;

	char	sgTitles[kNumOfSaveGameSlots][30];
	char	*currentMessage;
	long	msgDisplayTime;

	void	DrawHealthPanel();
	void	DrawOxygenPanel();

	void	SelectMenuItem();
	char	*ItemToItem(short item);

	short	ProcessKeyStrokes();
	short	MainPageEvents();
	short	NewGamePageEvents();
	short	LoadGamePageEvents();
	short	QuitPageEvents();

	void	OnOpenMainPage();
	void	OnOpenNewGamePage();
	void	OnOpenLoadPage(short whichPage);
	void	OnOpenHelpPage();
	void	OnOpenCreditsPage();
	void	OnOpenQuitPage();

	void	LoadMessages();

public:
	CGUI(CGraphicSurface *curMainScreen);
	~CGUI();

	void	SetFocus(CPlayer *focus);

	void	Update();
	void	UpdateWeapon();
	short	RunUserInterface(short page);
	void	ResetTicks(long ticks);
	void	OnPickUpSomething(short what);
	void	DisplayMessages();
	void	DisplayInfotext(short num);
};

#endif