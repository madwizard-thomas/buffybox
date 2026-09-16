# BuffyBox

BuffyBox is a suite of graphical applications for the terminal.

## Components

### Standalone apps

**[buffyboard]** – Touch-enabled on-screen keyboard for virtual terminals

**[unl0kr]** – Disk unlocking UI for the initramfs

**[f0rmz]** – A form/setup wizard for the initramfs

### Utilities

**[squeek2lvgl]** – Converter for transforming [Squeekboard] layouts into [LVGL]-compatible C code

**[iskey]** - A utility for checking for key presses in bash scripts.

**[shared]** – Internal code that is shared by some or all applications in the suite but not meant to be used externally 

## Contributing

You can join our development chat at [#buffybox:matrix.org].

## Building & running

Some dependencies are included as git submodules in this repository. You can clone the repository and initialise the submodules with

```
$ git clone https://gitlab.postmarketos.org/postmarketOS/buffybox.git
$ cd buffybox
$ git submodule init
$ git submodule update
```

When pulling changes from the remote later, either use `git pull --recurse-submodules` or manually run `git submodule update` as needed after pulling.

Once you have the sources, you can build the app and run it in a VT. Unless your user account has special privileges, `sudo` will be needed to access input device files.

```
$ meson setup _build
$ meson compile -C _build
$ sudo chvt 2
$ sudo ./_build/unl0kr/unl0kr # For Unl0kr
$ sudo ./_build/buffyboard/buffyboard # For Buffyboard
```

On distributions based on systemd, `unl0kr` can be used as a [password agent](https://systemd.io/PASSWORD_AGENTS/), to give systemd the ability to ask passwords on touchscreen-only devices:

```
# systemctl stop systemd-ask-password-console.path
# systemctl stop systemd-ask-password-wall.path
# systemctl start unl0kr-agent.path
# systemd-ask-password --no-tty # Unl0kr is started
```

**Note**: SDL can be used as a display backend for development purposes by setting the Meson option `lvgl_backends=sdl` and using `backend=sdl` in the configuration file.
Other uses besides development are not supported by Buffybox.

## Packaging

Create a full package:
```
meson setup builddir/
meson compile -C builddir/
meson install -C builddir/ --destdir "$pkgdir"
```

Create separate packages for Buffyboard and Unl0kr:
```
meson setup builddir/
meson compile -C builddir/
meson install -C builddir/ --tags=unl0kr --destdir "$pkgdir_unl0kr"
meson install -C builddir/ --tags=buffyboard --destdir "$pkgdir_buffyboard"
```

Create a package for Unl0kr only:
```
meson setup builddir/
meson compile -C builddir/ unl0kr/unl0kr
meson compile -C builddir/ unl0kr/unl0kr-agent # if configured with -Dsystemd=true
meson compile -C builddir/ man/unl0kr.{1,conf.5} # if configured with -Dman=true
meson install -C builddir/ --tags=unl0kr --destdir "$pkgdir"
```

Create a package for Buffyboard only:
```
meson setup builddir/
meson compile -C builddir/ buffyboard/buffyboard
meson compile -C builddir/ man/buffyboard.{1,conf.5} # if configured with -Dman=true
meson install -C builddir/ --tags=buffyboard --destdir "$pkgdir"
```

## Making a release

To make it easier for distributions to package BuffyBox, we include source tarballs including the LVGL submodule in GitLab releases. See [unl0kr#42] for more background on this.

Producing and uploading a source tarball requires [git-archive-all] to be installed as well as an access token for the GitLab API. To tag a release and upload a source tarball, run

```
./release.sh $VERSION $TOKEN
```

Once the upload completes, create a new release on GitLab and attach the URL of the uploaded source tarball.

## License

This project is licensed under the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

For the license of bundled images and fonts, see [shared/cursor] and [shared/fonts].

[#buffybox:matrix.org]: https://matrix.to/#/#buffybox:matrix.org
[buffyboard]: ./buffyboard
[git-archive-all]: https://github.com/Kentzo/git-archive-all
[LVGL]: https://github.com/lvgl/lvgl
[shared]: ./shared
[squeek2lvgl]: ./squeek2lvgl
[iskey]: ./iskey
[Squeekboard]: https://gitlab.gnome.org/World/Phosh/squeekboard
[shared/cursor]: ./shared/cursor
[shared/fonts]: ./shared/fonts
[unl0kr]: ./unl0kr
[unl0kr#42]: https://gitlab.com/cherrypicker/unl0kr/-/issues/42
[f0rmz]: ./f0rmz
