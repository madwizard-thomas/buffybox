#!/usr/bin/env python3

# Copyright 2021 Johannes Marbach
# SPDX-License-Identifier: GPL-3.0-or-later


import argparse
from typing import Set
import git
import os
import sys
import tempfile
import yaml


###
# Global constants

outfile_c = 'sq2lv_layouts.c'
outfile_h = 'sq2lv_layouts.h'

repository_url = 'https://gitlab.gnome.org/World/Phosh/squeekboard.git'
rel_layouts_dir = 'data/keyboards'


###
# General helpers
##

def die(msg):
    """Print an error message to STDERR and exit with a non-zero code.
    
    msg -- message to output on STDERR
    """
    sys.stderr.write(msg if msg.endswith('\n') else msg + '\n')
    sys.exit(1)


def warn(msg):
    """Print a warning message to STDERR.
    
    msg -- message to output on STDERR
    """
    sys.stderr.write(msg if msg.endswith('\n') else msg + '\n')


def parse_arguments():
    """ Parse commandline arguments.
    """
    parser = argparse.ArgumentParser(description='Convert squeekboard layouts to LVGL-compatible C code.')
    parser.add_argument('--input', dest='input', action='append', required=True, help='squeekboard layout to '
                        + 'use as input for generation. Has to be a YAML file path relative to data/keyboards. '
                        + 'Can be specified multiple times.')
    parser.add_argument('--name', dest='name', action='append', required=True, help='name for the layout. '
                        + 'Needs to be specified once for every --input flag.')
    parser.add_argument('--extra-top-row-base', dest='extra_top_row_base', type=str, required=False, help='additional '
                        + 'key row to add at the top of the base layer.')
    parser.add_argument('--extra-top-row-upper', dest='extra_top_row_upper', type=str, required=False, help='additional '
                        + 'key row to add at the top of the upper layer.')
    parser.add_argument('--shift-keycap', dest='shift_keycap', type=str, required=False, help='key caption for '
                        + 'the Shift key. Defaults to "Shift".')
    parser.add_argument('--surround-space-with-arrows', action='store_true', dest='arrows_around_space',
                        help='insert left / right arrow before / after space key')
    parser.add_argument('--generate-scancodes', action='store_true', dest='generate_scancodes', help='also '
                        + 'generate scancode tables (only works for US layout currently)')
    parser.add_argument('--output', dest='output', type=str, required=True, help='output directory for generated '
                        + 'files')
    args = parser.parse_args()

    if not args.output or not os.path.isdir(args.output):
        die('Error: no valid output directory specified')

    return args


def clone_squeekboard_repo(destination):
    """ Clone the squeekboard git repository.

    destination -- directory path to clone to
    """
    git.Repo.clone_from(repository_url, destination, depth=1)


def load_yaml(layouts_dir, rel_path):
    """ Load a YAML file and return its dictionary representation.
    
    rel_path -- path of the YAML file relative to the layouts root directory
    """
    path = os.path.join(layouts_dir, rel_path)
    if not os.path.isfile(path):
        die(f'could not find input file {path}')

    data = None
    with open(path, 'r') as stream:
        try:
            data = yaml.safe_load(stream)
        except yaml.YAMLError as exc:
            die(f'Could not load YAML file {path}: {exc}')
    if not data:
        die(f'could not load input file {path}')
    if not 'views' in data:
        die(f'no "views" element in YAML data loaded from input file {path}')
    
    return data


def write_files(lines_c, lines_h):
    """Write accumulated output to C and header file, respectively.

    lines_c -- sequence of lines to write to C file
    lines_h -- sequence of lines to write to header
    """
    with open(os.path.join(args.output, outfile_c), 'w') as fp:
        fp.write('\n'.join(lines_c))
    with open(os.path.join(args.output, outfile_h), 'w') as fp:
        fp.write('\n'.join(lines_h))


def comma_if_needed(sequence, idx):
    """Return a comma unless idx points to the last element in sequence.
    
    sequence -- a sequence of elements
    idx -- an index into the sequence
    """
    return ',' if idx < len(sequence) - 1 else ''


###
# SourceFileBuilder
##

