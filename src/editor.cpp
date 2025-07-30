/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

//Note: Routine "setSkinTransparency" inside this Cpp file was taken from Irrlicht engine
//"Example 009 Mesh Viewer" source code file, and was not written by myself. But I do not know
//who developed this routine in the first place.

#include "editor.h"
#include "draw/gametext.h"
#include "editorsession.h"
#include "utils/logging.h"
#include "utils/logger.h"
#include "utils/tprofile.h"
#include "editor/itemselector.h"
#include "models/levelterrain.h"
#include "models/levelblocks.h"
#include "editor/texturemode.h"
#include "editor/columndesign.h"

//fully initializes the remaining editor
//components
//before calling this function we need to be sure that all
//original game files were already extracted before and are
//available
bool Editor::InitEditorStep2() {
    if (DebugShowVariableBoxes) {
            //only for debugging
            dbgTimeProfiler = mGuienv->addStaticText(L"Location",
                   rect<s32>(100,150,300,200), false, true, nullptr, -1, true);

            dbgText = mGuienv->addStaticText(L"",
                   rect<s32>(100,250,300,350), false, true, nullptr, -1, true);

            /*dbgText2 = guienv->addStaticText(L"",
                   rect<s32>(350,200,450,300), false, true, nullptr, -1, true);*/
    }

    if (enableLightning) {
        mSmgr->addLightSceneNode(0, vector3df(0, 100, 100),
            video::SColorf(1.0f, 1.0f, 1.0f), 1000.0f, -1);
    }

    //set a minimum amount of light everywhere, to not have black areas
    //in the level
    if (enableLightning) {
       mSmgr->setAmbientLight(video::SColorf(0.4f, 0.4f, 0.4f));
    } else {
        //set max brightness everywhere
        mSmgr->setAmbientLight(video::SColorf(1.0f, 1.0f, 1.0f));
    }

    if (enableShadows) {
       mSmgr->setShadowColor(video::SColor(150,0,0,0));
    }

    //set Gui dialog transparency
    //between 0 and 255
    setSkinTransparency(200, mGuienv->getSkin());

    return true;
}

//creates the most basic game infrastructure, and
//extracts basic things to be able to show a first
//graphical screen
bool Editor::InitEditorStep1() {
    dimension2d<u32> targetResolution;

    //set target screen resolution
    //targetResolution.set(640,480);
    targetResolution.set(1280,960);

    //initialize my infrastructure
    this->InfrastructureInit(targetResolution, fullscreen, enableShadows);
    if (!GetInfrastructureInitOk())
        return false;

    //load the background image we need
    //for data extraction screen rendering and
    //main menue
    if (!LoadBackgroundImage()) {
        return false;
    }

    return true;
}

void Editor::RunEditor() {
    //further game data needs to be extracted?
    if (!mPrepareData->GameDataAvailable()) {
        mEditorState = DEF_EDITORSTATE_EXTRACTDATA;
    } else {
        if (!InitEditorStep2()) {
            mEditorState = DEF_EDITORSTATE_ERROR;
        } else {
            CreateMenue();
            mEditorState = DEF_EDITORSTATE_LOADDATA;
        }
    }

    EditorLoop();
}

