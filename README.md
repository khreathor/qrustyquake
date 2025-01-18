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

   - Can be forced on other modes with -stretchpixels

- General feature parity with the original WinQuake

   - "Use Mouse" option in windowed mode (also _windowed_mouse cvar)

   - Video configuration menu (mostly for show, use -width and -heigth)

- Proper UI scaling

- Advanced audio configuration

   - The default audio rate is 11025 for more muffled WinQuake sound

   - New flags -sndsamples and -sndpitch (try -sndpitch 5 or 15)

- vim-like keybinds that work in menus, enable with -vimmode flag

- Mouse sensitivity Y-axis scaling with sensitivityyscale cvar

- FPS counter, scr_showfps 1

# Planned

- Overhaul, modernization and trimming of the source code - removal of dead platforms and platform-specific code in favor of portable, properly formatted and readable code.

   - The codebase has been reduced by more than 50% compared to original WinQuake release in v0.3

   - The formatting has also been unified, along with tons of other minor readability improvements

   - Most of the changes to the original code (that wasn't deleted) are purely cosmetic so far, deeper refactoring with more meaningful improvements is planned

   - The long-term goal for this port keeping it as well-maintained as a 1996 game can be.

- (maybe) Modern mod support

- (definitely not) CD Audio

Contributions of any kind are very welcome. If someone implements CD audio or something I'll definitely try to merge it.

# Building

Linux: cd src && make

Other -nixes: cd src && make -f Makefile.freebsd/openbsd/...

Windows: cd src && make -f Makefile.w64

The windows makefile is for cross-compilation from under Linux, I have no plans of making it buildable under windows.

# Successful builds

x86_64 unless specified otherwise.

VM is VirtualBox unless specified otherwise.

- Arch Linux [HW]

   - The main platform that this port is developed on. The most likely one to work

   - Other Linuxes not tested yet, but should be the same with the gcc/make toolchain

   - TODO other compilers, Alpine

- FreeBSD [VM]

   - Seemingly perfect, though the audio sample rate sounds too high

- OpenBSD [VM, HW]

   - The sound is crackly both on VM and HW for some reason

- Ubuntu [HW, MangoPi MQ Pro, RISC-V]

   - Works just fine at a playable framerate (20-30~ FPS)

- Android [HW, Termux, AARCH64]

   - Ran through X11 with touch controls. *unpleasant*

- Windows [VM, HW]

   - Tested with w10 on hardware and w11 on a VM

# Credits

This port started out as a fork of https://github.com/atsb/sdlwinquake

Which was a fork of another fork. It's forks all the way down...

Features some fixes from Quakespasm and other ports. I wasn't the one to implement them, but will try and give credit as I peruse the source further.

A lot of code is adapted from Ironwail. Most of the netcode is a direct copy, other chunks are adapted with minor changes.

--CyanBun96 <3
