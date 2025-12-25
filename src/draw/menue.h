/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef MENUE_H
#define MENUE_H

#include <irrlicht.h>
#include <vector>
#include <cstdint>
#include <string>

//definition of available menue pages
#define MENUE_AFTERGAMESTART 0
#define MENUE_TOPENTRY 1
#define MENUE_OPTION 2
#define MENUE_CHANGENAME 3
#define MENUE_SELECTIONRACE 4
#define MENUE_CHAMPIONSHIP 5
#define MENUE_CHAMPIONSHIPSAVE 6
#define MENUE_CHAMPIONSHIPLOAD 7
#define MENUE_VIDEOOPTIONS 8
#define MENUE_SOUNDOPTIONS 9
#define MENUE_SELECTRACETRACK 10
#define MENUE_SELECTSHIP 11

//special "menue" pages
#define MENUE_HIGHSCORE 202
#define MENUE_RACESTATS 203
#define MENUE_POINTSTABLE 204

#define MENUE_ENTRY_TYPE_DEFAULT 0
#define MENUE_ENTRY_TYPE_SLIDER 1
#define MENUE_ENTRY_TYPE_TEXTINPUTFIELD 2
#define MENUE_ENTRY_TYPE_EMPTYSPACE 3

//definition of available menue action trigger types
#define MENUE_ACTION_NOACTION 0
#define MENUE_ACTION_RACE 1
#define MENUE_ACTION_QUITTOOS 2
#define MENUE_ACTION_SETCOMPUTERPLAYERENA 3
#define MENUE_ACTION_SETDIFFICULTYLEVEL 4
#define MENUE_ACTION_SETPLAYERNAME 5
#define MENUE_ACTION_SETMUSICVOLUME 6
#define MENUE_ACTION_SETSOUNDVOLUME 7
#define MENUE_ACTION_SETDOUBLERESOLUTION 8
#define MENUE_ACTION_SETVSYNC 9
#define MENUE_ACTION_SETENABLESHADOW 10
#define MENUE_ACTION_SETUPGRADEDSKY 11
#define MENUE_ACTION_RETURNFROMDETAILSMENUE 12

//"special" menue actions
#define MENUE_ACTION_CLOSERACESTATPAGE 101
#define MENUE_ACTION_STARTDEMO 102
#define MENUE_ACTION_SHOWHIGHSCOREPAGE 103
#define MENUE_ACTION_CLOSEPOINTSTABLEPAGE 104

//Championship menue actions
#define MENUE_ACTION_CHAMPIONSHIP_CONTINUE 110
#define MENUE_ACTION_CHAMPIONSHIP_LOADSLOT 111
#define MENUE_ACTION_CHAMPIONSHIP_SAVESLOT 112
#define MENUE_ACTION_CHAMPIONSHIP_SAVESLOT_FINALIZE 113
#define MENUE_ACTION_CHAMPIONSHIP_QUIT 114
#define MENUE_ACTION_CHAMPIONSHIP_ENTERMENUE 115

//definition of possible menue states
#define MENUE_STATE_TRANSITION 0  //menue window is currently moving, no item selection possible
#define MENUE_STATE_TYPETEXT 1    //menue window is open and stopped, but menue item text is written character by character
                                  //(like in original game in VGA mode)
#define MENUE_STATE_SELACTIVE 2   //menue window reached final location, user is able to select items

//sets the menue window animation time period in seconds
//therefore controls speed of window movements
#define MENUE_WINDOW_ANIMATION_PERIODTIME 0.01f

//defines the type writer effect in the
//menue
#define MENUE_ENABLETYPEWRITEREFFECT true
#define MENUE_TYPEWRITEREFFECT_TYPEDELAY 0.01f  //time between typing in seconds
#define MENUE_TYPEWRITEREFFECT_SOUNDFILE1 (char*)"extract/sound/sound2-PRINTTYP.WAV"
#define MENUE_TYPEWRITEREFFECT_SOUNDFILE2 (char*)"extract/sound/sound2-PRINTRET.WAV"
#define MENUE_SELECTOTHERITEM_SOUNDFILE (char*)"extract/sound/sound2-CHANGE.WAV"
#define MENUE_CHANGECHECKBOXVAL_SOUNDFILE (char*)"extract/sound/sound2-SELECT.WAV"
#define MENUE_WINDOWMOVEMENT_SOUND (char*)"extract/sound/sound2-REFUEL.WAV"