class SourceFileBuilder(object):
    """Builder for .c and .h files.
    """

    def __init__(self):
        """Constructor.
        """
        self.lines = [] 
        self._add_header_comment()

    def add_line(self, line=None):
        """Add a single line and return the builder.
        
        line - string representing the line to add (if None, an empty line will be added)
        """
        self.lines.append(line if line else '')
        return self

    def add_lines(self, lines):
        """Add multiple lines and return the builder.
        
        lines - a list of strings representing the lines to add
        """
        self.lines += lines
        return self

    def wrap_in_ifndef(self, macro):
        """Wrap all of the current content in a macro check to prevent double inclusion and
        return the builder.
        
        macro -- name of the macro to use
        """
        idx = 0
        while self.lines[idx][:2] in ['/*', ' *']:
            idx += 1
        self.lines.insert(idx, '')
        self.lines.insert(idx + 1, f'#ifndef {macro}')
        self.lines.insert(idx + 2, f'#define {macro}')
        self.add_lines([f'#endif /* {macro} */', ''])

    def _add_header_comment(self):
        """Add the generator header comment and return the builder.
        """
        self.add_lines(['/**', ' * Auto-generated with squeek2lvgl', ' **/', ''])
        return self

    def add_include(self, header):
        """Add an include statement and return the builder.
        
        header -- path to file to be included
        """
        self.add_line(f'#include "{header}"')
        return self

    def add_system_include(self, header):
        """Add a system include statement and return the builder.
        
        header -- path to file to be included
        """
        self.add_line(f'#include <{header}>')
        return self

    def add_section_comment(self, title):
        """Add a comment marking the beginning of a section and return the builder.
        
        title -- title of the section
        """
        self.add_lines(['/**', f' * {title}', ' **/'])
        return self

    def add_subsection_comment(self, title):
        """Add a comment marking the beginning of a subsection and return the builder.
        
        title -- title of the subsection
        """
        self.add_line(f'/* {title} */')
        return self

    def add_array(self, static, type, identifier, values, row_terminator, array_terminator):
        """Add a row-based C array and return the builder.
        
        static -- True if the variable is static
        type -- variable type
        identifier -- variable identifier
        values -- values per row as a list of lists,
        row_terminator -- element to append to each row except the last
        array_terminator -- element to append to the last row
        """
        prefix = 'static ' if static else ''

        if not values or not values[0]:
            self.add_line(f'{prefix}{type} * const {identifier} = NULL;')
            return self

        self.add_line(f'{prefix}{type} {identifier}[] = ' + '{ \\')
        for i, values_in_row in enumerate(values):
            elements = values_in_row
            if i < len(values) - 1 and row_terminator:
                elements.append(row_terminator)
            if i == len(values) - 1 and array_terminator:
                elements.append(array_terminator)
            joined = ', '.join([f'{e}' for e in elements])
            self.add_line(f'    {joined}{comma_if_needed(values, i)} \\')
        self.add_line('};')

        return self

    def add_flat_array(self, static, type, identifier, values, array_terminator):
        """Add a flat C array and return the builder.
        
        static -- True if the variable is static
        type -- variable type
        identifier -- variable identifier
        values -- list of values,
        array_terminator -- element to append after the last element
        """
        return self.add_array(static, type, identifier, [values], '', array_terminator)


###
# Layout processing
##

def layout_id_to_c_identifier(layout_id):
    """Return a string for a layout that is suitable to be used in a C identifier.
    
    layout_id -- ID of the layout
    """
    return layout_id.lower().replace('/', '_')


layer_name_for_view_id = {
    'base': 'Lowercase letters',
    'upper': 'Uppercase letters',
    'numbers': 'Numbers / symbols',
    'eschars': 'Special characters',
    'symbols': 'Symbols',
    'actions': 'Actions'
}

def view_id_to_layer_name(view_id):
    """Return a descriptive name for a layer based on its view ID.
    
    view_id -- the ID of the view representing the layer
    """
    if view_id not in layer_name_for_view_id:
        return None
    return layer_name_for_view_id[view_id]


layer_identifier_for_view_id = {
    'base': 'lower',
    'eschars': 'special'
}

def view_id_to_c_identifier(view_id):
    """Return a string for a view that is suitable to be used in a C identifier.
    
    view_id -- ID of the view
    """
    return layer_identifier_for_view_id[view_id] if view_id in layer_identifier_for_view_id else view_id


ignored_keys = {
    'preferences'
}

def is_key_ignored(key):
    """Return True if a key should be ignored or False otherwise.
    
    key -- the key in question
    """
    return key in ignored_keys


keycap_for_key = {
    '\\': '\\\\',
    '"': '\\"',
    '↑': 'LV_SYMBOL_UP',
    '↓': 'LV_SYMBOL_DOWN',
    '←': 'LV_SYMBOL_LEFT',
    '→': 'LV_SYMBOL_RIGHT',
    'BackSpace': 'LV_SYMBOL_BACKSPACE',
    'colon': ':',
    'period': '.',
    'Shift_L': 'SQ2LV_SYMBOL_SHIFT',
    'space': ' ',
    'Return': 'LV_SYMBOL_OK',
    'Up': 'LV_SYMBOL_UP',
    'Left': 'LV_SYMBOL_LEFT',
    'Down': 'LV_SYMBOL_DOWN',
    'Right': 'LV_SYMBOL_RIGHT'
}

