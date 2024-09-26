project('buffybox', 'c',
    version: '3.2.0',
    default_options: 'warning_level=3',
    meson_version: '>= 0.59.0'
)

add_project_arguments('-DPROJECT_VERSION="@0@"'.format(meson.project_version()), language: 'c')

depinih      = dependency('inih')
deplibinput  = dependency('libinput')
deplibudev   = dependency('libudev')

if get_option('man')
    depscdoc = dependency('scdoc')
endif

common_include_dirs = include_directories('.')

shared_sources = files(
    'shared/cursor/cursor.c',
    'shared/fonts/font_32.c',
    'shared/config.c',
    'shared/indev.c',
    'shared/log.c',
    'shared/theme.c',
    'shared/themes.c'
)

squeek2lvgl_sources = files(
    'squeek2lvgl/sq2lv.c'
)

lvgl_sources = files(
    run_command('find-lvgl-sources.sh', 'lvgl', check: true).stdout().strip().split('\n')
)

common_dependencies = [
    depinih,
    deplibinput,
    deplibudev
]

subdir('unl0kr')
subdir('buffyboard')

if get_option('man')
    subdir('man')
endif