//max text input field text length
#define MENUE_TEXTENTRY_MAXTEXTLEN 8

//text input field cursor blink period time in seconds
#define MENUE_TEXTENTRY_CURSORBLINKPERIODSEC 0.5f

//3D model update period time in seconds
#define MENUE_3DMODEL_UPDATEPERIODSEC 0.015f

//distance in pixels between menue entries
#define MENUE_ENTRY_DISTANCE_PIXEL 10

//empty space menue item height in pixels
//is also the fixed height for text input fields
#define MENUE_ENTRY_EMPTYSPACE_HEIGHT_PIXELS 30

//distance in pixels between menue entry and slider
#define MENUE_ENTRY_TOSLIDER_DISTANCE_PIXEL 10

//after the user is inactive for 30 seconds in the menue
//trigger an action automatically
//implemented as in the original game
const irr::f32 MENUE_USERINACTIVETIME_THRESHOLD = 30.0f;

//this struct holds the information for a menue
//graphical element (mostly logo)
typedef struct MenueGraphicPart {
     irr::core::vector2d<irr::s32> drawScrPosition;
     irr::video::ITexture* texture = nullptr;
     irr::core::dimension2d<irr::s32> sizeTex;
} MenueGraphicPart;

class MenueAction {
public:
    MenueAction(irr::u32 actionNrParam);
    ~MenueAction();

    //can be used for sliders/checkboxes to
    //handover current set value
    //for simple menue items which are activated with
    //Return key press this field is do not care
    irr::u32 currSetValue;

    //can be used to return a newly set text
    //input string (is null terminated)
    //for simple menue items which are activated with
    //Return key press this field is do not care
    char newSetTextInputString[30];

private:
    //integer index for type of action
    irr::u32 actionNr;
};

class MenuePage; //Forward declaration
struct RaceStatsEntryStruct; //Forward declaration
struct PointTableEntryStruct; //Forward declaration
struct ChampionshipSaveGameInfoStruct; //Forward declaration
class SoundEngine; //Forward declaration
class Assets; //Forward declaration
class Game; //Forward declaration
struct GameTextFont; //Forward declaration
class Menue;  //Forward declaration

//this struct holds the information about a single
//menue entry
class MenueSingleEntry {
public:
    MenuePage* mParentPage = nullptr;

    //Gets the needed width in pixels
    irr::u32 GetWidth();

    //Gets the needed height in pixels
    irr::u32 GetHeight();

    void SetText(char* newText);

    irr::u8 entryType;
    char* entryText;

    irr::u8 entryNumber;
    irr::core::vector2d<irr::s32> drawTextScrPosition;

    //if true this menue item can be selected by using
    //cursor key up/down; If false the item is static
    //and can not be used/changed
    bool itemSelectable = true;

    //the next items describe what does happen when we
    //press Return on this menue item
    //MenuePage nextMenuePage allows to change to
    //another menue page, if this is not wanted simply set
    //this pointer to nullptr
    MenuePage* nextMenuePage = nullptr;

    //the next item describes if the menue item
    //can trigger an action (and which kind of action)
    //if the user selects the item and presses Return key
    //if there is no action then this pointer is set to nullptr
    MenueAction* triggerAction = nullptr;

    //is for example used for player name entry
    //dialog
    //pointer which allows to point to a char array
    //content is used to initialize text input field
    char* initTextPntr = nullptr;

    bool isTextEntryField = false;
    //current text input string
    char currTextInputFieldStr[MENUE_TEXTENTRY_MAXTEXTLEN + 1];
    irr::u8 currTextInputFieldStrLen = 0;

    //some menu items can be directly used as a kind of checkbox (only two states)
    //or setting a range of value from 0 to some max value (for example sound
    //volume). The following variables will allow to implement this behavior.
    //for a menue item that does not have this behavior we simply set maxValue and
    //currValue both to 0
    irr::u8 currValue = 0;
    irr::u8 maxValue = 0;
    irr::core::recti checkBoxOutline;
    irr::u8 checkBoxPixelPerBlockWidth;
    irr::u8 checkBoxPixelPerBlockHeight;
    irr::u8 checkBoxNrBlocks;

