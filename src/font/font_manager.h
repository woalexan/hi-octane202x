// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

/*
  Note Wolf Alexander 02.10.2025: The source code below was taken from the HCraft project of Michael Zeilfelder; I only modified it slightly
  to be able to use it in my project. For example I renamed the name of the define if Freetype should be used or not. And I modified the
  logging to use my own logging implementation of my project.                                                                               */

#ifndef FONT_MANAGER_H
#define FONT_MANAGER_H

#include "../definitions.h"

#include <irrlicht.h>
#include <string>
#include <map>

namespace irr
{
namespace gui
{
    class IGUIFont;
};
};
class CGUITTFace;

class FontManager
{
public:
    FontManager();
    ~FontManager();
    irr::gui::IGUIFont* GetTtFont(irr::video::IVideoDriver* driver, irr::io::IFileSystem * fs, const char* filename_, unsigned int size_, bool antiAlias_=true, bool transparency_=true);

protected:
    std::string MakeFontIdentifier(const char* filename_, unsigned int size_, bool antiAlias_, bool transparency_) const;

private:
    typedef std::map<std::string, irr::gui::IGUIFont*> FontMap;
    FontMap mFontMap;
};

#endif // FONT_MANAGER_H