void Editor::CreateMenue() {
    // create menu
    gui::IGUIContextMenu* menu = mGuienv->addMenu();
    menu->addItem(L"File", -1, true, true);
    menu->addItem(L"Edit", -1, true, true);
    menu->addItem(L"Mode", -1, true, true);
    menu->addItem(L"View", -1, true, true);

    /*************************************
     * Submenue File                     *
     *************************************/

    gui::IGUIContextMenu* submenu;
    submenu = menu->getSubMenu(0);
    submenu->addItem(L"Open level", GUI_ID_OPEN_LEVEL);
    submenu->addItem(L"Save level", GUI_ID_SAVE_LEVEL);

    //submenu->addItem(L"Set Model Archive...", GUI_ID_SET_MODEL_ARCHIVE);
    //submenu->addItem(L"Load as Octree", GUI_ID_LOAD_AS_OCTREE);
    submenu->addSeparator();
    submenu->addItem(L"Quit", GUI_ID_QUIT);

    /*************************************
     * Mode View                         *
     *************************************/

    submenu = menu->getSubMenu(2);
    submenu->addItem(L"View", GUI_ID_MODE_VIEW, true, false);
    submenu->addItem(L"Texturing", GUI_ID_MODE_TEXTURING, true, false);
    submenu->addItem(L"Column Design", GUI_ID_MODE_COLUMNDESIGN, true, false);

    /*************************************
     * Submenue View                     *
     *************************************/

    submenu = menu->getSubMenu(3);
    submenu->addItem(L"Terrain", GUI_ID_VIEWMODE_TERRAIN, true, true);
    submenu->addItem(L"Blocks", GUI_ID_VIEWMODE_BLOCKS, true, true);

    submenu = menu->getSubMenu(3)->getSubMenu(0);
    submenu->addItem(L"Off", GUI_ID_VIEW_TERRAIN_OFF);
    submenu->addItem(L"Wireframe", GUI_ID_VIEW_TERRAIN_WIREFRAME);
    submenu->addItem(L"Default", GUI_ID_VIEW_TERRAIN_DEFAULT);
    submenu->addItem(L"Normals", GUI_ID_VIEW_TERRAIN_NORMALS);

    submenu = menu->getSubMenu(3)->getSubMenu(1);
    submenu->addItem(L"Off", GUI_ID_VIEW_BLOCKS_OFF);
    submenu->addItem(L"Wireframe", GUI_ID_VIEW_BLOCKS_WIREFRAME);
    submenu->addItem(L"Default", GUI_ID_VIEW_BLOCKS_DEFAULT);
    submenu->addItem(L"Normals", GUI_ID_VIEW_BLOCKS_NORMALS);

    // add a status line help text
    StatusLine = mGuienv->addStaticText( 0, rect<s32>( 5,  mScreenRes.Height - 30,  mScreenRes.Width - 5, mScreenRes.Height - 10),
                                false, false, 0, -1, true);
}

//Routine setSkinTransparency taken from Irrlicht engine
//"Example 009 Mesh Viewer"
void Editor::setSkinTransparency(s32 alpha, irr::gui::IGUISkin * skin)
{
    for (s32 i=0; i<irr::gui::EGDC_COUNT ; ++i)
    {
        video::SColor col = skin->getColor((EGUI_DEFAULT_COLOR)i);
        col.setAlpha(alpha);
        skin->setColor((EGUI_DEFAULT_COLOR)i, col);
    }
}

void Editor::ChangeViewModeTerrain(irr::u8 newViewMode) {
    if (mCurrentSession != nullptr) {
        if (mCurrentSession->mLevelTerrain != nullptr) {
           mCurrentSession->mLevelTerrain->SetViewMode(newViewMode);
        }
    }
}

void Editor::ChangeViewModeBlocks(irr::u8 newViewMode) {
    if (mCurrentSession != nullptr) {
        if (mCurrentSession->mLevelBlocks != nullptr) {
           mCurrentSession->mLevelBlocks->SetViewMode(newViewMode);
        }
    }
}

