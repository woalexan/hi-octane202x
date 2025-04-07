/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef INTROPLAYER_H
#define INTROPLAYER_H

#include "irrlicht.h"
#include <vector>
#include "../audio/music.h"
#include "../audio/sound.h"
#include "../infrabase.h"

//this struct holds the information for a sound trigger event
//used during the games intro playing
typedef struct IntroSoundTriggerStruct {
    //absolute time of intro playing progress
    //when to trigger this sound
    irr::f32 triggerAbsTime;

    //sound resource Id to trigger
    uint8_t soundResId;

    //if true looping sound is currently
    //playing
    bool loopSoundActive = false;

    //if true sound will loop
    bool looping;

    //absolute time of intro playing
    //to end looping sound again
    irr::f32 endLoopingAbsTime;

    //needed for looping sounds
    sf::Sound* soundBufPntr = NULL;
} IntroSoundTriggerStruct;

class SoundEngine; //Forward declaration
class MyMusicStream; //forward declaration

class IntroPlayer {

 public:
    IntroPlayer(InfrastructureBase* infra, SoundEngine* soundEngine, MyMusicStream* gameMusicPlayerParam);
    ~IntroPlayer();

    bool Init();

    void Play();

    void RenderIntro(irr::f32 frameDeltaTime);
    void HandleInput();

    bool introFinished = false;
    bool introPlaying = false;

 private:
    bool mInitOk = false;

    //we need a music player for the game intro music
    MyMusicStream* mMusicPlayer;
    SoundEngine* mSoundEngine;
    InfrastructureBase* mInfra;

    //stuff for game intro playing
    std::vector<irr::video::ITexture*>* introTextures;
    irr::u32 currIntroFrame;
    irr::u32 numIntroFrame;
    irr::core::vector2di introFrameScrDrawPos;
    irr::core::dimension2d<irr::u32> introFrameScrSize;

    //absolute time for intro rendering
    irr::f32 introTargetTimeBetweenFramesSec;
    irr::f32 introCurrTimeBetweenFramesSec;
    irr::f32 currFrameTimeErrorSec;
    irr::f32 introAbsTimeSound;

    //sound stuff for intro playing
    std::vector<IntroSoundTriggerStruct*>* introSoundEventVec;
    irr::u8 currIdxSoundEventVec;
    irr::u8 numSoundEvents;

    void AddIntroSoundTrigger(irr::f32 absTriggerTime, uint8_t soundIdNr,
                              bool looping = false,  irr::f32 endLoopingTime = 0.0f);
    void IntroProcessLoopingSounds(irr::f32 currSoundPlayingTime);

    void CleanupIntro();

};

#endif // INTROPLAYER_H
