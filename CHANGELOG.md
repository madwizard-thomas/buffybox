# Changelog

We're using semantic versioning and the following changelog types:

- feat: New features
- fix: Bug fixes
- misc: Anything else

If a change only affects particular applications, they are listed in parentheses after the type, e.g. feat(unl0kr).

## Unreleased

- fix(buffyboard): Suspend input processing when a terminal switches to KD_GRAPHICS mode (!95, thanks @vstoiakin)

## 3.6.0 (2026-09-06)

- fix(buffyboard): Remove initialization of .time.* in static input_event to fix build error on 32-bit platforms (!87, thanks @craftyguy)
- misc: Synchronize default values of options in binaries and configuration files (!88, thanks @vstoiakin)
- feat: Add support for styluses on tablets (!89, thanks @vstoiakin)

## 3.5.1 (2026-03-03)

- fix(unl0kr): Add install section to systemd path unit (!86, thanks @craftyguy)
- fix(unl0kr): Restore dynamic horizontal padding for input text box (!85, thanks @craftyguy)
- fix: Fix keys being dropped when typing quickly (!84, thanks @craftyguy)

## 3.5.0 (2026-03-02)

- fix(unl0kr): Two clicks were required to toggle the password when "obscured" is set to "false" in unl0kr.conf (!68, thanks @vstoiakin)
- feat: Use events instead of threads for input processing (!63, thanks @vstoiakin)
- fix(buffyboard): Fix initialization of uinput (!73, thanks @vstoiakin)
- fix(buffyboard): Send EV_KEY and EV_SYN simultaneously (!74, thanks @vstoiakin)
- feat(f0rmz,unl0kr): Add SDL backend to ease development (!75, thanks @DylanVanAssche)
- feat: Add config option for haptic feedback, default: true (!76, thanks @vstoiakin)
- feat: Make framebuffer backend optional and introduce new `lvgl_backends` build option (!78, thanks @vstoiakin)
- feat: Enable NEON optimizations on aarch64 in LVGL (!79, thanks @vstoiakin)
- feat: Allow building systemd password agent and installing unit files without systemd at build time. The `systemd-buffyboard-service` and `systemd-password-agent` build options have been replaced by a new `systemd` option. (!80, thanks @craftyguy)
- feat(buffyboard): Add sticky_shift config option to control shift key behavior (!82, thanks @nettika-cat)
- misc: Update LVGL to 9.5.0

## 3.4.2 (2025-10-02)

- fix(f0rmz): Add missing shutdown implementation (!67, thanks @Adrian)

## 3.4.1 (2025-09-30)

- misc: Update LVGL to master (2025-09-29) to potentially address DRM issues (!66)

## 3.4.0 (2025-09-24)

- misc: Update LVGL to 9.3.0 (!59, thanks @vstoiakin)
- misc: Use standard C library instead of builtin functions (!49, thanks @vstoiakin)
- feat: Add support for split packaging of buffyboard and unl0kr (!54, thanks @vstoiakin)
- fix: Do not hang if graphics backend is not available (!57, thanks @vstoiakin)
- fix!(unl0kr): Disable software rotation due to regressed performance (!56)
- feat(buffyboard): Rotate the keyboard according to /sys/class/graphics/fbcon/rotate by default (!60, thanks @vstoiakin)
- fix(buffyboard): Prevent overlap of the keyboard and the terminal (!58, thanks @vstoiakin)
- feat(buffyboard): Add a handler for SIGUSR1 to redraw the keyboard (!58, thanks @vstoiakin)
- feat(f0rmz): New subproject to create a form/wizard application (!62, thanks @craftyguy)

## 3.3.0 (2025-04-15)

- feat: Add a systemd password agent (!33, thanks @vstoiakin)
- feat: Load config from /usr/share aswell (!26 & !28, thanks @fossdd & @pabloyoyoista)
- feat(buffyboard): Add man pages (!23, thanks @Jarrah)
- misc: Unify build system (!23 & !29, thanks @Jarrah & @vladimir.stoyakin)
- misc: Ensure stable file order during builds (!24, thanks @devrtz)
- fix: Prevent logging internal LVGL errors to stdout
- misc: Make scdoc a native dependency to enable cross-compilation (!30, thanks @uninsane)
- fix: Use usleep to release CPU when possible (!31, thanks @uninsane)
- feat(buffyboard): Add a buffyboard.service systemd service (!34, @uninsane)
- fix(unl0kr): Select an available DRM device instead of a hard-coded one (!35, thanks @Sorcus)
- feat(unl0kr): Enable software rotation (!32, thanks @xs5871 & @craftyguy)
- misc: Do not hardcode endianess of a system (!41, thanks @vstoiakin)
- misc(unl0kr): Optimize the main loop (!38, thanks @vstoiakin)
- feat(unl0kr): Add the option to show a custom message to a user (!39, thanks @vstoiakin)
- misc: Update LVGL to 9.2.2 (!46, thanks @vstoiakin)

## 3.2.0 (2024-06-03)

- feat: Add adwaita-dark theme (thanks @topjor)
- feat: Add Nord themes
- misc: Update lvgl to git master (2023-06-03)

## 3.1.0 (2024-04-10)