void Editor::OnMenuItemSelected( IGUIContextMenu* menu )
{
    s32 id = menu->getItemCommandId(menu->getSelectedItem());

    switch(id)
    {
     /*   case GUI_ID_OPEN_MODEL: // FilOnButtonSetScalinge -> Open Model
            env->addFileOpenDialog(L"Please select a model file to open");
            break;
        case GUI_ID_SET_MODEL_ARCHIVE: // File -> Set Model Archive
            env->addFileOpenDialog(L"Please select your game archive/directory");
            break;
        case GUI_ID_LOAD_AS_OCTREE: // File -> LoadAsOctree
            Octree = !Octree;
            menu->setItemChecked(menu->getSelectedItem(), Octree);
            break;*/

        case GUI_ID_MODE_VIEW: {
           if (mCurrentSession != nullptr) {
               mCurrentSession->SetMode((EditorMode*)mCurrentSession->mViewMode);
           }
           break;
        }

        case GUI_ID_MODE_TEXTURING: {
           if (mCurrentSession != nullptr) {
               mCurrentSession->SetMode((EditorMode*)mCurrentSession->mTextureMode);
           }
           break;
        }

        case GUI_ID_MODE_COLUMNDESIGN: {
           if (mCurrentSession != nullptr) {
               mCurrentSession->SetMode((EditorMode*)mCurrentSession->mColumnDesigner);
           }
           break;
        }

        case GUI_ID_VIEW_TERRAIN_OFF:  {
           ChangeViewModeTerrain(LEVELTERRAIN_VIEW_OFF);
           break;
        }

        case GUI_ID_VIEW_TERRAIN_WIREFRAME:  {
           ChangeViewModeTerrain(LEVELTERRAIN_VIEW_WIREFRAME);
           break;
        }

        case GUI_ID_VIEW_TERRAIN_DEFAULT:  {
           ChangeViewModeTerrain(LEVELTERRAIN_VIEW_DEFAULT);
           break;
        }

        case GUI_ID_VIEW_TERRAIN_NORMALS:  {
           ChangeViewModeTerrain(LEVELTERRAIN_VIEW_DEBUGNORMALS);
           break;
        }

        case GUI_ID_VIEW_BLOCKS_OFF:  {
           ChangeViewModeBlocks(LEVELBLOCKS_VIEW_OFF);
           break;
        }

        case GUI_ID_VIEW_BLOCKS_WIREFRAME:  {
           ChangeViewModeBlocks(LEVELBLOCKS_VIEW_WIREFRAME);
           break;
        }

        case GUI_ID_VIEW_BLOCKS_DEFAULT:  {
           ChangeViewModeBlocks(LEVELBLOCKS_VIEW_DEFAULT);
           break;
        }

        case GUI_ID_VIEW_BLOCKS_NORMALS:  {
           ChangeViewModeBlocks(LEVELBLOCKS_VIEW_DEBUGNORMALS);
           break;
        }

        case GUI_ID_SAVE_LEVEL: {
            if (mCurrentSession != nullptr) {
                //mCurrentSession->mLevelRes->Save("mlevel0-1.dat");
                mCurrentSession->mLevelRes->Save("/home/wolfalex/hi/maps/level0-1.dat");
            }
            break;
        }

        case GUI_ID_QUIT: // File -> Quit
            ExitEditor = true;
            break;
    }
}

void Editor::OnButtonClicked(irr::s32 buttonId) {
    //depending over which window the user mouse cursor
    //is currently, sent the button click Id to the correct
    //window
    if (mCurrentSession == nullptr)
        return;

    if (mCurrentSession->mUserInDialogState == DEF_EDITOR_USERINTEXTUREDIALOG) {
        mCurrentSession->mTextureMode->OnButtonClicked(buttonId);
    }

    if (mCurrentSession->mUserInDialogState == DEF_EDITOR_USERINCOLUMNDESIGNERDIALOG) {
        mCurrentSession->mColumnDesigner->OnButtonClicked(buttonId);
    }
}

void Editor::OnScrollbarMoved(irr::s32 scrollBarId) {
   //switch (scrollBarId) {
     /*  case GUI_ID_SCROLLBAR: {
           std::cout << "scrollbar" << std::endl;
           break;
       }*/
   //}
}

void Editor::OnElementFocused(irr::s32 elementId) {
  //std::cout << "Element Focus " << elementId << std::endl;
}

void Editor::OnElementHovered(irr::s32 elementId) {
  //std::cout << "Element Hovered " << elementId << std::endl;

    if (mCurrentSession != nullptr) {
          if (mCurrentSession->mEditorMode != nullptr) {
                  mCurrentSession->mEditorMode->OnElementHovered(elementId);
          }
     }
}

