#!.venv/bin/python

import inflection
import click
import subprocess
import os
import re
from pathlib import Path
from scipy.io import wavfile

def get_c_symbol(symbol: str):
    return re.sub(r'[^[:alnum:]_]', '', symbol)

@click.command()
@click.argument("wav_file", type=click.Path(exists=True, readable=True))
@click.argument("sample_rate", type=int)
@click.argument("output_src_directory", type=click.Path(exists=True, file_okay=False, dir_okay=True))
@click.argument("output_inc_directory", type=click.Path(exists=True, file_okay=False, dir_okay=True))
@click.argument("include_path", default='')
def wav_to_c(wav_file, sample_rate, output_src_directory, output_inc_directory, include_path):
    wav_path = Path(wav_file)

    if wav_path.suffix != '.wav':
        click.echo("Input file is not a dang wave file! the heck??")
        return

    wav_resampled_path = wav_path.parent / (wav_path.stem + '_resampled.wav')

    # convert it with ffmpeg
    ret = subprocess.call(['/usr/bin/ffmpeg', '-y', '-i', wav_file, '-c:a', 'pcm_s16le', '-ac', '1', '-ar', str(sample_rate), wav_resampled_path ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    if ret != 0:
        print(f"Failed to convert wave file! ffmpeg exited with {ret}")
        exit(1)

    wav_sample_rate, data = wavfile.read(wav_resampled_path)

    assert(wav_sample_rate == sample_rate)

    wav_size = len(data)
    wav_symbol = get_c_symbol(wav_path.stem)
    header_filename = wav_symbol + '.h'

    output_c_path = Path(output_src_directory, wav_symbol + '.c')
    output_h_path = Path(output_inc_directory, header_filename)

    with open(output_h_path, 'w') as out:
        out.write("#pragma once\n\n")
        out.write("#include <stdint.h>\n\n")
        out.write("#include \"audio.h\"\n\n")
        out.write(f"extern const struct wave_data {wav_symbol};\n\n")

    with open(output_c_path, 'w') as out:
        out.write(f"#include \"{Path(include_path, header_filename)}\"\n\n")
        out.write("#include <stdint.h>\n\n")
        out.write(f"static const int16_t {wav_symbol}_data[{wav_size}] = {{\n")

        count = 0
        out.write('    ');

        for sample in data:
            out.write(f'{sample},')

            count += 1

            if count > 30:
                out.write('\n    ')
                count = 0

        out.write("\n};\n\n")
        out.write(f"const struct wave_data {wav_symbol} = {{\n")
        out.write(f"    .data = {wav_symbol}_data,\n")
        out.write(f"    .length = {wav_size},\n")
        out.write(f"}};\n")

    wav_resampled_path.unlink()


if __name__ == '__main__':
    wav_to_c()