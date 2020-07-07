![Screenshots](https://gbatemp.net/attachments/1-png.198109/ "uae3DS Screenshots")

# uae3DS
This is a port of Chui's UAE4ALL Amiga 500 emulator (http://chui.dcemu.co.uk/uae4all.html) to Nintendo 3DS.

Many Amiga games are well playable - a compatibility list (incomplete) can be found using the link above. The emulator runs pretty well on both O3DS and N3DS, however the extra speed on N3DS makes it even more enjoyable.

## Installation
- Create directory /3ds/uae3DS on your 3DS SD-card
- Put your kick.rom file in directory /3ds/uae3DS on your 3DS SD-card. It must be called kick.rom, be a kickstart 1.3 image, and be the 512KB overdumped version. The kickstart ROM is copyrighted material so don't ask me where to get it from. You might get lucky by asking big G for "uae kickstart rom" ...
- Copy any Amiga disk images that you might have (.adf, .adz) to a directory of your choice on your SD card
- Install CIA with FBI, run 3dsx from homebrew launcher (put 3dsx file in /3ds/uae3DS dir) or run 3ds from flash card.
- Apart from this, a DSP-dump is required for sound to work correctly in the CIA version. https://gbatemp.net/threads/dsp1-a-new-dsp-dumper-cia-for-better-stability.469461/

## Download
https://github.com/badda71/uae3ds/releases

## Usage
Emulator usage:

    SELECT: open menu
    START: Toggle SuperThrottle
    Bottom Screen: Virtual Keyboard / Touchpad (tap-to-click, double-tap-to-double-click, tap-and-drag)
    A button: joystick fire
    B button: joystick UP
    R button: joystick autofire
    X button / ZL-button / tap touchpad: left mouse button
    Y button / L button: right mouse button
    DPad: joystick
    CPad: joystick or mouse (configurable in menu)
    CStick up/down: adjust vertical image position
    CStick left/right: adjust zoom

Menu usage:

    CPad / DPad: Navigate cursor
    A button: select current entry
    B button: cancel / back
    X button: delete save state in "Load state"-menu
    other button functions given in parentheses in menu

For further usage instructions, check here:
https://gbatemp.net/threads/release-uae3ds-amiga-500-emulator-for-nintendo-3ds.558577/

## Compiling

You need devkitARM (provided by devkitPro) and the following packages:
3ds-curl, 3ds-mbedtls, 3ds-sdl_image, 3ds-sdl_gfx, 3ds-libpng and 3ds-zlib

Install these packages with pacman / dkp-pacman (use sudo if necessary):

    pacman -Sy 3ds-curl 3ds-mbedtls 3ds-sdl_image 3ds-sdl_gfx 3ds-libpng 3ds-zlib

Afterwards, the emulator can be compiled via

    make release
