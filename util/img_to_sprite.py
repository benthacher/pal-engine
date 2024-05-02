#!.venv/bin/python
"""img_to_sprite.py

Converts image file (.png or .gif) to sprite for PAL engine

"""
import re
from pathlib import Path

import click
from PIL import Image

NUM_SPACES_FOR_TAB = 4

def tabs(num: int):
    """Returns string of spaces with width equal to given number of tabs"""
    return ' ' * NUM_SPACES_FOR_TAB * num


def sanitize_image_name(name: str):
    """Converts input name to a valid c symbol

    Args:
        name: Input name

    Returns:
        Valid c symbol
    """
    return re.sub(r'\W+', '_', name)


def get_sprite_symbol(name: str):
    """Get sprite symbol from sprite name"""
    return f'sprite_{name}'


def get_image_symbol(name: str, frame: int):
    """Get image symbol from sprite name and frame number"""
    return f'{get_sprite_symbol(name)}_image{frame}'


def get_image_data_symbol(name: str, frame: int):
    """Get image data symbol from sprite name and frame number"""
    return f'{get_image_symbol(name, frame)}_data'


def get_sprite_frame_symbol(name: str, frame: int):
    """Get sprite frame symbol from sprite name and frame number"""
    return f'{get_sprite_symbol(name)}_frame{frame}'


def get_sprite_frame_array_symbol(name: str):
    """Get sprite frame array symbol from sprite name"""
    return f'{get_sprite_symbol(name)}_frames'


def generate_header(name: str):
    """Generates header file text from given sprite name"""
    return f"""#pragma once

#include "sprite.h"

extern struct sprite_def {get_sprite_symbol(name)};
"""


def generate_pixel_data_text(im: Image.Image, indent):
    """Generates pixel data string from PIL Image and number of tabs of indentation

    Args:
        im: PIL Image object
        indent: number of tabs to indent image data by

    Returns:
        C array styled string of image data
    """
    result = []
    line = ''

    width, _ = im.size

    palette = im.getpalette('RGBA')

    # column pointer
    c = 0

    for pixel in im.getdata():
        if palette:
            # if palette exists, pixel is the index into the palette of the desired color
            if pixel != 0:
                pixel = palette[pixel*4:(pixel+1)*4]
            else:
                # index 0 means transparent
                pixel = (0, 0, 0, 0)

        line += f'{{{pixel[0]},{pixel[1]},{pixel[2]},{pixel[3] if len(pixel) == 4 else 255}}},'

        if (c := c + 1) == width:
            c = 0
            result.append(line)
            line = ''

    return tabs(indent) + f'\n{tabs(indent)}'.join(result) + '\n'


def generate_image_data_def(im: Image.Image, name: str, frame: int):
    """Generates image data definition

    Args:
        im: PIL Image object
        name: Name of sprite
        frame: Frame number

    Returns:
        C array definition of given image
    """
    width, height = im.size

    im.seek(frame)

    return f"""static struct color {get_image_data_symbol(name, frame)}[{width} * {height}] = {{
{generate_pixel_data_text(im, 1)}}};
"""

def generate_image_def(im: Image.Image, name: str, frame: int):
    """Generates image descriptor definition

    Args:
        im: PIL Image object
        name: Sprite name
        frame: Frame number

    Returns:
        C struct definition of given image
    """
    width, height = im.size

    return f"""static struct image {get_image_symbol(name, frame)} = {{
    .data = {get_image_data_symbol(name, frame)},
    .width = {width},
    .height = {height}
}};
"""

def generate_sprite_frame_def(im: Image.Image, name: str, frame: int):
    """Generates sprite frame struct definition

    Args:
        im: PIL Image object
        name: Sprite name
        frame: Frame number

    Returns:
        C struct definition of sprite frame
    """
    im.seek(frame)

    return f"""static struct sprite_frame {get_sprite_frame_symbol(name, frame)} = {{
    .image = &{get_image_symbol(name, frame)},
    .duration = {im.info.get('duration', 0) / 1000}
}};
"""

def generate_sprite_frame_array(im: Image.Image, name: str):
    """Generates sprite frame array definition

    Args:
        im: PIL Image object
        name: Sprite name

    Returns:
        C array definition of sprite frames
    """
    separator = ',\n' + tabs(1)

    return f"""struct sprite_frame *{get_sprite_frame_array_symbol(name)}[{im.n_frames}] = {{
    {separator.join('&' + get_sprite_frame_symbol(name, i) for i in range(im.n_frames))}
}};
"""

def generate_sprite_def(im: Image.Image, name: str, loop: bool):
    """Generates sprite definition

    Args:
        im: PIL Image object
        loop: Whether or not the sprite should loop

    Returns:
        C struct definition of sprite
    """
    return f"""struct sprite_def {get_sprite_symbol(name)} = {{
    .frames = {get_sprite_frame_array_symbol(name)},
    .num_frames = {im.n_frames},
    .loop = {int(loop)}
}};
"""

@click.command()
@click.argument("image", type=click.Path(exists=True))
@click.argument("output-source-dir", type=click.Path(exists=True))
@click.argument("output-include-dir", type=click.Path(exists=True))
@click.argument("include_path", default='')
@click.option("--loop/--no-loop", is_flag=True, default=True)
def img_to_sprite(image, output_source_dir, output_include_dir, include_path, loop):
    """Click command to convert image to sprite

    Args:
        image: Path to image file
        output_source: Path to directory to output source file
        output_include: Path to directory to output include file
        include_path: Path to prepend to included header in source file
        loop: Whether or not the sprite should loop
    """
    output_source_path = Path(output_source_dir)
    output_include_path = Path(output_include_dir)
    image_name = sanitize_image_name(Path(image).stem)

    result = f'#include "{Path(include_path, get_sprite_symbol(image_name))}.h"\n#include "sprite.h"\n\n'

    in_image = Image.open(image)

    # create definitions for each frame's image
    for i in range(in_image.n_frames):
        result += generate_image_data_def(in_image, image_name, i)
        result += generate_image_def(in_image, image_name, i)
        result += generate_sprite_frame_def(in_image, image_name, i)

    # create definition for sprite
    result += generate_sprite_frame_array(in_image, image_name)
    result += generate_sprite_def(in_image, image_name, loop if in_image.n_frames > 1 else False)

    c_file_path = output_source_path.joinpath(image_name + '.c')
    h_file_path = output_include_path.joinpath(image_name + '.h')

    with open(c_file_path, 'w', encoding='utf8') as f:
        f.write(result)

    with open(h_file_path, 'w', encoding='utf8') as f:
        f.write(generate_header(image_name))


if __name__ == '__main__':
    # pylint: disable=no-value-for-parameter
    img_to_sprite()
