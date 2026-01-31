/*
 Copyright (C) 2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef GAMEDBGWND_H
#define GAMEDBGWND_H

#include "irrlicht.h"

/************************
 * Forward declarations *
 ************************/

class Race;

/* GUI Elements for the Game debug window
*/
struct GUIGameDbgWnd
{
    GUIGameDbgWnd ()
    {
        memset ( this, 0, sizeof ( *this ) );
    }

    irr::gui::IGUITabControl* tabCntrl;
    irr::gui::IGUITab* LevelTab;
    irr::gui::IGUITab* MovementTab;

    //Level Tab items
    irr::gui::IGUICheckBox* ShowWallsegmentLines;
    irr::gui::IGUICheckBox* ShowWallCollisionMesh;
    irr::gui::IGUICheckBox* ShowWaypointLinks;
    irr::gui::IGUICheckBox* ShowWaypointLinksSpace;
    irr::gui::IGUICheckBox* ShowCheckpoints;
    irr::gui::IGUICheckBox* ShowPOI;
    irr::gui::IGUICheckBox* ShowTriggerRegions;
    irr::gui::IGUICheckBox* LogTriggerEvents;
    irr::gui::IGUICheckBox* ActivateMorphKeyTrg;
    irr::gui::IGUICheckBox* ShowChargingStationInfo;

    //Movement Tab items
    irr::gui::IGUICheckBox* ShowCurrWayPointLink;
    irr::gui::IGUICheckBox* ShowPlayerActingForces;
    irr::gui::IGUICheckBox* ShowPlayerCPUCurrentSegment;
    irr::gui::IGUICheckBox* ShowPlayerCPUPathHistory;
    irr::gui::IGUICheckBox* ShowPlayerFreeSpace;
};

class GameDbgWnd {
private:
    void CreateWindow();

    bool mWindowHidden = true;

    Race* mParentRace = nullptr;
    irr::gui::IGUIWindow* Window = nullptr;
    irr::s32 mGuiWindowId = 1000;

public:
    GameDbgWnd(Race* parentRace);
    ~GameDbgWnd();

    void HideWindow();
    void OpenWindow();

    bool IsWindowVisible();

    void OnCheckBoxChanged(irr::s32 checkboxId);

    GUIGameDbgWnd mGuiGameDbgWnd;
};

#endif // GAMEDBGWND_H
