/* I don't expect anyone to run this program, ever again.  It is
 * included primarily as documentation for how the GlyphLessFont was
 * created.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GlyphLessFont.h"

#define LITTLE_ENDIAN

Offset_Table Offsets = {
#ifdef LITTLE_ENDIAN
    0x00000100,    /* sfnt_version */
    0x0A00,        /* numTables (10) */
    0x8000,        /* searchRange = Max power of 2 <= numTables*16 (128) */
    0x0300,        /* entrySelector Log2(searchRange) (3) */
    0x2000,        /* rangeShift = numTables*16 - searchRange (160 - 128 = 32) */
#else
    0x00010000,    /* sfnt_version */
    0x000A,        /* numTables (10) */
    0x0080,        /* searchRange = Max power of 2 <= numTables*16 (128) */
    0x0003,        /* entrySelector Log2(searchRange) (3) */
    0x0020,        /* rangeShift = numTables*16 - searchRange (160 - 128 = 32) */
#endif
};

head_table head = {
#ifdef LITTLE_ENDIAN
    0x00000100,    /* sfnt_version */
    0x00000100,    /* font_version */
    0,             /* checksum adjustment */
    0xF53C0F5F,    /* Magic number */
    0x0704,        /* flags:
                    * Bit 0 - 1 - baseline of font at y = 0
                    * Bit 1 - 1 - Left sidebearing at x = 0
                    * Bit 2 - 0 - instructions not dependent on font size
                    * Bit 3 - 1 - force integer ppem
                    * Bit 4 - 0 - instructions may not alter advance width
                    * Bit 5 - 0 - Not laid out vertically
                    * Bit 6 - 0 - required to be 0
                    * Bit 7 - 0 - Does not require layout for rendering
                    * Bit 8 - 0 - Not an AAT font with metamorphosis
                    * Bit 9 - 0 - Not strongly right to left
                    * Bit 10 - 0 - Does not require indic-style rearrangements
                    * Bit 11 - 0 - Font data is not 'lossless'
                    * Bit 12 - 0 - Font not 'covnerted'
                    * Bit 13 - 0 - Not optimised for ClearType
                    * Bit 14 - 1 - This is a 'last resort' font
                    * Bit 15 - 0 - Reserved, must be 0
                    */
    0x0001,        /* 16 units per em */
    0x0,0x6EFC9ACF,/* Creation time */
    0x0,0x6EFC9ACF,/* Modified time */
    0,             /* xMin */
    0x0080,        /* yMin */
    0,             /* xMax */
    0x0100,        /* yMax */
    0x0000,        /* macStyle (none) */
    0x1000,        /* Lowest readable size (16 pixels) */
    0x0200,        /* font direction (deprecated, should be 2) */
    0,             /* index to LOCA format (shorts) */
    0              /* glyph data format (must be 0) */
#else
    0x00010000,    /* afnt version */
    0x00010000,    /* font version */
    0,             /* checksum adjustment */
    0x5F0F3CF5,    /* Magic number */
    0x0407,        /* flags:
                    * Bit 0 - 1 - baseline of font at y = 0
                    * Bit 1 - 1 - Left sidebearing at x = 0
                    * Bit 2 - 0 - instructions not dependent on font size
                    * Bit 3 - 1 - force integer ppem
                    * Bit 4 - 0 - instructions may not alter advance width
                    * Bit 5 - 0 - Not laid out vertically
                    * Bit 6 - 0 - required to be 0
                    * Bit 7 - 0 - Does not require layout for rendering
                    * Bit 8 - 0 - Not an AAT font with metamorphosis
                    * Bit 9 - 0 - Not strongly right to left
                    * Bit 10 - 0 - Does not require indic-style rearrangements
                    * Bit 11 - 0 - Font data is not 'lossless'
                    * Bit 12 - 0 - Font not 'covnerted'
                    * Bit 13 - 0 - Not optimised for ClearType
                    * Bit 14 - 1 - This is a 'last resort' font
                    * Bit 15 - 0 - Reserved, must be 0
                    */
    0x0100,        /* 16 units per em */
    0x0,0xCF9AFC6E,/* Creation time */
    0x0,0xCF9AFC6E,/* Modified time */
    0,             /* xMin */
    0xFFFF,        /* yMin */
    0,             /* xMax */
    0x001,         /* yMax */
    0,             /* macStyle (none) */
    0x0010,        /* Lowest readable size (16 pixels) */
    0x0002,        /* font direction (deprecated, should be 2) */
    0,             /* index to LOCA format (shorts) */
    0              /* glyph data format (must be 0) */
#endif
};