    //which font should be used?
    GameTextFont* usedUnselectedTextFont = nullptr;
    GameTextFont* usedSelectedTextFont = nullptr;
};

//this struct collects information about one
//possible page of the menue (summary of multiple
//MenueSingleEntries)
class MenuePage {
public:
    MenuePage(Menue* parentMenue, MenuePage* parentPage, irr::u8 pageNr, MenueAction* pageEscKeyAction = nullptr);
    ~MenuePage();

    //returns pointer to newly created entry, if user needs this
    MenueSingleEntry* AddDefaultMenueEntry(const char* text, bool itemSelectable, MenuePage* goToPage, MenueAction* triggerAction);
    MenueSingleEntry* AddDefaultMenueEntry(const char* text, bool itemSelectable, MenuePage* goToPage, MenueAction* triggerAction,
                                                      GameTextFont* unselectedFont, GameTextFont* selectedFont);

    MenueSingleEntry* AddSliderMenueEntry(const char* text, bool itemSelectable,
                                                     irr::u8 currValueParam, irr::u8 maxValueParam, irr::u8 nrBlocksParam,
                                                     irr::u8 checkBoxPixelPerBlockWidthParam, irr::u8 checkBoxPixelPerBlockHeightParam,
                                                     MenueAction* triggerAction);
    MenueSingleEntry* AddTextInputMenueEntry(char* initTextPntrParam, bool itemSelectable, MenueAction* triggerAction);
    MenueSingleEntry* AddEmptySpaceMenueEntry();

    void RealignMenueEntries(irr::core::recti newMenueSpace);

    Menue* mParentMenue;

    std::vector<MenueSingleEntry*> pageEntryVec;

    //Allows to trigger an action when this page
    //is exit with ESC key
    MenueAction* pageEscKeyTriggerAction;

    irr::u8 pageNumber;

    //is used when program needs to return to menue page
    //one level above, for example when player presses ESC key
    //one level; or when text input is done in one player
    MenuePage* parentMenuePage = nullptr;
};

typedef struct {
    std::vector<irr::core::recti> coordVec;
} MenueWindowAnimationVec;

struct ShipStatLabel {
   char* text = nullptr;
   irr::core::vector2di drawPositionTxt;

   irr::core::recti statBoxOutline;
   irr::u8 statNrBlocks;
};

struct MenueTextLabel {
    char* text = nullptr;
    irr::core::vector2di drawPositionTxt;
    bool visible = true;
    GameTextFont* whichFont = nullptr;
};

class Menue {

private:
    SoundEngine* mSoundEngine = nullptr;
    Assets* mGameAssets = nullptr;

    std::vector<MenueGraphicPart*> GameLogo;

    //menue window graphic resources
    irr::video::ITexture* wndCornerElementUpperLeftTex = nullptr;
    irr::video::ITexture* wndCornerElementUpperRightTex = nullptr;
    irr::video::ITexture* wndCornerElementLowerLeftTex = nullptr;
    irr::video::ITexture* wndCornerElementLowerRightTex = nullptr;

    //text input field cursor variables
    //true if text cursor should be visible currently
    //during rendering
    bool blinkTextCursorVisible;
    irr::f32 blinkTextCursorNextStateChangeAbsTime;

    //the following variables are needed for the
    //type writer effect when the menu entries are
    //written in the menue window
    irr::u32 currNrCharsShownCnter;
    irr::u32 finalNrChardsShownMenuePageFinished;
    irr::f32 typeWriterEffectNextCharacterAbsTime;

    //current graphical region available for the
    //selection menue
    irr::core::recti currMenueSpace;

    void RealignMenuePageItems();

    bool InitMenueResources();
    void CreateMenueEntries();
    void RenderWindow(irr::core::recti position);
    void PrintMenueEntries();
    void RenderRaceSelection();
    void InitActions();
    void InitMenuePages();

    void AcceptedRaceSetup();

    void UpdateChampionshipSaveGames();

    void RenderIntro(irr::f32 frameDeltaTime);

