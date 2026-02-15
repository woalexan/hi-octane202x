/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef MUSIC_H
#define MUSIC_H

#include "SFML/Audio.hpp"
#include "adlmidi.h"
#include <cstdint>

#define MUSIC_BUFSIZE 8192
#define MUSIC_INSTRFILE_PATH "extract/InstrOPL.wopl"

/************************
 * Forward declarations *
 ************************/

class Game;

class MyMusicStream : public sf::SoundStream
{
public:
    MyMusicStream(Game* gamePnter, unsigned int sampleRate);
    ~MyMusicStream();
    virtual bool onGetData(Chunk& data);

    virtual void onSeek(sf::Time timeOffset);

    bool getInitOk();

    //loads an original game XMID file (extended midi file)
    //returns true in case of success, false otherwise
    bool loadGameMusicFile(const char* fileName);

    bool StartPlay();
    bool StopPlay();

    //sets a new music volume from 0...no music
    //up to 100.0 max volume
    void SetVolume(float newVolume);

private:
    //pointer to game
    Game* mGame = nullptr;

    //Its kind of dirty that we used two buffers
    //but leave it like this right now
    uint8_t *buffer = nullptr; /* Audio buffer */
    sf::Int16 *sfBuffer = nullptr;

    struct ADL_MIDIPlayer *midi_player = nullptr;
    struct ADLMIDI_AudioFormat s_audioFormat;

    uint32_t mSampleRate;

    bool mMusicLoaded = false;
    bool mInitOk = false;

    float mVolume = 100.0f;
    bool mMusicPlaying = true;

    //Converts the original game instrument information file (AIL OPL bank file)
    //to another file format we can load in ADLMIDI library (wOPL file format)
    //Returns true in case of success, False otherwise
    bool prepareInstrumentFile();

    //returns true if the necessary wOPL instrument file
    //was found, false otherwise
    bool VerifyInstrumentFile();
};

#endif // MUSIC_H