void Editor::OnElementLeft(irr::s32 elementId) {
  //std::cout << "Element Left " << elementId << std::endl;
  if (mCurrentSession != nullptr) {
       if (mCurrentSession->mEditorMode != nullptr) {
            mCurrentSession->mEditorMode->OnElementLeft(elementId);
       }
  }
}

//if function returns true the close action should be interrupted
bool Editor::OnElementClose(irr::s32 elementId) {
  //std::cout << "Element Close " << elementId << std::endl;

  //prevent that user can close the editor windows
  //otherwise the program will crash the next time we need the
  //window again; just hide the window and interrupt the close call
  if (mCurrentSession != nullptr) {
      if (mCurrentSession->mEditorMode != nullptr) {
            mCurrentSession->mEditorMode->HideWindow();

            return true;
      }
  }

  return false;
}

void Editor::OnComboBoxChanged(IGUIComboBox* comboBox) {
  u32 val = comboBox->getItemData ( comboBox->getSelected() );
  //std::cout << "ComboBox changed " << val << std::endl;

  if (comboBox->getID() == GUI_ID_TEXCATEGORYCOMBOBOX) {
    mCurrentSession->mTextureMode->TextureCategoryChanged(val);
  }

  if (comboBox->getID() == GUI_ID_TEXMODIFICATIONCOMBOBOX) {
    mCurrentSession->mTextureMode->TextureModificationChanged(val);
  }
}

void Editor::OnLeftMouseButtonDown() {
    MouseState.LeftButtonDown = true;

    //did the user select the currently highlighted
    //object? tell itemSelector that the user has clicked
    //with the left mouse button
    if (mCurrentSession != nullptr) {
      if (mCurrentSession->mItemSelector != nullptr) {
          mCurrentSession->mItemSelector->OnLeftMouseButtonDown();
      }

      if (mCurrentSession->mEditorMode != nullptr) {
          mCurrentSession->mEditorMode->OnLeftMouseButtonDown();
      }
    }
}

void Editor::OnLeftMouseButtonUp() {
    MouseState.LeftButtonDown = false;
}

//overwrite HandleMouseEvent method for Editor
void Editor::HandleMouseEvent(const irr::SEvent& event) {
    switch(event.MouseInput.Event)
    {
      case EMIE_LMOUSE_PRESSED_DOWN:
            OnLeftMouseButtonDown();
            break;

      case EMIE_LMOUSE_LEFT_UP:
            OnLeftMouseButtonUp();
            break;

      case EMIE_MOUSE_MOVED:
            MouseState.Position.X = event.MouseInput.X;
            MouseState.Position.Y = event.MouseInput.Y;
            break;

      default:
            // We won't use the wheel
            break;
      }
}

//overwrite HandleGuiEvent method for Editor
//returns true if Gui Event should be canceled
bool Editor::HandleGuiEvent(const irr::SEvent& event) {
    irr::s32 id = event.GUIEvent.Caller->getID();

    switch(event.GUIEvent.EventType)
    {
        case EGET_MENU_ITEM_SELECTED: {
          // a menu item was clicked
          OnMenuItemSelected( (IGUIContextMenu*)event.GUIEvent.Caller );
          break;
        }

        case EGET_BUTTON_CLICKED: {
            // a button was clicked
            OnButtonClicked(id);
            break;
        }

        case EGET_SCROLL_BAR_CHANGED: {
            //a scrollbar was moved
            OnScrollbarMoved(id);
            break;
        }

        case EGET_ELEMENT_FOCUSED: {
            //an element got the focus
            OnElementFocused(id);
            break;
        }

        case EGET_ELEMENT_HOVERED: {
            //user hovered over an element
            OnElementHovered(id);
            break;
        }

        case EGET_ELEMENT_LEFT : {
            //user left an element
            OnElementLeft(id);
            break;
        }

        case EGET_COMBO_BOX_CHANGED: {
            //user changed a combobox selection
            OnComboBoxChanged((IGUIComboBox*)event.GUIEvent.Caller);
            break;
        }

         case EGET_ELEMENT_CLOSED: {
            //user tried to close an Ui element/window
            if (OnElementClose(id)) {
                //we want to prevent closing this element
                return true;
            }
        }

        default: {
                break;
        }
     }

    //we do not want to cancel this event
    return false;
}

