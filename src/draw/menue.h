/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef MENUE_H
#define MENUE_H

#include <irrlicht/irrlicht.h>
#include <vector>
#include "gametext.h"
#include "../input/input.h"
#include <string.h>
#include "math.h"
#include "../audio/sound.h"
#include "../resources/assets.h"

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
#define MENUE_GAMETITLE 200
#define MENUE_LOADRACESCREEN 201

//definition of available menue action trigger types
#define MENUE_ACTION_NOACTION NULL
#define MENUE_ACTION_RACE 1
#define MENUE_ACTION_QUITTOOS 2
#define MENUE_ACTION_SETCOMPUTERPLAYERENA 3
#define MENUE_ACTION_SETDIFFICULTYLEVEL 4
#define MENUE_ACTION_SETPLAYERNAME 5
#define MENUE_ACTION_SETMUSICVOLUME 6
#define MENUE_ACTION_SETSOUNDVOLUME 7

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
#define MENUE_3DMODEL_UPDATEPERIODSEC 0.075f

//this struct holds the information for a menue
//graphical element (mostly logo)
typedef struct {
     irr::core::vector2d<irr::s32> drawScrPosition;
     irr::video::ITexture* texture;
     irr::core::dimension2d<irr::s32> sizeTex;
} MenueGraphicPart;

typedef struct {
    //integer index for type of action
    irr::u32 actionNr;

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
} MenueAction;

struct MenuePage; //Forward declaration

//this struct holds the information about a single
//menue entry
typedef struct {
    irr::u8 entryNumber;
    irr::core::vector2d<irr::s32> drawTextScrPosition;
    char* entryText;

    //if true this menue item can be selected by using
    //cursor key up/down; If false the item is static
    //and can not be used/changed
    bool itemSelectable = true;

    //the next items describe what does happen when we
    //press Return on this menue item
    //MenuePage nextMenuePage allows to change to
    //another menue page, if this is not wanted simply set
    //this pointer to NULL
    MenuePage* nextMenuePage = NULL;

    //the next item describes if the menue item
    //can trigger an action (and which kind of action)
    //if the user selects the item and presses Return key
    //if there is no action then this pointer is set to NULL
    MenueAction* triggerAction = NULL;

    //is for example used for player name entry
    //dialog
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
    irr::u8 checkBoxNrBlocks;
} MenueSingleEntry;

//this struct collects information about one
//possible page of the menue (summary of multiple
//MenueSingleEntries)
typedef struct MenuePage {
    irr::u8 pageNumber;
    std::vector<MenueSingleEntry*> pageEntryVec;

    //is used when program needs to return to menue page
    //one level above, for example when player presses ESC key
    //one level; or when text input is done in one player
    MenuePage* parentMenuePage;
} MenuePage;

typedef struct {
    std::vector<irr::core::recti> coordVec;
} MenueWindowAnimationVec;

struct ShipStatLabel {
   char* text;
   irr::core::vector2di drawPositionTxt;
   bool visible = true;

   irr::core::recti statBoxOutline;
   irr::u8 statNrBlocks;
};

struct MenueTextLabel {
    char* text;
    irr::core::vector2di drawPositionTxt;
    bool visible = true;
};

class Menue {

private:
    irr::video::IVideoDriver* myDriver;
    irr::IrrlichtDevice* myDevice;
    MyEventReceiver* myEventReceiver;
    GameText* myGameTextRenderer;
    SoundEngine* mSoundEngine;
    Assets* mGameAssets;

    irr::video::ITexture* backgnd;
    irr::core::dimension2d<irr::u32> screenResolution;
    std::vector<MenueGraphicPart*> GameLogo;

    //menue window graphic resources
    irr::video::ITexture* wndCornerElementUpperLeftTex;
    irr::video::ITexture* wndCornerElementUpperRightTex;
    irr::video::ITexture* wndCornerElementLowerLeftTex;
    irr::video::ITexture* wndCornerElementLowerRightTex;

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

    //all available menue page entries
    std::vector<MenuePage*> menuePageVector;