- feat(buffyboard): Handle input device connection/disconnection at runtime; adds new dependency libudev
- feat(buffyboard): Allow choosing theme via config and add all themes from unl0kr
- feat(buffyboard): Add fbdev force-refresh quirk via config
- feat(buffyboard): Allow disabling input devices via config
- feat(buffyboard): Add CLI flags for overriding geometry & DPI
- feat(buffyboard): Add CLI flag for verbose logging
- fix(unl0kr): Shutdown message box doesn't close on decline
- fix(unl0kr): Shutdown message box buttons and label are unstyled
- fix(unl0kr): Build fails when DRM is disabled
- misc: Update lvgl to git master (2023-04-10)

## 3.0.0 (2024-03-22)

- misc: Share the same lvgl submodule (and version) between buffyboard and unl0kr
- misc: Update lvgl to git master (2023-03-21) and eliminate lv_drivers

## unl0kr-2.0.3 (2023-12-19)

- fix: Clear the screen before exiting (https://gitlab.com/cherrypicker/unl0kr/-/issues/50)
- feat: Add extra top row for numbers & symbols, drop duplicative label (https://gitlab.com/cherrypicker/unl0kr/-/issues/49)
- fix: Stopgap solution for slow react on hardware keyboard (https://gitlab.com/cherrypicker/unl0kr/-/issues/36, thanks @jane400)

## unl0kr-2.0.2 (2023-10-07)

- fix: Prefer keyboard capability when input devices offer both keyboard and pointer (https://gitlab.com/cherrypicker/unl0kr/-/merge_requests/24, thanks @TravMurav)

## unl0kr-2.0.1 (2023-10-01)

- fix: Keyboard layout selection is broken (https://gitlab.com/cherrypicker/unl0kr/-/issues/45, thanks @Jarrah)

## unl0kr-2.0.0 (2023-07-28)

- feat: Add man page (https://gitlab.com/cherrypicker/unl0kr/-/issues/6, thanks @Jarrah)
- feat: Enable direct rendering
- feat: Add quirk for forcing fbdev refresh (https://gitlab.com/cherrypicker/unl0kr/-/issues/34, thanks @calebccff)
- fix: Fix slow react on hardware keyboard (https://gitlab.com/cherrypicker/unl0kr/-/issues/36, thanks @calebccff)

## unl0kr-1.0.0 (2023-03-14)

- feat: Handle input device connection/disconnection at runtime; adds new dependency libudev (https://gitlab.com/cherrypicker/unl0kr/-/issues/38)
- feat: Update lv_drivers to git master (2023-03-14)
- feat: Update lvgl to v8.3 (2023-03-14)
- feat: Deprecate -c CLI option and add support for reading from /etc/unl0kr.conf.d directory
- fix: Print correct version in logs
- feat: Add config options to enable terminal quirks (https://gitlab.com/cherrypicker/unl0kr/-/issues/34)

## unl0kr-0.3.1 (2023-02-25)

- fix: Changed Color of the cursor in breeze_dark to be able to see it (https://gitlab.com/cherrypicker/unl0kr/-/issues/39)

## unl0kr-0.3.0 (2022-11-13)

- feat: Update lvgl & lv_drivers to git master (2022-10-16)
- feat: Add config option to customise bullet character (https://gitlab.com/cherrypicker/unl0kr/-/issues/17)
- fix: Use actual screen DPI value to compute sizes and spaces
- feat: Allow shutting down the device on inactivity
- fix: Disable terminal keyboard while unl0kr is running
- feat: Extend --geometry flag to also accept display offsets
- fix: Swap order of KDSKBMODE and KDSETMODE to prevent blank screen (https://gitlab.com/cherrypicker/unl0kr/-/issues/34)
- fix: Increase maximum textarea width to prevent clipping on OnePlus 6 (https://gitlab.com/cherrypicker/unl0kr/-/issues/33)
- fix: Disable unused fonts, widgets and drivers (https://gitlab.com/cherrypicker/unl0kr/-/issues/13)
- fix: Rework pmOS theme to allow for more key space (https://gitlab.com/cherrypicker/unl0kr/-/issues/30)

## unl0kr-0.2.0 (2022-05-27)

- fix: Make meson actually install things (thanks @craftyguy)
- feat: Make en-US the default layout
- fix: Use correct pointer type in memset to avoid segfault
- feat: Set correct version in meson.build
- feat: Add postmarketOS theme (thanks @dylanvanassche)
- feat: Handle physical return key (https://gitlab.com/cherrypicker/unl0kr/-/issues/19)
- fix: Avoid TTY cursor blinking (https://gitlab.com/cherrypicker/unl0kr/-/issues/26)
- fix: Specify `check: true` when running external commands from meson (https://gitlab.com/cherrypicker/unl0kr/-/issues/23)
- feat: Hide partition name (https://gitlab.com/cherrypicker/unl0kr/-/issues/27)
- fix: Prevent scrolling when keyboard hides (https://gitlab.com/cherrypicker/unl0kr/-/issues/21)
- feat: Do not show last typed character when typing (https://gitlab.com/cherrypicker/unl0kr/-/issues/25)
- feat: Update lvgl & lv_drivers to git master (2022-02-21)
- feat: Allow runtime selection of the lvgl display driver ("backend")
- feat: Allow overriding dpi with the --dpi command line parameter

## buffyboard-0.2.0 (2021-11-23)

- feat: Enable software rotation

## buffyboard-0.1.0 (2021-11-21)

Initial release

## unl0kr-0.1.0 (2021-11-15)

Initial release
