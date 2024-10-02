#
# GenerateFontAtlas.py
#

# Part of the LLGL project
# Written by L. Hermanns 10/1/2024
# --------------------------------
# Generates a font atlas (PNG image) with texture-coordinate mappings (TXT file) using the PIL (Pillow) Python package

import sys
import os
from PIL import ImageFont, ImageDraw, Image

# Parse command line arguments
if len(sys.argv) != 3:
    sys.exit('Missing arguments! Usage: GenerateFontAtlas.py FILE SIZE')

inputFilename = sys.argv[1]

fontSize = int(sys.argv[2])
if fontSize < 2:
    sys.exit('Font size is too small: {fontSize}')
elif fontSize > 128:
    sys.exit('Font size is too big: {fontSize}')

outputFilenameBase = f'{os.path.dirname(inputFilename)}/{os.path.splitext(os.path.basename(inputFilename))[0]}.atlas-{fontSize}'

border = 1

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
        return self.bbox[2] - self.bbox[0] + border*2
    
    @property
    def height(self):
        return self.bbox[3] - self.bbox[1] + border*2

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
                self.bbox[0] - self.glyph.bbox[0] + border,
                self.bbox[1] - self.glyph.bbox[1] + border
            )
            context.text(glyphOrigin, self.glyph.char, font=font)
        if self.subnodes is not None:
            self.subnodes[0].draw(context, font)
            self.subnodes[1].draw(context, font)

    def collectLeafs(self):
        if self.subnodes is not None:
            return self.subnodes[0].collectLeafs() + self.subnodes[1].collectLeafs()
        if self.glyph is not None:
            return [self]
        return []

class FontAtlas:
    image = None
    context = None
    font = None
    glyphTree = None
    glyphLeafs = []

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
        self.glyphLeafs = self.glyphTree.collectLeafs()
        self.glyphLeafs.sort(key=lambda g: ord(g.glyph.char))

        # Draw font atlas
        self.image = Image.new(mode='RGB', size=atlasSize, color=bgColor)
        self.context = ImageDraw.Draw(self.image)

        self.glyphTree.draw(self.context, self.font)

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
            for glyph in self.glyphLeafs:
                print(
                    f'{ord(glyph.glyph.char)} ' +
                    f'{glyph.bbox[0] + border} {glyph.bbox[1] + border} {glyph.bbox[2] - border} {glyph.bbox[3] - border} ' +
                    f'{glyph.glyph.bbox[0]} {glyph.glyph.bbox[1]} ' +
                    f'{int(glyph.glyph.spacing)}',
                    file=file
                )

# Generate font atlas and save output
atlas = FontAtlas(inputFilename, size=fontSize)
atlas.saveImage(f'{outputFilenameBase}.png')
atlas.saveDataset(f'{outputFilenameBase}.map')
