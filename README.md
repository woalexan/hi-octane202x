# hi-octane202x
My attempt to recreate useable source code for one of my most favorite games from my childhood (Hi-Octane by Bullfrog), because I always wanted to learn more about the internals of 3D games, and how the original game could have worked inside.

This source code will only run if the user does possess and supply it with the original game data files.

## Getting Started

#### Prerequisites
- game data
- cmake
- Irrlicht 1.8.5
- SFML 2.6.2
- libADLMIDI 1.5.1

The library versions listed above are known to result in successful compilation. Other library versions may work too.

#### Compile
Change to the `build` directory and run
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
A successful build will place the `hi-octane202x` binary in the build directory.

#### Run
After copying the game data files into `build/originalgame`, you can run the game by invoking `./hi-octane202x` in the build directory.


## Acknowledgements
I was only able to start this project because of the great work, effort and information from the "HiOctaneTools" project. A big thank you!

Another valuable source of information was the DOS Game Modding Wiki. This project is based on the Irrlicht Engine.



