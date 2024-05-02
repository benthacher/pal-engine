#!.venv/bin/python
"""midi_to_c.py

Script to convert midi files to .c and .h source and header files

"""
import os
import re
from pathlib import Path

import click


def get_c_symbol(symbol: str):
    """Converts input symbol to a valid c symbol

    Args:
        symbol: Input symbol

    Returns:
        Valid c symbol
    """
    return re.sub(r'\W+', '_', symbol)


def midi_file_to_c(midi_path: str, output_src_directory: str, output_inc_directory: str, include_path: str = ''):
    """Creates .c and .h files containing midi file data

    Args:
        midi_path: Path to midi file
        output_src_directory: Directory to place generated midi source file
        output_inc_directory: Directory to place generated midi header file
        include_path: Include path to prepend to the included header in the source file

    Raises:
        click.ClickException: If input file isn't a midi
    """
    midi_path = Path(midi_path)

    if midi_path.suffix not in [ '.mid', '.midi' ]:
        raise click.ClickException("Input file is not a midi file!")

    midi_size = os.stat(midi_path).st_size
    midi_symbol = get_c_symbol(midi_path.stem) + '_midi'
    source_filename = midi_path.stem + '.c'
    header_filename = midi_path.stem + '.h'

    output_c_path = Path(output_src_directory, source_filename)
    output_h_path = Path(output_inc_directory, header_filename)

    with open(output_h_path, 'w', encoding='utf8') as out:
        out.write("#pragma once\n\n")
        out.write("#include <stdint.h>\n\n")
        out.write(f"extern const uint8_t {midi_symbol}_data[{midi_size}];\n\n")

    with open(output_c_path, 'w', encoding='utf8') as out:
        out.write(f"#include \"{Path(include_path, header_filename)}\"\n\n")
        out.write("#include <stdint.h>\n\n")
        out.write(f"const uint8_t {midi_symbol}_data[{midi_size}] = {{\n")

        with open(midi_path, 'rb') as midi_data:
            count = 0
            out.write('    ')

            while True:
                b = midi_data.read(1)

                if not b:
                    break

                out.write(f'0x{b[0]:02x},')

                count += 1

                if count > 30:
                    out.write('\n    ')
                    count = 0

        out.write("\n};")


@click.command()
@click.argument("midi_file", nargs=-1, type=click.Path(exists=True, readable=True))
@click.argument("output_src_directory", type=click.Path(exists=True, file_okay=False, dir_okay=True))
@click.argument("output_inc_directory", type=click.Path(exists=True, file_okay=False, dir_okay=True))
@click.argument("include_path", default='')
def midi_to_c(midi_file: str, output_src_directory: str, output_inc_directory: str, include_path: str):
    """Click command function to convert multiple midi files to c/h

    Args:
        midi_file: Tuple of midi file paths
        output_src_directory: Directory to place generated midi source files
        output_inc_directory: Directory to place generated midi header files
        include_path: Include path to prepend to the included header in the source file

    Raises:
        click.ClickException: If an input file isn't a midi
    """
    for midi in midi_file:
        midi_file_to_c(midi, output_src_directory, output_inc_directory, include_path)

if __name__ == '__main__':
    # (click injects the params)
    # pylint: disable=no-value-for-parameter
    midi_to_c()