    //renderOnlyNumberBlocks = optional parameter, needed for type writer effect in menue
    void RenderCheckBox(MenueSingleEntry* entry, irr::core::recti position, irr::video::SColor colorRect, irr::video::SColor lineColor,
           irr::u8 nrBlocks, irr::s8 renderOnlyNumberBlocks = -1);

    void RenderCursor(MenueSingleEntry* textEntryField);
    void PlayMenueSound(uint8_t sndResId);
    void StopMenueSound();

    //Menue definitions for main top entry menue
    MenuePage* TopMenuePage = nullptr;
    MenueSingleEntry* Race = nullptr;

    //Menue definitions for Race page
    MenuePage* RaceMenuePage = nullptr;
    MenueSingleEntry* SelectChampionsship = nullptr;
    MenueSingleEntry* SelectSingleRace = nullptr;

    //Menue definitions for Championship page
    MenuePage* ChampionshipMenuePage = nullptr;
    MenueSingleEntry* ContinueChampionshipEntry = nullptr;
    MenueSingleEntry* NewChampionshipEntry = nullptr;
    MenueSingleEntry* SaveChampionshipEntry = nullptr;

    //menue definitions for Championship save page
    MenuePage* ChampionshipSaveMenuePage = nullptr;
    MenueSingleEntry* ChampionshipSaveSlot1 = nullptr;
    MenueSingleEntry* ChampionshipSaveSlot2 = nullptr;
    MenueSingleEntry* ChampionshipSaveSlot3 = nullptr;
    MenueSingleEntry* ChampionshipSaveSlot4 = nullptr;
    MenueSingleEntry* ChampionshipSaveSlot5 = nullptr;

    //menue definitions for Championship load page
    MenuePage* ChampionshipLoadMenuePage = nullptr;
    MenueSingleEntry* ChampionshipLoadSlot1 = nullptr;
    MenueSingleEntry* ChampionshipLoadSlot2 = nullptr;
    MenueSingleEntry* ChampionshipLoadSlot3 = nullptr;
    MenueSingleEntry* ChampionshipLoadSlot4 = nullptr;
    MenueSingleEntry* ChampionshipLoadSlot5 = nullptr;

    //Menue definitions for option menue
    MenuePage* OptionMenuePage = nullptr;
    MenueSingleEntry* ComputerPlayersCheckBox = nullptr;
    MenueSingleEntry* DifficultyLevel = nullptr;
    MenueSingleEntry* OptionMenuePageBackToMainMenue = nullptr;

    //Menue definitions for Name change dialog/menue
    MenuePage* ChangeNamePage = nullptr;
    MenueSingleEntry* EnterNameLabel = nullptr;
    MenueSingleEntry* PlayerNameEnterField = nullptr;

    //Menue definitions for video details
    MenuePage* VideoDetailsPage = nullptr;
    MenueSingleEntry* EnableDoubleResolution = nullptr;
    MenueSingleEntry* EnableVSync = nullptr;
    MenueSingleEntry* EnableShadows = nullptr;
    MenueSingleEntry* UseUpgradedSky = nullptr;
    MenueSingleEntry* VideoPageBackToOptionsMenue = nullptr;

    //Menue definitions for sound options
    MenuePage* SoundOptionsPage = nullptr;
    MenueSingleEntry* MusicVolumeSlider = nullptr;
    MenueSingleEntry* EffectsVolumeSlider = nullptr;
    MenueSingleEntry* SoundOptionsBackToOptionsMenue = nullptr;

    //Race track selection menue page
    MenuePage* RaceTrackSelectionPage = nullptr;
    MenueSingleEntry* RaceTrackNameTitle = nullptr;

    //Ship selection menue page
    MenuePage* ShipSelectionPage = nullptr;
    MenueSingleEntry* ShipNameTitle = nullptr;

    MenuePage* currSelMenuePage = nullptr;
    MenueSingleEntry* currSelMenueSingleEntry = nullptr;
    MenueAction* currActionToExecute = nullptr;

