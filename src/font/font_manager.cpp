// Written by Michael Zeilfelder, please check licenseHCraft.txt for the zlib-style license text.

/*
  Note Wolf Alexander 02.10.2025: The source code below was taken from the HCraft project of Michael Zeilfelder; I only modified it slightly
  to be able to use it in my project. For example I renamed the name of the define if Freetype should be used or not. And I modified the
  logging to use my own logging implementation of my project.                                                                               */

#include "font_manager.h"
#include "CGUITTFont.h"
#include <sstream>
#include "../utils/logging.h"
#include "../definitions.h"

FontManager::FontManager()
{
}

FontManager::~FontManager()
{
    FontMap::iterator itFont = mFontMap.begin();
    for ( ; itFont != mFontMap.end(); ++itFont )
    {
        itFont->second->drop();
    }
}

irr::gui::IGUIFont* FontManager::GetTtFont(irr::video::IVideoDriver* driver, irr::io::IFileSystem * fs, const char* filename_, unsigned int size_, bool antiAlias_, bool transparency_)
{
    if ( !filename_ || !strlen(filename_) )
        return NULL;

    // Make a unique font name for the given settings.
    // We want to re-use the same font instead of create a new one for each setting
    std::string fontString(MakeFontIdentifier(filename_, size_, antiAlias_, transparency_));
    FontMap::const_iterator itFont = mFontMap.find(fontString);
    if  (itFont != mFontMap.end() )
        return itFont->second;

#ifdef USE_FREETYPE
	irr::gui::CGUITTFont * font = irr::gui::CGUITTFont::createTTFont(driver, fs, irr::io::path(filename_), size_, antiAlias_, transparency_);
	if ( font )
	{
		font->setFontHinting(false, false);
		mFontMap[fontString] = font;
	}
	return font;
#else
    logging::Info("Project set to not use FreeType Library. Need to fallback to default Irrlicht font.");
	return NULL;
#endif
}

// make a unique font name for different settings.
std::string FontManager::MakeFontIdentifier(const char* filename_, unsigned int size_, bool antiAlias_, bool transparency_) const
{
    std::ostringstream stream;
    stream << filename_ << size_;
    if ( antiAlias_ )
        stream << 'a';
    if ( transparency_ )
        stream << 't';

    //fprintf(stderr, "font: %s", stream.str().c_str());

    return stream.str();
}

