# uae3DS
Amiga 500 emulator for Nintendo 3DS

## Compiling

You need devkitARM (provided by devkitPro) and the following packages: 3ds-sdl, 3ds-sdl_mixer, 3ds-mikmod, 3ds-zlib, 3ds-libogg, 3ds-libvorbisidec and 3ds-libmad

Install these packages with pacman / dkp-pacman (use sudo if necessary):

    pacman -Sy 3ds-sdl 3ds-sdl_mixer 3ds-mikmod 3ds-zlib 3ds-libogg 3ds-libvorbisidec 3ds-libmad

Afterwards, the emulator can be compiled via

    make
