/*
 Copyright (C) 2024 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "music.h"
#include "../utils/fileutils.h"
#include <cstdlib>
#include <iostream>

bool MyMusicStream::getInitOk() {
    return mInitOk;
}

//Converts the original game instrument information file (AIL bank file)
//to another file format we can load in ADLMIDI (wOPL file format)
//Returns true in case of success, False otherwise
bool MyMusicStream::prepareInstrumentFile() {
    //first Part: load GTL bank which is part of the original game
    //is file SAMPLE.OPL
     AIL_GTL *AILBank = new AIL_GTL;
     FmBank *testAILBank = new FmBank();

     //find original instrument file
     irr::io::path inputDatFile =
             mInfra->LocateFileInFileList(mInfra->mOriginalGame->soundFolder, irr::core::string<fschar_t>("sample.opl"));

     if (inputDatFile.empty()) {
         //input file not found!
          std::cout << "Music: Could not locate the original games sound data file sample.opl" << std::endl;
          delete testAILBank;
          delete  AILBank;
          return false;
     }

     std::string inputFileName(inputDatFile.c_str());

     if (AILBank->loadFile(inputFileName, *testAILBank) != FfmtErrCode::ERR_OK) {
         //there was an error loading the file
         std::cout << "Music: Failed loading originalgame/sound/sample.opl instrument file" << std::endl;
         delete testAILBank;
         delete  AILBank;
         return false;
     }

     //create the instrument information in the
     //so called wOPL (WohlstandOPL) file format
     WohlstandOPL3 *wOPLBank = new WohlstandOPL3();

     std::string outputFileName((char*)(MUSIC_INSTRFILE_PATH));
     if (wOPLBank->saveFile(outputFileName, *testAILBank) != FfmtErrCode::ERR_OK) {
         //there was an error writing the new file
         std::cout << "Music: Failed writing " << outputFileName << " wOPLinstrument file" << std::endl;
         delete wOPLBank;
         delete testAILBank;
         delete  AILBank;
         return false;
     }

     //all was ok
     delete wOPLBank;
     delete testAILBank;
     delete  AILBank;

     return true;
}

//returns true if the necessary wOPL instrument file
//was found, false otherwise
bool MyMusicStream::VerifyInstrumentFile() {
    int fileExists;

    fileExists = FileExists((char*)(MUSIC_INSTRFILE_PATH));

    //-1 if item can not be accessed, means file does not exit
    //1 if file exists
    //0 specified element is not a file, but something else
    if (fileExists != 1) {
        //file does not exist
        //try to create it

        //first make sure "extract" folder does exist
        //if not create it
        try {
            PrepareSubDir("extract");
        }
        catch (const std::string &e) {
            //problem creating sub dir
            std::cout << "Music: Problem creating missing Subdir for WOPL instrument file (" << e << ")" << std::endl;
            return false;
        }

        std::cout << "Music: Create " << std::string(MUSIC_INSTRFILE_PATH) << " wOPL instrument file" << std::endl;
        if (!prepareInstrumentFile()) {
            std::cout << "Music: File creation failed" << std::endl;
            return false;
        }

        fileExists = FileExists((char*)(MUSIC_INSTRFILE_PATH));
        if (fileExists == 1) {
            std::cout << "Music: File creation succeded" << std::endl;
            return true;
        }

        std::cout << "Music: File creation failed" << std::endl;
        return false;
    } else if (fileExists == 1) {
        std::cout << "Music: Instrument file " << std::string(MUSIC_INSTRFILE_PATH) << " found" << std::endl;
        //file exists already
        return true;
    }

    return false;
}

//sets a new music volume from 0...no music
//up to 100.0 max volume
void MyMusicStream::SetVolume(float newVolume) {
    if (newVolume < 1.0f) {
        //stop music
        mMusicPlaying = false;
        mVolume = newVolume;
        this->StopPlay();
    } else {
        //we want to play music
        mVolume = newVolume;
        this->setVolume(newVolume);

        if (!mMusicPlaying) {
          this->StartPlay();
        }
    }
}

MyMusicStream::MyMusicStream(InfrastructureBase* infraPnter, unsigned int sampleRate) {
    mInfra = infraPnter;

    //verify and if necessary create the
    //OPL3 instrument file
    if (!VerifyInstrumentFile()) {
        mInitOk = false;
        return;
    }

    mSampleRate = sampleRate;

    //allocate buffer
    buffer = new uint8_t[MUSIC_BUFSIZE];
    sfBuffer = new sf::Int16[MUSIC_BUFSIZE / 2];

    //configure SFML audio output settings
    //we want 2 channels and a specified sample rate
    this->initialize(2, mSampleRate);

    /* Initialize ADLMIDI */
    midi_player = adl_init(mSampleRate);

    if (!midi_player)
    {
        fprintf(stderr, "Couldn't initialize ADLMIDI: %s\n", adl_errorString());
        mInitOk = false;
        return;
    }

    //select OPL emulator

    //adl_switchEmulator(midi_player, ADLMIDI_EMU_NUKED);  NUKED emulator gives best emu quality according to i-net,
                                                           //but takes 10% CPU load on my computer
    adl_switchEmulator(midi_player, ADLMIDI_EMU_DOSBOX);   //Dosbox emu takes 1% on my computer, and I can not hear
                                                           //much difference => take as default right now

    //configure the audio format we want to get from ADLMIDI
    s_audioFormat.type = ADLMIDI_SampleType_S16;
    s_audioFormat.containerSize = sizeof(int16_t);
    s_audioFormat.sampleOffset = sizeof(int16_t) * 2;

    /* Optionally Setup ADLMIDI as you want */

    //enable endless looping of game music

    //loopEn 0 - disabled, 1 - enabled
    adl_setLoopEnabled(midi_player, 1);

    //I would also like to activate the next line, but
    //the problem is my linked does not find adl_setLoopCount, even
    //though it is mentioned in the header file
    //the good thing is the default value seems to be already
    //loop infinitely :)
    //so keep the next line commented out right now
    //loopCount Number of loops or -1 to loop infinitely
    //adl_setLoopCount(midi_player, -1);

    /* Set using of custom bank (WOPL format) loaded from a file */
    //if we do not setup the OPL instruments correctly
    //the player music will sound completely wrong
    adl_openBankFile(midi_player, (char*)(MUSIC_INSTRFILE_PATH));

    mInitOk = true;
}

