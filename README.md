# Qrusty Quake

Quake like you remember it.

A modernized, SDL2-based WinQuake port aimed at faithfulness to the original and easy portability.

# Features

- A "New Options" menu, can be toggled with "newoptions 0/1"

- Integer scaling

- Borderless window with -borderless parameter

- Auto-resolution fullscreen with -fullscreen_desktop

- Hardware-accelerated frame > screen rendering

   - Boosts performance massively on systems with GPUs

   - Tanks performance on machines without GPUs

   - Use the new -forceoldrender flag to disable

- High resolution support

   - Maximum tested is 16K, 2000 times bigger that 320x200
   
   - Defined by MAXHEIGHT and MAXWIDTH in r_shared.h
   
   - Can probably be set higher for billboard gaming

- Non-square pixels for 320x200 and 640x400 modes

   - aspectr cvar can be used for more granular adjustment

- General feature parity with the original WinQuake

   - "Use Mouse" option in windowed mode (also _windowed_mouse cvar)

   - Video configuration menu 

      - Mostly for show, use -width and -heigth or the new menu

- Proper UI scaling

   - scr_uiscale 1 for no scaling, scr_uiscale 2 for 200% size etc.

- Modern UI layouts (hudstyle 0/1/2/3)

- VGA text blurbs after shutdown

- The default audio rate is 11025 for more muffled WinQuake sound

- vim-like keybinds that work in menus, enable with -vimmode flag

- Mouse sensitivity Y-axis scaling with sensitivityyscale cvar

- FPS counter, scr_showfps 1

- Unlocked FPS with host_maxfps cvar

   - No client-server separation yet, be careful with values > 72 (the default)

- Expanded limits, Fitzquake protocol allowing for moden mod support

   - Underdark Overbright and Honey have been tested and are 100% playable (with minor visual bugs)

   - BSP2 and other modern features are being actively worked on, expect bugs and crashes on bigger maps

   - AD's Foggy Bogbottom and The Forgotten Sepulcher are playable with minor visual glitches

   - Other Arcane Dimensions maps load but are untested

- Software imitations of modern rendering features

   - Translucent liquids on supported maps (r_{water,slime,lava,tele}alpha 0-1)

   - Translucent entities with the "alpha" tag

   - r_alphastyle cvar and respective menu entry for the translucency rendering variations

      - Mix (default) - emulates the hardware-accelerated translucency by mixing the surface colors

      - Dither - much less memory-hungry option, but with less gradual translusency levels

   - Cubemapped skyboxes ("sky filename_", "r_skyfog [0.0-1.0]"), only .tga format for now

   - Cutout textures (transparency)

      - Very limited and glitch-prone in their current implementation

   - Fog (Quakespasm-like syntax "fog" command)

      - r_nofog 1 to disable

      - r_fogstyle 0/1/2/3, with 3 being the default and most "modern-looking"

# Planned

- Overhaul, modernization and trimming of the source code - removal of dead platforms and platform-specific code in favor of portable, properly formatted and readable code.

   - The codebase has been reduced by more than 50% compared to original WinQuake release in v0.3

      - ... and then bloated back up in v0.4. i'm working on it though.

   - The formatting has also been unified, along with tons of other minor readability improvements

      - ... except for the v0.4 bits

   - Most of the changes to the original code (that wasn't deleted) are purely cosmetic so far, deeper refactoring with more meaningful improvements is planned

   - The long-term goal for this port keeping it as well-maintained as a 1996 game can be.

- Modern mod support

   - Partially implemented, currently being worked on

- Other modern features (optional)

   - Colored lighting, .lit file support

   - More modern console

   - Client-server separation (mostly for 72< framerates)

   - Different FOV modes

   - Independent world and UI scaling

- (definitely not) CD Audio

   - Might have been pulled from Quakespasm along with the rest of the sound system, though I have no way or desire to test it.

Contributions of any kind are very welcome. If someone implements CD audio or something I'll definitely try to merge it.

# Building

Linux: cd src && make

Other -nixes: cd src && make -f Makefile.freebsd/openbsd/...

Windows: cd src && make -f Makefile.w64

The windows makefile is for cross-compilation from under Linux, I have no plans of making it buildable under windows.

# Successful builds

x86_64 unless specified otherwise.

VM is VirtualBox unless specified otherwise.

- Arch Linux [HW] v0.4.5

   - The main platform that this port is developed on. The most likely one to work

   - Tweaked and built on kernel 6.12.10-arch1-1 with TCC by erysdren

   - Other Linuxes not tested yet, but should be the same with the gcc/make toolchain

   - TODO other compilers, Alpine

- FreeBSD [VM] v0.4

   - Seemingly perfect, though the audio sample rate sounds too high

- OpenBSD [VM, HW] v0.4.1

   - Seemingly perfect

- Ubuntu [HW, MangoPi MQ Pro, RISC-V] v0.3

   - Works just fine at a playable framerate (20-30~ FPS)

- HaikuOS [QEMU] v0.4.3

   - Not tested on real hardware yet, TODO

- Android [HW, Termux, AARCH64] v0.4.2

   - Ran through X11 with touch controls. *unpleasant*

- Windows [VM, HW] v0.4.5

   - Tested with w10 on hardware and w11 on a VM

   - Has more strict surface limits and such, to prevent crashing 

   - Network fails on UDP initialization, but proceeds to work fine?

# Credits

This port started out as a fork of https://github.com/atsb/sdlwinquake

Which was a fork of another fork. It's forks all the way down...

Some code, including VGA text blurbs, has been taken from https://github.com/erysdren/sdl3quake/

Big thanks to Erysdren for contributing quite a lot to the backend and makefiles to make compilation on exotic systems possible.

The Fitzquake protocol implementation, both client and server, sound system, model loading, filesystem functions, cvars and a whole lot more has been pulled directly from Quakespasm.

A lot of code is adapted from Ironwail. Most of the netcode is a direct copy, other chunks are adapted with minor changes.

TGA image loading taken from MarkV, lots of software rendering code taken from there too. Most of it comes from ToChriS engine by Vic.

--CyanBun96 <3
