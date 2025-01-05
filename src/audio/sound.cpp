/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "sound.h"
#include <iostream>

bool SoundEngine::getInitOk() {
    return mInitOk;
}

void SoundEngine::StopAllSounds() {
    if (this->SoundVec->size() > 0) {
        std::vector<sf::Sound*>::iterator it;
        enum sf::SoundSource::Status stat;
        for (it = SoundVec->begin(); it != SoundVec->end(); ++it) {
            stat = (*it)->getStatus();
            if ((stat == (*it)->Playing) || (stat == (*it)->Paused)) {
                (*it)->stop();
            }
        }
    }

    StopEngineSound();
}

bool SoundEngine::IsAnySoundPlaying() {
    if (this->SoundVec->size() > 0) {
        std::vector<sf::Sound*>::iterator it;
        enum sf::SoundSource::Status stat;
        for (it = SoundVec->begin(); it != SoundVec->end(); ++it) {
            stat = (*it)->getStatus();
            if (stat == (*it)->Playing)
                return true;
            }
        }

    if (engineSound != NULL) {
        if (engineSound->getStatus() == (engineSound->Playing))
            return true;
    }

   return false;
}

void SoundEngine::StartEngineSound() {
    SoundResEntry* pntr = SearchSndRes(SRES_GAME_LARGECAR);

    engineSound = new sf::Sound();

    engineSound->setVolume(mSoundVolume);
    engineSound->setBuffer(*pntr->pntrSoundBuf);
    engineSound->setLoop(true);

    if (mSoundVolume > 1.0f) {
        engineSound->play();
    }
}

void SoundEngine::SetPlayerSpeed(float speed, float maxSpeed) {
    this->playerSpeed = speed;
    this->playerMaxSpeed = maxSpeed;

    engineSound->setPitch(float(0.6) + (speed/maxSpeed) * float(0.8));
}

void SoundEngine::StopEngineSound() {
    if (engineSound != NULL) {
        this->engineSound->stop();
    }
}

SoundEngine::SoundEngine() {
    //create new vector where we can store our soundbuffers/soundresources
    SoundResVec = new std::vector<SoundResEntry*>();

    //create new vector where we can store sound sources
    SoundVec = new std::vector<sf::Sound*>();

    mNrSoundSources = 0;

    engineSound = NULL;

    //load all sound resource files
    LoadSoundResources();
}

//searches for a sound resource entry with a certain specified sound ID
//if no sound under this sound ID is found returns NULL
SoundResEntry* SoundEngine::SearchSndRes(uint8_t soundResId) {
    if (this->SoundResVec->size() < 1)
        return NULL;

    std::vector<SoundResEntry*>::iterator it;

    for (it = this->SoundResVec->begin(); it != this->SoundResVec->end(); ++it) {
        if ((*it)->soundResId == soundResId) {
            //found the correct sound resource entry
            return (*it);
        }
    }

    //did not find correct entry
    //return NULL
    return NULL;
}

sf::Sound* SoundEngine::GetFreeSoundSource() {
    //if we have already sources available try to
    //reuse an already existing inactive source
    if (this->SoundVec->size() > 0) {
        std::vector<sf::Sound*>::iterator it;
        for (it = this->SoundVec->begin(); it != this->SoundVec->end(); ++it) {
           if ((*it)->getStatus() == (*it)->Stopped) {
               //found useful source
               return (*it);
           }
        }
    }

    //we did not find a currently
    //useful sound source, maybe all currently
    //playing
    //can we create a new one?
    if (this->mNrSoundSources < SOUND_MAXNR) {
        sf::Sound *newSnd = new sf::Sound();

        //also set current sound volue
        newSnd->setVolume(mSoundVolume);

        this->SoundVec->push_back(newSnd);
        this->mNrSoundSources++;

        return newSnd;
    }

    //already too much sounds, do not do
    //anything
    return NULL;
}

//sets a new sound volume for all available sound
//sources, 0 = sounds off, 100 = max volume
void SoundEngine::SetVolume(float soundVolume) {
    if (soundVolume < 1.0f) {
        mSoundVolume = 0.0f;
        mPlaySound = false;
        StopEngineSound();
    } else {
        mPlaySound = true;
        mSoundVolume = soundVolume;
        if (engineSound != NULL) {
            engineSound->setVolume(soundVolume);
        }

        //updating volume of existing sound sources
        if (this->SoundVec->size() > 0) {
            std::vector<sf::Sound*>::iterator it;
            for (it = this->SoundVec->begin(); it != this->SoundVec->end(); ++it) {
               (*it)->setVolume(soundVolume);
            }
        }
    }
}

