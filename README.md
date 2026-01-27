# GravelBoy

A Game Boy Colour emulator written in C++ that can run CGB games and is backwards compatible with DMG games. This emulator passes the standard Blargg's test roms, acid2 tests, and some of the mooneye test suites.

Most games I've tested were in a playable state, though testing has been limited to a small handful of games and short play sessions.

The emulator is locked at 60 fps by default instead of 59.7 fps. So games will run a bit faster in comparison to the actual Game Boy. 

## How to compile and run
This program requires SDL2, and CMake should be installed. 

Windows systems do not need to install SDL2 as the necessary files have been placed in the **deps** folder, however, the program will require SDL2.dll to be in the same directory in order for it to run.

To compile the program:
```
git clone git@github.com:Dapples0/GravelBoy.git
mkdir build
cd build
cmake ..
cmake --build .
```
**Note:** Windows systems may need to use other build methods such as using ```cmake -G "MinGW Makefiles" ..  ``` instead of ```cmake .. ``` to compile.

How to run the program:
```
./GravelBoy path/to/rom
```

## Controls
| Controls | Keyboard Mapping |
| ----------- | ----------- |
| A | X |
| B | Z | 
| Select | C |
| Start | Space |
| Up | ^ |
| Down | v | 
| Left | > |
| Right | < |



| Additional Controls | Keyboard Mapping |
| ----------- | ----------- |
| Speed Up Game | = |
| Slow Down Game | - | 

Game Speeds = [ 60, 120, 180, 240 ]
## Saves
If a game supports saves, a **saves** folder be created and the respective save will be stored there. Moreover, CGB games will have "cgb" be appended at the end of the save name to prevent DMG and CGB games from sharing the same save filenames.

## Development Notes
Some hardware quirks such as the "halt-bug" have not been implemented.

### Cycles and Timings
This emulator attempts to be somewhat cycle and timing accurate. Each CPU tick, ticks 1 M-Cycle where it is then converted to either 2 T-Cycles or 4 T-Cycles - depending on if the CPU is in double speed mode or not - for the GPU and APU.
#### Resources
[Game Boy: Complete Technical Reference](https://gekkio.fi/files/gb-docs/gbctr.pdf) -> Useful for identifying when M-Cycles occur during multi-cycle CPU instructions

[Pandocs Opcode Table](https://gbdev.io/gb-opcodes/optables/)

[gbops](https://izik1.github.io/gbops/index.html) --> Some instruction timings are inaccurate or aren't documented/explained that well, but is still useful.

[RGBDS Game Boy CPU instruction reference](https://rgbds.gbdev.io/docs/v1.0.1/gbz80.7)
### Game Cartridges
This emulator only supports games of the following cartridge types:
- No MBC
- MBC1
- MBC3
- MBC5

MBC1 will be defaulted to, if a game uses an unsupported cartridge type.

MBC3's RTC is not implemented correctly. So games that track how much real-time has passed may be a bit jank. Moreover, speeding up the game does not speed up how much real-time has passed as it only speeds up in-game time.

### Graphics
To render the graphics on screen, I chose to use a scanline buffer instead of the FIFO approach. So graphical glitches wil be apparent for demos and games that require precise accuracy for its graphics.
### Audio
APU implementation is very bare bones, and only passes a few of Blargg's audio tests. Most games sound fine but there is audio popping and distortions. Likewise, audio may break in WSL environments.
