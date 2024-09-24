buffyboard(1) "buffyboard"

# NAME

BUFFYBOARD - Touch-enabled framebuffer keyboard (not only) for vampire slayers.

# SYNOPSIS

buffyboard [OPTION]

# DESCRIPTION

Buffyboard is a touch-enabled on-screen keyboard running on the Linux
framebuffer. It's primarily intended for vampire hunting but you can
also use it as a general purpose keyboard.

Buffyboard uses LVGL for input processing and rendering. Key events are
forwarded directly to the kernel via a uinput device. Since the latter
emulates a hardware keyboard, the terminal keymap must match with
buffyboard's layout or else on-screen keys might not produce the correct
result.

# OPTIONS

## Optional

*-C, --config-override*
	Path to a config override file. Can be supplied multiple times. Config
	files are merged in the following order:
		- /usr/share/buffyboard/buffyboard.conf
		- /usr/share/buffyboard/buffyboard.conf.d/\* (alphabetically)
		- /etc/buffyboard.conf
		- /etc/buffyboard.conf.d/\* (alphabetically)
		- Override files (in supplied order)
*-g, --geometry=NxM[@X,Y]*
	Force a display size of N horizontal times M vertical pixels, offset
	horizontally by X pixels and vertically by Y pixels.
*-d  --dpi=N*
	Override the display's DPI value.
*-r, --rotate=[0-3]*
	Rotate the UI to the given orientation. The
	values match the ones provided by the kernel in
	/sys/class/graphics/fbcon/rotate.
	* 0 - normal orientation (0 degree)
	* 1 - clockwise orientation (90 degrees)
	* 2 - upside down orientation (180 degrees)
	* 3 - counterclockwise orientation (270 degrees)
*-h, --help*
	Print this message and exit.
*-v, --verbose*
	Enable more detailed logging output on STDERR.
*-V, --version*
	Print the unl0kr version and exit.

# EXAMPLES

*Execute Buffyboard using the default config*
	buffyboard

# SEE ALSO
	*buffyboard.conf*(5)

# AUTHORS

*Undef* <debian@undef.tools>
