/*
 Copyright (C) 2026 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "gamedbgwnd.h"
#include "../race.h"
#include "../models/player.h"
#include "../game.h"
#include <iostream>

GameDbgWnd::GameDbgWnd(Race* parentRace) {
     mParentRace = parentRace;
}

GameDbgWnd::~GameDbgWnd() {
    //make sure my window is hidden at the end
    HideWindow();

    if (mGuiGameDbgWnd.ShowWallsegmentLines != nullptr) {
        mGuiGameDbgWnd.ShowWallsegmentLines->remove();
    }

    if (mGuiGameDbgWnd.ShowWallCollisionMesh != nullptr) {
        mGuiGameDbgWnd.ShowWallCollisionMesh->remove();
    }

    if (mGuiGameDbgWnd.ShowWaypointLinks != nullptr) {
        mGuiGameDbgWnd.ShowWaypointLinks->remove();
    }

    if (mGuiGameDbgWnd.ShowWaypointLinksSpace != nullptr) {
        mGuiGameDbgWnd.ShowWaypointLinksSpace->remove();
    }

    if (mGuiGameDbgWnd.ShowCheckpoints != nullptr) {
        mGuiGameDbgWnd.ShowCheckpoints->remove();
    }

    if (mGuiGameDbgWnd.ShowPOI != nullptr) {
        mGuiGameDbgWnd.ShowPOI->remove();
    }

    if (mGuiGameDbgWnd.ShowTriggerRegions != nullptr) {
        mGuiGameDbgWnd.ShowTriggerRegions->remove();
    }

    if (mGuiGameDbgWnd.LogTriggerEvents != nullptr) {
        mGuiGameDbgWnd.LogTriggerEvents->remove();
    }

    if (mGuiGameDbgWnd.ActivateMorphKeyTrg != nullptr) {
        mGuiGameDbgWnd.ActivateMorphKeyTrg->remove();
    }

    if (mGuiGameDbgWnd.ShowChargingStationInfo != nullptr) {
        mGuiGameDbgWnd.ShowChargingStationInfo->remove();
    }

    if (mGuiGameDbgWnd.ShowCurrWayPointLink != nullptr) {
        mGuiGameDbgWnd.ShowCurrWayPointLink->remove();
    }

    if (mGuiGameDbgWnd.ShowPlayerActingForces != nullptr) {
        mGuiGameDbgWnd.ShowPlayerActingForces->remove();
    }

    if (mGuiGameDbgWnd.ShowPlayerCPUCurrentSegment != nullptr) {
        mGuiGameDbgWnd.ShowPlayerCPUCurrentSegment->remove();
    }

    if (mGuiGameDbgWnd.ShowPlayerCPUPathHistory != nullptr) {
        mGuiGameDbgWnd.ShowPlayerCPUPathHistory->remove();
    }

    if (mGuiGameDbgWnd.ShowPlayerFreeSpace != nullptr) {
        mGuiGameDbgWnd.ShowPlayerFreeSpace->remove();
    }

    if (mGuiGameDbgWnd.ShowPlayerParallelWayPointLinks != nullptr) {
        mGuiGameDbgWnd.ShowPlayerParallelWayPointLinks->remove();
    }

    if (mGuiGameDbgWnd.TakeOverCpuControl != nullptr) {
        mGuiGameDbgWnd.TakeOverCpuControl->remove();
    }

    if (mGuiGameDbgWnd.LevelTab != nullptr) {
        mGuiGameDbgWnd.LevelTab->remove();
    }

    if (mGuiGameDbgWnd.MovementTab != nullptr) {
        mGuiGameDbgWnd.MovementTab->remove();
    }

    if (mGuiGameDbgWnd.tabCntrl != nullptr) {
        mGuiGameDbgWnd.tabCntrl->remove();
    }

    if (Window != nullptr) {
        //remove the window of this Mode object
        Window->remove();
    }
}

void GameDbgWnd::CreateWindow() {
    irr::core::dimension2d<irr::u32> dim ( 350, 250 );

    //finally create the window
    Window = mParentRace->mGame->mGuienv->addWindow ( rect<s32> ( 0, 0, dim.Width, dim.Height ), false, L"Debug", 0, mGuiWindowId);

    //lets create tabs in the debugging window
    mGuiGameDbgWnd.tabCntrl = mParentRace->mGame->mGuienv->addTabControl(
            core::rect<s32>(1, 20, dim.Width - 2 , dim.Height - 2), Window, true, true);

    mGuiGameDbgWnd.LevelTab = mGuiGameDbgWnd.tabCntrl->addTab(L"Level");
    mGuiGameDbgWnd.MovementTab = mGuiGameDbgWnd.tabCntrl->addTab(L"Movement");

    /********************************
     * Create the Level Tab items   *
     ********************************/

    irr::core::vector2di initialPos(15, 10);
    irr::core::vector2di pos = initialPos;
    irr::s32 width = 110;
    irr::s32 height = 25;
    irr::s32 columnSpacing = 20;

    bool currState = mParentRace->GetDebugFlag(DEF_RACE_DBG_WALLSEGMENTS);
    mGuiGameDbgWnd.ShowWallsegmentLines = mParentRace->mGame->mGuienv->addCheckBox(currState, rect<s32> ( pos.X, pos.Y, pos.X + width, pos.Y + height),
                                                                                         mGuiGameDbgWnd.LevelTab, GUI_ID_LEVEL_SHOWWALLSEGMENTLINES_CHECKBOX, L"Wallseg Lines");

    pos.Y += height;
    currState = mParentRace->GetDebugFlag(DEF_RACE_DBG_WALLCOLLISIONMESH);
    mGuiGameDbgWnd.ShowWallCollisionMesh = mParentRace->mGame->mGuienv->addCheckBox(currState, rect<s32> ( pos.X, pos.Y, pos.X + width, pos.Y + height),
                                                                                         mGuiGameDbgWnd.LevelTab, GUI_ID_LEVEL_SHOWWALLCOLLISIONMESH_CHECKBOX, L"Wallcoll Mesh");

    pos.Y += height;
    currState = mParentRace->GetDebugFlag(DEF_RACE_DBG_WAYPOINTLINKS);
    mGuiGameDbgWnd.ShowWaypointLinks = mParentRace->mGame->mGuienv->addCheckBox(currState, rect<s32> ( pos.X, pos.Y, pos.X + width, pos.Y + height),
                                                                                           mGuiGameDbgWnd.LevelTab, GUI_ID_LEVEL_SHOWWAYPOINTLINKS_CHECKBOX, L"Waypnt Links");

    pos.Y += height;
    currState = mParentRace->GetDebugFlag(DEF_RACE_DBG_WAYPOINTLINKSSPACE);
    mGuiGameDbgWnd.ShowWaypointLinksSpace = mParentRace->mGame->mGuienv->addCheckBox(currState, rect<s32> ( pos.X, pos.Y, pos.X + width, pos.Y + height),
                                                                                           mGuiGameDbgWnd.LevelTab, GUI_ID_LEVEL_SHOWWAYPOINTLINKSSPACE_CHECKBOX, L"Waypnt Link Space");

    pos.Y += height;
    currState = mParentRace->GetDebugFlag(DEF_RACE_DBG_CHECKPOINTS);
    mGuiGameDbgWnd.ShowCheckpoints = mParentRace->mGame->mGuienv->addCheckBox(currState, rect<s32> ( pos.X, pos.Y, pos.X + width, pos.Y + height),
                                                                                           mGuiGameDbgWnd.LevelTab, GUI_ID_LEVEL_SHOWCHECKPOINTS_CHECKBOX, L"Checkpoints");

    pos.Y += height;
    currState = mParentRace->GetDebugFlag(DEF_RACE_DBG_POI);
    mGuiGameDbgWnd.ShowPOI = mParentRace->mGame->mGuienv->addCheckBox(currState, rect<s32> ( pos.X, pos.Y, pos.X + width, pos.Y + height),
                                                                                           mGuiGameDbgWnd.LevelTab, GUI_ID_LEVEL_SHOWPOI_CHECKBOX, L"POI");

    pos.Y += height;
    currState = mParentRace->GetDebugFlag(DEF_RACE_DBG_TRIGGERREGIONS);
    mGuiGameDbgWnd.ShowTriggerRegions = mParentRace->mGame->mGuienv->addCheckBox(currState, rect<s32> ( pos.X, pos.Y, pos.X + width, pos.Y + height),
                                                                                           mGuiGameDbgWnd.LevelTab, GUI_ID_LEVEL_SHOWTRIGGERREGIONS_CHECKBOX, L"Trigger Regions");
    pos.X = initialPos.X + width + columnSpacing;
    pos.Y = initialPos.Y;
    currState = mParentRace->GetDebugFlag(DEF_RACE_DBG_LOGTRIGGEREVENTS);
    mGuiGameDbgWnd.LogTriggerEvents = mParentRace->mGame->mGuienv->addCheckBox(currState, rect<s32> ( pos.X, pos.Y, pos.X + width, pos.Y + height),
                                                                                           mGuiGameDbgWnd.LevelTab, GUI_ID_LEVEL_LOGTRIGGEREVENTS_CHECKBOX, L"Log Trigger Events");

    pos.Y += height;
    currState = mParentRace->GetDebugFlag(DEF_RACE_DBG_ACTIVATEMORPHKEYTRG);
    mGuiGameDbgWnd.ActivateMorphKeyTrg = mParentRace->mGame->mGuienv->addCheckBox(currState, rect<s32> ( pos.X, pos.Y, pos.X + width, pos.Y + height),
                                                                                           mGuiGameDbgWnd.LevelTab, GUI_ID_LEVEL_ACTIVATEMORPHKEYTRG_CHECKBOX, L"Ena Morph Key Trg");

    pos.Y += height;
    currState = mParentRace->GetDebugFlag(DEF_RACE_DBG_CHARGINGSTATIONINFO);
    mGuiGameDbgWnd.ShowChargingStationInfo = mParentRace->mGame->mGuienv->addCheckBox(currState, rect<s32> ( pos.X, pos.Y, pos.X + width, pos.Y + height),
                                                                                           mGuiGameDbgWnd.LevelTab, GUI_ID_LEVEL_SHOWCHARGINGSTATIONINFO_CHECKBOX, L"Charging Station");

    pos.Y += height;
    currState = mParentRace->GetDebugFlag(DEF_RACE_DBG_SHOWPARALLELWAYPOINTLINKS);
    mGuiGameDbgWnd.ShowPlayerParallelWayPointLinks = mParentRace->mGame->mGuienv->addCheckBox(currState, rect<s32> ( pos.X, pos.Y, pos.X + width, pos.Y + height),
                                                                                           mGuiGameDbgWnd.LevelTab, GUI_ID_LEVEL_SHOWPARALLELWAYPOINTLINKS_CHECKBOX, L"Parallel WayLinks");

    /***********************************
     * Create the Movement Tab items   *
     ***********************************/

    pos = initialPos;

    currState = mParentRace->GetPlayersDbgFlagState(DEF_PLAYER_DBG_CURRWAYPOINTLINK);
    mGuiGameDbgWnd.ShowCurrWayPointLink = mParentRace->mGame->mGuienv->addCheckBox(currState, rect<s32> ( pos.X, pos.Y, pos.X + width, pos.Y + height),
                                                                                         mGuiGameDbgWnd.MovementTab, GUI_ID_MOVEMENT_CURRWAYPOINTLINK_CHECKBOX, L"Curr Waypointlink");

    pos.Y += height;
    currState = mParentRace->GetPlayersDbgFlagState(DEF_PLAYER_DBG_ACTINGFORCES);
    mGuiGameDbgWnd.ShowPlayerActingForces = mParentRace->mGame->mGuienv->addCheckBox(currState, rect<s32> ( pos.X, pos.Y, pos.X + width, pos.Y + height),
                                                                                         mGuiGameDbgWnd.MovementTab, GUI_ID_MOVEMENT_ACTINGFORCES_CHECKBOX, L"Acting Forces");

    pos.Y += height;
    currState = mParentRace->GetPlayersDbgFlagState(DEF_PLAYER_DBG_CPU_CURRSEGMENT);
    mGuiGameDbgWnd.ShowPlayerCPUCurrentSegment = mParentRace->mGame->mGuienv->addCheckBox(currState, rect<s32> ( pos.X, pos.Y, pos.X + width, pos.Y + height),
                                                                                         mGuiGameDbgWnd.MovementTab, GUI_ID_MOVEMENT_CPU_CURRSEGMENT_CHECKBOX, L"CPU Current Seg");

    pos.Y += height;
    currState = mParentRace->GetPlayersDbgFlagState(DEF_PLAYER_DBG_CPU_PATHHISTORY);
    mGuiGameDbgWnd.ShowPlayerCPUPathHistory = mParentRace->mGame->mGuienv->addCheckBox(currState, rect<s32> ( pos.X, pos.Y, pos.X + width, pos.Y + height),
                                                                                         mGuiGameDbgWnd.MovementTab, GUI_ID_MOVEMENT_CPU_PATHHISTORY_CHECKBOX, L"CPU Path History");

    pos.Y += height;
    currState = mParentRace->GetPlayersDbgFlagState(DEF_PLAYER_DBG_FREESPACE);
    mGuiGameDbgWnd.ShowPlayerFreeSpace = mParentRace->mGame->mGuienv->addCheckBox(currState, rect<s32> ( pos.X, pos.Y, pos.X + width, pos.Y + height),
                                                                                         mGuiGameDbgWnd.MovementTab, GUI_ID_MOVEMENT_CPU_FREESPACE_CHECKBOX, L"Free Space");

    pos.X = initialPos.X + width + columnSpacing;
    pos.Y = initialPos.Y;
    currState = mParentRace->GetDebugFlag(DEF_RACE_DBG_TAKEOVERCPUCONTROL);
    mGuiGameDbgWnd.TakeOverCpuControl = mParentRace->mGame->mGuienv->addCheckBox(currState, rect<s32> ( pos.X, pos.Y, pos.X + width, pos.Y + height),
                                                                                           mGuiGameDbgWnd.MovementTab, GUI_ID_MOVEMENT_CPU_TAKEOVERCONTROL_CHECKBOX, L"Takeover CPU Control");



    //move window to a better start location
    Window->move(irr::core::vector2d<irr::s32>(250,50));
}

