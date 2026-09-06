/*
 Copyright (C) 2024-2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef HUD_H
#define HUD_H

#include "irrlicht.h"
#include <vector>
#include "../audio/sound.h"

#define WaitTimeBeforeNextBannerState 0.1f  //in seconds
#define DEF_HUD_BANNERTEXT_MINSHOWTIME 1.0f //in seconds
#define DEF_HUD_TARGETNAME_BLINKPERIODE 0.5f //in seconds

//blink period time for big green Hud text
#define DEF_HUD_BIGGREENTEXT_BLINKPERIODTIME 0.5f //in seconds

//blink period time for blinking hud bars
#define DEF_HUD_BARS_BLINKPERIODE 0.25f //in seconds

#define DEF_HUD_STATE_NOTDRAWN 0
#define DEF_HUD_STATE_STARTSIGNAL 1
#define DEF_HUD_STATE_RACE 2
#define DEF_HUD_STATE_BROKENPLAYER 3

#define DEF_HUD_STARTSIGNAL_NOTHING_LIT 0
#define DEF_HUD_STARTSIGNAL_RED_LIT 1
#define DEF_HUD_STARTSIGNAL_YELLOW_LIT 2
#define DEF_HUD_STARTSIGNAL_GREEN_LIT 3

//Threshold values taken from the original game
//I added -1 at the end myself that I know when the list is over :)
const int16_t FuelBarThresholds[18] = {     0, 0x180, 0x300,  0x480,  0x600,  0x780,  0x900,  0xA80,
                                        0xC00, 0xD80, 0xF00, 0x1080, 0x1200, 0x1500, 0x1980, 0x1E00,
                                       0x2280, -1};

const int16_t VelocityBarThresholds[17] = { 0, 0x180, 0x300,  0x480,  0x600,  0x780,  0x900,  0xA80,
                                        0xC00, 0xD80, 0xF00, 0x1200, 0x1500, 0x1980, 0x1E00, 0x2280,
                                           -1};

const int16_t BoosterBarThresholds[20] = {  0,  0x20E,  0x41C,  0x62A,  0x838,  0xA46,  0xC54,  0xE62,
                                       0x1070, 0x127E, 0x148C, 0x169A, 0x18A8, 0x1AB6, 0x1CC4, 0x1ED2,
                                       0x20E0, 0x22EE, 0x24FC,     -1};

const int16_t ThrottleBarThresholds[20] = {  0,  0x20E,  0x41C,  0x62A,  0x838,  0xA46,  0xC54,  0xE62,
                                       0x1070, 0x127E, 0x148C, 0x169A, 0x18A8, 0x1AB6, 0x1CC4, 0x1ED2,
                                       0x20E0, 0x22EE, 0x24FC,     -1};

const int16_t ShieldBarThresholds[7] = { 0,  0x682,  0xD04,  0x1386,  0x1A08,  0x208A,  -1};

const int16_t AmmoBarThresholds[7] = { 0,  0x682,  0xD04,  0x1386,  0x1A08,  0x208A,  -1};

const int16_t HeatBarThresholds[17] = { 0,  0x271,  0x4E2,  0x753,  0x9C4,  0xC35, 0xEA6,
                                       0x1117, 0x1388, 0x15F9, 0x186A, 0x1ADB, 0x1D4C, 0x1FBD,
                                       0x222E, 0x249F, -1};

struct HudDisplayPart{
     irr::core::vector2d<irr::s32> drawScrPosition;
     irr::video::ITexture* texture = nullptr;
     irr::video::ITexture* altTexture = nullptr;
     irr::core::dimension2d<irr::s32> sizeTex;
     irr::core::rect<irr::s32> sourceRect;
};

class VVehicle; //Forward declaration
class Game; //Forward declaration
class Race; //Forward declaration

struct BannerTextMessageStruct {
    char* text = nullptr;
    bool isWarning;
    bool permanentMsg;
    irr::core::vector2di textPosition;
    irr::f32 textStillShownSec;
    irr::f32 textAlreadyShownSec;
};

class HUD {

private:
    Game* mGame = nullptr;
    Race* mRace = nullptr;

    //a negative altPanelTexNr input value means no alternative texture (image) is used
    void Add1PlayerHudDisplayPart(std::vector<HudDisplayPart*>* addToWhichBar,
                                       irr::u16 panelTexNr, irr::u16 drawPosX,
                                       irr::u16 drawPosY, irr::s16 altPanelTexNr = -1);

    void RenderTextBannerGraphics();
    void RenderPlayerLapTimes();
    void InitShieldBar();
    void InitAmmoBar();
    void InitGasolineBar();
    void InitThrottleBar();
    void InitSpeedBar();
    void InitMGHeatBar();
    void InitRacePosition();
    void InitUpgradeBar();
    void InitHudBannerText();
    void InitStartSignal();
    void InitBrokenGlas();

    //******************************
    // Variables start for Race HUD
    //******************************

    VVehicle* monitorWhichPlayer = nullptr;

    std::vector<HudDisplayPart*>* shieldBar = nullptr;
    std::vector<HudDisplayPart*>* ammoBar = nullptr;
    std::vector<HudDisplayPart*>* gasolineBar = nullptr;
    std::vector<HudDisplayPart*>* throttleBar = nullptr;
    std::vector<HudDisplayPart*>* speedBar = nullptr;
    std::vector<HudDisplayPart*>* currRacePlayerPosition = nullptr;
    std::vector<HudDisplayPart*>* numberPlayers = nullptr;
    std::vector<HudDisplayPart*>* mgHeatBar = nullptr;

    std::vector<HudDisplayPart*>* upgradeBar = nullptr;

    std::vector<HudDisplayPart*>* startSignal = nullptr;

    //internal variables for text banner on lower part of screen
    //shows information during gameplay
    std::vector<HudDisplayPart*>* bannerTextState1 = nullptr;
    std::vector<HudDisplayPart*>* bannerTextState2 = nullptr;
    std::vector<HudDisplayPart*>* bannerTextState3 = nullptr;
    std::vector<HudDisplayPart*>* bannerTextState4 = nullptr;
    std::vector<HudDisplayPart*>* bannerTextState5 = nullptr;
    std::vector<HudDisplayPart*>* bannerTextState6 = nullptr;
    std::vector<HudDisplayPart*>* bannerTextState7 = nullptr;

    //************** Text Banner start ******//
    int currentBannerTextState; //0 means no banner visible, 1 means step 1 of deployment
    //2 means next step of deployment and so on..., 7 means banner text is shown

    irr::f32 timerNextBannerState;
    bool bannerDoesCurrentlyOpen;
    bool bannerDoesCurrentlyClose;
    bool mCurrentMessageWarningActive = false;

    std::vector<BannerTextMessageStruct*>* bannerMessageVec = nullptr;

    //preset with nullptr, important!
    BannerTextMessageStruct* currShownBannerMsg = nullptr;

    void BannerTextLogic(irr::f32 deltaTime);
    void CleanUpBannerMessage(BannerTextMessageStruct* msgToDeletePntr);
    void CleanUpAllBannerMessages();

    //returns true if there is a next banner message
    //available that can be shown, false otherwise
    bool SelectNextBannerMessage();

    void RemoveAllPermanentBannerMessagesMsgVec();

    //************** Text Banner end ******//

    //************** Big green text start ******//

    //preset with nullptr, important!
    char* currentBigGreenText = nullptr;

    irr::f32 currentBigGreenTextStillShownSec;
    bool blinkBigGreenText;
    bool permanentBigGreenText;
    irr::core::vector2di currentBigGreenTextDrawPosition;
    irr::u8 currentBigGreenTextState = 0;
    irr::f32 currentBigGreenTextBlinkPeriod;

    //************** Big green text end ******//

    void CleanUpHudDisplayPartVector(std::vector<HudDisplayPart*> &pntrVector);

    //************************************
    //* Player target resources          *
    //************************************

    HudDisplayPart *targetSymbol = nullptr;
    HudDisplayPart *targetArrowLeft = nullptr;
    HudDisplayPart *targetArrowAbove = nullptr;
    HudDisplayPart *targetArrowBelow = nullptr;
    HudDisplayPart *targetArrowRight = nullptr;
    void InitTargetStuff();
    void RenderTargetSymbol(irr::f32 deltaTime);
    irr::core::position2d<irr::s32> getScreenCoordinatesFrom3DPosition(
        const irr::core::vector3df & pos3d, irr::scene::ICameraSceneNode* camera);

    irr::f32 targetNameBlinkTimer = 0.0f;
    bool currShowTargetName = false;

    //**********************************
    // Variables start for start signal
    //**********************************

    //0.. means no light lit
    //with increasing value the start signal
    //advances towards the final state
    irr::u8 mStartSignalState = 0;

    void DrawHUD1PlayerRace(irr::f32 deltaTime);
    void DrawHUD1PlayerStartSignal(irr::f32 deltaTime);
    void DrawHUD1PlayerBrokenPlayer(irr::f32 deltaTime);

    //************************************
    //* Render list of finished players  *
    //************************************

    void DrawFinishedPlayerList();
    void DrawGasolineBar();
    int GetNumberCurrentFuelBars(int16_t fuelVal);
    int GetNumberCurrentVelocityBars(irr::f32 velocityVal);
    int GetNumberCurrentBoosterBars(int16_t boosterTriggerTime);
    int GetNumberCurrentShieldBars(int16_t healthVal);
    int GetNumberCurrentAmmoBars(int16_t ammoVal);
    int GetNumberCurrentThrottleBars(irr::f32 movementSpeed);
    int GetNumberCurrentMGunHeatBars(int16_t triggerTime);
    void DrawAmmoBar();
    void DrawShieldBar();

    void RenderBigGreenText(irr::f32 deltaTime);

    bool mDrawAmmoBarTransparent = false;
    bool mDrawGasolineBarTransparent = false;
    bool mDrawShieldBarTransparent = false;

    irr::f32 blinkBarTimer = 0.0f;
    bool mDrawBarsTransparent = false;

    irr::video::SColor* mColorSolid = nullptr;
    irr::video::SColor* mColorTransparent = nullptr;

    irr::video::SColor* mColorTargetSymbolHealthBar = nullptr;

    //Stores some precalculated positions
    irr::core::vector2di mHudDrawPositionOffset;

    irr::core::vector2d<irr::s32> mPosLapSymbol;
    irr::core::vector2d<irr::s32> mPosLapCount;
    irr::core::vector2d<irr::s32> mPosRedSkull;
    irr::core::vector2d<irr::s32> mPosFragCnt;

    void PrecalculatePositions();

    sf::Sound* mMissileCloseSoundSource = nullptr;