//loads an original game XMID file (extended midi file)
//returns true in case of success, false otherwise
bool MyMusicStream::loadGameMusicFile(char* fileName) {
    /* Open the MIDI (or MUS, IMF or CMF) file to play */
    if (adl_openFile(midi_player, fileName) < 0)
        {
            fprintf(stderr, "Couldn't open music file: %s\n", adl_errorInfo(midi_player));

            this->mMusicLoaded = false;
            return false;
        }

    this->mMusicLoaded = true;
    return true;
}

bool MyMusicStream::StartPlay() {
    if (!mInitOk)
        return false;

    if (!mMusicLoaded)
        return false;

    //already playing?
    if (this->getStatus() == this->Playing)
        return true;

    //only play if we have a volume set
    if (this->mVolume > 1.0f) {
        //start playing music
        this->play();
    }

    return true;
}

bool MyMusicStream::StopPlay() {
    if (!mInitOk)
        return false;

    //music already stopped?
    if (this->getStatus() == this->Stopped)
         return true;

    if (this->getStatus() != this->Playing)
        return false;

    //stop playing music
    this->stop();

    return true;
}

MyMusicStream::~MyMusicStream() {
    //if we are right now playing music
    //stop it
    if (this->getStatus() == this->Playing) {
        this->StopPlay();
    }

    adl_close(midi_player);

    //delete midi_player;
    //delete sfBuffer;
    //delete buffer;
}

bool MyMusicStream::onGetData(Chunk& data)
{
    struct ADL_MIDIPlayer* p = (struct ADL_MIDIPlayer*)midi_player;

    /* Convert bytes length into total count of samples in all channels */
    int samples_count = MUSIC_BUFSIZE / s_audioFormat.containerSize;

    /* Take some samples from the ADLMIDI */
    samples_count = adl_playFormat(p, samples_count,
                                   buffer,
                                   buffer + s_audioFormat.containerSize,
                                   &s_audioFormat);

    if(samples_count <= 0) {
       //no more samples to play, stop playing
       return false;
    }

    int idxSource = 0;

    //convert from one buffer to the other
    for (int idx = 0; idx < samples_count; idx++) {
        sfBuffer[idx] = sf::Int16(buffer[idxSource]) + (((sf::Int16)(buffer[idxSource + 1])) << 8);
        idxSource += 2;
    }

    /* Send buffer to the audio device */
    data.samples = sfBuffer; /* put the pointer to the new audio samples */;
    data.sampleCount = samples_count; /* put the number of audio samples available in the new chunk */;

    return true;
}

void MyMusicStream::onSeek(sf::Time timeOffset) {
      std::cout << "MyMusicStream::onSeek Please implement me!" << std::endl;
}




