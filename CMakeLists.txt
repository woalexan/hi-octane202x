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

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

INCLUDE_DIRECTORIES(${IRRLICHT_INCLUDE_DIR})

add_executable(hi-octane202x src/main.cpp)

TARGET_LINK_LIBRARIES(hi-octane202x ${IRRLICHT_LIBRARY})

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