bool Editor::LoadAdditionalGameImages() {
     mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

     //load gameTitle
     gameTitle = mDriver->getTexture("extract/images/title.png");

     if (gameTitle == nullptr) {
         //there was a texture loading error
         //just return with false
         return false;
     }

     gameTitleSize = gameTitle->getSize();
     //calculate position to draw gameTitle so that it is centered on the screen
     //because most likely target resolution does not fit with image resolution
     gameTitleDrawPos.X = (mScreenRes.Width - gameTitleSize.Width) / 2;
     gameTitleDrawPos.Y = (mScreenRes.Height - gameTitleSize.Height) / 2;

     //load race loading screen
     raceLoadingScr = mDriver->getTexture("extract/images/onet0-1.png");

     if (raceLoadingScr == nullptr) {
         //there was a texture loading error
         //just return with false
         return false;
     }

     raceLoadingScrSize = raceLoadingScr->getSize();
     //calculate position to draw race loading screen so that it is centered on the screen
     //because maybe target resolution does not fit with image resolution
     raceLoadingScrDrawPos.X = (mScreenRes.Width - raceLoadingScrSize.Width) / 2;
     raceLoadingScrDrawPos.Y = (mScreenRes.Height - raceLoadingScrSize.Height) / 2;

     mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

     return true;
}

bool Editor::LoadBackgroundImage() {
    mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

    //first load background image for menue
    backgnd = mDriver->getTexture("extract/images/oscr0-1.png");

    if (backgnd == nullptr) {
        //there was a texture loading error
        //just return with false
        return false;
    }

    irr::core::dimension2d<irr::u32> backgndSize;

    backgndSize = backgnd->getSize();
   /* if ((backgndSize.Width != mScreenRes.Width) ||
        (backgndSize.Height != mScreenRes.Height)) {
        logging::Error("Background image does not fit with the selected screen resolution");
        //background texture size does not fit with selected screen resolution
        return false;
    }*/

    mDriver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, true);

    return true;
}

void Editor::RenderDataExtractionScreen() {
    //first draw background picture
     mDriver->draw2DImage(backgnd, irr::core::vector2di(0, 0),
                         irr::core::recti(0, 0, mScreenRes.Width, mScreenRes.Height)
                         , 0, irr::video::SColor(255,255,255,255), true);

     char* infoText = strdup("EXTRACTING GAME DATA, PLEASE STANDBY...");
     char* currStepText = strdup(mPrepareData->currentStepDescription.c_str());

     irr::u32 textHeight = mGameTexts->GetHeightPixelsGameText(infoText, mGameTexts->GameMenueWhiteTextSmallSVGA);
     irr::u32 textWidth = mGameTexts->GetWidthPixelsGameText(infoText, mGameTexts->GameMenueWhiteTextSmallSVGA);

     irr::core::position2di txtPos;
     txtPos.X = mScreenRes.Width / 2 - textWidth / 2;
     txtPos.Y = mScreenRes.Height / 2 - textHeight / 2;

     //write info text, warning: at the data extraction stage of the game only single font GameMenueWhiteTextSmallSVGA is
     //available!
     mGameTexts->DrawGameText(infoText, mGameTexts->GameMenueWhiteTextSmallSVGA, txtPos);

     textHeight = mGameTexts->GetHeightPixelsGameText(currStepText, mGameTexts->GameMenueWhiteTextSmallSVGA);
     textWidth = mGameTexts->GetWidthPixelsGameText(currStepText, mGameTexts->GameMenueWhiteTextSmallSVGA);

     txtPos.X = mScreenRes.Width / 2 - textWidth / 2;
     txtPos.Y += 25;

     mGameTexts->DrawGameText(currStepText, mGameTexts->GameMenueWhiteTextSmallSVGA, txtPos);

     free(infoText);
     free(currStepText);
}