void GameDbgWnd::HideWindow() {
    if (Window == nullptr)
        return;

    Window->setEnabled(false);
    Window->setVisible(false);

    mWindowHidden = true;

    scene::ICameraSceneNode * camera =
            mParentRace->mGame->mDevice->getSceneManager()->getActiveCamera();

    if (camera) {
       camera->setInputReceiverEnabled(true);
    }
}

bool GameDbgWnd::IsWindowVisible() {
    return (!mWindowHidden);
}

void GameDbgWnd::OpenWindow() {
   if (Window == nullptr) {
       CreateWindow();
   } else if (mWindowHidden) {
       //make sure existing window is not
       //hidden
       //unhide the existing window
       Window->setEnabled(true);
       Window->setVisible(true);

       mWindowHidden = false;
   }

   scene::ICameraSceneNode * camera =
           mParentRace->mGame->mDevice->getSceneManager()->getActiveCamera();

   if (camera) {
      camera->setInputReceiverEnabled(false);
   }
}

void GameDbgWnd::OnCheckBoxChanged(irr::s32 checkboxId) {
    if (checkboxId == GUI_ID_LEVEL_SHOWWALLSEGMENTLINES_CHECKBOX) {
         mParentRace->SetDebugFlag(DEF_RACE_DBG_WALLSEGMENTS, mGuiGameDbgWnd.ShowWallsegmentLines->isChecked());
    } else if (checkboxId == GUI_ID_LEVEL_SHOWWALLCOLLISIONMESH_CHECKBOX) {
        mParentRace->SetDebugFlag(DEF_RACE_DBG_WALLCOLLISIONMESH, mGuiGameDbgWnd.ShowWallCollisionMesh->isChecked());
    } else if (checkboxId == GUI_ID_LEVEL_SHOWWAYPOINTLINKS_CHECKBOX) {
        mParentRace->SetDebugFlag(DEF_RACE_DBG_WAYPOINTLINKS, mGuiGameDbgWnd.ShowWaypointLinks->isChecked());
    } else if (checkboxId == GUI_ID_LEVEL_SHOWWAYPOINTLINKSSPACE_CHECKBOX) {
        mParentRace->SetDebugFlag(DEF_RACE_DBG_WAYPOINTLINKSSPACE, mGuiGameDbgWnd.ShowWaypointLinksSpace->isChecked());
    } else if (checkboxId == GUI_ID_LEVEL_SHOWCHECKPOINTS_CHECKBOX) {
        mParentRace->SetDebugFlag(DEF_RACE_DBG_CHECKPOINTS, mGuiGameDbgWnd.ShowCheckpoints->isChecked());
    } else if (checkboxId == GUI_ID_LEVEL_SHOWPOI_CHECKBOX) {
        mParentRace->SetDebugFlag(DEF_RACE_DBG_POI, mGuiGameDbgWnd.ShowPOI->isChecked());
    } else if (checkboxId == GUI_ID_LEVEL_SHOWTRIGGERREGIONS_CHECKBOX) {
        mParentRace->SetDebugFlag(DEF_RACE_DBG_TRIGGERREGIONS, mGuiGameDbgWnd.ShowTriggerRegions->isChecked());
    } else if (checkboxId == GUI_ID_LEVEL_LOGTRIGGEREVENTS_CHECKBOX) {
        mParentRace->SetDebugFlag(DEF_RACE_DBG_LOGTRIGGEREVENTS, mGuiGameDbgWnd.LogTriggerEvents->isChecked());
    } else if (checkboxId == GUI_ID_LEVEL_ACTIVATEMORPHKEYTRG_CHECKBOX) {
        mParentRace->SetDebugFlag(DEF_RACE_DBG_ACTIVATEMORPHKEYTRG, mGuiGameDbgWnd.ActivateMorphKeyTrg->isChecked());
    } else if (checkboxId == GUI_ID_LEVEL_SHOWPARALLELWAYPOINTLINKS_CHECKBOX) {
        mParentRace->SetDebugFlag(DEF_RACE_DBG_SHOWPARALLELWAYPOINTLINKS, mGuiGameDbgWnd.ShowPlayerParallelWayPointLinks->isChecked());
    } else if (checkboxId == GUI_ID_LEVEL_SHOWCHARGINGSTATIONINFO_CHECKBOX) {
        mParentRace->SetDebugFlag(DEF_RACE_DBG_CHARGINGSTATIONINFO, mGuiGameDbgWnd.ShowChargingStationInfo->isChecked());
    } else if (checkboxId == GUI_ID_MOVEMENT_CURRWAYPOINTLINK_CHECKBOX) {
        mParentRace->UpdatePlayersDbgFlag(DEF_PLAYER_DBG_CURRWAYPOINTLINK, mGuiGameDbgWnd.ShowCurrWayPointLink->isChecked());
    } else if (checkboxId == GUI_ID_MOVEMENT_ACTINGFORCES_CHECKBOX) {
        mParentRace->UpdatePlayersDbgFlag(DEF_PLAYER_DBG_ACTINGFORCES, mGuiGameDbgWnd.ShowPlayerActingForces->isChecked());
    } else if (checkboxId == GUI_ID_MOVEMENT_CPU_CURRSEGMENT_CHECKBOX) {
        mParentRace->UpdatePlayersDbgFlag(DEF_PLAYER_DBG_CPU_CURRSEGMENT, mGuiGameDbgWnd.ShowPlayerCPUCurrentSegment->isChecked());
    } else if (checkboxId == GUI_ID_MOVEMENT_CPU_PATHHISTORY_CHECKBOX) {
        mParentRace->UpdatePlayersDbgFlag(DEF_PLAYER_DBG_CPU_PATHHISTORY, mGuiGameDbgWnd.ShowPlayerCPUPathHistory->isChecked());
    } else if (checkboxId == GUI_ID_MOVEMENT_CPU_FREESPACE_CHECKBOX) {
        mParentRace->UpdatePlayersDbgFlag(DEF_PLAYER_DBG_FREESPACE, mGuiGameDbgWnd.ShowPlayerFreeSpace->isChecked());
    } else if (checkboxId == GUI_ID_MOVEMENT_CPU_TAKEOVERCONTROL_CHECKBOX) {
        mParentRace->SetDebugFlag(DEF_RACE_DBG_TAKEOVERCPUCONTROL, mGuiGameDbgWnd.TakeOverCpuControl->isChecked());
    }
}