hhea_table hhea = {
#ifdef LITTLE_ENDIAN
    0x00000100,    /* table version */
    0x0100,        /* Ascender */
#else
    0x00001000,    /* table version */
    0x0001,        /* Ascender */
#endif
    0xFFFF,        /* Descender */
    0x0000,        /* LineGap */
    0x0000,        /* AdvanceWidthMax */
    0x0000,        /* MinLeftSideBearing */
    0x0000,        /* MinRightSideBearing */
    0x0000,        /* xMaxExtent */
#ifdef LITTLE_ENDIAN
    0x0100,        /* caretSlopeRise (1 = vertical) */
#else
    0x0001,        /* caretSlopeRise (1 = vertical) */
#endif
    0x0000,        /* caretslopeRun (0 = vertical) */
    0x0000,        /* caretOffset */
    0x0000,        /* Reserved1 */
    0x0000,        /* Reserved2 */
    0x0000,        /* Reserved3 */
    0x0000,        /* Reserved4 */
    0x0000,        /* merticDataFormat (must be 0) */
#ifdef LITTLE_ENDIAN
    0x0200,        /* number of hMetric entries in hmtx */
#else
    0x0002,        /* number of hMetric entries in hmtx */
#endif
};

maxp_table maxp = {
#ifdef LITTLE_ENDIAN
    0x00000100,    /* table version */
    0x0200,        /* numGlyphs */
    0x00000000,    /* maxPoints */
    0x00000000,    /* maxContours */
    0x00000000,    /* maxCompositePoints */
    0x00000000,    /* maxCompositeContours */
    0x00000100,    /* maxZones */
    0x00000000,    /* maxTwilightPoints */
    0x00000000,    /* maxStorage */
    0x00000000,    /* maxFunctionDefs */
    0x00000000,    /* maxInstructionDefs */
    0x00000000,    /* maxStackElements */
    0x00000000,    /* maxSizeOfInstructions */
    0x00000000,    /* maxComponentElements */
    0x00000000,    /* maxComponentDepth */
#else
    0x00001000,    /* table version */
    0x0002,        /* numGlyphs */
    0x00000000,    /* maxPoints */
    0x00000000,    /* maxContours */
    0x00000000,    /* maxCompositePoints */
    0x00000000,    /* maxCompositeContours */
    0x00000001,    /* maxZones */
    0x00000000,    /* maxTwilightPoints */
    0x00000000,    /* maxStorage */
    0x00000000,    /* maxFunctionDefs */
    0x00000000,    /* maxInstructionDefs */
    0x00000000,    /* maxStackElements */
    0x00000000,    /* maxSizeOfInstructions */
    0x00000000,    /* maxComponentElements */
    0x00000000,    /* maxComponentDepth */
#endif
};