    irr::u32 MaxMenueEntryNumber(MenuePage* inputPage);
    void SelectNewMenueEntryWithEntryNr(MenuePage* newMenuePage, irr::u32 nextSelEntryNr);
    MenueSingleEntry* GetMenueSingleEntryForEntryNr(MenuePage* newMenuePage, irr::u32 inputEntryNr);
    irr::u32 MenuePageFindFirstSelectableEntryNr(MenuePage* newMenuePage);
    void ReturnToParentMenuePage(MenuePage* currentMenuePage);
    irr::u32 GetNrOfCharactersOnMenuePage(MenuePage* whichMenuePage);
    irr::u32 GetNrofCharactersForMenueItem(MenueSingleEntry* whichMenueEntry);

    //if parameter championshipMode is true, then race selection screen only allows to select exactly
    //one preselected race track. Which race track is shown and selected depends on
    //call this->mGameAssets->GetLastSelectedRaceTrack(). Also the number of laps for the following race does not depend on the
    //user/player selection, but on the default number of laps for this racetrack. As in the original game.
    void ShowRaceSelection(MenueSingleEntry* callerItem, bool championshipMode);

    void DrawBackground(bool addLogo);

    void ShowShipSelection();

    void InterruptRaceSelection();
    void HandleInputRaceSelection();

    void ItemUp();
    void ItemDown();
    void ItemReturn();

    //for items that have "checkbox/slider" functionality
    void ItemLeft();
    void ItemRight();

    //special functions for text input field
    void HandleTextInputField(MenueSingleEntry* textInputEntry);
    void AddInputTextFieldChar(MenueSingleEntry* textInputEntry, char newCharToAdd);
    void RemoveInputTextFieldChar(MenueSingleEntry* textInputEntry);
    void AcceptInputTextFieldValue(MenueSingleEntry* textInputEntry);
    void SetInputTextField(MenueSingleEntry* textInputEntry, char* newText);

    void UnhideRaceTrackModels();
    void HideRaceTrackModels();
    void UnhideShipModels();
    void HideShipModels();

    //vector which holds a list of preprogrammed window states for
    //a window animation that will start next
    MenueWindowAnimationVec* windowMenueAnimationStartGame  = nullptr; //only active right at game start
                                                                       //before main menue opens first time

    MenueWindowAnimationVec* windowMenueAnimationBeforeTrackSelection = nullptr; //is shown during transition from
                                                                                 //main menue to race track selection

    MenueWindowAnimationVec* windowMenueAnimationQuitTrackSelection  = nullptr;  //is shown during transition from
                                                                                 //race track selection to main menue

    MenueWindowAnimationVec* windowMenueAnimationBetweenRaceAndShipSelection = nullptr; //is shown during transition from
                                                                                        //race track selection to ship selection

    //pointer to a currently active/selected window animation sequence
    MenueWindowAnimationVec* currSelWindowAnimation = nullptr;

    //This function defines the proper
    //coordinates for menue window animation coordinates
    void SetWindowAnimationVec();

    //stores the current menue state, and variables we need
    //for menue animation as in the original game
    irr::u8 currMenueState = MENUE_STATE_TRANSITION;
    irr::u8 currMenueWindowAnimationIdx;
    irr::u8 currMenueWindowAnimationFinalIdx;
    irr::f32 lastAnimationUpdateAbsTime;

    irr::f32 absoluteTime;

    //Irrlicht SceneManagers for race track and ship selection page
    irr::scene::ISceneManager* smgrMenue = nullptr;

    //stores the number of race tracks we have available
    //and we get from assets class for selection
    irr::u8 numRaceTracksAvailable;
    std::vector<irr::scene::IMeshSceneNode*> ModelTrackSceneNodeVec;

    //camera used for the race track/ship selection 3D rendering
    irr::scene::ICameraSceneNode* MenueCamera = nullptr;

    //stores the number of crafts we have available
    //and we get from assets class for selection
    irr::u8 numCraftsAvailable;
    //additional 3D models necessary for ship selection menue
    //we have a vector of sceneNodes, because each ship
    //is available in different color schemes!
    std::vector<std::vector<irr::scene::IMeshSceneNode*>*> ModelCraftsSceneNodeVec;

    //timing variables for 3D model rendering
    irr::f32 last3DModelUpdateAbsTime;
    irr::f32 curr3DModelRotationDeg = 0.0f;
    irr::u8 currSelectedRaceTrack;
    irr::f32 currRaceTrackWheelAngleDeg;
    irr::f32 targetRaceTrackWheelAngleDeg;

    irr::u8 currentSelRaceLapNumberVal;

