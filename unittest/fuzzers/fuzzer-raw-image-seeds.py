#!/usr/bin/env python3
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Write the seed corpus for fuzzer-raw-image as a ZIP archive.

Usage: fuzzer-raw-image-seeds.py fuzzer-raw-image_seed_corpus.zip

The seeds are small synthetic pages (text lines, columns, a table, ruling
lines, a photo like region) in the input format of fuzzer-raw-image.cpp.
They are drawn with a built-in 5 x 7 pixel font, so nothing but Python is
needed and no image files have to be stored in the repository.
"""

import sys
import zipfile

# Glyphs of 5 x 7 pixels: 7 rows from top to bottom, each one a hexadecimal
# number with 2 digits, most significant of the 5 bits for the left pixel.
FONT = {
    ' ': '00000000000000', ',': '000000000C0408', '-': '0000001F000000',
    '.': '00000000000C0C', '0': '0E11131519110E', '1': '040C040404040E',
    '2': '0E11010204081F', '3': '1E01010E01011E', '4': '02060A121F0202',
    '5': '1F101E0101110E', '6': '0608101E11110E', '7': '1F010204080808',
    '8': '0E11110E11110E', '9': '0E11110F01020C', 'A': '0E11111F111111',
    'B': '1E11111E11111E', 'C': '0E11101010110E', 'D': '1E11111111111E',
    'E': '1F10101E10101F', 'F': '1F10101E101010', 'G': '0E11101711110F',
    'H': '1111111F111111', 'I': '0E04040404040E', 'J': '0702020202120C',
    'K': '11121418141211', 'L': '1010101010101F', 'M': '111B1515111111',
    'N': '11191513111111', 'O': '0E11111111110E', 'P': '1E11111E101010',
    'Q': '0E11111115120D', 'R': '1E11111E141211', 'S': '0F10100E01011E',
    'T': '1F040404040404', 'U': '1111111111110E', 'V': '11111111110A04',
    'W': '11111115151B11', 'X': '11110A040A1111', 'Y': '11110A04040404',
    'Z': '1F01020408101F',
}

TEXT = ('THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG. A PAGE OF TEXT IS '
        'SPLIT INTO BLOCKS, LINES AND WORDS BEFORE THE WORDS ARE READ. '
        'IN 1985 THE FIRST VERSION RAN ON 2 MACHINES, AND 30 YEARS LATER '
        'IT READS MORE THAN 100 LANGUAGES. ').split()

# Values for the header of the fuzz input (see fuzzer-raw-image.cpp).
PSM_AUTO = 3
PSM_SINGLE_COLUMN = 4
PSM_SINGLE_BLOCK_VERT_TEXT = 5
PSM_SINGLE_BLOCK = 6
PSM_SINGLE_LINE = 7
PSM_SINGLE_CHAR = 10
PSM_SPARSE_TEXT = 11
OTSU = 0
LEPTONICA_OTSU = 1
SAUVOLA = 2

WHITE = 255
BLACK = 0


class Page:
    """Grey image with simple drawing functions."""

    def __init__(self, width, height, paper=WHITE):
        self.width = width
        self.height = height
        self.rows = [bytearray([paper]) * width for _ in range(height)]

    def rect(self, left, top, width, height, grey):
        for y in range(max(top, 0), min(top + height, self.height)):
            for x in range(max(left, 0), min(left + width, self.width)):
                self.rows[y][x] = grey

    def photo(self, left, top, width, height):
        """Draw a region with a gradient and noise, like a halftone photo."""
        state = 1
        for y in range(top, min(top + height, self.height)):
            for x in range(left, min(left + width, self.width)):
                state = (state * 1103515245 + 12345) % 2**31
                grey = 60 + 120 * (x - left) // width + (state >> 16) % 64
                self.rows[y][x] = grey

    def text(self, left, top, text, scale, ink=BLACK):
        """Draw a text line and return the x position after it."""
        for char in text:
            glyph = FONT[char]
            for row in range(7):
                bits = int(glyph[2 * row:2 * row + 2], 16)
                for column in range(5):
                    if bits & (0x10 >> column):
                        self.rect(left + column * scale, top + row * scale,
                                  scale, scale, ink)
            left += 6 * scale
        return left

    def paragraph(self, left, top, width, height, scale, ink=BLACK, skip=0):
        """Fill a box with left aligned lines of words from TEXT."""
        columns = width // (6 * scale)
        index = skip
        for y in range(top, top + height - 7 * scale + 1, 10 * scale):
            line = ''
            while True:
                word = TEXT[index % len(TEXT)]
                if len(line) + len(word) + (line != '') > columns:
                    break
                line += (' ' if line else '') + word
                index += 1
            self.text(left, y, line, scale, ink)

    def blur(self):
        """Smooth the image, so the edges get grey values like in a scan."""
        rows = [bytes(row) for row in self.rows]
        for y in range(1, self.height - 1):
            above, here, below = rows[y - 1], rows[y], rows[y + 1]
            for x in range(1, self.width - 1):
                self.rows[y][x] = (above[x] + below[x] + here[x - 1] +
                                   here[x + 1] + 4 * here[x]) // 8

    def rotate(self):
        """Rotate the image clockwise by 90 degrees."""
        rows = self.rows
        self.rows = [bytearray(rows[self.height - 1 - x][y]
                               for x in range(self.height))
                     for y in range(self.width)]
        self.width, self.height = self.height, self.width

    def pixels(self, depth, ink, paper):
        """Return the pixel data for the fuzz input."""
        data = bytearray()
        for row in self.rows:
            if depth == 1:
                for x in range(0, self.width, 8):
                    bits = 0
                    for grey in row[x:x + 8].ljust(8, b'\xff'):
                        bits = bits << 1 | (grey < 128)
                    data.append(bits)
            elif depth == 8:
                data += row
            else:
                for grey in row:
                    data += bytes((i * (255 - grey) + p * grey) // 255
                                  for i, p in zip(ink, paper))
        return bytes(data)


def seed(page, mode, depth, method=None, resolution=None,
         ink=(0, 0, 0), paper=(255, 255, 255)):
    """Return the fuzz input (header line and pixel data) for a page.

    The keys for method and resolution are left out if they are None, so
    that the seeds also cover the default values.
    """
    header = {'psm': mode, 'depth': depth, 'thresholding_method': method,
              'width': page.width, 'height': page.height,
              'resolution': resolution}
    line = ' '.join(f'{key}={value}' for key, value in header.items()
                    if value is not None)
    return line.encode() + b'\n' + page.pixels(depth, ink, paper)


def text_block(width, height, scale):
    page = Page(width, height)
    margin = 4 * scale
    page.paragraph(margin, margin, width - 2 * margin, height - 2 * margin,
                   scale)
    return page


def two_columns(width, height, photo):
    """Heading, ruling line and two columns of text, one with a photo."""
    page = Page(width, height)
    column = (width - 64) // 2
    right = width - 12 - column
    page.text(12, 6, 'THE DAILY PAGE', 2)
    page.rect(12, 26, width - 24, 2, BLACK)
    page.paragraph(12, 36, column, height - 40, 2)
    if photo:
        page.photo(right, 36, column, 34)
        page.paragraph(right, 76, column, height - 80, 2, skip=17)
    else:
        page.paragraph(right, 36, column, height - 40, 2, skip=17)
    return page


def table(width, height):
    """Table with ruling lines and a short text in each cell."""
    page = Page(width, height)
    columns, rows = 4, 4
    cell_width = (width - 20) // columns
    cell_height = (height - 20) // rows
    for row in range(rows + 1):
        page.rect(10, 10 + row * cell_height, columns * cell_width + 2, 2,
                  BLACK)
    for column in range(columns + 1):
        page.rect(10 + column * cell_width, 10, 2, rows * cell_height + 2,
                  BLACK)
    for row in range(rows):
        for column in range(columns):
            word = TEXT[(5 * row + 3 * column) % len(TEXT)].strip('.,')
            if column > 0 and row > 0:
                word = str(137 * row * column + 1985)
            page.text(18 + column * cell_width, 22 + row * cell_height,
                      word[:cell_width // 12 - 1], 2)
    return page


def sparse(width, height):
    """Words of different sizes without any alignment."""
    page = Page(width, height)
    for index in range(9):
        x = 10 + (index * 151) % (width - 130)
        y = 6 + (index * 67) % (height - 30)
        page.text(x, y, TEXT[3 * index % len(TEXT)].strip('.,'),
                  2 + index % 2)
    return page


def single_column(width, height):
    """Text of different sizes with an inverse heading."""
    page = Page(width, height)
    page.rect(0, 0, width, 44, 40)
    page.text(12, 8, 'LARGE TITLE', 4, WHITE)
    page.paragraph(12, 54, width - 24, 60, 3)
    page.paragraph(12, 118, width - 24, height - 122, 2, skip=9)
    return page


def line(text, scale, margin=6):
    page = Page(6 * scale * len(text) + 2 * margin, 7 * scale + 2 * margin)
    page.text(margin, margin, text, scale)
    return page


def blurred(page):
    page.blur()
    return page


def rotated(page):
    page.rotate()
    return page


def seeds():
    """Return the seeds as a dictionary of file names and contents."""
    blue_on_yellow = {'ink': (20, 30, 120), 'paper': (250, 245, 200)}
    green_on_grey = {'ink': (10, 70, 20), 'paper': (215, 215, 220)}
    sentence = ' '.join(TEXT[:9])
    return {
        'auto-columns-1bpp':
            seed(two_columns(448, 144, False), PSM_AUTO, 1),
        'auto-columns-photo-8bpp':
            seed(blurred(two_columns(448, 144, True)), PSM_AUTO, 8, OTSU, 0),
        'auto-columns-photo-32bpp':
            seed(blurred(two_columns(400, 160, True)), PSM_AUTO, 32, SAUVOLA,
                 150, **blue_on_yellow),
        'auto-table-1bpp':
            seed(table(448, 144), PSM_AUTO, 1, resolution=200),
        'auto-table-8bpp':
            seed(blurred(table(400, 160)), PSM_AUTO, 8, LEPTONICA_OTSU, 300),
        'single-column-8bpp':
            seed(blurred(single_column(360, 180)), PSM_SINGLE_COLUMN, 8,
                 SAUVOLA),
        'single-block-vert-text-1bpp':
            seed(rotated(text_block(400, 160, 2)), PSM_SINGLE_BLOCK_VERT_TEXT,
                 1, resolution=300),
        'single-block-1bpp':
            seed(text_block(448, 144, 2), PSM_SINGLE_BLOCK, 1),
        'single-block-32bpp':
            seed(blurred(text_block(320, 160, 2)), PSM_SINGLE_BLOCK, 32,
                 LEPTONICA_OTSU, 72, **green_on_grey),
        'single-line-1bpp':
            seed(line(sentence, 3), PSM_SINGLE_LINE, 1, resolution=300),
        'single-line-32bpp':
            seed(blurred(line('100 LANGUAGES', 4)), PSM_SINGLE_LINE, 32,
                 resolution=4000, **blue_on_yellow),
        'single-line-8bpp':
            seed(blurred(line(sentence[:30], 3)), PSM_SINGLE_LINE, 8,
                 LEPTONICA_OTSU, 30),
        'single-char-1bpp':
            seed(line('R', 5), PSM_SINGLE_CHAR, 1),
        'sparse-text-1bpp':
            seed(sparse(448, 144), PSM_SPARSE_TEXT, 1, resolution=120),
        'sparse-text-8bpp':
            seed(blurred(sparse(400, 160)), PSM_SPARSE_TEXT, 8, SAUVOLA, 600),
    }


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    with zipfile.ZipFile(sys.argv[1], 'w', zipfile.ZIP_DEFLATED) as archive:
        for name, data in seeds().items():
            # Use a fixed time stamp to get a reproducible archive.
            archive.writestr(zipfile.ZipInfo(name, (1980, 1, 1, 0, 0, 0)),
                             data, zipfile.ZIP_DEFLATED)


if __name__ == '__main__':
    main()