void Editor::EditorLoopExtractData() {
    //execute next data extraction step
    //returns true if data extraction process is finished
    try {
        if (mPrepareData->ExecuteNextStep()) {
            //data extraction is finished
            //continue editor initialization
            if (!InitEditorStep2()) {
                mEditorState = DEF_EDITORSTATE_ERROR;
                return;
            }
            mEditorState = DEF_EDITORSTATE_LOADDATA;
            return;
        }
    }
     catch (const std::string &msg) {
         std::string msgExt("Step 2 of game assets preparation operation failed: ");
         msgExt.append(msg);
         logging::Error(msgExt);
         mEditorState = DEF_EDITORSTATE_ERROR;
         return;
    }

    mDriver->beginScene(true,true,
    video::SColor(255,100,101,140));

    //update graphical image about current status
    RenderDataExtractionScreen();

    mDriver->endScene();
}
/*
void Game::GameLoopTitleScreenLoadData() {
    //we need to load additional images
    if (!LoadAdditionalGameImages()) {
        logging::Error("Loading of game tile and race loading images failed");
        mGameState = DEF_GAMESTATE_ERROR;
        return;
    }

    mInfra->mDriver->beginScene(true,true,
    video::SColor(255,100,101,140));

    //first draw a black rectangle over the whole screen to make sure that the parts of the
    //screen that are outside of the drawn image regions are black as well
    mInfra->mDriver->draw2DRectangle(irr::video::SColor(255,0,0,0),
                   irr::core::rect<irr::s32>(0, 0, mInfra->mScreenRes.Width, mInfra->mScreenRes.Height));

    //draw game tile screen
    mInfra->mDriver->draw2DImage(gameTitle, gameTitleDrawPos, irr::core::recti(0, 0,
                     gameTitleSize.Width, gameTitleSize.Height)
                     , 0, irr::video::SColor(255,255,255,255), true);

    mInfra->mDriver->endScene();

    //now load data
    if (!LoadGameData()) {
        mGameState = DEF_GAMESTATE_ERROR;
        return;
    } else {
        if (!mDebugRace && !mDebugDemoMode) {
            //was succesfull, now continue to main menue
            mGameState = DEF_GAMESTATE_MENUE;
            MainMenue->ShowMainMenue();
        } else if (mDebugRace) {
            //we want to directly create a race for debugging
            //of game mechanics and enter it
            SetupDebugGame();
        } else if (mDebugDemoMode) {
            //we want to directly create a demo for debugging
            SetupDebugDemo();
        }
    }
}

void Game::GameLoopLoadRaceScreen() {
    mInfra->mDriver->beginScene(true,true,
    video::SColor(255,100,101,140));

    //first draw a black rectangle over the whole screen to make sure that the parts of the
    //screen that are outside of the drawn image regions are black as well
    mInfra->mDriver->draw2DRectangle(irr::video::SColor(255,0,0,0),
                   irr::core::rect<irr::s32>(0, 0, mInfra->mScreenRes.Width, mInfra->mScreenRes.Height));

    //draw load race screen
    mInfra->mDriver->draw2DImage(raceLoadingScr, raceLoadingScrDrawPos, irr::core::recti(0, 0, raceLoadingScrSize.Width, raceLoadingScrSize.Height)
                     , 0, irr::video::SColor(255,255,255,255), true);

    //at 190, 240 write "LOADING LEVEL"
    mInfra->mGameTexts->DrawGameText((char*)("LOADING LEVEL"), mInfra->mGameTexts->HudWhiteTextBannerFont, irr::core::position2di(190, 240));

    mInfra->mDriver->endScene();

    if (mGameState == DEF_GAMESTATE_INITRACE) {
        //player wants to start the race
        mPilotsNextRace = mGameAssets->GetPilotInfoNextRace(true, mGameAssets->GetComputerPlayersEnabled());

        if (this->CreateNewRace(nextRaceLevelNr, mPilotsNextRace, false, mDebugRace)) {
             mGameState = DEF_GAMESTATE_RACE;
             CleanupPilotInfo(mPilotsNextRace);
        } else {
            CleanupPilotInfo(mPilotsNextRace);

            mGameState = DEF_GAMESTATE_MENUE;

            //there was an error while creating the race
            //Go back to top of main menue
            MainMenue->ShowMainMenue();
        }
    } else if (mGameState == DEF_GAMESTATE_INITDEMO) {
        //for the demo do not add a human player, but add computer players
        mPilotsNextRace = mGameAssets->GetPilotInfoNextRace(false, true);

        if (this->CreateNewRace(nextRaceLevelNr, mPilotsNextRace, true, mDebugRace)) {
             mGameState = DEF_GAMESTATE_RACE;
             CleanupPilotInfo(mPilotsNextRace);
        } else {
            CleanupPilotInfo(mPilotsNextRace);

            mGameState = DEF_GAMESTATE_MENUE;

            //there was an error while creating the race
            //Go back to top of main menue
            MainMenue->ShowMainMenue();
        }
    }
}*/