    irr::u8 currSelectedShip;
    irr::f32 currShipWheelAngleDeg;
    irr::f32 targetShipWheelAngleDeg;
    char currSelShipName[50];

    //ship color scheme selection
    irr::u8 currSelectedShipColorScheme = 0;
    char currSelShipColorSchemeName[50];
    irr::core::vector2d<irr::s32> currSelectedShipColorSchemeTextPos;

    //0 = wheel not moving
    //1 = wheel moving clock wise
    //2 = wheel moving counter clock wise
    //3 = immediatley update position (no rotation), used for restoring last state
    irr::u8 RaceTrackWheelMoving;

    //variable to remembar from which menue item we initially
    //called the race track selection menue so that when the player
    //interrupts we can return to the correct caller menue
    MenueSingleEntry* RaceTrackSelectionCallerMenueEntry = nullptr;

    //MenueTextLabel* SelRaceTrackNrLapsLabel = nullptr;
    //MenueTextLabel* SelRaceTrackBestLapLabel = nullptr;
    //MenueTextLabel* SelRaceTrackBestRaceLabel = nullptr;

    MenueSingleEntry* SelRaceTrackNrLapsLabel = nullptr;
    MenueSingleEntry* SelRaceTrackBestLapLabel = nullptr;
    MenueSingleEntry* SelRaceTrackBestRaceLabel = nullptr;

    //0 = wheel not moving
    //1 = wheel moving clock wise
    //2 = wheel moving counter clock wise
    //3 = immediatley update position (no rotation), used for restoring last state
    irr::u8 ShipWheelMoving;

    //current selected ship stats
    irr::u8 currSelShipStatSpeed;
    irr::u8 currSelShipStatArmour;
    irr::u8 currSelShipStatWeight;
    irr::u8 currSelShipStatFirePower;

    ShipStatLabel* ShipStatSpeedLabel = nullptr;
    ShipStatLabel* ShipStatArmourLabel = nullptr;
    ShipStatLabel* ShipStatWeightLabel = nullptr;
    ShipStatLabel* ShipStatFirePowerLabel = nullptr;

    //create a dummy menue page for
    //high score table page
    MenuePage* gameHighscoreMenuePage = nullptr;
    MenuePage* raceStatsMenuePage = nullptr;
    MenuePage* pointsTablePage = nullptr;

    //dummy menue entry which is not visible
    //for gameHiscoreMenuePage
    MenueSingleEntry* gameHiscoreMenueDummyEntry = nullptr;

    //dummy menue entry which is not visible
    //for raceStatsMenuePage
    MenueSingleEntry* raceStatsMenueDummyEntry = nullptr;

    //dummy menue entry which is not visible
    //for pointsTablePage
    MenueSingleEntry* pointsTableMenueDummyEntry = nullptr;

    void CalcStatLabelHelper(irr::u8 currStatVal, ShipStatLabel &label, irr::core::vector2di centerCoord);
    void InitStatLabels();
    void RecalculateShipStatLabels();
    void CleanUpStatLabels();

    void CalcMenueTextLabelHelper(MenueTextLabel &label, irr::core::vector2di centerCoord);
    void RecalculateRaceTrackStatLabels();

    void InitRaceTrackSceneNodes();
    void Update3DModels();
    void Set3DModelRaceTrackWheelPositionIdx(irr::u8 newPosition, irr::u8 newWheelMoveState);
    void SetRotation3DModelAroundYAxis(irr::scene::ISceneNode* sceneObj, irr::f32 rotDeg);
    void Update3DModelRaceTrackWheelPosition();
    void FinalPositionRaceTrackWheelReached(irr::u8 newPosition);

    void InitVehicleModels();
    void Update3DModelShipWheelPosition();
    void Set3DModelShipWheelPositionIdx(irr::u8 newPosition, irr::u8 newWheelMoveState);
    void SetShipColorScheme(irr::u8 newSelectedColorScheme);
    void HandleInputShipSelection();
    void PrintMenueEntriesShipSelection();
    void UpdateShipSelectionTypeWriterEffect();
    void FinalPositionShipSelectionWheelReached(irr::u8 newPosition);