    bool InitMenueResources();
    void InitMenuePageEntries();
    void RenderWindow(irr::core::recti position);
    void PrintMenueEntries();
    void RenderRaceSelection();

    void AcceptedRaceSetup();

    //renderOnlyNumberBlocks = optional parameter, needed for type writer effect in menue
    void RenderCheckBox(MenueSingleEntry* entry, irr::core::recti position, irr::video::SColor colorRect, irr::video::SColor lineColor,
           irr::u8 nrBlocks, irr::s8 renderOnlyNumberBlocks = -1);

    void RenderCursor(MenueSingleEntry* textEntryField);
    void PlayMenueSound(uint8_t sndResId);
    void StopMenueSound();

    //Menue definitions for main top entry menue
    MenuePage* TopMenuePage;
    MenueSingleEntry* Race;
    MenueSingleEntry* Options;
    MenueSingleEntry* QuitToOS;

    //Menue definitions for Race page
    MenuePage* RaceMenuePage;
    MenueSingleEntry* SelectChampionsship;
    MenueSingleEntry* SelectSingleRace;
    MenueSingleEntry* RaceMenuePageBackToMainMenue;

    //Menue definitions for Championship page
    MenuePage* ChampionshipMenuePage;
    MenueSingleEntry* NewChampionshipEntry;
    MenueSingleEntry* LoadChampionshipEntry;
    MenueSingleEntry* SaveChampionshipEntry;
    MenueSingleEntry* QuitChampionshipEntry;

    //menue definitions for Championship save page
    MenuePage* ChampionshipSaveMenuePage;
    MenueSingleEntry* ChampionshipSaveTxtLabel;
    MenueSingleEntry* ChampionshipSaveSlot1;
    MenueSingleEntry* ChampionshipSaveSlot2;
    MenueSingleEntry* ChampionshipSaveSlot3;
    MenueSingleEntry* ChampionshipSaveSlot4;
    MenueSingleEntry* ChampionshipSaveSlot5;
    MenueSingleEntry* ChampionshipSaveReturnToChampionsShipMenue;

    //menue definitions for Championship load page
    MenuePage* ChampionshipLoadMenuePage;
    MenueSingleEntry* ChampionshipLoadTxtLabel;
    MenueSingleEntry* ChampionshipLoadSlot1;
    MenueSingleEntry* ChampionshipLoadSlot2;
    MenueSingleEntry* ChampionshipLoadSlot3;
    MenueSingleEntry* ChampionshipLoadSlot4;
    MenueSingleEntry* ChampionshipLoadSlot5;
    MenueSingleEntry* ChampionshipLoadReturnToChampionsShipMenue;

    //Menue definitions for option menue
    MenuePage* OptionMenuePage;
    MenueSingleEntry* ChangeName;
    MenueSingleEntry* DetailOptions;
    MenueSingleEntry* SoundOptions;
    MenueSingleEntry* ReinitializeJoystick;
    MenueSingleEntry* ComputerPlayersCheckBox;
    MenueSingleEntry* DifficultyLevel;
    MenueSingleEntry* OptionMenuePageBackToMainMenue;

    //Menue definitions for Name change dialog/menue
    MenuePage* ChangeNamePage;
    MenueSingleEntry* EnterNameLabel;
    MenueSingleEntry* PlayerNameEnterField;

    //Menue definitions for video details
    MenuePage* VideoDetailsPage;
    MenueSingleEntry* EnableVSync;
    MenueSingleEntry* VideoPageBackToOptionsMenue;

    //Menue definitions for sound options
    MenuePage* SoundOptionsPage;
    MenueSingleEntry* MusicVolumeSlider;
    MenueSingleEntry* EffectsVolumeSlider;
    MenueSingleEntry* SoundOptionsBackToOptionsMenue;

    //Race track selection menue page
    MenuePage* RaceTrackSelectionPage;
    MenueSingleEntry* RaceTrackNameTitle;

    //Ship selection menue page
    MenuePage* ShipSelectionPage;
    MenueSingleEntry* ShipNameTitle;

    MenuePage* currSelMenuePage = NULL;
    MenueSingleEntry* currSelMenueSingleEntry = NULL;
    MenueAction* currActionToExecute = NULL;

