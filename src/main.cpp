#include <iostream>
#include <irrlicht/irrlicht.h>

using namespace std;

using namespace irr;
using namespace irr::core;
using namespace irr::video;
using namespace irr::scene;

int main()
{
    IrrlichtDevice *device =
           createDevice( video::EDT_SOFTWARE, dimension2d<u32>(640, 480), 16,
               false, false, false, 0);

    if (!device)
           return 1;

    device->setWindowCaption(L"Hello World with Irrlicht added");

    IVideoDriver* driver = device->getVideoDriver();
    ISceneManager* smgr = device->getSceneManager();

    smgr->addCameraSceneNode(0, vector3df(0,30,-40), vector3df(0,5,0));

    while(device->run())
     {
        driver->beginScene(true, true, SColor(255,100,101,140));

        smgr->drawAll();

        driver->endScene();
     }

    device->drop();

    return 0;
}
