/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#ifndef COLLECTABLE_H
#define COLLECTABLE_H

#include "../resources/levelfile.h"
#include "../resources/mapentry.h"
#include "../race.h"

//preset values, also used in
//HiOctaneTools
const irr::f32 CollectableSize_w = 0.45f;
const irr::f32 CollectableSize_h = 0.45f;

//time in seconds after which a dynamically spawned entity/collectable
//disappears again, if it was not collected by any player until then
const irr::f32 DEF_TYPE2_COLLECTABLE_LIFETIME = 30.0f;

class Race; //Forward declaration

//Note 02.02.2025: In this project there are two different types of Collectables
// Type 1: Entities (Collectables) that are stored inside the original game map files, and always
// exist when creating a race; For this collectable object the EntityItem* pointer variable "mEntityItem" points to the
// EntityItem object created from the map file; In this case the type of entity Item is stored inside the mEntityItem object
//
// Type 2: Collectables that are temporarily spawned when a player craft breaks down; during the craft explosion additional
// collectables are spawned; For this second type of collectables the EntityItem* pointer variable "mEntityItem" is always nullptr, and
// the is no EntityItem object behind the collectable; This should also reflect the fact that this second type of temporary collectables
// are not stored in a map file, and can also not be save in a map file. To know which type of collectable it is in this case,
// there is an additioanl member variable

class Collectable {
public:
    //this constructor is for the first type of entity/collectable (which is created based on a game map file entity item)
    Collectable(Race* race, EntityItem* entityItem, vector3d<irr::f32> pos,
                irr::scene::ISceneManager* mSmgr, irr::video::IVideoDriver *driver);

    //this constructor is for the second type of entity/collectable (which is temporarily spawned when a player craft breaks down)
    Collectable(Race* race, Entity::EntityType type, vector3d<irr::f32> pos,
                irr::scene::ISceneManager* mSmgr, irr::video::IVideoDriver *driver);

    ~Collectable();

    vector3d<irr::f32> Position;

    irr::core::dimension2d<irr::u32> texturesize;
    vector3d<irr::f32> m_Size;

    irr::video::ITexture* collectable_tex = nullptr;

    irr::scene::IBillboardSceneNode *billSceneNode = nullptr;
    irr::core::aabbox3df boundingBox;

    bool GetIfVisible();
    void PickedUp();
    void Trigger();

    irr::f32 GetCollectableCenterHeight();

    EntityItem* mEntityItem = nullptr;

    //for the second type of (temporary spawned) collectable, the item type is not stored
    //in a EntityItem (is nullptr in this case), but instead in the member variable mEntityType below
    Entity::EntityType mEntityType = Entity::EntityType::Unknown;

    irr::core::vector2df GetMyBezierCurvePlaningCoord(irr::core::vector3df &threeDCoord);

    //this function must be used to get the type of collectable
    //because this function takes care of the two different types
    //of collectable possible
    Entity::EntityType GetCollectableType();

    //allows to update position if collectable
    //for example used by the collectablespawner
    //Important note: does NOT update the position of an
    //underlying entityItem, only useful for type 2 collectable!
    void UpdatePosition(irr::core::vector3df newPostion);

    void SetVisible(bool visible);

    //this function takes care that the dynamically spawned items
    //are removed again after a certain lifetime (if the were not yet picked
    //up by any player)
    void UpdateType2Collectable(irr::f32 deltaTime);

    //only used for type 2 collectable, if this function returns true
    //this item needs to be cleaned up now
    bool GetType2CollectableCleanUpNecessary();

private:
    Race* mRace = nullptr;

    bool mEnableLightning;

    void SetupSceneNode(Entity::EntityType type, irr::core::vector3df pos);

    //for type2 collectable (dynamically spawned) there seems to be
    //a lifetime in game after which the item disappears again, if it was
    //not yet collected; not used for type 1 collectable (contained in level file itself)
    irr::f32 remainingLifeTime = DEF_TYPE2_COLLECTABLE_LIFETIME;

protected:
    irr::video::IVideoDriver* m_driver = nullptr;
    irr::scene::ISceneManager *m_smgr = nullptr;

    //default is non visible after
    //start of game, and before entity group
    //1 is triggered (group 1 is triggered once
    //at game start to make initial collectables visible)
    bool isVisible;
};

#endif // COLLECTABLE_H