    irr::u32 MaxMenueEntryNumber(MenuePage* inputPage);
    void SelectNewMenueEntryWithEntryNr(MenuePage* newMenuePage, irr::u32 nextSelEntryNr);
    MenueSingleEntry* GetMenueSingleEntryForEntryNr(MenuePage* newMenuePage, irr::u32 inputEntryNr);
    irr::u32 MenuePageFindFirstSelectableEntryNr(MenuePage* newMenuePage);
    void ReturnToParentMenuePage(MenuePage* currentMenuePage);
    irr::u32 GetNrOfCharactersOnMenuePage(MenuePage* whichMenuePage);
    irr::u32 GetNrofCharactersForMenueItem(MenueSingleEntry* whichMenueEntry);

    void ChangeToRaceSelection(MenueSingleEntry* callerItem);
    void InterruptRaceSelection();
    void HandleInputRaceSelection();
    void PrintMenueEntriesRaceSelection();

    void ChangeToShipSelection();

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

    void UnhideRaceTrackModels();
    void HideRaceTrackModels();
    void UnhideShipModels();
    void HideShipModels();

    //vector which holds a list of preprogrammed window states for
    //a window animation that will start next
    MenueWindowAnimationVec* windowMenueAnimationStartGame; //only active right at game start
                                                            //before main menue opens first time

    MenueWindowAnimationVec* windowMenueAnimationBeforeTrackSelection; //is shown during transition from
                                                                       //main menue to race track selection

    MenueWindowAnimationVec* windowMenueAnimationQuitTrackSelection; //is shown during transition from
                                                                     //race track selection to main menue

    MenueWindowAnimationVec* windowMenueAnimationBetweenRaceAndShipSelection; //is shown during transition from
                                                                              //race track selection to ship selection

    //pointer to a currently active/selected window animation sequence
    MenueWindowAnimationVec* currSelWindowAnimation;

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
    irr::scene::ISceneManager* smgrMenue;

    //additional 3D models necessary for race track selection menue
    //irr::scene::IMeshSceneNode* ModelTrack1;
    //irr::scene::IMeshSceneNode* ModelTrack2;
    //irr::scene::IMeshSceneNode* ModelTrack3;
    //irr::scene::IMeshSceneNode* ModelTrack4;
    //irr::scene::IMeshSceneNode* ModelTrack5;
    //irr::scene::IMeshSceneNode* ModelTrack6;

    //stores the number of race tracks we have available
    //and we get from assets class for selection
    irr::u8 numRaceTracksAvailable;
    std::vector<irr::scene::IMeshSceneNode*> ModelTrackSceneNodeVec;

    /*irr::scene::IMesh* MeshTrack1;
    irr::scene::IMesh* MeshTrack2;
    irr::scene::IMesh* MeshTrack3;
    irr::scene::IMesh* MeshTrack4;
    irr::scene::IMesh* MeshTrack5;
    irr::scene::IMesh* MeshTrack6;*/

    //camera used for the race track/ship selection 3D rendering
    scene::ICameraSceneNode* MenueCamera;

    //stores the number of crafts we have available
    //and we get from assets class for selection
    irr::u8 numCraftsAvailable;
    //additional 3D models necessary for ship selection menue
    //we have a vector of sceneNodes, because each ship
    //is available in different color schemes!
    std::vector<std::vector<irr::scene::IMeshSceneNode*>*> ModelCraftsSceneNodeVec;

    //additional 3D models necessary for ship selection menue
    //we have a vector of sceneNodes, because each ship
    //is available in different color schemes!
    //std::vector<irr::scene::IMeshSceneNode*> ModelShip1;
    //std::vector<irr::scene::IMeshSceneNode*> ModelShip2;
    //std::vector<irr::scene::IMeshSceneNode*> ModelShip3;
    //std::vector<irr::scene::IMeshSceneNode*> ModelShip4;
    //std::vector<irr::scene::IMeshSceneNode*> ModelShip5;
    //std::vector<irr::scene::IMeshSceneNode*> ModelShip6;

