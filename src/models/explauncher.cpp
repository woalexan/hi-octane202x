#include "explauncher.h"

//Returns true in case of success
//False otherwise
bool ExplosionLauncher::LoadSprites() {
    //get the explosion sprites

    irr::video::ITexture* newTex;
    //char fileName[50];
    //char fname[20];

    //there are 12 explosion sprites to be loaded
    //for the explosion animation
    for (long i = 0; i < 13; i++) {

        /*strcpy(fileName, "extract/sprites/tmaps");
        sprintf (fname, "%0*lu.png", 4, i);
        strcat(fileName, fname);*/

        //newTex = mDriver->getTexture(fileName);
        //get pointer to preloaded texture
        newTex = this->mParentRace->mTexLoader->spriteTex.at(i);

        if (newTex == NULL)
            return false;

        animTexList.push_back(newTex);
    }

    return true;
}

void ExplosionLauncher::Trigger(irr::core::vector3df targetLocation) {

    Explosion* newExplosion = new Explosion(targetLocation, this);

    this->mCurrentExplosionVec.push_back(newExplosion);
}

void ExplosionLauncher::Update(irr::f32 DeltaTime) {

   std::vector <Explosion*>::iterator it;

   if (mCurrentExplosionVec.size() > 0 ) {

    //search for missiles that have exploded already and should be deleted
    //erase this missile from my list of current missiles vector
   /* for (it = this->mCurrentMissilesVec.begin(); it != this->mCurrentMissilesVec.end();) {
       if ((*it)->objToBeDeleted) {
            mPntr = (*it);
            it = mCurrentMissilesVec.erase(it);

            //delete the missile object itself
            delete mPntr;
            mPntr = NULL;
       } else ++it;
    }*/
   }

   if (mCurrentExplosionVec.size() > 0 ) {
     //update all my current active explosions
    for (it = this->mCurrentExplosionVec.begin(); it != this->mCurrentExplosionVec.end(); ++it) {
        (*it)->UpdateDetonations(DeltaTime);
        (*it)->UpdateDebris(DeltaTime);
    }
   }

    /*if (mShotSound != NULL) {
        if (mShotSound->getStatus() == mShotSound->Stopped) {
            mShotSound = NULL;
        }
    }*/
}

ExplosionLauncher::ExplosionLauncher(Race* myParentRace, irr::scene::ISceneManager* smgr, irr::video::IVideoDriver *driver) {
    mParentRace = myParentRace;
    mSmgr = smgr;
    mDriver = driver;

    ready = true;

    if (!LoadSprites()) {
        ready = false;
    }
}

ExplosionLauncher::~ExplosionLauncher() {
    //TODO:: Cleanup whole explosion launcher stuff here!
}