OS2_table OS2 = {
#ifdef LITTLE_ENDIAN
    0x0300,    /* table version */
    0x0000,    /* xAvgCharWidth */
    0x9001,    /* usWeight Class (400 = FW_NORMAL) */
    0x0500,    /* usWidthClass (5 = FWIDTH_NORMAL) */
    0x0000,    /* fsType (0 = no embedding restrictions) */
    0x0000,    /* ySubscriptXSize */
    0x0000,    /* ySubscriptYSize */
    0x0000,    /* ySubscriptXOffset */
    0x0000,    /* ySubscriptYOffset */
    0x0000,    /* ySuperscriptXSize */
    0x0000,    /* ySuperscriptYSize */
    0x0000,    /* ySuperscriptXOffset */
    0x0000,    /* ySuperscriptYOffset */
    0x0000,    /* yStikeoutPosition */
    0x0000,    /* sFamilyClass (0 = no classification) */
    0,5,0,1,0,1,0,0,0,0,0,       /* PANOSE */
    0x00000000,  /* ulUnicodeRanges1 */
    0x00000000,  /* ulUnicodeRanges2 */
    0x00000000,  /* ulUnicodeRanges3 */
    0x00000000,  /* ulUnicodeRanges4 */
    'G', 'O', 'O', 'G',   /* achVendID (GOOG = Google) */
    0x4000,    /* fsSelection (bit 6 set = regular font) */
    0xFFFF,    /* fsFirstCharIndex */
    0x0000,    /* fsLastCharIndex */
    0x0100,    /* sTypoAscender */
    0xFFFF,    /* StypoDescender */
    0x0000,    /* STypoLineGap */
    0x0100,    /* usWinAscent */
    0x0100,    /* usWinDescent */
    0x00000080,/* ulCodePageRange1 */
    0x00000000,/* ulCodePageRange2 */
    0x0000,    /* sxHeight */
    0x0000,    /* sCapHeight */
    0x0000,    /* usDefaultChar */
    0x0100,    /* usBreakChar */
    0x0000,    /* usMaxContent */
#else
    0x0003,    /* table version */
    0x0000,    /* xAvgCharWidth */
    0x0190,    /* usWeight Class (400 = FW_NORMAL) */
    0x0005,    /* usWidthClass (5 = FWIDTH_NORMAL) */
    0x0000,    /* fsType (0 = no embedding restrictions) */
    0x0000,    /* ySubscriptXSize */
    0x0000,    /* ySubscriptYSize */
    0x0000,    /* ySubscriptXOffset */
    0x0000,    /* ySubscriptYOffset */
    0x0000,    /* ySuperscriptXSize */
    0x0000,    /* ySuperscriptYSize */
    0x0000,    /* ySuperscriptXOffset */
    0x0000,    /* ySuperscriptYOffset */
    0x0000,    /* yStikeoutPosition */
    0x0000,    /* sFamilyClass (0 = no classification) */
    0,5,0,1,0,1,0,0,0,0,0,       /* PANOSE */
    0x00000000,/* ulUnicodeRanges1 */
    0x00000000,/* ulUnicodeRanges2 */
    0x00000000,/* ulUnicodeRanges3 */
    0x00000000,/* ulUnicodeRanges4 */
    'G', 'O', 'O', 'G',   /* achVendID (GOOG = Google) */
    0x0040,    /* fsSelection (bit 6 set = regular font) */
    0xFFFF,    /* fsFirstCharIndex */
    0x0000,    /* fsLastCharIndex */
    0x0001,    /* sTypoAscender */
    0xFFFF,    /* StypoDescender */
    0x0000,    /* STypoLineGap */
    0x0001,    /* usWinAscent */
    0x0001,    /* usWinDescent */
    0x80000000,/* ulCodePageRange1 */
    0x00000000,/* ulCodePageRange2 */
    0x0000,    /* sxHeight */
    0x0000,    /* sCapHeight */
    0x0000,    /* usDefaultChar */
    0x0001,    /* usBreakChar */
    0x0000,    /* usMaxContent */
#endif
};

hmtx_table hmtx = {
0x0000, 0x0000,
0x0000, 0x0000
};

cmap_table cmap = {
    0x0000,          /* Cmap version (0) */
#ifdef LITTLE_ENDIAN
    0x0200,          /* numTables (2) */
    0x0100,          /* Start of first subtable record, platformID = 1 */
    0x0000,          /* encodingID = 0 */
    0x14000000,      /* Offset of data */
    0x0300,          /* Start of second subtable record, platformID = 3 */
    0x0000,          /* encodingID = 0 */
    0x20000000,      /* Offset of data */
    0x0600,          /* STart of Apple table (format 6) */
    0x0C00,          /* lenght of table (12) */
    0x0000,          /* Language must be 0 for non-Apple or
                        non-specific language */
    0x0000,          /* firstCode = 0 */
    0x0100,          /* number of codes is 1 */
    0x0000,          /* GID is 0 */
    0x0600,          /* Start of MS Table (format 4) */
    0x0C00,          /* lenght of table (12) */
    0x0000,          /* Language must be 0 for non-Apple or
                        non-specific language */
    0x0000,          /* firstCode = 0 */
    0x0100,          /* number of codes is 1 */
    0x0000,          /* GID is 0 */
#else
    0x0002,          /* numTables (2) */
    0x0001,
    0x0000,
    0x00000014,
    0x0003,
    0x0000,
    0x00000020,
    0x0006,
    0x000C,
    0x0000,
    0x0000,
    0x0001,
    0x0000,
    0x0006,
    0x000C,
    0x0000,
    0x0000,
    0x0001,
    0x0000,
#endif
};

