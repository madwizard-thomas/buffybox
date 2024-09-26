unl0kr(1) "unl0kr"

# NAME

UNL0KR - LVGL based initramfs disk unlocker

# SYNOPSIS

unl0kr [OPTION]

# DESCRIPTION

UNL0KR is a disk unlocker for the initramfs based on LVGL. By using LVGL and
rendering directly to the framebuffer it can work without relying on GPU
hardware acceleration. This makes for much more managable initramfs sizes.

Unl0kr utilises the CRYPTTAB_TRIED variable. Upon completion, the entered
password is printed to STDOUT. All other output happens on STDERR.

# OPTIONS

## Optional

*-C, --config-override*
	Path to a config override file. Can be supplied multiple times. Config
	files are merged in the following order:
		- /usr/share/unl0kr/unl0kr.conf
		- /usr/share/unl0kr/unl0kr.conf.d/\* (alphabetically)
		- /etc/unl0kr.conf
		- /etc/unl0kr.conf.d/\* (alphabetically)
		- Override files (in supplied order)
*-g, --geometry=NxM[@X,Y]*
	Force a display size of N horizontal times M vertical pixels, offset 
	horizontally by X pixels and vertically by Y pixels.
*-d  --dpi=N*               
	Override the display's DPI value.
*-h, --help*                
	Print this message and exit.
*-v, --verbose*             
	Enable more detailed logging output on STDERR.
*-V, --version*             
	Print the unl0kr version and exit.

# EXAMPLES

*Decrypt /dev/sda1 to name "root"*
	CRYPTTAB_SOURCE="/dev/sda1" CRYPTTAB_TRIED="0" unl0kr | cryptsetup --perf-no_read_workqueue --perf-no_write_workqueue open "/dev/sda1" root -

# SEE ALSO
	*unl0kr.conf*(5)

# AUTHORS

*Undef* <debian@undef.tools>