bool SoundEngine::GetIsSoundActive() {
    return mPlaySound;
}

sf::Sound* SoundEngine::PlaySound(uint8_t soundResId, bool looping) {
    //if we should not play sounds exit
    if (!mPlaySound)
        return NULL;

    //first search resource
    SoundResEntry* pntr = SearchSndRes(soundResId);

    if (pntr != NULL) {
        //if found the sound resource to play
            sf::Sound* sndPntr = GetFreeSoundSource();
            if (sndPntr != NULL) {
                //we found a free sound source to play buffer
                sndPntr->setBuffer(*pntr->pntrSoundBuf);
                sndPntr->setLoop(looping);
                sndPntr->play();

                //return sound source we have used
                //(is important to be able to stop
                //looping sound afterwards)
                return sndPntr;
            }
    }

    return NULL;
}

void SoundEngine::StopLoopingSound(sf::Sound *pntrSound) {
    if (pntrSound != NULL) {
        if (pntrSound->getStatus() == pntrSound->Playing) {
            pntrSound->stop();
        }
    }
}

//Loads the available sound resources for the game
void SoundEngine::LoadSoundResources() {
    mInitOk = true;

    //load all the sound resource files we need for this game
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_MENUE_TYPEWRITEREFFECT1, SRES_MENUE_TYPEWRITEREFFECT1);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_MENUE_TYPEWRITEREFFECT2, SRES_MENUE_TYPEWRITEREFFECT2);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_MENUE_SELECTOTHERITEM, SRES_MENUE_SELECTOTHERITEM);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_MENUE_CHANGECHECKBOXVAL, SRES_MENUE_CHANGECHECKBOXVAL);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_MENUE_WINDOWMOVEMENT, SRES_MENUE_WINDOWMOVEMENT);

    mInitOk &= mInitOk && LoadSoundResouce(SFILE_GAME_PICKUP, SRES_GAME_PICKUP);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_GAME_REFUEL, SRES_GAME_REFUEL);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_GAME_LARGECAR, SRES_GAME_LARGECAR);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_GAME_WARNING, SRES_GAME_WARNING);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_GAME_TURBO, SRES_GAME_TURBO);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_GAME_BOOSTER, SRES_GAME_BOOSTER);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_GAME_COLLIDED, SRES_GAME_COLLISION);

    mInitOk &= mInitOk && LoadSoundResouce(SFILE_GAME_MGUN_SINGLESHOT, SRES_GAME_MGUN_SINGLESHOT);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_GAME_MGUN_SHOTFAILED, SRES_GAME_MGUN_SHOTFAILED);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_GAME_MGUN_LONGSHOT, SRES_GAME_MGUN_LONGSHOT);

    mInitOk &= mInitOk && LoadSoundResouce(SFILE_GAME_MISSILE_SHOT, SRES_GAME_MISSILE_SHOT);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_GAME_EXPLODE, SRES_GAME_EXPLODE);

    mInitOk &= mInitOk && LoadSoundResouce(SFILE_GAME_FINALLAP, SRES_GAME_FINALLAP);

    //load all the intro sounds as well
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_INTRO_FIRE, SRES_INTRO_FIRE);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_INTRO_EXPLODE, SRES_INTRO_EXPLODE);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_INTRO_TURBO, SRES_INTRO_TURBO);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_INTRO_SMALLCAR, SRES_INTRO_SMALLCAR);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_INTRO_SCRAPE2, SRES_INTRO_SCRAPE2);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_INTRO_RICCOS, SRES_INTRO_RICCOS);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_INTRO_PAST, SRES_INTRO_PAST);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_INTRO_MISSILE, SRES_INTRO_MISSILE);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_INTRO_MINIGUN, SRES_INTRO_MINIGUN);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_INTRO_HELEHIT, SRES_INTRO_HELEHIT);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_INTRO_FIREPAST, SRES_INTRO_FIREPAST);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_INTRO_CURTAIN, SRES_INTRO_CURTAIN);
    mInitOk &= mInitOk && LoadSoundResouce(SFILE_INTRO_BOOSTER, SRES_INTRO_BOOSTER);
}

