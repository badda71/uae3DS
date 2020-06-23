# uae3DS
Amiga 500 emulator for Nintendo 3DS

## Compiling

You need devkitARM (provided by devkitPro) and the following packages:
3ds-curl, 3ds-mbedtls, 3ds-sdl_image, 3ds-sdl_gfx, 3ds-libpng and 3ds-zlib

Install these packages with pacman / dkp-pacman (use sudo if necessary):

    pacman -Sy 3ds-curl 3ds-mbedtls 3ds-sdl_image 3ds-sdl_gfx 3ds-libpng 3ds-zlib

Afterwards, the emulator can be compiled via

    make release
