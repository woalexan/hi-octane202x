/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "introplayer.h"

IntroPlayer::IntroPlayer(InfrastructureBase* infra, SoundEngine* soundEngine, MyMusicStream* gameMusicPlayerParam) {
    mInfra = infra;
    mSoundEngine = soundEngine;
    mMusicPlayer = gameMusicPlayerParam;
}

void IntroPlayer::HandleInput() {
    //allow player to interrupt the intro with ESC
    if (mInfra->mEventReceiver->IsKeyDownSingleEvent(irr::KEY_ESCAPE)) {
       //interrupt the intro
       //we just need to set the current frame number
       //to the number of the last frame
       currIntroFrame = numIntroFrame;
    }
}

void IntroPlayer::RenderIntro(irr::f32 frameDeltaTime) {
    //render the next frame
    if (introPlaying && (currIntroFrame >= numIntroFrame)) {
        //start again from the first frame, for example for debugging
        //currIntroFrame = 0;

        //stop the game intro music again
        if ((mMusicPlayer->getStatus() == mMusicPlayer->Playing) ||
           (mMusicPlayer->getStatus() == mMusicPlayer->Paused)) {
                //stop music
                mMusicPlayer->StopPlay();
        }

        //make sure to also stops all sounds
        if (mSoundEngine->IsAnySoundPlaying()) {
            mSoundEngine->StopAllSounds();
        }

        //playing intro finished => tell main loop
        introFinished = true;
        introPlaying = false;

        return;
    }

    //first draw a black rectangle over the whole screen to make sure that the parts of the
    //screen that are outside of the intro frames region are black as well
    mInfra->mDriver->draw2DRectangle(irr::video::SColor(255,0,0,0),
                   irr::core::rect<irr::s32>(0, 0, mInfra->mScreenRes.Width, mInfra->mScreenRes.Height));

    if (introPlaying) {
        //draw the current intro frame
        mInfra->mDriver->draw2DImage(this->introTextures->at(currIntroFrame), introFrameScrDrawPos,
                          irr::core::recti(0, 0, introFrameScrSize.Width, introFrameScrSize.Height)
                            , 0, irr::video::SColor(255,255,255,255), true);

        if (currIntroFrame > 0) {
            introCurrTimeBetweenFramesSec += frameDeltaTime;

            //lets add 1 second to align video and audio of intro better :)
            introAbsTimeSound = introTargetTimeBetweenFramesSec * currIntroFrame + 1.0f;
        } else {
            introCurrTimeBetweenFramesSec = 0.0f;
            introAbsTimeSound = 0.0f;
            currIntroFrame = 1;
        }

        //time to draw the next frame?
        if ((introTargetTimeBetweenFramesSec - introCurrTimeBetweenFramesSec - currFrameTimeErrorSec) < 0.0f)  {
            //next time render the next frame
            currIntroFrame++;

            //add up overall time error
            currFrameTimeErrorSec += (introCurrTimeBetweenFramesSec - introTargetTimeBetweenFramesSec);

            //std::cout << (introCurrTimeBetweenFramesSec * 1000.0f) << " ms" << std::endl << std::flush;

            introCurrTimeBetweenFramesSec =  0.0f;
        }

        //more sound events to trigger?
        if (currIdxSoundEventVec < numSoundEvents) {
            IntroSoundTriggerStruct* pntr = introSoundEventVec->at(currIdxSoundEventVec);

            //time to trigger the next sound event?
            if (introAbsTimeSound >= pntr->triggerAbsTime) {
                //looping sound?
                if (!pntr->looping) {
                    mSoundEngine->PlaySound(pntr->soundResId);
                } else {
                    //is a looping sound
                    //we need the sound buffer pointer to be able to
                    //stop the sound later
                    pntr->soundBufPntr =
                            mSoundEngine->PlaySound(pntr->soundResId, true);
                    pntr->loopSoundActive = true;
                }

                //is there one additional intro sound trigger event available
                //if so set index to next event
                if (currIdxSoundEventVec < numSoundEvents) {
                    currIdxSoundEventVec++;
                }
            }
        }

        //make sure to shop looping sounds that are
        //over at this point in time
        IntroProcessLoopingSounds(introAbsTimeSound);
    }
}

IntroPlayer::~IntroPlayer() {
    //delete intro data
    CleanupIntro();
}

