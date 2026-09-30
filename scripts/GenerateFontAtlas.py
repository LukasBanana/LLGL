#
# GenerateFontAtlas.py
#

# Part of the LLGL project
# Written by L. Hermanns 10/1/2024
# --------------------------------
# Generates a font atlas (PNG image) with texture-coordinate mappings (TXT file) using the PIL (Pillow) Python package

import sys
import os
import argparse
from pathlib import Path
from PIL import ImageFont, ImageDraw, Image


class Options:
    inputFilename = ''
    outputDir = ''
    fontSize = 12
    useAlphaChannel = False
    border = 1


def ParseArguments():
    # Parse command line arguments
    parser = argparse.ArgumentParser(
        description='Generate font atlas and texture-coordinate mappings.'
    )
    parser.add_argument(
        "input_positional",
        nargs="?",
        metavar="INPUT",
        help="Search folder; equivalent to --input.",
    )
    parser.add_argument(
        '-i', '--input',
        metavar='INPUT',
        help='Input font filename.',
    )
    parser.add_argument(
        '-o', '--output',
        metavar='OUTPUT',
        help='Output directory (default: derived from input filename).',
    )
    parser.add_argument(
        '-s', '--size',
        metavar='SIZE',
        type=int,
        default=12,
        help='Font size in range [2, 128] (default: 12).',
    )
    parser.add_argument(
        '-b', '--border',
        metavar='BORDER',
        type=int,
        default=1,
        help='Border size around each glyph in the atlas in range [0, 10] (default: 1).',
    )
    parser.add_argument(
        '-a', '--alpha',
        action="store_true",
        help='Enable alpha channel for the font atlas.',
    )
    arguments = parser.parse_args()

    if arguments.input and arguments.input_positional:
        parser.error("specify the input font file either positionally or with -i/--input, not both")
    if arguments.input is None and arguments.input_positional is None:
        parser.error("missing input font file; use either positionally or with -i/--input")

    Options.inputFilename = arguments.input or arguments.input_positional
    Options.outputDir = os.path.dirname(Options.inputFilename) if arguments.output is None else Path(arguments.output)

    Options.fontSize = arguments.size
    if Options.fontSize < 2:
        sys.exit(f'Font size is too small: {Options.fontSize}; Range is [2, 128]')
    elif Options.fontSize > 128:
        sys.exit(f'Font size is too big: {Options.fontSize}; Range is [2, 128]')

    Options.border = arguments.border
    if Options.border < 0:
        sys.exit(f'Border size is too small: {Options.border}')
    elif Options.border > 10:
        sys.exit(f'Border size is too big: {Options.border}')

    Options.useAlphaChannel = arguments.alpha


class Glyph:
    char = chr(0)
    bbox = (0, 0, 0, 0)
    spacing = 0

    def __init__(self, font, index):
        self.char = chr(index)
        self.bbox = font.getbbox(self.char)
        self.spacing = font.getlength(self.char)

    @property
    def width(self):
        return self.bbox[2] - self.bbox[0] + Options.border*2

    @property
    def height(self):
        return self.bbox[3] - self.bbox[1] + Options.border*2


class GlyphNode:
    subnodes = None # tuple[2]
    glyph = None # Glyph
    bbox = (0, 0, 0, 0)

    def __init__(self, bbox):
        self.subnodes = None
        self.glyph = None
        self.bbox = bbox

    @property
    def width(self):
        return self.bbox[2] - self.bbox[0]

    @property
    def height(self):
        return self.bbox[3] - self.bbox[1]

    def putGlyph(self, glyph):
        if self.width < glyph.width or self.height < glyph.height:
            # Image does not fit into this node at all
            return False

        if self.subnodes is not None:
            # Try to put image into one of the subnodes
            return self.subnodes[0].putGlyph(glyph) or self.subnodes[1].putGlyph(glyph)

        if self.glyph is not None:
            # Node is already occupied
            return False

        if self.width == glyph.width and self.height == glyph.height:
            # Image fits exactly into this node
            self.glyph = glyph
            return True

        # Split up node into subnodes
        if self.width - glyph.width > self.height - glyph.height:
            # Split up node horizontally
            self.subnodes = (
                GlyphNode((self.bbox[0]              , self.bbox[1], self.bbox[0] + glyph.width, self.bbox[3])),
                GlyphNode((self.bbox[0] + glyph.width, self.bbox[1], self.bbox[2]              , self.bbox[3]))
            )
        else:
            # Split up node vertically
            self.subnodes = (
                GlyphNode((self.bbox[0], self.bbox[1]               , self.bbox[2], self.bbox[1] + glyph.height)),
                GlyphNode((self.bbox[0], self.bbox[1] + glyph.height, self.bbox[2], self.bbox[3]               ))
            )

        return self.subnodes[0].putGlyph(glyph)

    def draw(self, context, font):
        if self.glyph is not None:
            glyphOrigin = (
                self.bbox[0] - self.glyph.bbox[0] + Options.border,
                self.bbox[1] - self.glyph.bbox[1] + Options.border
            )
            context.text(glyphOrigin, self.glyph.char, font=font)
        if self.subnodes is not None:
            self.subnodes[0].draw(context, font)
            self.subnodes[1].draw(context, font)

    def collectLeaves(self):
        if self.subnodes is not None:
            return self.subnodes[0].collectLeaves() + self.subnodes[1].collectLeaves()
        if self.glyph is not None:
            return [self]
        return []


