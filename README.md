# Qrusty Quake

Quake like you remember it.

A modernized, SDL3-based WinQuake port aimed at faithfulness to the original and easy portability.

# Features

- A "New Options" menu, can be toggled with "newoptions 0/1"

- Integer/nearest neighbor scaling

- Borderless window with -borderless parameter

- Auto-resolution fullscreen with -fullscreen_desktop

- Horizontal FOV scaling for modern widescreen resolutions (yaspectscale cvar)

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

   - UI can be scaled independently with scr_uixscale and scr_uiyscale

- General feature parity with the original WinQuake

   - "Use Mouse" option in windowed mode (also _windowed_mouse cvar)

   - Video configuration menu 

      - Mostly for show, use -width and -heigth or the new menu

- Proper UI scaling

   - scr_uiscale 1 for no scaling, scr_uiscale 2 for 200% size etc.

- 9 HUD styles (hudstyle 0-9)

   - Classic, Modern, QW, Arcade, Minimalist and variations

   - Transparent UI elements for modern mods

- VGA text blurbs after shutdown

- BGM support

   - All the formats you can think of (i.e. whatever sdl3_mixer supports)

   - Optional, requires sdl3_mixer, currently missing from the windows releases

- The default audio rate is 11025 for more muffled WinQuake sound

- vim-like keybinds that work in menus, enable with -vimmode flag

- Mouse sensitivity Y-axis scaling with sensitivityyscale cvar

- FPS counter, scr_showfps 1

- Unlocked FPS with host_maxfps cvar

   - Render-server separation for 72<FPS

- Expanded limits, Fitzquake protocol allowing for moden mod support

   - 2021 rerelease support, place QuakeEX.kpf in the base folder for working localization

- Custom palette support (put the files at gfx/custompalette.lmp and gfx/palette.lmp)

- Software imitations of modern rendering features

   - Colored lighting, .lit file support

      - r_rgblighting 0,1 to toggle

      - Lit water is supported with r_litwater cvar

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

      - r_fognoise to adjust noise bias level: 0 - disable, 1 - noisy

      - r_fogfactor to adjust fog thickness: 0.5 - lighter, 1 - full

      - r_fogscale to adjust the distance to the fog: 0.5 - further, 2 - closer

      - r_fogstyle 0/1/2/3, with 3 being the default and most "modern-looking"

      - r_fogbrightness (0.5 for half brightness, 2 for double), independent of the fog color dictated by the map

- Miscellaneous graphics cvars

   - r_particlescale for higher resolutions

   - r_mipscale for LOD distance adjustment

- Modern gamepad support

# Planned

- An actual design document. Lots of documentation, really.

- Overhaul, modernization and trimming of the source code - removal of dead platforms and platform-specific code in favor of portable, properly formatted and readable code.

   - The long-term goal for this port keeping it as well-maintained as a 1996 game can be.

- Other modern features (optional)

   - More modern console

   - More palette options

   - Better mipmaps

- Probably not

   - Multithreading (would require a total rewrite with multithreading as the foundation for everything to get any performance benefits after threading overhead)

   - Advanced mapping/speedrunning/development features that are not directly related to gameplay (I'm not a mapper, speedrunner or developer)

Contributions of any kind are very welcome. If someone implements their favorite speedrunning cvar or something I'll definitely try to merge it. Issue reports are also greatly appreciated.

# Building

Linux: cd src && make

Other -nixes: cd src && make -f Makefile.freebsd/openbsd/...

Windows: cd src && make -f Makefile.w64

The windows makefile is for cross-compilation from under Linux. Use CMakeLists.txt for building natively.

# Successful builds

x86_64 unless specified otherwise.

VM is VirtualBox unless specified otherwise.

- Arch Linux [HW] v0.6.0

   - The main platform that this port is developed on. The most likely one to work

   - Tweaked and built on kernel 6.12.10-arch1-1 with TCC by erysdren

   - TODO other compilers, Alpine

- Debian 11 [VM] v0.5.1

   - The oldest tested distro

   - Used to build the release AppImage

- FreeBSD [VM] v0.4

   - Seemingly perfect, though the audio sample rate sounds too high

- OpenBSD [VM, HW] v0.4.1

   - Seemingly perfect

- Ubuntu [HW, MangoPi MQ Pro, RISC-V] v0.3

   - Works just fine at a playable framerate (20-30~ FPS)

- HaikuOS [QEMU] v0.4.3

   - Not tested on real hardware yet, TODO

- Android [HW, Termux, AARCH64, clang] v0.5.0

   - Ran through X11 with touch controls. *unpleasant*

- macOS [HW] v0.6.0

   - Several successes were reported with the default Makefile and SDL3 from brew

- Emscripten [Browsers with WebAssembly] v0.6.0

   - Dark magic performed by Erysdren, please don't ask me about it

- Windows [VM, HW] v0.6.0

   - Tested with w10 on hardware and w11 on a VM

   - Has more strict surface limits and such, to prevent crashing 

   - Network fails on UDP initialization, but proceeds to work fine?

   - Release .exes are cross-compiled

   - Use CMakeLists.txt for native builds

# Credits

This port started out as a fork of https://github.com/atsb/sdlwinquake

Which was a fork of another fork. It's forks all the way down...

It has since evolved into the wonderfully unhinged NakedWinQuake https://github.com/atsb/NakedWinQuake

Atsb has also contributed some crucial fixes to long-standing issues and some general improvements.

Some code, including VGA text blurbs, has been taken from https://github.com/erysdren/sdl3quake/

Big thanks to Erysdren for contributing quite a lot to the backend and makefiles to make compilation on exotic systems possible, implementinig BGM, BMP screenshots, and lots of other cool stuff.

Izhido heroically made the native windows build possible, and in process brought the windows version to the same feature level as the others.

The Fitzquake protocol implementation, both client and server, sound system, model loading, filesystem functions, cvars and a whole lot more has been pulled directly from Quakespasm.

A lot of code is adapted from Ironwail. Most of the netcode is a direct copy, other chunks are adapted with minor changes.

TGA image loading taken from MarkV, lots of software rendering code taken from there too. Most of it comes from ToChriS engine by Vic.

--CyanBun96 <3