bool IntroPlayer::Init() {
    //first we need to load the additional intro
    //sound files
    if (!mSoundEngine->LoadSoundResourcesIntro()) {
        return false;
    }

    //first we need to load all textures (each frame is a texture) for the game intro
    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

    char frameFileName[50];
    char fname[20];

    introTextures = new std::vector<irr::video::ITexture*>();
    introTextures->clear();

    irr::video::ITexture* newTex;

    for (long frameNr = 0; frameNr < 548; frameNr++) {
        //first build the name for the image file to load
        //that we have prepared during the first start of the game
        strcpy(frameFileName, "extract/intro/frame");
        sprintf (fname, "%0*lu.png", 4, frameNr);
        strcat(frameFileName, fname);

        //load this image (texture)
        newTex = mInfra->mDriver->getTexture(frameFileName);
        if (newTex == NULL) {
            //there was a texture loading error
            std::cout << "Intro image loading error" << std::endl;

            return false;
        }

        introTextures->push_back(newTex);
    }

    mInfra->mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

    //get size of texture from first frame
    introFrameScrSize = introTextures->at(0)->getSize();

    //calculate position to draw intro frames so that the are centered on the screen
    //because maybe target resolution does not fit with image resolution
    introFrameScrDrawPos.X = (mInfra->mScreenRes.Width - introFrameScrSize.Width) / 2;
    introFrameScrDrawPos.Y = (mInfra->mScreenRes.Height - introFrameScrSize.Height) / 2;

    //define sounds for intro
    introSoundEventVec = new std::vector<IntroSoundTriggerStruct*>();
    introSoundEventVec->clear();

    //make sure to add the sound trigger events of the intro
    //in increasing time order! otherwise this want work
    AddIntroSoundTrigger(0.0f, SRES_INTRO_FIRE, true, 8.15f);
    AddIntroSoundTrigger(4.7f, SRES_INTRO_PAST);
    AddIntroSoundTrigger(5.7f, SRES_INTRO_PAST);
    AddIntroSoundTrigger(7.1f, SRES_INTRO_SMALLCAR, true, 10.32f);
    AddIntroSoundTrigger(8.2f, SRES_INTRO_MINIGUN, true, 9.24f);
    AddIntroSoundTrigger(8.2f, SRES_INTRO_RICCOS);
    AddIntroSoundTrigger(9.8f, SRES_INTRO_TURBO);
    AddIntroSoundTrigger(9.96f, SRES_INTRO_BOOSTER);
    AddIntroSoundTrigger(12.1f, SRES_INTRO_SMALLCAR, true, 14.4f);
    AddIntroSoundTrigger(12.8f, SRES_INTRO_MISSILE);
    AddIntroSoundTrigger(13.1f, SRES_INTRO_EXPLODE);
    AddIntroSoundTrigger(15.395f, SRES_INTRO_PAST);
    AddIntroSoundTrigger(17.32f, SRES_INTRO_SMALLCAR, true, 20.4f);
    AddIntroSoundTrigger(19.175f, SRES_INTRO_TURBO);
    AddIntroSoundTrigger(19.33f, SRES_INTRO_BOOSTER);
    AddIntroSoundTrigger(20.4f, SRES_INTRO_PAST);
    AddIntroSoundTrigger(22.2f, SRES_INTRO_SCRAPE2);
    AddIntroSoundTrigger(26.0f, SRES_INTRO_EXPLODE);
    AddIntroSoundTrigger(26.3f, SRES_INTRO_PAST);
    AddIntroSoundTrigger(30.1f, SRES_INTRO_FIRE, true, 34.8f);
    AddIntroSoundTrigger(33.6f, SRES_INTRO_TURBO);
    AddIntroSoundTrigger(34.76f, SRES_INTRO_BOOSTER);
    AddIntroSoundTrigger(35.8f, SRES_INTRO_PAST);
    AddIntroSoundTrigger(38.8f, SRES_INTRO_CURTAIN);

    //load the intro music
    if (!mMusicPlayer->loadGameMusicFile((char*)"extract/music/TINTRO2.XMI")) {
         std::cout << "Music load failed" << std::endl;
         return false;
    }

    mInitOk = true;

    //all was ok
    return true;
}

void IntroPlayer::Play() {
    if (mInitOk != true)
        return;

   currIntroFrame = 0;
   numIntroFrame = 548;

   //start with the first sound event element at idx 0
   currIdxSoundEventVec = 0;
   numSoundEvents = (irr::u8)(this->introSoundEventVec->size());

   //the speed field in the game intro FLI file had the value 5
   //this means we need to draw each 5 * (1/70) seconds a new frame => 71.4285714285714 ms
   introTargetTimeBetweenFramesSec = 5.0f * (1.0f / 70.0f);

   currFrameTimeErrorSec = 0.0f;
   introCurrTimeBetweenFramesSec = 0.0f;

   //start music playing at the same when
   //we start playing the video
   mMusicPlayer->StartPlay();

   introPlaying = true;
   introFinished = false;
}

void IntroPlayer::IntroProcessLoopingSounds(irr::f32 currSoundPlayingTime) {
  std::vector<IntroSoundTriggerStruct*>::iterator it;

  for (it = this->introSoundEventVec->begin(); it != this->introSoundEventVec->end(); ++it) {
      if ((*it)->loopSoundActive) {
          //is it time to stop the looping sound
          if (currSoundPlayingTime >= (*it)->endLoopingAbsTime) {
              (*it)->soundBufPntr->stop();
              (*it)->loopSoundActive = false;
          }
      }
  }
}

void IntroPlayer::AddIntroSoundTrigger(irr::f32 absTriggerTime, uint8_t soundIdNr, bool looping,
                                 irr::f32 endLoopingTime) {
   IntroSoundTriggerStruct* newTrigger = new IntroSoundTriggerStruct();

   newTrigger->triggerAbsTime = absTriggerTime;
   newTrigger->soundResId = soundIdNr;
   newTrigger->looping = looping;
   newTrigger->endLoopingAbsTime = endLoopingTime;

   //add to vector of sound trigger events for the intro
   this->introSoundEventVec->push_back(newTrigger);
}

//This function cleans up the 548 textures we had to load for the intro
//Important after the intro!
void IntroPlayer::CleanupIntro() {
   std::vector<irr::video::ITexture*>::iterator it;

   //free each of the textures we loaded before
   for (it = this->introTextures->begin(); it != this->introTextures->end(); ++it) {
        //remove underlying texture
        mInfra->mDriver->removeTexture((*it));
   }

   //free vector at the end
   delete introTextures;

   //unload additional sound resources
   mSoundEngine->UnLoadSoundResourcesIntro();
}

