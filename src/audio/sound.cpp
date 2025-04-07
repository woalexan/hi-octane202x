/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "sound.h"
#include <iostream>

bool SoundEngine::getSoundResourcesLoadOk() {
    return mSoundResourcesLoadOk;
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

    StopEngineSoundForAllPlayers();
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

    //is any engine sound playing?
    //which players engine sound needs to be updated?
    std::vector<std::pair<Player*, sf::Sound*>>::iterator it;

    for (it = this->mEngineSoundVector.begin(); it != this->mEngineSoundVector.end(); ++it) {
        if ((*it).second != NULL) {
            if ((*it).second->getStatus() == ((*it).second->Playing))
                return true;
        }
    }

   return false;
}

void SoundEngine::RequestEngineSoundForPlayer(Player* player) {
    SoundResEntry* pntr = SearchSndRes(SRES_GAME_LARGECAR);

    sf::Sound *newEngineSound = new sf::Sound();

    newEngineSound->setVolume(mSoundVolume);
    newEngineSound->setBuffer(*pntr->pntrSoundBuf);
    newEngineSound->setLoop(true);

    //make sound source location absolute to listener
    newEngineSound->setRelativeToListener(false);

    this->mEngineSoundVector.push_back(std::make_pair(player, newEngineSound));
}

void SoundEngine::StartEngineSoundForPlayer(Player* player) {
    //which players engine sound needs to be updated?
    std::vector<std::pair<Player*, sf::Sound*>>::iterator it;

    for (it = this->mEngineSoundVector.begin(); it != this->mEngineSoundVector.end(); ++it) {
        if ((*it).first == player) {
            //correct player found

            if (mSoundVolume > 1.0f) {
                (*it).second->play();
            }

            break;
        }
    }
}

//with directional sound effect, engine sound in this case has also a location
//used for all the players that are not controlled by the human player
void SoundEngine::SetPlayerSpeed(Player* player, float speed, float maxSpeed, irr::core::vector3df playerLocation, bool spatialSound) {
    //which players engine sound needs to be updated?
    std::vector<std::pair<Player*, sf::Sound*>>::iterator it;

    for (it = this->mEngineSoundVector.begin(); it != this->mEngineSoundVector.end(); ++it) {
        if ((*it).first == player) {
            //correct player found
            //update values for engine sound

            (*it).second->setPitch(float(0.6) + (speed/maxSpeed) * float(0.8));

            //update sound source position
            (*it).second->setPosition(playerLocation.X, playerLocation.Y, playerLocation.Z);

            if (!spatialSound) {
                (*it).second->setAttenuation(0.0f);
                (*it).second->setRelativeToListener(true);
            } else {
                //restore default attenuation setting of 1.0f
                (*it).second->setAttenuation(1.0f);
                (*it).second->setRelativeToListener(false);
            }

            break;
        }
    }
}

//without directional sound effect, engine sound in this case has no location
//is used by the human controller player craft, to not get weird sound effects of human
//players engine sound (sound modulation vs. rotation of player craft etc...)
void SoundEngine::SetPlayerSpeed(Player* player, float speed, float maxSpeed) {
    SetPlayerSpeed(player, speed, maxSpeed, *this->mNonLocalizedSoundPos, false);
}

void SoundEngine::StopEngineSoundForAllPlayers() {
    //which players engine sound needs to be updated?
    std::vector<std::pair<Player*, sf::Sound*>>::iterator it;

    for (it = this->mEngineSoundVector.begin(); it != this->mEngineSoundVector.end(); ++it) {
            if ((*it).second != NULL) {
                (*it).second->stop();
            }
    }
}

void SoundEngine::StopEngineSoundForPlayer(Player* player) {
    //which players engine sound needs to be updated?
    std::vector<std::pair<Player*, sf::Sound*>>::iterator it;

    for (it = this->mEngineSoundVector.begin(); it != this->mEngineSoundVector.end(); ++it) {
        if ((*it).first == player) {
            //correct player found

            if ((*it).second != NULL) {
                (*it).second->stop();
            }

            break;
        }
    }
}

void SoundEngine::UpdateListenerLocation(irr::core::vector3df location, irr::core::vector3df frontDirVec) {
   sf::Listener::setPosition(location.X, location.Y, location.Z);

   frontDirVec.normalize();
   sf::Listener::setDirection(frontDirVec.X, frontDirVec.Y, -frontDirVec.Z);
}