def key_to_keycap(args, key):
    """Return the keycap for a key.
    
    args -- commandline arguments
    key -- the key
    """
    return keycap_for_key[key] if key in keycap_for_key else key


def key_is_modifier(key, data_buttons):
    """Return true if a key acts as a modifier.

    key -- the key in question
    data_buttons -- the "buttons" object from the layout's YAML file
    """
    return key in data_buttons and 'modifier' in data_buttons[key]


repeatable_keys = {
    'BackSpace',
    'Del',
    'PgUp',
    'PgDn',
    'Return',
    'space',
    '↑',
    '←',
    '↓',
    '→',
    'Up',
    'Left',
    'Down',
    'Right'
}

def key_can_repeat(key):
    """Return True if a key can repeatedly emit while being held down.

    key -- the key
    """
    return key in repeatable_keys


def key_to_attributes(key, is_locked, is_lockable, is_extra_top_row, data_buttons):
    """Return the LVGL button attributes for a key.
    
    key -- the key in question
    is_locked - whether the key is locked  in the current view
    is_lockable - whether the key can be locked in the current view
    is_extra_top_row - whether the key is in the extra top row
    data_buttons -- the "buttons" object from the layout's YAML file
    """
    attributes = []

    if key_is_modifier(key, data_buttons):
        attributes.append('SQ2LV_CTRL_MOD_INACTIVE')
    elif is_locked:
        attributes.append('SQ2LV_CTRL_MOD_ACTIVE')
    elif is_lockable:
        attributes.append('SQ2LV_CTRL_MOD_INACTIVE')
    elif key in data_buttons and key not in ['"', 'colon', 'period', 'space'] or key in ['↑', '←', '↓', '→']:
        attributes.append('SQ2LV_CTRL_NON_CHAR')
    elif key not in ['space']:
        attributes.append('LV_BUTTONMATRIX_CTRL_POPOVER')

    if not key_can_repeat(key):
        attributes.append('LV_BUTTONMATRIX_CTRL_NO_REPEAT')

    if is_extra_top_row:
        attributes.append('SQ2LV_CTRL_NON_CHAR')
    if key == '<hidden>':
        attributes.append('LV_BUTTONMATRIX_CTRL_HIDDEN')

    if key not in data_buttons or key in ['"', 'colon', 'period']:
        attributes.append('2')
    elif key == 'space':
        attributes.append('7')
    else:
        attributes.append('3')

    return ' | '.join(attributes)


def keycap_to_c_value(keycap):
    """Return the right-hand side C value for a keycap
    """
    return keycap if keycap.startswith('LV_') or keycap.startswith('SQ2LV_') else f'"{keycap}"'


