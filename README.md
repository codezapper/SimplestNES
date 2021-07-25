# Yet another NES emulator

- Why?

    - Because it's incredibly fun, essentially. Also, the challenges that rise from such a project are completely different from what the current software developers/engineers commonly requires.

- Why plain C instead of C++?

    - The idea is to be able to code an initial working release for a Linux environment and then convert it for an embedded system.
      Looking at the memory requirement, I realized that this would not work on smaller boards like the Arduino Nano, but it just might work on the Raspberry Pico (using the zero or the standard ones would mean using just a different desktop).

- Why the "Simplest NES"?

    - Because even though there is extensive documentation, the way it's currently presented is a really big challenge. So I though I could try and later on write my own documentation by making at least the simplest possible emulator. "Simplest" meaning in terms of how easy it would be to read the code. I tried to follow a practical approach, even at the cost of losing a bit of performance, although the supported games still have a playable framerate.

- What is the current status?

    - [x] CPU implemented
    - [ ] PPU implemented
        - [x] Background renders properly
        - [x] Sprites render properly
        - [x] Sprite zero hits rendered properly
        - [x] Sprite transparency handled properly
        - [x] Scroll renders properly
        - [ ] Sync with CPU is cycle accurate
        - [ ] Color emphasis supported
        - [ ] Draw priority supported (sprite rendered behind background)
    - [ ] Memory mappers implemented
        - [x] Memory mapper 0 implemented (no mapper)
        - [ ] Memory mapper MMC1 implemented
        - [ ] Memory mapper MMC2 implemented
        - [ ] Memory mapper MMC3 implemented
        - [ ] Memory mapper MMC4 implemented
        - [ ] Memory mapper MMC6 implemented
    - [ ] APU implemented

- What happens after all the features work?

    - [ ] Clean up the code. Remove some of the duplication due to trying out new things.
    - [ ] Possibly cache the rendering of at least the background. The tiles do not really handle transparency, so they do not need a dynamic render 60 times per second (unlike sprites).

- Screenshots:

![Screenshot](https://github.com/codezapper/SimplestNES/blob/main/sm.png?raw=true)
![Screenshot](https://github.com/codezapper/SimplestNES/blob/main/pm.png?raw=true)
