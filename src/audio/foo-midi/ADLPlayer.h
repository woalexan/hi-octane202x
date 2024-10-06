
/** $VER: ADLPlayer.h (2023.09.27) **/

/* Note Wolf Alexander: The source code below is not 100% the original code anymore,
 * as I had to make changes to it to be able to compile it on my system.  
 * I also removed everything that is not needed for my game project 
 
 * This is the link to the original authors project:
 * https://github.com/stuerp/foo_midi                                              */

#pragma once

//#include <CppCoreCheck/Warnings.h>

#include "MIDIPlayer.h"

//#include <../libADLMIDI/adlmidi.h>
#include <adlmidi.h>

class ADLPlayer : public MIDIPlayer
{
public:
    ADLPlayer();
    virtual ~ADLPlayer();

    enum
    {
        ADLMIDI_EMU_NUKED = 0,
        ADLMIDI_EMU_NUKED_174,
        ADLMIDI_EMU_DOSBOX
    };

    void SetCore(uint32_t);
    void SetBank(uint32_t);
    void SetChipCount(uint32_t);
    void Set4OpCount(uint32_t);
    void SetFullPanning(bool);
    void SetBankFilePath(std::string filePath);

protected:
    virtual bool Startup() override;
    virtual void Shutdown() override;
    virtual void Render(audio_sample *, uint32_t) override;

    virtual void SendEvent(uint32_t) override;
    virtual void SendSysEx(const uint8_t *, size_t, uint32_t) override;

private:
    struct ADL_MIDIPlayer * _Player[3];

    uint32_t _EmuCore;
    uint32_t _BankNumber;
    uint32_t _ChipCount;
    uint32_t _4OpCount;
    bool _FullPanning;
    std::string _BankFilePath;
};