scancodes_for_keycap = {
    '0': ['KEY_0'],
    '1': ['KEY_1'],
    '2': ['KEY_2'],
    '3': ['KEY_3'],
    '4': ['KEY_4'],
    '5': ['KEY_5'],
    '6': ['KEY_6'],
    '7': ['KEY_7'],
    '8': ['KEY_8'],
    '9': ['KEY_9'],
    'a': ['KEY_A'],
    'b': ['KEY_B'],
    'c': ['KEY_C'],
    'd': ['KEY_D'],
    'e': ['KEY_E'],
    'f': ['KEY_F'],
    'g': ['KEY_G'],
    'h': ['KEY_H'],
    'i': ['KEY_I'],
    'j': ['KEY_J'],
    'k': ['KEY_K'],
    'l': ['KEY_L'],
    'm': ['KEY_M'],
    'n': ['KEY_N'],
    'o': ['KEY_O'],
    'p': ['KEY_P'],
    'q': ['KEY_Q'],
    'r': ['KEY_R'],
    's': ['KEY_S'],
    't': ['KEY_T'],
    'u': ['KEY_U'],
    'v': ['KEY_V'],
    'w': ['KEY_W'],
    'x': ['KEY_X'],
    'y': ['KEY_Y'],
    'z': ['KEY_Z'],
    'A': ['KEY_LEFTSHIFT', 'KEY_A'],
    'B': ['KEY_LEFTSHIFT', 'KEY_B'],
    'C': ['KEY_LEFTSHIFT', 'KEY_C'],
    'D': ['KEY_LEFTSHIFT', 'KEY_D'],
    'E': ['KEY_LEFTSHIFT', 'KEY_E'],
    'F': ['KEY_LEFTSHIFT', 'KEY_F'],
    'G': ['KEY_LEFTSHIFT', 'KEY_G'],
    'H': ['KEY_LEFTSHIFT', 'KEY_H'],
    'I': ['KEY_LEFTSHIFT', 'KEY_I'],
    'J': ['KEY_LEFTSHIFT', 'KEY_J'],
    'K': ['KEY_LEFTSHIFT', 'KEY_K'],
    'L': ['KEY_LEFTSHIFT', 'KEY_L'],
    'M': ['KEY_LEFTSHIFT', 'KEY_M'],
    'N': ['KEY_LEFTSHIFT', 'KEY_N'],
    'O': ['KEY_LEFTSHIFT', 'KEY_O'],
    'P': ['KEY_LEFTSHIFT', 'KEY_P'],
    'Q': ['KEY_LEFTSHIFT', 'KEY_Q'],
    'R': ['KEY_LEFTSHIFT', 'KEY_R'],
    'S': ['KEY_LEFTSHIFT', 'KEY_S'],
    'T': ['KEY_LEFTSHIFT', 'KEY_T'],
    'U': ['KEY_LEFTSHIFT', 'KEY_U'],
    'V': ['KEY_LEFTSHIFT', 'KEY_V'],
    'W': ['KEY_LEFTSHIFT', 'KEY_W'],
    'X': ['KEY_LEFTSHIFT', 'KEY_X'],
    'Y': ['KEY_LEFTSHIFT', 'KEY_Y'],
    'Z': ['KEY_LEFTSHIFT', 'KEY_Z'],
    'Alt': ['KEY_LEFTALT'],
    'Ctrl': ['KEY_LEFTCTRL'],
    'LV_SYMBOL_UP': ['KEY_UP'],
    'LV_SYMBOL_DOWN': ['KEY_DOWN'],
    'LV_SYMBOL_LEFT': ['KEY_LEFT'],
    'LV_SYMBOL_RIGHT': ['KEY_RIGHT'],
    'LV_SYMBOL_BACKSPACE': ['KEY_BACKSPACE'],
    'LV_SYMBOL_OK': ['KEY_ENTER'],
    ' ': ['KEY_SPACE'],
    'ABC': [],
    'abc': [],
    '123': [],
    'PgUp': ['KEY_PAGEUP'],
    'PgDn': ['KEY_PAGEDOWN'],
    'Home': ['KEY_HOME'],
    'End': ['KEY_END'],
    '*': ['KEY_LEFTSHIFT', 'KEY_8'],
    '#': ['KEY_LEFTSHIFT', 'KEY_3'],
    '$': ['KEY_LEFTSHIFT', 'KEY_4'],
    '/': ['KEY_SLASH'],
    '&': ['KEY_LEFTSHIFT', 'KEY_7'],
    '-': ['KEY_MINUS'],
    '_': ['KEY_LEFTSHIFT', 'KEY_MINUS'],
    '+': ['KEY_LEFTSHIFT', 'KEY_EQUAL'],
    '(': ['KEY_LEFTSHIFT', 'KEY_9'],
    ')': ['KEY_LEFTSHIFT', 'KEY_0'],
    ',': ['KEY_COMMA'],
    '\\"': ['KEY_LEFTSHIFT', 'KEY_APOSTROPHE'],
    '\'': ['KEY_APOSTROPHE'],
    ':': ['KEY_LEFTSHIFT', 'KEY_SEMICOLON'],
    ';': ['KEY_SEMICOLON'],
    '!': ['KEY_LEFTSHIFT', 'KEY_1'],
    '?': ['KEY_LEFTSHIFT', 'KEY_SLASH'],
    '.': ['KEY_DOT'],
    '~': ['KEY_LEFTSHIFT', 'KEY_GRAVE'],
    '`': ['KEY_GRAVE'],
    '|': ['KEY_LEFTSHIFT', 'KEY_BACKSLASH'],
    # '·': [],
    # '√': [],
    # 'π': [],
    # 'τ': [],
    # '÷': [],
    # '×': [],
    # '¶': [],
    # '©': [],
    # '®': [],
    # '£': [],
    # '€': [],
    # '¥': [],
    '\\\\': ['KEY_BACKSLASH'],
    '^': ['KEY_LEFTSHIFT', 'KEY_6'],
    # '°': [],
    '@': ['KEY_LEFTSHIFT', 'KEY_2'],
    '{': ['KEY_LEFTSHIFT', 'KEY_LEFTBRACE'],
    '}': ['KEY_LEFTSHIFT', 'KEY_RIGHTBRACE'],
    '%': ['KEY_LEFTSHIFT', 'KEY_5'],
    '<': ['KEY_LEFTSHIFT', 'KEY_COMMA'],
    '>': ['KEY_LEFTSHIFT', 'KEY_DOT'],
    '=': ['KEY_EQUAL'],
    '[': ['KEY_LEFTBRACE'],
    ']': ['KEY_RIGHTBRACE'],
    'F1': ['KEY_F1'],
    'F2': ['KEY_F2'],
    'F3': ['KEY_F3'],
    'F4': ['KEY_F4'],
    'F5': ['KEY_F5'],
    'F6': ['KEY_F6'],
    'F7': ['KEY_F7'],
    'F8': ['KEY_F8'],
    'F9': ['KEY_F9'],
    'F10': ['KEY_F10'],
    'F11': ['KEY_F11'],
    'F12': ['KEY_F12'],
    'Esc': ['KEY_ESC'],
    'Tab': ['KEY_TAB'],
    'Pause': ['KEY_PAUSE'],
    'Insert': ['KEY_INSERT'],
    'Del': ['KEY_DELETE'],
    'Menu': ['KEY_COMPOSE'],
    'Break': ['KEY_BREAK'],
    '↑': ['KEY_UP'],
    '←': ['KEY_LEFT'],
    '↓': ['KEY_DOWN'],
    '→': ['KEY_RIGHT']
}

