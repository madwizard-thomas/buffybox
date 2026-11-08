# Copyright 2021 Johannes Marbach
# SPDX-License-Identifier: GPL-3.0-or-later

buffyboard_sources = files(
    'command_line.c',
    'config.c',
    'ebpf.c',
    'main.c',
    'sq2lv_layouts.c',
    'terminal.c',
    'uinput_device.c'
)

buffyboard_dependencies = [
    common_dependencies
]

buffyboard_args = [
    '-DBBX_APP_BUFFYBOARD'
]

executable('buffyboard',
    include_directories: common_include_dirs,
    sources: buffyboard_sources + shared_sources + squeek2lvgl_sources + lvgl_sources,
    dependencies: buffyboard_dependencies,
    c_args: buffyboard_args,
    install: true,
    install_tag: 'buffyboard'
)

install_data('buffyboard.conf',
    install_dir: get_option('sysconfdir'),
    install_tag: 'buffyboard'
)

if get_option('systemd')
    configure_file(
        configuration: {'bindir': get_option('prefix') / get_option('bindir')},
        input: 'buffyboard.service.in',
        output: 'buffyboard.service',
        install: true,
        install_dir: systemd_system_unit_dir,
        install_tag: 'buffyboard'
    )

    install_data('getty-buffyboard.conf',
        rename: 'buffyboard.conf',
        install_dir: systemd_system_unit_dir / 'getty@.service.d',
        install_tag: 'buffyboard'
    )
endif