//Loads a single specified sound resource
bool SoundEngine::LoadSoundResouce(char *fileName, uint8_t soundResId) {
    //first make sure the specified new sound resource ID is not
    //existing yet
    if (SearchSndRes(soundResId) != NULL) {
        //is already existing, output error
        std::cout << "LoadSoundResouce: Sound resource with Id " << soundResId << " already existing. Error!" << std::endl;
        return false;
    }

    //we do not have this resouce ID yet => all ok
    sf::SoundBuffer* newBuf = new sf::SoundBuffer();
    SoundResEntry* newRes = new SoundResEntry();
    newRes->soundResId = soundResId;
    newRes->pntrSoundBuf = newBuf;
    std::string fileNameStr(fileName);

    if ((newRes != NULL) && (newBuf != NULL)) {
        if (!newRes->pntrSoundBuf->loadFromFile(fileNameStr)) {
                std::cout << "LoadSoundResouce: Could not load sound resource file " << fileName << std::endl;
                return false;
        }
    } else {
        std::cout << "LoadSoundResouce: Could not load sound resource file " << fileName << std::endl;
        return false;
    }

    //add new resource to vector
    this->SoundResVec->push_back(newRes);
    std::cout << "LoadSoundResouce: Succesfully loaded sound resource file " << fileName << std::endl;

    return true;
}

//Deletes a sound resource again
void SoundEngine::DeleteSoundResource(uint8_t soundResId) {
   SoundResEntry* pntr;

   if (this->SoundResVec->size() < 1) {
       std::cout << "DeleteSoundResource: Specified sound resource ID (" << soundResId << ") for deletion does not exist." << std::endl;
       return;
   }

   std::vector<SoundResEntry*>::iterator it;
   bool found = false;

   for (it = this->SoundResVec->begin(); it != this->SoundResVec->end(); ++it) {
       if ((*it)->soundResId == soundResId) {
           //found the correct sound resource entry
           pntr = (*it);
           it = SoundResVec->erase(it);

           //delete sound buffer as well
           delete pntr->pntrSoundBuf;

           //delete entry struct
           delete pntr;

           found = true;

           break;
       }
   }

   if (!found) {
       std::cout << "DeleteSoundResource: Specified sound resource ID (" << soundResId << ") for deletion does not exist." << std::endl;
   }
}

SoundEngine::~SoundEngine() {
    StopAllSounds();

    //stop engine sound as well
    StopEngineSound();

    //delete all available sound resource
    DeleteSoundResource(SRES_MENUE_TYPEWRITEREFFECT1);
    DeleteSoundResource(SRES_MENUE_TYPEWRITEREFFECT2);
    DeleteSoundResource(SRES_MENUE_SELECTOTHERITEM);
    DeleteSoundResource(SRES_MENUE_CHANGECHECKBOXVAL);
    DeleteSoundResource(SRES_MENUE_WINDOWMOVEMENT);

    DeleteSoundResource(SRES_GAME_PICKUP);
    DeleteSoundResource(SRES_GAME_REFUEL);
    DeleteSoundResource(SRES_GAME_LARGECAR);
    DeleteSoundResource(SRES_GAME_WARNING);
    DeleteSoundResource(SRES_GAME_TURBO);
    DeleteSoundResource(SRES_GAME_BOOSTER);
    DeleteSoundResource(SRES_GAME_COLLISION);

    DeleteSoundResource(SRES_GAME_MGUN_SINGLESHOT);
    DeleteSoundResource(SRES_GAME_MGUN_SHOTFAILED);
    DeleteSoundResource(SRES_GAME_MGUN_LONGSHOT);

    DeleteSoundResource(SRES_GAME_MISSILE_SHOT);
    DeleteSoundResource(SRES_GAME_EXPLODE);

    DeleteSoundResource(SRES_GAME_FINALLAP);

    //remove all the intro sounds as well
    DeleteSoundResource(SRES_INTRO_FIRE);
    DeleteSoundResource(SRES_INTRO_EXPLODE);
    DeleteSoundResource(SRES_INTRO_TURBO);
    DeleteSoundResource(SRES_INTRO_SMALLCAR);
    DeleteSoundResource(SRES_INTRO_SCRAPE2);
    DeleteSoundResource(SRES_INTRO_RICCOS);
    DeleteSoundResource(SRES_INTRO_PAST);
    DeleteSoundResource(SRES_INTRO_MISSILE);
    DeleteSoundResource(SRES_INTRO_MINIGUN);
    DeleteSoundResource(SRES_INTRO_HELEHIT);
    DeleteSoundResource(SRES_INTRO_FIREPAST);
    DeleteSoundResource(SRES_INTRO_CURTAIN);
    DeleteSoundResource(SRES_INTRO_BOOSTER);

    delete SoundResVec;
}