    //we have a vector of meshes, because each ship
    //is available in different color schemes!
    //std::vector<irr::scene::IMesh*> MeshShip1;
    //std::vector<irr::scene::IMesh*> MeshShip2;
    //std::vector<irr::scene::IMesh*> MeshShip3;
    //std::vector<irr::scene::IMesh*> MeshShip4;
    //std::vector<irr::scene::IMesh*> MeshShip5;
    //std::vector<irr::scene::IMesh*> MeshShip6;

    //timing variables for 3D model rendering
    irr::f32 last3DModelUpdateAbsTime;
    irr::f32 curr3DModelRotationDeg = 0.0f;
    irr::u8 currSelectedRaceTrack;
    irr::f32 currRaceTrackWheelAngleDeg;
    irr::f32 targetRaceTrackWheelAngleDeg;

    char currSelRaceTrackName[50];
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
    irr::u8 RaceTrackWheelMoving;

    //variable to remembar from which menue item we initially
    //called the race track selection menue so that when the player
    //interrupts we can return to the correct caller menue
    MenueSingleEntry* RaceTrackSelectionCallerMenueEntry;

    MenueTextLabel* SelRaceTrackNrLapsLabel;
    MenueTextLabel* SelRaceTrackBestLapLabel;
    MenueTextLabel* SelRaceTrackBestRaceLabel;

    //0 = wheel not moving
    //1 = wheel moving clock wise
    //2 = wheel moving counter clock wise
    irr::u8 ShipWheelMoving;

    //current selected ship stats
    irr::u8 currSelShipStatSpeed;
    irr::u8 currSelShipStatArmour;
    irr::u8 currSelShipStatWeight;
    irr::u8 currSelShipStatFirePower;

    ShipStatLabel* ShipStatSpeedLabel;
    ShipStatLabel* ShipStatArmourLabel;
    ShipStatLabel* ShipStatWeightLabel;
    ShipStatLabel* ShipStatFirePowerLabel;

    //special images
    irr::video::ITexture* gameTitle;
    irr::core::vector2di gameTitleDrawPos;
    irr::core::dimension2d<irr::u32> gameTitleSize;

    irr::video::ITexture* raceLoadingScr;
    irr::core::vector2di raceLoadingScrDrawPos;
    irr::core::dimension2d<irr::u32> raceLoadingScrSize;

    void RenderImageMenuePage();

    //create a dummy menue page for showing
    //game title screen and race loading screen
    MenuePage* gameTitleMenuePage;
    MenuePage* raceLoadingMenuePage;

    void CalcStatLabelHelper(irr::u8 currStatVal, ShipStatLabel &label, irr::core::vector2di centerCoord);
    void InitStatLabels();
    void RecalculateShipStatLabels();
    void CleanUpStatLabels();

    void CalcMenueTextLabelHelper(MenueTextLabel &label, irr::core::vector2di centerCoord);
    void RecalculateRaceTrackStatLabels();
    void UpdateRaceTrackSelectionTypeWriterEffect();

    void InitRaceTrackSceneNodes();
    void Update3DModels(irr::f32 frameDeltaTime);
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

public:
    //if you do not want any Menue Sounds just put NULL pointer into soundEngine
    Menue(irr::IrrlichtDevice* device, irr::video::IVideoDriver* driver, irr::core::dimension2d<irr::u32> screenRes, GameText* textRenderer,
          MyEventReceiver* eventReceiver, irr::scene::ISceneManager* mainSceneManager, SoundEngine* soundEngine, Assets* assets);
    ~Menue();

    bool MenueInitializationSuccess;

    MenueAction* ActRace;
    MenueAction* ActQuitToOS;

    MenueAction* ActSetComputerPlayerEnable;
    MenueAction* ActSetDifficultyLevel;

    MenueAction* ActSetPlayerName;
    MenueAction* ActSetMusicVolume;
    MenueAction* ActSetSoundVolume;

    void Render();
    void HandleInput();
    bool HandleActions(MenueAction* &pendingAction);
    void AdvanceTime(irr::f32 frameDeltaTime);

    void ShowGameTitle();
    void ShowMainMenue();
};

#endif // MENUE_H
