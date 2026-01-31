/*
 Copyright (C) 2024-2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "drawdebug.h"
#include <cmath>

DrawDebug::DrawDebug(irr::scene::ISceneManager* sceneManager, irr::video::IVideoDriver *driver) {
    myDriver = driver;
    mySmgr = sceneManager;

    red = AddColor(255,255,0,0);
    green = AddColor(255,0,255,0);
    blue = AddColor(255,0,0,255);
    black = AddColor(255,0,0,0);
    white = AddColor(255,255,255,255);
    pink = AddColor(255, 255, 0, 255);
    brown = AddColor(255, 128, 64, 0);
    cyan = AddColor(255, 0, 255, 255);
    orange = AddColor(255, 255, 128, 0);
    grey = AddColor(255, 195, 194, 190);

    colorShieldCharger = AddColor(255, 24, 254, 56);
    colorFuelCharger = AddColor(255, 121, 65, 198);
    colorAmmoCharger = AddColor(255, 254, 222, 48);

    origin = new irr::core::vector3df(0.0f, 0.0f, 0.0f);
    XAxis = new irr::core::vector3df(10.0f, 0.0f, 0.0f);
    YAxis = new irr::core::vector3df(0.0f, 10.0f, 0.0f);
    ZAxis = new irr::core::vector3df(0.0f, 0.0f, 10.0f);

    InitCubeMesh();
}

ColorStruct* DrawDebug::AddColor(irr::u32 alpha, irr::u32 r, irr::u32 g, irr::u32 b) {
    ColorStruct* newColorStruct = new ColorStruct();
    newColorStruct->color = new irr::video::SColor(alpha, r, g, b);

    newColorStruct->material = new irr::video::SMaterial();
    newColorStruct->material->MaterialType = irr::video::EMT_SOLID;
    newColorStruct->material->AmbientColor = *newColorStruct->color;

    return newColorStruct;
}

void DrawDebug::CleanUpColor(ColorStruct* whichColor) {
    if (whichColor != nullptr) {
        if (whichColor->material != nullptr) {
            delete whichColor->material;
        }

        if (whichColor->color != nullptr) {
            delete whichColor->color;
        }

        delete whichColor;
    }
}

void DrawDebug::Draw3DTriangle(const irr::core::triangle3df *triangle, ColorStruct* color) {
    myDriver->draw3DTriangle(*triangle, *color->color);
}

void DrawDebug::Draw3DTriangleOutline(const irr::core::triangle3df *triangle, ColorStruct* color) {
    myDriver->setMaterial(*color->material);
    myDriver->draw3DLine(triangle->pointA, triangle->pointB);
    myDriver->draw3DLine(triangle->pointB, triangle->pointC);
    myDriver->draw3DLine(triangle->pointC, triangle->pointA);
}

void DrawDebug::Draw3DLine(irr::core::vector3df startPos, irr::core::vector3df endPos, ColorStruct* color) {
    myDriver->setMaterial(*color->material);
    myDriver->draw3DLine(startPos, endPos);
}

void DrawDebug::Draw3DRectangle(irr::core::vector3df v1, irr::core::vector3df v2, irr::core::vector3df v3, irr::core::vector3df v4,
                     ColorStruct* color) {
    myDriver->setMaterial(*color->material);
    myDriver->draw3DLine(v1, v2);
    myDriver->draw3DLine(v2, v3);
    myDriver->draw3DLine(v3, v4);
    myDriver->draw3DLine(v4, v1);
}

DrawDebug::~DrawDebug() {
    CleanupAllCubeMesh();

    delete ZAxis;
    delete YAxis;
    delete XAxis;
    delete origin;

    CleanUpColor(white);
    CleanUpColor(black);
    CleanUpColor(blue);
    CleanUpColor(green);
    CleanUpColor(red);
    CleanUpColor(pink);
    CleanUpColor(brown);
    CleanUpColor(cyan);
    CleanUpColor(orange);
    CleanUpColor(grey);

    CleanUpColor(colorShieldCharger);
    CleanUpColor(colorFuelCharger);
    CleanUpColor(colorAmmoCharger);
}

irr::core::vector3df DrawDebug::GetOrthogonalVector(irr::core::vector3df inVec) {
  irr::f32 x = inVec.X;
  irr::f32 y = inVec.Y;
  irr::f32 z = inVec.Z;
  const double s = std::sqrt(x*x + y*y + z*z);
  const double g = std::copysign(s, z);  // note s instead of 1
  const double h = z + g;
  return irr::core::vector3df((irr::f32)(g*h - x*x), (irr::f32)(- x * y), (irr::f32)(- x * h));
}

void DrawDebug::Draw3DArrow(irr::core::vector3df startPos, irr::core::vector3df arrowPosition, irr::f32 arrowOffset, ColorStruct* color, irr::f32 arrowSize) {
    myDriver->setTransform(irr::video::ETS_WORLD, irr::core::IdentityMatrix);

    myDriver->setMaterial(*color->material);
    myDriver->draw3DLine(startPos, arrowPosition);

    irr::core::vector3df dirVec = (startPos - arrowPosition);
    dirVec.normalize();

    irr::core::vector3df dp = arrowPosition + dirVec * arrowSize + dirVec * arrowOffset;

    irr::core::vector3df oneNormal = GetOrthogonalVector(dp);
    oneNormal.normalize();

    irr::core::vector3df otherVec = dp.crossProduct(oneNormal);
    otherVec.normalize();

    myDriver->draw3DLine(arrowPosition, dp + oneNormal * arrowSize);
    myDriver->draw3DLine(arrowPosition, dp - oneNormal * arrowSize);
    myDriver->draw3DLine(arrowPosition, dp + otherVec * arrowSize);
    myDriver->draw3DLine(arrowPosition, dp - otherVec * arrowSize);
}

void DrawDebug::DrawWorldCoordinateSystemArrows(void) {
    Draw3DArrow(*origin, *XAxis, 0.0f, red);
    Draw3DArrow(*origin, *YAxis, 0.0f, green);
    Draw3DArrow(*origin, *ZAxis, 0.0f, blue);
}

void DrawDebug::DrawAround3DBoundingBox(irr::core::aabbox3df* boundingBox, ColorStruct* color) {
    if (boundingBox == nullptr)
        return;

    myDriver->setMaterial(*color->material);
    myDriver->setTransform(irr::video::ETS_WORLD, irr::core::IdentityMatrix);

    myDriver->draw3DBox(*boundingBox);
}

void DrawDebug::InitCubeMesh() {
    //create a simple small Cube Mesh for Waypoint and
    //Wallsegment Editor Entity item objects
    mCubeMeshVec.clear();

    //Important note: Keep cube with Color White at the first position of the std::Vector!
    mCubeMeshVec.push_back(std::make_pair(white, CreateCubeMesh(0.2f, white)));

    //order of all remaining colors does not matter!
    mCubeMeshVec.push_back(std::make_pair(grey, CreateCubeMesh(0.2f, grey)));
    mCubeMeshVec.push_back(std::make_pair(orange, CreateCubeMesh(0.2f, orange)));
    mCubeMeshVec.push_back(std::make_pair(red, CreateCubeMesh(0.2f, red)));
    mCubeMeshVec.push_back(std::make_pair(colorShieldCharger, CreateCubeMesh(0.2f, colorShieldCharger)));
    mCubeMeshVec.push_back(std::make_pair(colorFuelCharger, CreateCubeMesh(0.2f, colorFuelCharger)));
    mCubeMeshVec.push_back(std::make_pair(colorAmmoCharger, CreateCubeMesh(0.2f, colorAmmoCharger)));
}

//If specified color is not available, returns a white cube
irr::scene::IMesh* DrawDebug::GetCubeMeshWithColor(ColorStruct* whichColor) {
    std::vector<std::pair<ColorStruct*, irr::scene::IMesh*>>::iterator it;

    for (it = mCubeMeshVec.begin(); it != mCubeMeshVec.end(); ++it) {
        if ((*it).first == whichColor) {
            //correct cube Mesh with this color found
            return (*it).second;
        }
    }

    //correct color not found, return white cube
    //which is at index0
    return mCubeMeshVec.at(0).second;
}

irr::scene::IMesh* DrawDebug::CreateCubeMesh(irr::f32 size, ColorStruct* cubeColor) {
   //This Source code was copied from Irrlicht source code
   //CGeometryCreator.cpp, function CGeometryCreator::createCubeMesh
   irr::scene::SMeshBuffer* buffer = new irr::scene::SMeshBuffer();

   // Create indices
   const irr::u16 u[36] = {   0,2,1,   0,3,2,   1,5,4,   1,2,5,   4,6,7,   4,5,6,
             7,3,0,   7,6,3,   9,5,2,   9,8,5,   0,11,10,   0,10,7};

   buffer->Indices.set_used(36);

   for (irr::u32 i=0; i<36; ++i)
       buffer->Indices[i] = u[i];

   // Create vertices
   buffer->Vertices.reallocate(12);

   buffer->Vertices.push_back(irr::video::S3DVertex(0,0,0, -1,-1,-1, *cubeColor->color, 0, 1));
   buffer->Vertices.push_back(irr::video::S3DVertex(1,0,0,  1,-1,-1, *cubeColor->color, 1, 1));
   buffer->Vertices.push_back(irr::video::S3DVertex(1,1,0,  1, 1,-1, *cubeColor->color, 1, 0));
   buffer->Vertices.push_back(irr::video::S3DVertex(0,1,0, -1, 1,-1, *cubeColor->color, 0, 0));
   buffer->Vertices.push_back(irr::video::S3DVertex(1,0,1,  1,-1, 1, *cubeColor->color, 0, 1));
   buffer->Vertices.push_back(irr::video::S3DVertex(1,1,1,  1, 1, 1, *cubeColor->color, 0, 0));
   buffer->Vertices.push_back(irr::video::S3DVertex(0,1,1, -1, 1, 1, *cubeColor->color, 1, 0));
   buffer->Vertices.push_back(irr::video::S3DVertex(0,0,1, -1,-1, 1, *cubeColor->color, 1, 1));
   buffer->Vertices.push_back(irr::video::S3DVertex(0,1,1, -1, 1, 1, *cubeColor->color, 0, 1));
   buffer->Vertices.push_back(irr::video::S3DVertex(0,1,0, -1, 1,-1, *cubeColor->color, 1, 1));
   buffer->Vertices.push_back(irr::video::S3DVertex(1,0,1,  1,-1, 1, *cubeColor->color, 1, 0));
   buffer->Vertices.push_back(irr::video::S3DVertex(1,0,0,  1,-1,-1, *cubeColor->color, 0, 0));

   // Recalculate bounding box
   buffer->BoundingBox.reset(0,0,0);

   for (irr::u32 i=0; i<12; ++i)
    {
        buffer->Vertices[i].Pos -= irr::core::vector3df(0.5f, 0.5f, 0.5f);
        buffer->Vertices[i].Pos *= size;
        buffer->BoundingBox.addInternalPoint(buffer->Vertices[i].Pos);
    }

    irr::scene::SMesh* mesh = new irr::scene::SMesh;
    mesh->addMeshBuffer(buffer);
    buffer->drop();

    mesh->recalculateBoundingBox();
    return mesh;
}

void DrawDebug::CleanupAllCubeMesh() {
    std::vector<std::pair<ColorStruct*, irr::scene::IMesh*>>::iterator it;
    irr::scene::IMesh* meshPntr = nullptr;

    for (it = mCubeMeshVec.begin(); it != mCubeMeshVec.end(); ) {
           meshPntr = (*it).second;

           it = mCubeMeshVec.erase(it);

           //get rid of the custom mesh from the Meshcache
           mySmgr->getMeshCache()->removeMesh(meshPntr);
    }
}