    void RenderShipStatBoxes(irr::core::recti position, irr::video::SColor colorRect, irr::video::SColor lineColor,
                               irr::u8 nrBlocks, irr::s8 renderOnlyNumberBlocks);

    //stuff for the highscore page
    std::vector<MenueTextLabel*>* highScorePageTextVec = nullptr;

    //stuff for the race stat page
    std::vector<MenueTextLabel*>* raceStatsPageTextVec = nullptr;

    //stuff for the point table page
    std::vector<MenueTextLabel*>* pointsTablePageTextVec = nullptr;

    void RenderStatTextPage(irr::f32 frameDeltaTime);
    void CleanupHighScorepage();
    irr::f32 absTimeElapsedAtHighScorePage;

    //Timer which counts menue user inactive time
    irr::f32 mMenueUserInactiveTimer;

    void UpdateChampionshipLoadSlotMenueEntry(MenueSingleEntry &whichEntry, std::vector<ChampionshipSaveGameInfoStruct*>::iterator it);
    char* mNewChampionshipNameInputText = nullptr;

    irr::core::position2di mLogoExtensionStrPos;

    void ScalePositionMenueTextLabel(MenueTextLabel& whichLabel);

public:
    //if you do not want any Menue Sounds just put NULL pointer into soundEngine
    Menue(Game* game,
           SoundEngine* soundEngine, Assets* assets);
    ~Menue();

    Game* mGame = nullptr;

    bool MenueInitializationSuccess;

    //remembers if we entered the last race selection
    //page in Championship mode or not
    bool mChampionshipMode = false;

    MenueAction* ActRace = nullptr;
    MenueAction* ActQuitToOS = nullptr;

    MenueAction* ActSetComputerPlayerEnable = nullptr;
    MenueAction* ActSetDifficultyLevel = nullptr;

    MenueAction* ActSetDoubleResolution = nullptr;
    MenueAction* ActSetVSync = nullptr;
    MenueAction* ActSetEnableShadows = nullptr;
    MenueAction* ActSetUpgradedSky = nullptr;
    MenueAction* ActReturnFromDetailsMenue = nullptr;

    MenueAction* ActSetPlayerName = nullptr;
    MenueAction* ActSetMusicVolume = nullptr;
    MenueAction* ActSetSoundVolume = nullptr;

    //special menue actions
    MenueAction* ActCloseRaceStatPage = nullptr;
    MenueAction* ActStartDemo = nullptr;
    MenueAction* ActShowHighScorePage = nullptr;
    MenueAction* ActClosePointsTablePage = nullptr;

    //championship menue actions
    MenueAction* ActContinueChampionship = nullptr;
    MenueAction* ActLoadChampionshipSlot = nullptr;
    MenueAction* ActSaveChampionshipSlot = nullptr;
    MenueAction* ActFinalizeChampionshipSaveSlot = nullptr;
    MenueAction* ActQuitChampionship = nullptr;
    MenueAction* ActEnterChampionshipMenue = nullptr;

    void Render(irr::f32 frameDeltaTime);
    void HandleUserInactiveTimer(irr::f32 frameDeltaTime);

    void HandleInput();
    bool HandleActions(MenueAction* &pendingAction);
    void AdvanceTime(irr::f32 frameDeltaTime);

    void ShowGameTitle();
    void ShowGameLoadingScreen();
    void ShowMainMenue();
    void ShowChampionshipMenue(bool enaWindowAnimation = false);
    void ShowRaceMenue();
    void ShowIntro();
    void ShowHighscore();
    void ShowRaceStats(std::vector<RaceStatsEntryStruct*>* finalRaceStatistics);
    void CleanupRaceStatsPage();

    //parameter overallPoints just controls which header text is used
    void ShowPointsTablePage(std::vector<PointTableEntryStruct*>* pointTable, bool overallPoints);
    void CleanupPointsTablePage();

    irr::u8 GetChampionShipSlotNrForSaveAction(irr::u8 whichEntryNumber);
    irr::u8 GetChampionShipSlotNrForLoadAction(irr::u8 whichEntryNumber);

    void ContinueChampionship();
    void StartChampionshipNameInputAtSlot(irr::u8 whichSlotNr);
    void EndChampionshipNameInputAtSlot(irr::u8 whichSlotNr);
};

#endif // MENUE_H