public:
    HUD(Game* game, Race* parentRace);
    ~HUD();

    void SetMonitorWhichPlayer(VVehicle* newPlayer);
    void DrawHUD1(irr::f32 deltaTime);

    //if showDurationSec is negative a permanent banner text message is created instead of the
    //default time limited one
    void ShowBannerText(char* text, irr::f32 showDurationSec, bool warningSoundActive = false);
    void CancelAllPermanentBannerTextMsg();

    //if showDurationSec is negative, the text will be shown until it is deleted
    //with a call to function RemoveGreenBigText
    //if blinking is true text will blink (for example used for final lap text), If false
    //text does not blink (as used when player died and waits for repair craft)
    void ShowGreenBigText(char* text, irr::f32 showDurationSec, bool blinking);

    //RemovePermanentGreenBigText does only remove a permanent green text,
    //not one with a specified duration
    void RemovePermanentGreenBigText();

    //returns true if HUD currently shows a permanent big green text
    bool DoesHudShowPermanentGreenBigText();

    //0.. means no light lit
    //with increasing value the start signal
    //advances towards the final state
    void SetStartSignalState(irr::u8 newSignalState);
    irr::u8 GetStartSignalState();

    //************************************
    //* Broken glas resource             *
    //************************************

    HudDisplayPart* brokenGlas = nullptr;
};

#endif // HUD_H