/* Changing these strings requires you to change the offset and lengths
   in the name table below */
char Macnamestring[] = {'V', 'e', 'r', 's', 'i', 'o', 'n', ' ', '1', '.', '0'};
char Unamestring[] = {0x00, 'V', 0x00, 'e', 0x00, 'r', 0x00, 's', 0x00, 'i',
                      0x00, 'o', 0x00, 'n', 0x00, ' ', 0x00, '1', 0x00, '.',
                      0x00, '0', 0x00, 0x00, 0x00};
name_table name = {
    0x0000,      /* format 0 */
#ifdef LITTLE_ENDIAN
    0x0300,      /* 3 records */
    0x2A00,      /* Offset of string storage */

    0x0000,      /* Start of 1st name record, platform = 0 (Unicode) */
    0x0300,      /* Platform-specific ID = 0 */
    0x0000,      /* Language ID (0 = none) */
    0x0500,      /* name ID (5 = version string) */
    0x1600,      /* String length */
    0x0B00,      /* Offset from start of storage */

    0x0100,      /* Start of 2nd name record, platform = 1 (Mac) */
    0x0000,
    0x0000,
    0x0500,      /* name ID (5 = version string) */
    0x0B00,      /* String length */
    0x0000,      /* Offset from start of storage */

    0x0300,      /* Start of 3rd name record, platform = 3 */
    0x0100,      /* Platform-specific ID = 1 */
    0x0904,      /* Language ID (0x409 = US English) */
    0x0500,      /* name ID (5 = version string) */
    0x1600,      /* String length */
    0x0B00,      /* Offset from start of storage */
#else
    0x0003,      /* 3 record2 */
    0x002A,      /* Offset of string storage */

    0x0000,      /* Start of 1st name record, platform = 0 (Unicode) */
    0x0003,      /* Platform-specific ID = 0 */
    0x0000,      /* Language ID (0 = none) */
    0x0005,      /* name ID (5 = version string) */
    0x0016,      /* String length */
    0x000B,      /* Offset from start of storage */

    0x0001,      /* Start of 2nd name record, platform = 1 (Mac) */
    0x0000,
    0x0000,
    0x0500,      /* name ID (5 = version string) */
    0x000B,      /* String length */
    0x0000,      /* Offset from start of storage */

    0x0003,      /* Start of 3rd name record, platform = 3 */
    0x0001,      /* Platform-specific ID = 0 */
    0x0409,      /* Language ID (0 = none) */
    0x0005,      /* name ID (5 = version string) */
    0x0016,      /* String length */
    0x000B,      /* Offset from start of storage */
#endif
};

post_table post = {
#ifdef LITTLE_ENDIAN
    0x0100,      /* Version (2) */
#else
    0x0001,      /* Version (2) */
#endif
    0x00000000,  /* italicAngle */
    0x0000,      /* underlinePosition */
    0x0000,      /* underlineThickness */
#ifdef LITTLE_ENDIAN
    0x01000000,  /* isFixedPitch */
#else
    0x00000001,  /* isFixedPitch */
#endif
    0x00000000,  /* minMemType42 */
    0x00000000,  /* maxMemType42 */
    0x00000000,  /* minMemType1 */
    0x00000000,  /* maxMemType1 */
};

