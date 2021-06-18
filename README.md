# C8E - A Chip 8 emulator written using SDL2 in C++


### Screenshot
![pong](https://github.com/0xhh/C8E/blob/main/screenshot.PNG)


### Building
Open terminal, clone this repo and cd into the repo

Make sure that you have ```SDL2``` installed on your OS.

On Windows, run the following command, you might need to change the ```C:/MinGW/include/``` and ```C:/MinGW/lib``` to wherever your ```SDL``` source files are present.

```g++.exe -c src/*.cpp -std=c++14 -g -Wall -I ./ -I C:/MinGW/include/ && g++.exe *.o -o ./main -L C:/MinGW/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image```

On Linux, I never tried it. I think removing ```.exe``` and changing ```C:/MinGW/include/``` and ```C:/MinGW/lib``` to wherever your ```SDL``` source files are present from the above command will just work fine. Also don't forget to remove ```windows.h``` header from ```main.cpp``` file.


### Running
Once you build the binary, you just need to run ```main.exe <ROM File>``` on window and if you are on Linux, run ```./main <ROM File>``` and it will emulate the ROM file you've provided.


### Resources
Cowgod's CHIP-8 Technical Referrence - http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#0.0
I shamelessly copied the giant switch statement in ```cpu.cpp``` from https://github.com/JamesGriffin/CHIP-8-Emulator


### Contribution
All contributions that won't break the code are accepted.