SoundEngine::SoundEngine(InfrastructureBase* infraPnter) {
    mInfra = infraPnter;

    //create new vector where we can store our soundbuffers/soundresources
    SoundResVec = new std::vector<SoundResEntry*>();

    //create new vector where we can store sound sources
    SoundVec = new std::vector<sf::Sound*>();

    mNrSoundSources = 0;

    mNonLocalizedSoundPos = new irr::core::vector3df(0.0f, 0.0f, 0.0f);
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
        StopEngineSoundForAllPlayers();
    } else {
        mPlaySound = true;
        mSoundVolume = soundVolume;

        //which players engine sound needs to be updated?
        std::vector<std::pair<Player*, sf::Sound*>>::iterator it;

        for (it = this->mEngineSoundVector.begin(); it != this->mEngineSoundVector.end(); ++it) {
            if ((*it).second != NULL) {
                (*it).second->setVolume(soundVolume);
            }
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

//Localized sound source
sf::Sound* SoundEngine::PlaySound(uint8_t soundResId, irr::core::vector3df sourceLocation, bool looping) {
    return PlaySound(soundResId, true, sourceLocation, looping);
}

//non Localized sound source
sf::Sound* SoundEngine::PlaySound(uint8_t soundResId, bool looping) {
    return PlaySound(soundResId, false, *mNonLocalizedSoundPos, looping);
}

sf::Sound* SoundEngine::PlaySound(uint8_t soundResId, bool localizedSoundSource, irr::core::vector3df sourceLocation, bool looping) {
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
                sndPntr->setPosition(sourceLocation.X, sourceLocation.Y, sourceLocation.Z);

                //if this is a non localized sound source also set sound attenuation
                //over distance to 0, to make sure sounds are not getting more faint over distance
                if (!localizedSoundSource) {
                    sndPntr->setAttenuation(0.0f);
                } else {
                    //restore the default attenuation of 1.0f
                    sndPntr->setAttenuation(1.0f);
                }

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

//returns true if successful, false otherwise
bool SoundEngine::LoadSoundResourcesIntro() {
    bool initOk = true;

    //load all the intro sounds as well
    initOk &= initOk && LoadSoundResource(SFILE_INTRO_FIRE, SRES_INTRO_FIRE);
    initOk &= initOk && LoadSoundResource(SFILE_INTRO_EXPLODE, SRES_INTRO_EXPLODE);
    initOk &= initOk && LoadSoundResource(SFILE_INTRO_TURBO, SRES_INTRO_TURBO);
    initOk &= initOk && LoadSoundResource(SFILE_INTRO_SMALLCAR, SRES_INTRO_SMALLCAR);
    initOk &= initOk && LoadSoundResource(SFILE_INTRO_SCRAPE2, SRES_INTRO_SCRAPE2);
    initOk &= initOk && LoadSoundResource(SFILE_INTRO_RICCOS, SRES_INTRO_RICCOS);
    initOk &= initOk && LoadSoundResource(SFILE_INTRO_PAST, SRES_INTRO_PAST);
    initOk &= initOk && LoadSoundResource(SFILE_INTRO_MISSILE, SRES_INTRO_MISSILE);
    initOk &= initOk && LoadSoundResource(SFILE_INTRO_MINIGUN, SRES_INTRO_MINIGUN);
    initOk &= initOk && LoadSoundResource(SFILE_INTRO_HELEHIT, SRES_INTRO_HELEHIT);
    initOk &= initOk && LoadSoundResource(SFILE_INTRO_FIREPAST, SRES_INTRO_FIREPAST);
    initOk &= initOk && LoadSoundResource(SFILE_INTRO_CURTAIN, SRES_INTRO_CURTAIN);
    initOk &= initOk && LoadSoundResource(SFILE_INTRO_BOOSTER, SRES_INTRO_BOOSTER);

    return initOk;
}

void SoundEngine::UnLoadSoundResourcesIntro() {
    //remove all intro sounds again
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
}

//Loads the available sound resources for the game
void SoundEngine::LoadSoundResources() {
    mSoundResourcesLoadOk = true;

    //load all the sound resource files we need for this game
    mSoundResourcesLoadOk &= mSoundResourcesLoadOk && LoadSoundResource(SFILE_MENUE_TYPEWRITEREFFECT1, SRES_MENUE_TYPEWRITEREFFECT1);
    mSoundResourcesLoadOk &= mSoundResourcesLoadOk && LoadSoundResource(SFILE_MENUE_TYPEWRITEREFFECT2, SRES_MENUE_TYPEWRITEREFFECT2);
    mSoundResourcesLoadOk &= mSoundResourcesLoadOk && LoadSoundResource(SFILE_MENUE_SELECTOTHERITEM, SRES_MENUE_SELECTOTHERITEM);
    mSoundResourcesLoadOk &= mSoundResourcesLoadOk && LoadSoundResource(SFILE_MENUE_CHANGECHECKBOXVAL, SRES_MENUE_CHANGECHECKBOXVAL);
    mSoundResourcesLoadOk &= mSoundResourcesLoadOk && LoadSoundResource(SFILE_MENUE_WINDOWMOVEMENT, SRES_MENUE_WINDOWMOVEMENT);

    mSoundResourcesLoadOk &= mSoundResourcesLoadOk && LoadSoundResource(SFILE_GAME_PICKUP, SRES_GAME_PICKUP);
    mSoundResourcesLoadOk &= mSoundResourcesLoadOk && LoadSoundResource(SFILE_GAME_REFUEL, SRES_GAME_REFUEL);
    mSoundResourcesLoadOk &= mSoundResourcesLoadOk && LoadSoundResource(SFILE_GAME_LARGECAR, SRES_GAME_LARGECAR);
    mSoundResourcesLoadOk &= mSoundResourcesLoadOk && LoadSoundResource(SFILE_GAME_WARNING, SRES_GAME_WARNING);
    mSoundResourcesLoadOk &= mSoundResourcesLoadOk && LoadSoundResource(SFILE_GAME_TURBO, SRES_GAME_TURBO);
    mSoundResourcesLoadOk &= mSoundResourcesLoadOk && LoadSoundResource(SFILE_GAME_BOOSTER, SRES_GAME_BOOSTER);
    mSoundResourcesLoadOk &= mSoundResourcesLoadOk &&LoadSoundResource(SFILE_GAME_COLLIDED, SRES_GAME_COLLISION);

    mSoundResourcesLoadOk &= mSoundResourcesLoadOk && LoadSoundResource(SFILE_GAME_MGUN_SINGLESHOT, SRES_GAME_MGUN_SINGLESHOT);
    mSoundResourcesLoadOk &= mSoundResourcesLoadOk && LoadSoundResource(SFILE_GAME_MGUN_SHOTFAILED, SRES_GAME_MGUN_SHOTFAILED);
    mSoundResourcesLoadOk &= mSoundResourcesLoadOk && LoadSoundResource(SFILE_GAME_MGUN_LONGSHOT, SRES_GAME_MGUN_LONGSHOT);

    mSoundResourcesLoadOk &= mSoundResourcesLoadOk && LoadSoundResource(SFILE_GAME_MISSILE_SHOT, SRES_GAME_MISSILE_SHOT);
    mSoundResourcesLoadOk &= mSoundResourcesLoadOk && LoadSoundResource(SFILE_GAME_EXPLODE, SRES_GAME_EXPLODE);

    mSoundResourcesLoadOk &= mSoundResourcesLoadOk && LoadSoundResource(SFILE_GAME_FINALLAP, SRES_GAME_FINALLAP);

    mSoundResourcesLoadOk &= mSoundResourcesLoadOk && LoadSoundResource(SFILE_GAME_RICCO1, SRES_GAME_RICCO1);
    mSoundResourcesLoadOk &= mSoundResourcesLoadOk && LoadSoundResource(SFILE_GAME_RICCO2, SRES_GAME_RICCO2);
    mSoundResourcesLoadOk &= mSoundResourcesLoadOk && LoadSoundResource(SFILE_GAME_RICCO3, SRES_GAME_RICCO3);
    mSoundResourcesLoadOk &= mSoundResourcesLoadOk && LoadSoundResource(SFILE_GAME_LOCKON, SRES_GAME_LOCKON);
    mSoundResourcesLoadOk &= mSoundResourcesLoadOk && LoadSoundResource(SFILE_GAME_START1, SRES_GAME_START1);
    mSoundResourcesLoadOk &= mSoundResourcesLoadOk && LoadSoundResource(SFILE_GAME_START2, SRES_GAME_START2);
}

//Loads a single specified sound resource
bool SoundEngine::LoadSoundResource(char *fileName, uint8_t soundResId) {
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
    StopEngineSoundForAllPlayers();

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

    DeleteSoundResource(SRES_GAME_RICCO1);
    DeleteSoundResource(SRES_GAME_RICCO2);
    DeleteSoundResource(SRES_GAME_RICCO3);
    DeleteSoundResource(SRES_GAME_LOCKON);
    DeleteSoundResource(SRES_GAME_START1);
    DeleteSoundResource(SRES_GAME_START2);

    delete SoundResVec;

    delete mNonLocalizedSoundPos;
}