int main (int argc, char **argv)
{
    FILE *OutputFile;
    TableRecord Table[10];
    unsigned long offset =
        sizeof(Offset_Table) + (sizeof(TableRecord) * 10),
        length = 0, checksum = 0, HeadTableOffset, Working;
    short fword = -1;
    short loca = 0;
    long glyf = 0;
    unsigned int NameLength, i, FileLength;

    printf("Ken's Glyph-free font creator\n");
    if (argc != 2) {
    	fprintf (stderr, "Usage: GlyphLessFont <output filename>\n");
	    exit (1);
    }

    OutputFile = fopen (argv[1], "wb+");
    if (OutputFile == 0) {
    	fprintf (stderr, "Couldn't open file %s for writing\n", argv[1]);
	    exit (1);
    }

    fwrite (&Offsets, sizeof(Offset_Table), 1, OutputFile);
    memset(&Table, 0x00, sizeof(TableRecord) + 10);
    fwrite (&Table, sizeof (TableRecord), 10, OutputFile);
    offset = ftell(OutputFile);
    Table[3].offset = HeadTableOffset = offset;

    /* The whole business of writing a TrueType file is complicated by
     * the way its laid out Firstly there is the fact that it wants
     * the tables to be laid out in alphabetical order, but it wants
     * the actual table data (which the table record points to) to be
     * in quite a different order.  Then there's the requirement to
     * have all the table offsets be a multiple of 4 bytes.  Finally
     * we have to calculate a checksum for each table as well, which
     * we cna't realistically do until we have written the table data,
     * but which gets stored in the table record at the start of the
     * file.
     *
     * So we start by writing a dumm set of table records, we'll fill
     * in the array as we go and once we've written all the data and
     * worked out the offsets and checksums of all the tables, we'll
     * come back and write the table records into the area we left
     * reserved.
     */
    fwrite (&head, sizeof(head_table), 1, OutputFile);
    offset = ftell(OutputFile);
    Table[4].offset = offset;

    fwrite (&hhea, sizeof(hhea_table), 1, OutputFile);
    offset = ftell(OutputFile);
    Table[7].offset = offset;

    fwrite (&maxp, sizeof(maxp_table), 1, OutputFile);
    offset = ftell(OutputFile);
    Table[0].offset = offset;

    fwrite (&OS2, sizeof(OS2_table), 1, OutputFile);
    offset = ftell(OutputFile);
    Table[5].offset = offset;

    fwrite (&hmtx, sizeof(hmtx_table), 1, OutputFile);
    offset = ftell(OutputFile);
    Table[1].offset = offset;

    fwrite (&cmap, sizeof(cmap_table), 1, OutputFile);
    offset = ftell(OutputFile);
    Table[6].offset = offset;

    fwrite (&loca, sizeof(short), 1, OutputFile);
    fwrite (&loca, sizeof(short), 1, OutputFile);
    fwrite (&loca, sizeof(short), 1, OutputFile);
    fwrite (&loca, sizeof(short), 1, OutputFile);
    offset = ftell(OutputFile);
    Table[2].offset = offset;

    fwrite (&glyf, sizeof(long), 1, OutputFile);
    offset = ftell(OutputFile);
    Table[8].offset = offset;

    length = (sizeof(name_table) + sizeof(Macnamestring) +
              sizeof(Unamestring) + 3) / 4;
    length *= 4;
    NameLength = length;
    fwrite (&name, sizeof(name_table), 1, OutputFile);
    fwrite (&Macnamestring, sizeof(Macnamestring), 1, OutputFile);
    fwrite (&Unamestring, NameLength -
            (sizeof(name_table) + sizeof(Macnamestring)), 1, OutputFile);
    offset = ftell(OutputFile);
    Table[9].offset = offset;

    fwrite (&post, sizeof(post_table), 1, OutputFile);
    FileLength = ftell(OutputFile);

    Table[3].tag[0] = 'h';
    Table[3].tag[1] = 'e';
    Table[3].tag[2] = 'a';
    Table[3].tag[3] = 'd';
    Table[3].checkSum = 0;
    Table[3].length = sizeof(head_table) - 2; /* Don't count size
                                                 of padding bytes in table */

    Table[4].tag[0] = 'h';
    Table[4].tag[1] = 'h';
    Table[4].tag[2] = 'e';
    Table[4].tag[3] = 'a';
    Table[4].checkSum = 0;
    Table[4].length = sizeof(hhea_table);

    Table[7].tag[0] = 'm';
    Table[7].tag[1] = 'a';
    Table[7].tag[2] = 'x';
    Table[7].tag[3] = 'p';
    Table[7].checkSum = 0;
    Table[7].length = sizeof(maxp_table);

    Table[0].tag[0] = 'O';
    Table[0].tag[1] = 'S';
    Table[0].tag[2] = '/';
    Table[0].tag[3] = '2';
    Table[0].checkSum = 0;
    Table[0].length = sizeof(OS2_table);

    Table[5].tag[0] = 'h';
    Table[5].tag[1] = 'm';
    Table[5].tag[2] = 't';
    Table[5].tag[3] = 'x';
    Table[5].checkSum = 0;
    Table[5].length = sizeof(hmtx_table);

    Table[1].tag[0] = 'c';
    Table[1].tag[1] = 'm';
    Table[1].tag[2] = 'a';
    Table[1].tag[3] = 'p';
    Table[1].checkSum = 0;
    Table[1].length = sizeof(cmap_table);

    Table[6].tag[0] = 'l';
    Table[6].tag[1] = 'o';
    Table[6].tag[2] = 'c';
    Table[6].tag[3] = 'a';
    Table[6].checkSum = 0;
    Table[6].length = sizeof(USHORT) * 3;

    Table[2].tag[0] = 'g';
    Table[2].tag[1] = 'l';
    Table[2].tag[2] = 'y';
    Table[2].tag[3] = 'f';
    Table[2].checkSum = 0;
    Table[2].length = 1;

    Table[8].tag[0] = 'n';
    Table[8].tag[1] = 'a';
    Table[8].tag[2] = 'm';
    Table[8].tag[3] = 'e';
    Table[8].checkSum = 0;
    Table[8].length = (sizeof(name_table) +
                       sizeof(Macnamestring) +
                       sizeof(Unamestring) + 3) / 4;
    Table[8].length *= 4;
    NameLength = Table[8].length;

    Table[9].tag[0] = 'p';
    Table[9].tag[1] = 'o';
    Table[9].tag[2] = 's';
    Table[9].tag[3] = 't';
    Table[9].checkSum = 0;
    Table[9].length = sizeof(post_table);

    for (i=0;i<10;i++) {
        ULONG LENGTH, Sum = 0L;
        ULONG *EndPtr, *Data, *Current;

        offset = Table[i].offset;
        length = Table[i].length;
        LENGTH = (length + 3 & ~3);
        Data = (ULONG *)malloc(LENGTH);
        memset(Data, 0x00, LENGTH);
        fseek(OutputFile, offset, SEEK_SET);
        fread(Data, length, 1, OutputFile);

        Current = Data;
        EndPtr = Data + (LENGTH / sizeof(ULONG));
        while(Current < EndPtr){
#ifdef LITTLE_ENDIAN
            Working = *Current++;
            Sum += ((Working & 0xff) << 24) +
                ((Working & 0xff00) << 8) +
                ((Working & 0xff0000) >> 8) +
                (Working >> 24);
#else
            Sum += *Current++;
#endif
        }
        free(Data);

#ifdef LITTLE_ENDIAN
        Table[i].offset =
            ((offset & 0xff) << 24) +
            ((offset & 0xff00) << 8) +
            ((offset & 0xff0000) >> 8) +
            (offset >> 24);
        Table[i].length =
            ((length & 0xff) << 24) +
            ((length & 0xff00) << 8) +
            ((length & 0xff0000) >> 8) +
            (length >> 24);
        Table[i].checkSum =
            ((Sum & 0xff) << 24) +
            ((Sum & 0xff00) << 8) +
            ((Sum & 0xff0000) >> 8) +
            (Sum >> 24);
#else
        Table[i].checkSum = Sum;
#endif
    }

    fseek(OutputFile, sizeof(Offset_Table), SEEK_SET);
    fwrite (&Table, sizeof(TableRecord), 10, OutputFile);

    fseek(OutputFile, 0, SEEK_SET);

    for (i=0;i < FileLength / sizeof(long);i++) {
        fread(&Working, sizeof(long), 1, OutputFile);
#ifdef LITTLE_ENDIAN
            checksum += ((Working & 0xff) << 24) +
                ((Working & 0xff00) << 8) +
                ((Working & 0xff0000) >> 8) +
                (Working >> 24);
#else
            checksum += Working;
#endif
    }
    checksum = 0xB1B0AFBA - checksum;
#ifdef LITTLE_ENDIAN
    head.checkSumAdjustment =
        ((checksum & 0xff) << 24) +
        ((checksum & 0xff00) << 8) +
        ((checksum & 0xff0000) >> 8) +
        (checksum >> 24);
#else
    head.checkSumAdjustment = checksum;
#endif
    fseek(OutputFile, HeadTableOffset, SEEK_SET);
    fwrite (&head, sizeof(head_table), 1, OutputFile);
    fclose(OutputFile);

    return 0;
}
