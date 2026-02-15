/*
 Copyright (C) 2024-2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef SOUND_H
#define SOUND_H

#include "SFML/Audio.hpp"
#include "irrlicht.h"
#include <cstdint>

//File name definition
#define SFILE_MENUE_TYPEWRITEREFFECT1 "extract/sound/sound2-PRINTTYP.WAV"
#define SFILE_MENUE_TYPEWRITEREFFECT2 "extract/sound/sound2-PRINTRET.WAV"
#define SFILE_MENUE_SELECTOTHERITEM "extract/sound/sound2-CHANGE.WAV"
#define SFILE_MENUE_CHANGECHECKBOXVAL "extract/sound/sound2-SELECT.WAV"
#define SFILE_MENUE_WINDOWMOVEMENT "extract/sound/sound2-REFUEL.WAV"
#define SFILE_GAME_PICKUP "extract/sound/sound2-PICKUP.WAV"
#define SFILE_GAME_REFUEL "extract/sound/sound2-REFUEL.WAV"
#define SFILE_GAME_LARGECAR "extract/sound/sound4-LARGECAR.WAV"
#define SFILE_GAME_WARNING "extract/sound/sound2-WARNING.WAV"
#define SFILE_GAME_TURBO "extract/sound/sound4-TURBO.WAV"
#define SFILE_GAME_BOOSTER "extract/sound/sound4-BOOSTER.WAV"
#define SFILE_GAME_COLLIDED "extract/sound/sound4-HELEHIT.WAV"
#define SFILE_GAME_MGUN_SINGLESHOT "extract/sound/sound2-MINISHOT.WAV"
#define SFILE_GAME_MGUN_SHOTFAILED "extract/sound/sound2-MINBLANK.WAV"
#define SFILE_GAME_MGUN_LONGSHOT "extract/sound/sound4-MINIGUN.WAV"
#define SFILE_GAME_MISSILE_SHOT "extract/sound/sound2-MISSILE.WAV"
#define SFILE_GAME_EXPLODE "extract/sound/sound2-EXPLODE.WAV"
#define SFILE_GAME_FINALLAP "extract/sound/sound2-STUNT720.WAV"
#define SFILE_GAME_RICCO1 "extract/sound/sound2-RICCO1.WAV"
#define SFILE_GAME_RICCO2 "extract/sound/sound2-RICCO2.WAV"
#define SFILE_GAME_RICCO3 "extract/sound/sound2-RICCO3.WAV"
#define SFILE_GAME_LOCKON "extract/sound/sound2-LOCKON4.WAV"
#define SFILE_GAME_START1 "extract/sound/sound2-START1.WAV"
#define SFILE_GAME_START2 "extract/sound/sound2-START2.WAV"

//the sound0 files seem to be the audio files for the intro
#define SFILE_INTRO_FIRE "extract/sound/sound0-FIRE.WAV"
#define SFILE_INTRO_EXPLODE "extract/sound/sound0-EXPLODE.WAV"
#define SFILE_INTRO_TURBO "extract/sound/sound0-TURBO.WAV"
#define SFILE_INTRO_SMALLCAR "extract/sound/sound0-SMALLCAR.WAV"
#define SFILE_INTRO_SCRAPE2 "extract/sound/sound0-SCRAPE2.WAV"
#define SFILE_INTRO_RICCOS "extract/sound/sound0-RICCOS.WAV"
#define SFILE_INTRO_PAST "extract/sound/sound0-PAST.WAV"
#define SFILE_INTRO_MISSILE "extract/sound/sound0-MISSILE.WAV"
#define SFILE_INTRO_MINIGUN "extract/sound/sound0-MINIGUN.WAV"
#define SFILE_INTRO_HELEHIT "extract/sound/sound0-HELEHIT.WAV"
#define SFILE_INTRO_FIREPAST "extract/sound/sound0-FIREPAST.WAV"
#define SFILE_INTRO_CURTAIN "extract/sound/sound0-CURTAIN.WAV"
#define SFILE_INTRO_BOOSTER "extract/sound/sound0-BOOSTER.WAV"

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
#define SRES_GAME_RICCO1 23
#define SRES_GAME_RICCO2 24
#define SRES_GAME_RICCO3 25
#define SRES_GAME_LOCKON 26
#define SRES_GAME_START1 27
#define SRES_GAME_START2 28

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

/************************
 * Forward declarations *
 ************************/

class Player;
class Game;

class SoundResource {
public:
    SoundResource(std::string fileName, u_int8_t soundResId);
    ~SoundResource();

    bool loadOk;

    uint8_t mSoundResId;
    sf::SoundBuffer mSoundBuf;
};

class SoundEngine {
public:
  SoundEngine(Game* gamePnter);
  ~SoundEngine();

  bool GetSoundResourcesLoadOk();

  //play sound with localized sound source
  sf::Sound* PlaySound(uint8_t soundResId, irr::core::vector3df sourceLocation, bool looping = false);
  //play sound without localized sound source
  sf::Sound* PlaySound(uint8_t soundResId, bool looping = false);

  //non Localized sound source with pitch control, we need this to control
  //speed of booster sound playback
  sf::Sound* PlaySound(uint8_t soundResId, irr::f32 playPitch, bool looping = false);

  void StopLoopingSound(sf::Sound* pntrSound);
  bool IsAnySoundPlaying();
  void StopAllSounds();

  void StartEngineSoundForPlayer(Player* player);
  void RequestEngineSoundForPlayer(Player* player);
  void StopEngineSoundForPlayer(Player* player);
  void StopEngineSoundForAllPlayers();

  //with directional sound effect, engine sound in this case has also a location
  //used for all the players that are not controlled by the human player
  void SetPlayerSpeed(Player* player, float speed, float maxSpeed, irr::core::vector3df playerLocation, bool spatialSound = true);

  //without directional sound effect, engine sound in this case has no location
  //is used by the human controller player craft, to not get weird sound effects of human
  //players engine sound (sound modulation vs. rotation of player craft etc...)
  void SetPlayerSpeed(Player* player, float speed, float maxSpeed);

  bool GetIsSoundActive();

  //sets a new sound volume for all available sound
  //sources, 0 = sounds off, 100 = max volume
  void SetVolume(float soundVolume);

  void UpdateListenerLocation(irr::core::vector3df location, irr::core::vector3df frontDirVec);

  //returns true if successful, false otherwise
  void LoadSoundResourcesIntro();
  void UnLoadSoundResourcesIntro();

  void LoadSoundResources();

private:
  //pointer to Game
  Game* mGame = nullptr;

  bool mPlaySound = true;

  sf::Sound* PlaySound(uint8_t soundResId, bool localizedSoundSource, irr::core::vector3df sourceLocation, irr::f32 playPitch, bool looping = false);

  bool LoadSoundResource(std::string fileName, uint8_t soundResId);
  void DeleteSoundResource(uint8_t soundResId);

  sf::Sound* GetFreeSoundSource(SoundResource* resToPlay);

  //searches for a sound resource entry with a certain specified sound ID
  //if no sound under this sound ID is found returns nullptr
  SoundResource* SearchSndRes(uint8_t soundResId);

  std::vector<SoundResource*> *SoundResVec = nullptr;
  std::vector<sf::Sound*> *SoundVec = nullptr;

  uint8_t mNrSoundSources = 0;
  float mSoundVolume = 100.0f;

  irr::core::vector3df* mNonLocalizedSoundPos = nullptr;

  sf::Sound* engineSound = nullptr;

  std::vector<std::pair<Player*, sf::Sound*>> mEngineSoundVector;
};

#endif // SOUND_H
