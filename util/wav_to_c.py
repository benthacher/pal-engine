#!.venv/bin/python

import click
import subprocess
import os
import re
from pathlib import Path
from scipy.io import wavfile

DESIRED_SAMPLE_RATE = 44100

def get_c_symbol(symbol: str):
    return re.sub(r'[^[:alnum:]_]', '', symbol)

@click.command()
@click.argument("wav_file", type=click.Path(exists=True, readable=True))
@click.argument("output_directory", type=click.Path(exists=True, file_okay=False, dir_okay=True))
def wav_to_c(wav_file, output_directory):
    wav_path = Path(wav_file)

    if wav_path.suffix != '.wav':
        click.echo("Input file is not a dang wave file! the heck??")
        return

    wav_resampled_path = wav_path.parent / (wav_path.stem + '_resampled.wav')

    # convert it with ffmpeg
    subprocess.call(['/usr/bin/ffmpeg', '-i', wav_file, '-c:a', 'pcm_s16le', '-ac', '1', '-ar', str(DESIRED_SAMPLE_RATE), wav_resampled_path ])

    samplerate, data = wavfile.read(wav_resampled_path)

    assert(samplerate == DESIRED_SAMPLE_RATE)

    wav_size = len(data)
    wav_symbol = get_c_symbol(wav_path.stem) + '_wav'
    header_filename = wav_symbol + '.h'

    output_c_path = Path(output_directory, wav_symbol + '.c')
    output_h_path = Path(output_directory, header_filename)

    with open(output_h_path, 'w') as out:
        out.write("#pragma once\n\n")
        out.write("#include <stdint.h>\n\n")
        out.write("#include \"audio.h\"\n\n")
        out.write(f"extern const struct wave_data {wav_symbol};\n\n")

    with open(output_c_path, 'w') as out:
        out.write(f"#include \"{header_filename}\"\n\n")
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

    # wav_resampled_path.unlink()


if __name__ == '__main__':
    wav_to_c()