def keycap_to_scancodes(args, keycap, is_switcher):
    """Return the scancodes needed to produce a keycap
    
    args -- commandline arguments
    keycap -- keycap to produce
    is_switcher -- whether the key is a layer switcher
    """
    if is_switcher:
        return []
    if keycap not in scancodes_for_keycap:
        warn(f'Cannot determine scancodes for unknown keycap "{keycap}"')
        return []
    return scancodes_for_keycap[keycap]


def get_keycaps_attrs_modifiers_switchers_scancodes(args, view_id, data_views, data_buttons, extra_top_row):
    """Return keycaps, LVGL button attributes, modifier key indexes, layer switching key indexes,
    layer switching key destinations and scancodes for a view
    
    args -- commandline arguments
    view_id -- ID of the view
    data_views -- the "views" object from the layout's YAML file
    data_buttons -- the "buttons" object from the layout's YAML file
    extra_top_row -- additional row of keys to insert at the top or "<hidden>" to insert an empty row
    """
    keycaps = []
    attrs = []
    modifier_idxs = []
    switcher_idxs = []
    switcher_dests = []
    scancodes = []

    idx = 0

    rows = data_views[view_id]
    if extra_top_row:
        rows = [extra_top_row] + rows

    for index, row in enumerate(rows):
        keycaps_in_row = []
        attrs_in_row = []
        scancodes_in_row = []

        keys = row.split()

        if args.arrows_around_space:
            space_idx = None
            try:
                space_idx = keys.index('space')
            except ValueError:
                pass
            if space_idx != None:
                keys.insert(space_idx, '←')
                keys.insert(space_idx + 2, '→')

        for key in keys:
            if is_key_ignored(key):
                continue

            keycap = None

            if key in data_buttons and 'label' in data_buttons[key] and key not in ['Up', 'Left', 'Down', 'Right']:
                keycap = data_buttons[key]['label'].replace('\\', '\\\\')
            else:
                keycap = key_to_keycap(args, key)

            if not keycap:
                continue

            keycaps_in_row.append(keycap_to_c_value(keycap))

            if key_is_modifier(key, data_buttons):
                modifier_idxs.append(idx)

            is_locked = False
            is_lockable = False
            is_switcher = False

            if key in data_buttons and 'action' in data_buttons[key]:
                action = data_buttons[key]['action']
                dest = None

                if 'set_view' in action:
                    dest = action['set_view']
                elif 'locking' in action and 'lock_view' in action['locking'] and 'unlock_view' in action['locking']:
                    if action['locking']['lock_view'] == view_id:
                        dest = action['locking']['unlock_view']
                        is_locked = True
                    else:
                        dest = action['locking']['lock_view']
                        is_lockable = True
                if dest:
                    switcher_idxs.append(idx)
                    switcher_dests.append(dest)
                    is_switcher = True

            attrs_in_row.append(key_to_attributes(key, is_locked, is_lockable, extra_top_row and index == 0, data_buttons))

            if args.generate_scancodes:
                scancodes_in_row.append(keycap_to_scancodes(args, keycap, is_switcher))

            idx += 1

        keycaps.append(keycaps_in_row)
        attrs.append(attrs_in_row)
        scancodes.append(scancodes_in_row)

    return keycaps, attrs, modifier_idxs, switcher_idxs, switcher_dests, scancodes