void Editor::UpdateStatusbarText(const wchar_t *text) {
    wcscpy(mCurrentStatusBarText, text);

    if ( StatusLine != nullptr ) {
          StatusLine->setText(mCurrentStatusBarText);
    }
}

void Editor::EditorLoopSession(irr::f32 frameDeltaTime) {

    mTimeProfiler->StartOfGameLoop();

    mCurrentSession->HandleBasicInput();
    mCurrentSession->TrackActiveDialog();

    mTimeProfiler->Profile(mTimeProfiler->tIntHandleInput);

    //Update Time Profiler results
    mTimeProfiler->UpdateWindow();

    if (DebugShowVariableBoxes) {

            wchar_t* text2 = new wchar_t[400];

            swprintf(text2, 390, L"");

            /*if (mCurrentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_CELL) {
                swprintf(text2, 390, L"Cell X = %d Y = %d  %u", this->mCurrentSession->mItemSelector->mCurrSelectedItem.mCellCoordSelected.X, this->mCurrentSession->mItemSelector->mCurrSelectedItem.mCellCoordSelected.Y,
                         this->mCurrentSession->mItemSelector->columTrianglesHitCnt);
            } else if (mCurrentSession->mItemSelector->mCurrSelectedItem.SelectedItemType == DEF_EDITOR_SELITEM_BLOCK) {
                swprintf(text2, 390, L"Blocks %d %u", this->mCurrentSession->mItemSelector->mCurrSelectedItem.mSelBlockNrStartingFromBase, this->mCurrentSession->mItemSelector->columTrianglesHitCnt);
            } else {
               swprintf(text2, 390, L"%u", this->mCurrentSession->mItemSelector->columTrianglesHitCnt);
            }*/

            dbgText->setText(text2);

            delete[] text2;
    }

   /* if (mCurrentSession->UpdateBlockPreview) {
        mCurrentSession->UpdateBlockPreview = false;
        mCurrentSession->mLevelBlocks->CreateBlockPreview();
    }*/

    mDriver->beginScene(true,true,
     video::SColor(255,100,101,140));

    //render scene: terrain, blocks, player craft, entities...
    mCurrentSession->Render();

    mSmgr->drawAll();

    mTimeProfiler->Profile(mTimeProfiler->tIntRender3DScene);

    //render log window
    mLogger->Render();

    //last draw text debug output from Irrlicht
    mGuienv->drawAll();

    mTimeProfiler->Profile(mTimeProfiler->tIntRender2D);

    mDriver->endScene();

    //does the player want to end the race?
    if (mCurrentSession->exitEditorSession) {
        mCurrentSession->End();

        //clean up current editor session data
        delete mCurrentSession;
        mCurrentSession = nullptr;

        ExitEditor = true;

//        //if we were in game debugging mode simply skip
//        //main menue, and exit game immediately
//        if (mDebugRace || mDebugDemoMode) {
//            ExitGame = true;
//        } else {
//            mGameState = DEF_GAMESTATE_MENUE;

//            if (this->lastRaceStat != nullptr) {
//                //Show race statistics
//                MainMenue->ShowRaceStats(lastRaceStat);
//            } else {
//                //there are no race statistics to show
//                //simply return to the main menue
//                MainMenue->ShowMainMenue();
//            }
//        }
    }
}

