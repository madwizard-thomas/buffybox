# Copyright 2021 Clayton Craft
# SPDX-License-Identifier: GPL-3.0-or-later

depxkbcommon = dependency('xkbcommon')

unl0kr_sources = files(
    'backends.c',
    'command_line.c',
    'config.c',
    'main.c',
    'sq2lv_layouts.c',
    'terminal.c'
)

unl0kr_dependencies = [
    common_dependencies,
    depxkbcommon
]

deplibdrm = dependency('libdrm', required: get_option('with-drm'))
if deplibdrm.found()
    unl0kr_dependencies += deplibdrm
    add_project_arguments('-DLV_USE_LINUX_DRM=1', language: 'c')
endif

executable('unl0kr',
    include_directories: common_include_dirs,
    sources: unl0kr_sources + shared_sources + squeek2lvgl_sources + lvgl_sources,
    dependencies: unl0kr_dependencies,
    install: true
)

install_data('unl0kr.conf', install_dir: get_option('sysconfdir'))