def flatten_scancodes(scancodes):
    """Process a nested list of scancodes per row and key and return a flattened list of scancodes per row,
    a list of starting indexes and a list of scancode counts.
    
    scancodes -- list (rows) of list (keys) of list (scancodes) of scancodes
    """
    flat = []
    idxs = []
    nums = []

    if args.generate_scancodes:
        idx = 0
        num = 0

        for scancodes_in_row in scancodes:
            flat_in_row = []
            idxs_in_row = []
            nums_in_row = []

            for codes in scancodes_in_row:
                flat_in_row += codes
                idxs_in_row.append(idx if len(codes) > 0 else -1)
                nums_in_row.append(len(codes))
                idx += len(codes)

            flat.append(flat_in_row)
            idxs.append(idxs_in_row)
            nums.append(nums_in_row)

    return flat, idxs, nums


###
# Main
##

if __name__ == '__main__':
    args = parse_arguments()

    c_builder = SourceFileBuilder()
    c_builder.add_include(outfile_h)
    if args.generate_scancodes:
        c_builder.add_system_include('linux/input.h')
    c_builder.add_line()

    shift_keycap = args.shift_keycap if args.shift_keycap else 'Shift'
    c_builder.add_line(f'#define SQ2LV_SYMBOL_SHIFT "{shift_keycap}"')
    c_builder.add_line()

    h_builder = SourceFileBuilder()
    h_builder.add_include('lvgl/lvgl.h')
    h_builder.add_line()
    h_builder.add_include('stdint.h')
    h_builder.add_line()
    h_builder.add_line(f'#define SQ2LV_CTRL_NON_CHAR     (LV_BUTTONMATRIX_CTRL_CLICK_TRIG | LV_BUTTONMATRIX_CTRL_CHECKED)')
    h_builder.add_line(f'#define SQ2LV_CTRL_MOD_ACTIVE   (LV_BUTTONMATRIX_CTRL_CLICK_TRIG | LV_BUTTONMATRIX_CTRL_CHECKABLE)')
    h_builder.add_line(f'#define SQ2LV_CTRL_MOD_INACTIVE (LV_BUTTONMATRIX_CTRL_CLICK_TRIG | LV_BUTTONMATRIX_CTRL_CHECKABLE | LV_BUTTONMATRIX_CTRL_CHECKED)')
    h_builder.add_line()
    h_builder.add_line(f'#define SQ2LV_SCANCODES_ENABLED {1 if args.generate_scancodes else 0}')
    h_builder.add_line()

    layouts = []
    unique_scancodes = {}

    with tempfile.TemporaryDirectory() as tmp:
        clone_squeekboard_repo(tmp)

        layouts_dir = os.path.join(tmp, rel_layouts_dir)

        for file, layout_name in zip(args.input, args.name):
            layout_id, _ = os.path.splitext(file)
            layout_identifier = layout_id_to_c_identifier(layout_id)

            data = load_yaml(layouts_dir, file)
            data_views = data['views']
            data_buttons = data['buttons'] if 'buttons' in data else {}

            c_builder.add_section_comment(f'Layout: {layout_name} - generated from {layout_id}')
            c_builder.add_line()
            c_builder.add_line(f'static const char * const name_{layout_identifier} = "{layout_name}";')
            c_builder.add_line(f'static const char * const short_name_{layout_identifier} = "{layout_id}";')
            c_builder.add_line()

            layer_identifiers = []

            view_ids = [view_id for view_id in data_views if view_id_to_layer_name(view_id) != None]

            for view_id in data_views:
                layer_name = view_id_to_layer_name(view_id)
                if not layer_name:
                    warn(f'Ignoring unknown view_id {view_id}')
                    continue

                layer_identifier = f'{view_id_to_c_identifier(view_id)}_{layout_identifier}'
                layer_identifiers.append(layer_identifier)

                c_builder.add_subsection_comment(f'Layer: {layer_name} - generated from {view_id}')
                c_builder.add_line()
 
                extra_top_row = None
                if view_id == "base":
                    extra_top_row = args.extra_top_row_base
                if view_id == "upper":
                    extra_top_row = args.extra_top_row_upper
                if not extra_top_row and (args.extra_top_row_base or args.extra_top_row_upper):
                    extra_top_row = "<hidden>"

                keycaps, attrs, modifier_idxs, switcher_idxs, switcher_dests, scancodes = get_keycaps_attrs_modifiers_switchers_scancodes(
                    args, view_id, data_views, data_buttons, extra_top_row)

                for dest in switcher_dests:
                    if dest not in view_ids:
                        die(f'Unhandled layer switch destination {dest}')
                switcher_dests = [view_ids.index(d) for d in switcher_dests if d in view_ids]

                c_builder.add_line(f'static const uint32_t num_keys_{layer_identifier} = {sum([len(row) for row in keycaps])};')
                c_builder.add_line()
                c_builder.add_array(True, 'const char * const', f'keycaps_{layer_identifier}', keycaps, '"\\n"', '""')
                c_builder.add_line()
                c_builder.add_array(True, 'const lv_buttonmatrix_ctrl_t', f'attributes_{layer_identifier}', attrs, '', '')
                c_builder.add_line()

                c_builder.add_line(f'static const uint32_t num_modifiers_{layer_identifier} = {len(modifier_idxs)};')
                c_builder.add_line()
                c_builder.add_flat_array(True, 'const uint32_t', f'modifier_idxs_{layer_identifier}', modifier_idxs, '')
                c_builder.add_line()

                c_builder.add_line(f'static const uint32_t num_switchers_{layer_identifier} = {len(switcher_idxs)};')
                c_builder.add_line()
                c_builder.add_flat_array(True, 'const uint32_t', f'switcher_idxs_{layer_identifier}', switcher_idxs, '')
                c_builder.add_line()
                c_builder.add_flat_array(True, 'const int', f'switcher_dests_{layer_identifier}', switcher_dests, '')
                c_builder.add_line()

                if args.generate_scancodes:
                    scancodes_flat, scancode_idxs, scancode_nums = flatten_scancodes(scancodes)

                    for scancodes_in_row in scancodes_flat:
                        for scancode in scancodes_in_row:
                            unique_scancodes[scancode] = True

                    c_builder.add_line(f'static const int num_scancodes_{layer_identifier} = {len(scancodes)};')
                    c_builder.add_line()
                    c_builder.add_array(True, 'const int', f'scancodes_{layer_identifier}', scancodes_flat, '', '')
                    c_builder.add_line()
                    c_builder.add_array(True, 'const int', f'scancode_idxs_{layer_identifier}', scancode_idxs, '', '')
                    c_builder.add_line()
                    c_builder.add_array(True, 'const int', f'scancode_nums_{layer_identifier}', scancode_nums, '', '')
                    c_builder.add_line()

            c_builder.add_subsection_comment(f'Layer array')
            c_builder.add_line()
            c_builder.add_line(f'static const int num_layers_{layout_identifier} = {len(layer_identifiers)};')
            c_builder.add_line()
            c_builder.add_line(f'static const sq2lv_layer_t layers_{layout_identifier}[] = ' + '{')
            for i, identifier in enumerate(layer_identifiers):
                c_builder.add_line('    {')
                fields = ['num_keys', 'keycaps', 'attributes', 'num_modifiers', 'modifier_idxs', 'num_switchers', 'switcher_idxs', 'switcher_dests']
                if args.generate_scancodes:
                    fields += ['num_scancodes', 'scancodes', 'scancode_idxs', 'scancode_nums']
                for k, field in enumerate(fields):
                    c_builder.add_line(f'        .{field} = {field}_{identifier}{comma_if_needed(fields, k)}')
                c_builder.add_line('    }' + comma_if_needed(layer_identifiers, i))
            c_builder.add_line('};')
            c_builder.add_line()

            layouts.append({
                'name': layout_name,
                'short_name': layout_id,
                'identifier': layout_identifier
            })

    h_builder.add_line('/* Layout IDs, values can be used as indexes into the sq2lv_layouts array */')
    h_builder.add_line('typedef enum {')
    h_builder.add_line('    SQ2LV_LAYOUT_NONE = -1,')
    for i, layout in enumerate(layouts):
        identifier = layout['identifier'].upper()
        h_builder.add_line(f'    SQ2LV_LAYOUT_{identifier} = {i}{comma_if_needed(layouts, i)}')
    h_builder.add_line('} sq2lv_layout_id_t;')
    h_builder.add_line()

    h_builder.add_line('/* Layer type */')
    h_builder.add_line('typedef struct {')
    h_builder.add_line('    /* Number of keys */')
    h_builder.add_line('    const uint32_t num_keys;')
    h_builder.add_line('    /* Key caps */')
    h_builder.add_line('    const char * const * const keycaps;')
    h_builder.add_line('    /* Key attributes */')
    h_builder.add_line('    const lv_buttonmatrix_ctrl_t * const attributes;')
    h_builder.add_line('    /* Number of modifier keys */')
    h_builder.add_line('    const uint32_t num_modifiers;')
    h_builder.add_line('    /* Button indexes of modifier keys */')
    h_builder.add_line('    const uint32_t * const modifier_idxs;')
    h_builder.add_line('    /* Number of buttons that trigger a layer switch */')
    h_builder.add_line('    const uint32_t num_switchers;')
    h_builder.add_line('    /* Button indexes that trigger a layer switch */')
    h_builder.add_line('    const uint32_t * const switcher_idxs;')
    h_builder.add_line('    /* Indexes of layers to jump to when triggering layer switch buttons */')
    h_builder.add_line('    const int * const switcher_dests;')
    if args.generate_scancodes:
        h_builder.add_line('    /* Total number of scancodes */')
        h_builder.add_line('    const int num_scancodes;')
        h_builder.add_line('    /* Flat array of scancodes */')
        h_builder.add_line('    const int * const scancodes;')
        h_builder.add_line('    /* Start index in scancodes array for key cap */')
        h_builder.add_line('    const int * const scancode_idxs;')
        h_builder.add_line('    /* Number of scancodes for key cap */')
        h_builder.add_line('    const int * const scancode_nums;')
    h_builder.add_line('} sq2lv_layer_t;')
    h_builder.add_line()

    h_builder.add_line('/* Layout type */')
    h_builder.add_line('typedef struct {')
    h_builder.add_line('    /* Layout name */')
    h_builder.add_line('    const char * const name;')
    h_builder.add_line('    /* Layout short name */')
    h_builder.add_line('    const char * const short_name;')
    h_builder.add_line('    /* Total number of layers */')
    h_builder.add_line('    const int num_layers;')
    h_builder.add_line('    /* Layers array */')
    h_builder.add_line('    const sq2lv_layer_t * const layers;')
    h_builder.add_line('} sq2lv_layout_t;')
    h_builder.add_line()

    h_builder.add_line('/* Layouts */')
    h_builder.add_line('extern const int sq2lv_num_layouts;')
    h_builder.add_line('extern const sq2lv_layout_t sq2lv_layouts[];')
    h_builder.add_line()

    h_builder.add_line('/* Layout names (suitable for use in lv_dropdown_t) */')
    h_builder.add_line('extern const char * const sq2lv_layout_names;') 
    h_builder.add_line('extern const char * const sq2lv_layout_short_names;') 
    h_builder.add_line()

    if args.generate_scancodes:
        h_builder.add_line('/* Unique scancodes from all layout (suitable for setting up uinput devices) */')
        h_builder.add_line('extern const int sq2lv_num_unique_scancodes;')
        h_builder.add_line('extern const int sq2lv_unique_scancodes[];')
        h_builder.add_line()

    c_builder.add_section_comment('Public interface')
    c_builder.add_line()

    c_builder.add_line('const int sq2lv_num_layouts = ' + str(len(layouts)) + ';')
    c_builder.add_line()

    c_builder.add_line('const sq2lv_layout_t sq2lv_layouts[] = {')
    for i, layout in enumerate(layouts):
        c_builder.add_line('    /* ' + layout['name'] + ' */')
        c_builder.add_line('    {')
        fields = ['name', 'short_name', 'num_layers', 'layers']
        identifier = layout['identifier']
        for j, field in enumerate(fields):
            c_builder.add_line(f'        .{field} = {field}_{identifier}{comma_if_needed(fields, j)}')
        c_builder.add_line('    }' + comma_if_needed(layouts, i))
    c_builder.add_line('};')
    c_builder.add_line()

    names = [layout['name'] for layout in layouts]
    names = '\n    ' + ' "\\n"\n    '.join([f'"{name}"' for name in names])
    c_builder.add_line(f'const char * const sq2lv_layout_names ={names};')
    c_builder.add_line()

    short_names = [layout['short_name'] for layout in layouts]
    short_names = '\n    ' + ' "\\n"\n    '.join([f'"{short_name}"' for short_name in short_names])
    c_builder.add_line(f'const char * const sq2lv_layout_short_names ={short_names};')
    c_builder.add_line()

    if args.generate_scancodes:
        c_builder.add_line(f'const int sq2lv_num_unique_scancodes = {len(unique_scancodes)};')
        c_builder.add_line()
        c_builder.add_line('const int sq2lv_unique_scancodes[] = {')
        scancodes = list(unique_scancodes.keys())
        chunks = [scancodes[i:i + 10] for i in range(0, len(scancodes), 10)]
        for i, chunk in enumerate(chunks):
            joined = ', '.join(chunk)
            c_builder.add_line(f'    {joined}{comma_if_needed(chunks, i)}')
        c_builder.add_line('};')
        c_builder.add_line()

    h_builder.wrap_in_ifndef('SQ2LV_LAYOUTS_H')

    write_files(c_builder.lines, h_builder.lines)
