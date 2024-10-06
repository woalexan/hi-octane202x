/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef SOUND_H
#define SOUND_H

#include "SFML/Audio.hpp"
#include "../definitions.h"

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
