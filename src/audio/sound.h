/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef SOUND_H
#define SOUND_H

#include "SFML/Audio.hpp"
#include "../definitions.h"
#include <cstdint>

//File name definition
#define SFILE_MENUE_TYPEWRITEREFFECT1 (char*)"extract/sound/sound2-PRINTTYP.WAV"
#define SFILE_MENUE_TYPEWRITEREFFECT2 (char*)"extract/sound/sound2-PRINTRET.WAV"
#define SFILE_MENUE_SELECTOTHERITEM (char*)"extract/sound/sound2-CHANGE.WAV"
#define SFILE_MENUE_CHANGECHECKBOXVAL (char*)"extract/sound/sound2-SELECT.WAV"
#define SFILE_MENUE_WINDOWMOVEMENT (char*)"extract/sound/sound2-REFUEL.WAV"
#define SFILE_GAME_PICKUP (char*)"extract/sound/sound2-PICKUP.WAV"
#define SFILE_GAME_REFUEL (char*)"extract/sound/sound2-REFUEL.WAV"
#define SFILE_GAME_LARGECAR (char*)"extract/sound/sound4-LARGECAR.WAV"
#define SFILE_GAME_WARNING (char*)"extract/sound/sound2-WARNING.WAV"
#define SFILE_GAME_TURBO (char*)"extract/sound/sound4-TURBO.WAV"
#define SFILE_GAME_BOOSTER (char*)"extract/sound/sound4-BOOSTER.WAV"
#define SFILE_GAME_COLLIDED (char*)"extract/sound/sound4-HELEHIT.WAV"
#define SFILE_GAME_MGUN_SINGLESHOT (char*)"extract/sound/sound2-MINISHOT.WAV"
#define SFILE_GAME_MGUN_SHOTFAILED (char*)"extract/sound/sound2-MINBLANK.WAV"
#define SFILE_GAME_MGUN_LONGSHOT (char*)"extract/sound/sound4-MINIGUN.WAV"
#define SFILE_GAME_MISSILE_SHOT (char*)"extract/sound/sound2-MISSILE.WAV"
#define SFILE_GAME_EXPLODE (char*)"extract/sound/sound2-EXPLODE.WAV"
#define SFILE_GAME_FINALLAP (char*)"extract/sound/sound2-STUNT720.WAV"

//the sound0 files seem to be the audio files for the intro
#define SFILE_INTRO_FIRE (char*)"extract/sound/sound0-FIRE.WAV"
#define SFILE_INTRO_EXPLODE (char*)"extract/sound/sound0-EXPLODE.WAV"
#define SFILE_INTRO_TURBO (char*)"extract/sound/sound0-TURBO.WAV"
#define SFILE_INTRO_SMALLCAR (char*)"extract/sound/sound0-SMALLCAR.WAV"
#define SFILE_INTRO_SCRAPE2 (char*)"extract/sound/sound0-SCRAPE2.WAV"
#define SFILE_INTRO_RICCOS (char*)"extract/sound/sound0-RICCOS.WAV"
#define SFILE_INTRO_PAST (char*)"extract/sound/sound0-PAST.WAV"
#define SFILE_INTRO_MISSILE (char*)"extract/sound/sound0-MISSILE.WAV"
#define SFILE_INTRO_MINIGUN (char*)"extract/sound/sound0-MINIGUN.WAV"
#define SFILE_INTRO_HELEHIT (char*)"extract/sound/sound0-HELEHIT.WAV"
#define SFILE_INTRO_FIREPAST (char*)"extract/sound/sound0-FIREPAST.WAV"
#define SFILE_INTRO_CURTAIN (char*)"extract/sound/sound0-CURTAIN.WAV"
#define SFILE_INTRO_BOOSTER (char*)"extract/sound/sound0-BOOSTER.WAV"

#define SRES_NORES 0
#define SRES_MENUE_TYPEWRITEREFFECT1 1
#define SRES_MENUE_TYPEWRITEREFFECT2 2
#define SRES_MENUE_SELECTOTHERITEM 3
#define SRES_MENUE_CHANGECHECKBOXVAL 4
#define SRES_MENUE_WINDOWMOVEMENT 5
#define SRES_GAME_PICKUP 10
#define SRES_GAME_REFUEL 11
#define SRES_GAME_LARGECAR 12
#define SRES_GAME_WARNING 13
#define SRES_GAME_TURBO 14
#define SRES_GAME_BOOSTER 15
#define SRES_GAME_COLLISION 16
#define SRES_GAME_MGUN_SINGLESHOT 17
#define SRES_GAME_MGUN_SHOTFAILED 18
#define SRES_GAME_MGUN_LONGSHOT 19
#define SRES_GAME_MISSILE_SHOT 20
#define SRES_GAME_EXPLODE 21
#define SRES_GAME_FINALLAP 22

#define SRES_INTRO_FIRE 50
#define SRES_INTRO_EXPLODE 51
#define SRES_INTRO_TURBO 52
#define SRES_INTRO_SMALLCAR 53
#define SRES_INTRO_SCRAPE2 54
#define SRES_INTRO_RICCOS 55
#define SRES_INTRO_PAST 56
#define SRES_INTRO_MISSILE 57
#define SRES_INTRO_MINIGUN 58
#define SRES_INTRO_HELEHIT 59
#define SRES_INTRO_FIREPAST 60
#define SRES_INTRO_CURTAIN 61
#define SRES_INTRO_BOOSTER 62

//maximum number of allowed sound sources
#define SOUND_MAXNR 20

struct SoundResEntry {
    sf::SoundBuffer* pntrSoundBuf;
    uint8_t soundResId;
};

class SoundEngine {
public:
  SoundEngine();
  ~SoundEngine();

  bool getInitOk();
  sf::Sound* PlaySound(uint8_t soundResId, bool looping = false);
  void StopLoopingSound(sf::Sound *pntrSound);
  bool IsAnySoundPlaying();
  void StopAllSounds();

  void StartEngineSound();
  void StopEngineSound();
  void SetPlayerSpeed(float speed, float maxSpeed);
  bool GetIsSoundActive();

  //sets a new sound volume for all available sound
  //sources, 0 = sounds off, 100 = max volume
  void SetVolume(float soundVolume);

private:
  bool mInitOk = false;
  bool mPlaySound = true;
  void LoadSoundResources();

  bool LoadSoundResouce(char* fileName, uint8_t soundResId);
  void DeleteSoundResource(uint8_t soundResId);

  sf::Sound* GetFreeSoundSource();

  //searches for a sound buffer resource entry with a certain specified sound ID
  //if no sound under this sound ID is found returns NULL
  SoundResEntry* SearchSndRes(uint8_t soundResId);

  std::vector<SoundResEntry*> *SoundResVec;
  std::vector<sf::Sound*> *SoundVec;

  uint8_t mNrSoundSources = 0;
  float mSoundVolume = 100.0f;

  sf::Sound* engineSound;
  float playerSpeed;
  float playerMaxSpeed;
};

#endif // SOUND_H
