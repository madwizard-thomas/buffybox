buffyboard(5) "buffyboard"

# NAME

BUFFYBOARD - configuration file

# DESCRIPTION

BUFFYBOARD expects a configuration file with options in the following format, with
one option/value pair per line:

```
[section]
<option>=<value>
```

On launch, BUFFYBOARD searches for configuration files in various locations and
successively merges all files together into a single configuration. Files are searched
for and, if found, merged in the following order:
	- /usr/share/buffyboard/buffyboard.conf
	- /usr/share/buffyboard/buffyboard.conf.d/ (alphabetically)
	- /etc/buffyboard.conf
	- /etc/buffyboard.conf.d/ (alphabetically)
	- Override files (in supplied order)

# OPTIONS

## Theme
*default* = <adwaita-light|adwaita-dark|breezy-light|breezy-dark|nord-light|nord-dark|pmos-light|pmos-dark>
	Selects the default theme on boot. Can be changed at runtime to the
	alternative theme. Default: breezy-dark.

## Input
*pointer* = <true|false>
	Enable or disable the use of a hardware mouse or other pointing device.
	Default: true.

*touchscreen* = <true|false>
	Enable or disable the use of the touchscreen.
	Default: true.

## Quirks
*fbdev_force_refresh* = <true|false>
	If true and using the framebuffer backend, this triggers a display refresh
	after every draw operation. This has a negative performance impact.
	Default: false.

# SEE ALSO
	*buffyboard*(1)

# AUTHORS

*Undef* <debian@undef.tools>
*Johannes Marbach* <n0-0ne@mailbox.org>
