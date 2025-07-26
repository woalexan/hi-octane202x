/*
 The source code in this file was based on/derived from/translated from
 the GitHub project https://github.com/movAX13h/HiOctaneTools to C++ by myself.
 This project also uses the GPL3 license which is attached to this project repo as well.
 
 Copyright (C) 2024-2025 Wolf Alexander       (I did just translation to C++)
 Copyright (C) 2016 movAX13h and srtuss  (authors of original source code)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef BLOCKDEFINITION_H
#define BLOCKDEFINITION_H

#include "tableitem.h"
#include <cstdint>
#include "irrlicht.h"

#define DEF_BLOCKDEF_STATE_DEFAULT 0
#define DEF_BLOCKDEF_STATE_NEWLYADDED_KEEP 1
#define DEF_BLOCKDEF_STATE_NEWLYUNASSIGNEDONE 2

class BlockDefinition : public TableItem {
public:
    BlockDefinition(int id, int offset, std::vector<uint8_t> bytes);
    ~BlockDefinition();

    //alternative constructor for usage with the level editor
    //to add a new block definition to the level file
    //if offset is set to -1 (for example for special column for block preview) writing to
    //map file data vector is disabled.
    BlockDefinition(int id, int offset, uint8_t newN, uint8_t newE, uint8_t newS, uint8_t newW, uint8_t newT, uint8_t newB,
                    uint8_t newNMod, uint8_t newEMod, uint8_t newSMod, uint8_t newWMod, uint8_t newTMod, uint8_t newBMod, int16_t newUnknown1, int16_t newUnknown2);

    bool WriteChanges() override;

    uint8_t get_N();
    uint8_t get_E();
    uint8_t get_S();
    uint8_t get_W();
    uint8_t get_T();
    uint8_t get_B();
    int16_t get_Unknown1();
    int16_t get_Unknown2();

    uint8_t get_NMod();
    uint8_t get_EMod();
    uint8_t get_SMod();
    uint8_t get_WMod();
    uint8_t get_TMod();
    uint8_t get_BMod();

    //for the levelEditor functionality
    void set_N(uint8_t newVal);
    void set_E(uint8_t newVal);
    void set_S(uint8_t newVal);
    void set_W(uint8_t newVal);
    void set_T(uint8_t newVal);
    void set_B(uint8_t newVal);

    void setNMod(uint8_t newVal);
    void setEMod(uint8_t newVal);
    void setSMod(uint8_t newVal);
    void setWMod(uint8_t newVal);
    void setTMod(uint8_t newVal);
    void setBMod(uint8_t newVal);

    /*****************************************
     * LevelEditor only used variables start *
     *****************************************/

    //usage count, how often is this blockdefinition
    //used in the map
    uint32_t usageCnt = 0;

    //only used for the level editor
    //while block definition changes
    int m_initialID;

    //this blockdefinition state variable is
    //only used for the leveleditor
    uint8_t mState = DEF_BLOCKDEF_STATE_DEFAULT;

    //two preview textures only used in the leveleditor
    //Important note: Do not create or delete the Textures
    //in the Blockdefinition class itself, the LevelBlocks class
    //takes care about this all!
    irr::video::ITexture* mPreviewFront = nullptr;
    irr::video::ITexture* mPreviewBack = nullptr;

    /*****************************************
     * LevelEditor only used variables end   *
     *****************************************/

private:
    uint8_t mN;
    uint8_t mE;
    uint8_t mS;
    uint8_t mW;
    uint8_t mT;
    uint8_t mB;

    uint8_t mNMod;
    uint8_t mEMod;
    uint8_t mSMod;
    uint8_t mWMod;
    uint8_t mTMod;
    uint8_t mBMod;

    int16_t mUnknown1;
    int16_t mUnknown2;

    uint8_t decode_N();
    uint8_t decode_E();
    uint8_t decode_S();
    uint8_t decode_W();
    uint8_t decode_T();
    uint8_t decode_B();

    uint8_t decode_NMod();
    uint8_t decode_EMod();
    uint8_t decode_SMod();
    uint8_t decode_WMod();
    uint8_t decode_TMod();
    uint8_t decode_BMod();

    void save_N(uint8_t newVal);
    void save_E(uint8_t newVal);
    void save_S(uint8_t newVal);
    void save_W(uint8_t newVal);
    void save_T(uint8_t newVal);
    void save_B(uint8_t newVal);

    void save_NMod(uint8_t newVal);
    void save_EMod(uint8_t newVal);
    void save_SMod(uint8_t newVal);
    void save_WMod(uint8_t newVal);
    void save_TMod(uint8_t newVal);
    void save_BMod(uint8_t newVal);
};

#endif // BLOCKDEFINITION_H
