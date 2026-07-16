project('buffybox', 'c',
    version: '3.5.1',
    default_options: ['warning_level=3', 'b_ndebug=if-release', 'b_lto=true'],
    meson_version: '>= 0.60.0'
)

add_project_arguments(
    '-DPROJECT_VERSION="@0@"'.format(meson.project_version()),
    '-DLV_BIG_ENDIAN_SYSTEM=' + (host_machine.endian() == 'big'? '1' : '0'),
    language: 'c'
)

if host_machine.cpu_family() == 'aarch64'
    add_project_arguments('-DLV_USE_DRAW_SW_ASM=LV_DRAW_SW_ASM_NEON', language: 'c')
else
    add_project_arguments('-DLV_USE_DRAW_SW_ASM=LV_DRAW_SW_ASM_NONE', language: 'c')
endif

depinih      = dependency('inih')
deplibinput  = dependency('libinput')
deplibudev   = dependency('libudev')
depxkbcommon = dependency('xkbcommon') # For unl0kr and f0rmz only

if get_option('man')
    depscdoc = dependency('scdoc', native: true)
endif

depsystemd = dependency('systemd', required: false)
systemd_system_unit_dir = depsystemd.get_variable(
    pkgconfig: 'systemd_system_unit_dir',
    pkgconfig_define: ['prefix', get_option('prefix')],
    default_value: get_option('prefix') / 'lib' / 'systemd' / 'system'
)

common_include_dirs = include_directories('.')

shared_sources = files(
    'shared/cursor/cursor.c',
    'shared/fonts/font_32.c',
    'shared/cli_common.c',
    'shared/config.c',
    'shared/force_feedback.c',
    'shared/indev.c',
    'shared/log.c',
    'shared/theme.c',
    'shared/themes.c'
)

shared_sources_ul_f0 = files(
    'shared/backends.c',
    'shared/display.c',
    'shared/header.c',
    'shared/terminal.c'
)

squeek2lvgl_sources = files(
    'squeek2lvgl/sq2lv_keyboard.c'
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
subdir('f0rmz')

if get_option('man')
    subdir('man')
endif
