## Boot Process

[Paper Mario Boot Process via Devon from PM64 Decomp Discord](https://discord.com/channels/1141775228440158338/1142012036746858536/1456714607526678674)

paraphased from my personal documentation, this is basically the flow in a nutshell:
- [entrypoint assembly calls nuBoot](/src/boot/entry_point.s#L29-L30>)
- [nuBoot creates the idle thread, assigning it the static IdleStack and writing its ID to IdleThread](/src/os/nusys/nuboot.c#L23>)
- [the idle thread is signalled to start](/src/os/nusys/nuboot.c#L24>)
- system-end boot process ends

-# on the idle thread..
- [boot_idle configures most of the southbridge stuff like establishing comms with the reality coprocessor and setting up NTSC/PAL mode](<https://github.com/pmret/papermario/blob/main/src/os/nusys/nuboot.c#L30-L32>)
- [boot_idle creates the main thread, assigning it the static nuMainStack and writing its ID to MainThread](/src/os/nusys/nuboot.c#L33>)
- [the main thread is signalled to start](/src/os/nusys/nuboot.c#L34>)
- [the idle thread decreases its own priority](/src/os/nusys/nuboot.c#L35>)
- [the idle thread busy waits on nuIdleFunc, which presumably is either set to some system wait by nusys or just runs very rarely due to low prio](/src/os/nusys/nuboot.c#L37-L41>)

-# on the main thread...
- [boot_main turns the display off and configures some graphical setup to be used for communication with the RDP](/src/main.c#L49-L81>)
- [crash screen and debug handlers are applied on the relevant versions](/src/main.c#L80-L85>)
- [the graphics pipeline is initialized](/src/main.c#L86>)
- [boot_main sets the gfx callback to `gfxRetrace_Callback`, which tells nusys what to call every frame after flushing and sync to the RDP finishes](/src/main.c#L95-L96>)
- [the display is turned back on](/src/main.c#L98>)
- [the main thread busy waits, but this only runs to pad the gap between one frame and the next if there is time left over after flushing to the RDP](/src/main.c#L100>)

-# then each frame...
- [the game checks if the reset button has been pressed on the console](/src/main.c#L104-L123>)
- [the game alternates a frame timer between 0 and 1, and if the game is on a cold frame, nothing happens and the frame is reprojected from the framebuffer - this is what caps the game at 30fps](/src/main.c#L124-L125>)
- [if the game is on a hot frame, `step_game_loop` and some other gfx logic is called](/src/main.c#L126-L133>)

** **
-# in `step_game_loop`
- [frame counting and input polling is done](</src/main_loop.c#L54-L70>)
- [scripts, effects etc are processed](</src/main_loop.c#L83-L89>)
- [`step_current_game_mode` is run](</src/main_loop.c#L90>)
- [other ongoing processes are managed, mostly music and overlays](</src/main_loop.c#L91-L153>)
- [rng state is advanced](</src/main_loop.c#L155>)

-# in `step_current_game_mode`
- [the current game mode is stepped](</src/game_states.c#L121>)

the game mode table itself is in here, it starts with startup and then goes to logos and then title screen etc /src/game_modes.c#L40-L60

you can see from this table that, e.g. the startup state calls `state_init_startup` when it initializes

so it calls this: /src/state_startup.c#L8>

and at the end of it you can see it transition to GAME_MODE_LOGOS: /src/state_startup.c#L92>

then go back to the table etc

init for logos sets up the 3d camera and blah blah

then step checks what the logo state is on each frame /src/state_logos.c#L159-L248>

and when it's done it transitions to GAME_MODE_INTRO <https://github.com/pmret/papermario/blob/main/src/state_logos.c#L245>
