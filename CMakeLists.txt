cmake_minimum_required(VERSION 3.5)

project(hi-octane202x LANGUAGES CXX)

# Configuration options
set(SFML_STATIC_LIBRARIES FALSE CACHE BOOL "Use static SFML librares")

# General compiler options
if (SFML_STATIC_LIBRARIES)
        add_definitions(-DSFML_STATIC)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Irrlicht REQUIRED)
find_package(ADLMIDI REQUIRED)

###########################

# The following text was taken from SFML-Game-Development-Book Start
# Find SFML
# Note: SFML_STATIC_LIBRARIES determines which libraries are found by find_package()
find_package(SFML 2 COMPONENTS audio system network)

# If found, include and link; otherwise output error message
if(SFML_FOUND)
        include_directories(${SFML_INCLUDE_DIR})
else()
        set(SFML_ROOT "" CACHE PATH "SFML top-level directory")
        message("\n-> SFML directory not found. Set SFML_ROOT to SFML's top-level path (containing \"include\" and \"lib\" directories).")
        message("-> Make sure the SFML libraries with the same configuration (Release/Debug, Static/Dynamic) exist.\n")
endif()

# SFML-Game-Development-Book End

###########################

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

INCLUDE_DIRECTORIES(${IRRLICHT_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${ADLLIB_INCLUDE_DIR})

add_executable(hi-octane202x
    src/definitions.h
    src/resources/assets.h
    src/resources/assets.cpp
    src/resources/blockdefinition.h
    src/resources/blockdefinition.cpp
    src/resources/columndefinition.h
    src/resources/columndefinition.cpp
    src/resources/entityitem.h
    src/resources/entityitem.cpp
    src/resources/levelfile.h
    src/resources/levelfile.cpp
    src/resources/mapentry.h
    src/resources/mapentry.cpp
    src/resources/tableitem.h
    src/resources/tableitem.cpp
    src/resources/texture.h
    src/resources/texture.cpp

    src/resources/readgamedata/bulcommn.h
    src/resources/readgamedata/bulcommn.cpp
    src/resources/readgamedata/dernc.h
    src/resources/readgamedata/dernc.cpp
    src/resources/readgamedata/xtabdat8.h
    src/resources/readgamedata/xtabdat8.cpp
    src/resources/readgamedata/objectdatfile.h
    src/resources/readgamedata/objectdatfile.cpp
    src/resources/readgamedata/preparedata.h
    src/resources/readgamedata/preparedata.cpp

    src/resources/xbrz-1-8/xbrz_config.h
    src/resources/xbrz-1-8/xbrz_tools.h
    src/resources/xbrz-1-8/xbrz.h
    src/resources/xbrz-1-8/xbrz.cpp

    src/resources/intro/flifile.h
    src/resources/intro/flifile.cpp
    src/resources/intro/flifix.h
    src/resources/intro/flifix.cpp
    src/resources/intro/prgtools.h
    src/resources/intro/prgtools.cpp

    src/resources/intro/flic.h
    src/resources/intro/flic_details.h
    src/resources/intro/stdio.cpp
    src/resources/intro/decoder.cpp
    src/resources/intro/encoder.cpp

    src/draw/drawdebug.h
    src/draw/drawdebug.cpp
    src/draw/gametext.h
    src/draw/gametext.cpp
    src/draw/hud.h
    src/draw/hud.cpp
    src/draw/menue.h
    src/draw/menue.cpp

    src/input/input.h
    src/input/input.cpp

    src/audio/sound.h
    src/audio/sound.cpp
    src/audio/music.h
    src/audio/music.cpp

    src/audio/ail/bank.h
    src/audio/ail/bank.cpp
    src/audio/ail/common.h
    src/audio/ail/common.cpp
    src/audio/ail/ffmt_base.h
    src/audio/ail/ffmt_base.cpp
    src/audio/ail/ffmt_enums.h
    src/audio/ail/format_ail2_gtl.h
    src/audio/ail/format_ail2_gtl.cpp
    src/audio/ail/format_wohlstand_opl3.h
    src/audio/ail/format_wohlstand_opl3.cpp
    src/audio/ail/wopl_file.h
    src/audio/ail/wopl_file.cpp

    src/audio/foo-midi/Range.h
    src/audio/foo-midi/Configuration.h
    src/audio/foo-midi/ADLPlayer.h
    src/audio/foo-midi/ADLPlayer.cpp
    src/audio/foo-midi/MIDIContainer.h
    src/audio/foo-midi/MIDIContainer.cpp
    src/audio/foo-midi/MIDIPlayer.h
    src/audio/foo-midi/MIDIPlayer.cpp

    src/models/collectable.h
    src/models/collectable.cpp
    src/models/column.h
    src/models/column.cpp
    src/models/cone.h
    src/models/cone.cpp
    src/models/explosion.h
    src/models/explosion.cpp
    src/models/explauncher.h
    src/models/explauncher.cpp
    src/models/levelblocks.h
    src/models/levelblocks.cpp
    src/models/levelterrain.h
    src/models/levelterrain.cpp
    src/models/mgun.h
    src/models/mgun.cpp
    src/models/missile.h
    src/models/missile.cpp
    src/models/morph.h
    src/models/morph.cpp
    src/models/particle.h
    src/models/particle.cpp
    src/models/player.h
    src/models/player.cpp
    src/models/recovery.h
    src/models/recovery.cpp

    src/utils/crc32.h
    src/utils/crc32.cpp
    src/utils/fileutils.h
    src/utils/fileutils.cpp
    src/utils/physics.h
    src/utils/physics.cpp
    src/utils/tprofile.h
    src/utils/tprofile.cpp
    src/utils/worldaware.h
    src/utils/worldaware.cpp
    src/utils/bezier.h
    src/utils/bezier.cpp
    src/utils/path.h
    src/utils/path.cpp

    src/utils/boundingbox/coord_frame.h
    src/utils/boundingbox/basis.h
    src/utils/boundingbox/vector.h
    src/utils/boundingbox/matrix.h
    src/utils/boundingbox/collision.h
    src/utils/boundingbox/collision.cpp

    src/game.h
    src/game.cpp
    src/race.h
    src/race.cpp
    src/main.cpp)

TARGET_LINK_LIBRARIES(hi-octane202x ${IRRLICHT_LIBRARY})
TARGET_LINK_LIBRARIES(hi-octane202x ${ADLMIDI_LIBRARY})

###########################

# The following text was taken from SFML-Game-Development-Book Start

# Build static or shared libraries? Set chapter-specific DLL import macro
if(SFML_STATIC_LIBRARIES)
        set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "SFML_STATIC")
endif()

# Link SFML. For versions < 2.2, SFML_DEPENDENCIES is not defined, and we do not need to link dependencies
if(SFML_VERSION_MINOR LESS 2)
set(SFML_DEPENDENCIES "")
endif()
target_link_libraries(${PROJECT_NAME} ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})

# SFML-Game-Development-Book End

###########################