class FontAtlas:
    image = None
    context = None
    font = None
    glyphTree = None
    glyphLeaves = []

    def __init__(self, filename, size = 15, bgColor = (0, 0, 0), glyphSet = range(32, 128)):
        self.font = ImageFont.truetype(filename, size=size)

        # Create flyph tree
        numAttempts = 0
        fontFinalized = False
        atlasSize = (256, 256)

        while not fontFinalized:
            numAttempts += 1

            fontFinalized = True
            self.glyphTree = GlyphNode(bbox=(0, 0, atlasSize[0], atlasSize[1]))

            for ch in glyphSet:
                glyph = Glyph(self.font, ch)
                if not self.glyphTree.putGlyph(glyph):
                    if atlasSize[0] < atlasSize[1]:
                        atlasSize = (atlasSize[0]*2, atlasSize[1])
                    else:
                        atlasSize = (atlasSize[0], atlasSize[1]*2)
                    fontFinalized = False
                    break

        # Collect ordered list of all glyphs
        self.glyphLeaves = self.glyphTree.collectLeaves()
        self.glyphLeaves.sort(key=lambda g: ord(g.glyph.char))

        # Draw font atlas
        self.image = Image.new(mode='RGB', size=atlasSize, color=bgColor)
        self.context = ImageDraw.Draw(self.image)

        self.glyphTree.draw(self.context, self.font)

        if Options.useAlphaChannel:
            alpha = self.image.convert('L')
            self.image = Image.new(mode='RGBA', size=atlasSize, color=(255, 255, 255, 0))
            self.image.putalpha(alpha)

        # Log statistics
        print( 'Generated font atlas:')
        print(f' - Font filename: {filename}')
        print(f' - Font size:     {size}')
        print(f' - Atlas size:    {atlasSize[0]}x{atlasSize[1]} ({numAttempts} {"attempt" if numAttempts == 1 else "attempts"})')

    def saveImage(self, filename):
        print(f' - Save image:    {filename}')
        self.image.save(filename)

    def saveDataset(self, filename):
        print(f' - Save dataset:  {filename}')
        with open(filename, 'w') as file:
            print('# char x0 y0 x1 y1 x_offset y_offset spacing', file=file)
            for glyph in self.glyphLeaves:
                print(
                    f'{ord(glyph.glyph.char)} ' +
                    f'{glyph.bbox[0] + Options.border} {glyph.bbox[1] + Options.border} {glyph.bbox[2] - Options.border} {glyph.bbox[3] - Options.border} ' +
                    f'{glyph.glyph.bbox[0]} {glyph.glyph.bbox[1]} ' +
                    f'{int(glyph.glyph.spacing)}',
                    file=file
                )


if __name__ == "__main__":
    ParseArguments()

    # Generate font atlas and save output
    outputFilenameBase = f'{Options.outputDir}/{os.path.splitext(os.path.basename(Options.inputFilename))[0]}.atlas-{Options.fontSize}'
    atlas = FontAtlas(Options.inputFilename, size=Options.fontSize)
    atlas.saveImage  (f'{outputFilenameBase}.png')
    atlas.saveDataset(f'{outputFilenameBase}.map')