void Editor::EditorLoop() {

    // In order to do framerate independent movement, we have to know
    // how long it was since the last frame
    u32 then = mDevice->getTimer()->getTime();

    while (mDevice->run() && (ExitEditor == false)) {
       /*  if (device->isWindowActive())
            {*/

        //Work out a frame delta time.
        const u32 now = mDevice->getTimer()->getTime();
        f32 frameDeltaTime = (f32)(now - then) / 1000.f; // Time in seconds
        then = now;

        switch (mEditorState) {
            case DEF_EDITORSTATE_EXTRACTDATA: {
                EditorLoopExtractData();
                break;
            }

            case DEF_EDITORSTATE_LOADDATA: {
                if (!CreateNewEditorSession(1)) {
                    mEditorState = DEF_EDITORSTATE_ERROR;
                } else {
                    mEditorState = DEF_EDITORSTATE_SESSIONACTIVE;
                }
                break;
            }

          /*  //shows game title, loads game data
            case DEF_GAMESTATE_GAMETITLE: {
                   GameLoopTitleScreenLoadData();
                   break;
            }

            case DEF_GAMESTATE_INITDEMO:
            case DEF_GAMESTATE_INITRACE: {
                   GameLoopLoadRaceScreen();
                   break;
            }*/

            case DEF_EDITORSTATE_SESSIONACTIVE: {
                EditorLoopSession(frameDeltaTime);
                break;
            }

            case DEF_EDITORSTATE_ERROR: {
                //there was an error, exit game
                ExitEditor = true;
                break;
            }
        }

        int fps = mDriver->getFPS();

        if (lastFPS != fps) {
                         core::stringw tmp(L"Hi-Editor [");
                         tmp += mDriver->getName();
                         tmp += L"] Triangles drawn: ";
                         tmp += mDriver->getPrimitiveCountDrawn();
                         tmp += " @ fps: ";
                         tmp += fps;

                         mDevice->setWindowCaption(tmp.c_str());
                         lastFPS = fps;
                     }
            //}
   }

   mDriver->drop();
}

bool Editor::CreateNewEditorSession(int load_levelnr) {
    if (mCurrentSession != nullptr)
        return false;

    //create a new editor session
    mCurrentSession = new EditorSession(this, load_levelnr);

    mCurrentSession->Init();

    if (!mCurrentSession->ready) {
        //there was a problem with EditorSession initialization
        logging::Error("EditorSession creation failed!");
        return false;
    }

    return true;
}

Editor::Editor() {
    //allocate memory for current editor statusbar text
    mCurrentStatusBarText = new wchar_t[400];
    swprintf(mCurrentStatusBarText, 390, L"");
}

Editor::~Editor() {
    //cleanup background images
    if (backgnd != nullptr) {
        backgnd->drop();
        backgnd = nullptr;
    }

    if (gameTitle != nullptr) {
        gameTitle->drop();
        gameTitle = nullptr;
    }

    if (raceLoadingScr != nullptr) {
        raceLoadingScr->drop();
        raceLoadingScr = nullptr;
    }

    if (mCurrentStatusBarText != nullptr) {
           delete[] mCurrentStatusBarText;
    }
}
