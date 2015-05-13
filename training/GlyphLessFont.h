/* I don't expect anyone to run this program, ever again.  It is
 * included primarily as documentation for how the GlyphLessFont was
 * created.
 */

/* The OpenType data types, we'll duplicate the definitions so that
 * the code shall be (as far as possible) self-documenting simply by
 * referencing the OpenType specification. Note that the specification
 * is soemwhat inconsistent with regards to usage, naming and capitalisation
 * of the names for these data types.
 */
typedef char BYTE;
typedef char CHAR;
typedef unsigned short USHORT;
typedef short SHORT;
typedef struct _uint24 {char top8;unsigned short bottom16;} UINT24;
typedef unsigned long ULONG;
typedef long LONG;
typedef unsigned long Fixed;
typedef SHORT FWORD;
typedef USHORT UFWORD;
typedef unsigned short F2DOT14;
typedef struct _datetime {long upper;long lower;} LONGDATETIME;
typedef char Tag[4];
typedef USHORT GlyphId;
typedef USHORT Offset;
typedef struct _longHorMetric {USHORT advanceWidth;SHORT lsb;} longHorMetric;

/* And now definitions for each of the OpenType tables we will wish to use */

typedef struct {
    Fixed sfnt_version;
    USHORT numTables;
    USHORT searchRange;
    USHORT entrySelector;
    USHORT rangeShift;
} Offset_Table;

typedef struct {
    Tag tag;        /* The spec defines this as a ULONG,
                       but also as a 'Tag' in its own right */
    ULONG checkSum;
    ULONG offset;
    ULONG length;
} TableRecord;

typedef struct {
    USHORT version;
    USHORT numTables;
} cmap_header;

typedef struct {
    USHORT platformID;
    USHORT encodingID;
    ULONG Offset;
} cmap_record;

typedef struct {
    USHORT format;
    USHORT length;
    USHORT language;
    BYTE glyphIDArray[256];
} format0_cmap_table;

/* This structure only works for single segment format 4 tables,
   for multiple segments it must be constructed */
typedef struct {
    USHORT format;
    USHORT length;
    USHORT language;
    USHORT segCountx2;
    USHORT searchRange;
    USHORT entrySelector;
    USHORT rangeShift;
    USHORT endcount;
    USHORT reservedPad;
    USHORT startCount;
    SHORT idDelta;
    USHORT idRangeOffset;
    USHORT glyphIdArray[2];
} format4_cmap_table;

typedef struct {
    USHORT format;
    USHORT length;
    USHORT language;
    USHORT firstCode;
    USHORT entryCount;
    USHORT glyphIDArray;
} format6_cmap_table;

typedef struct {
    cmap_header header;
    cmap_record records[2];
    format6_cmap_table AppleTable;
    format6_cmap_table MSTable;
} cmap_table;

typedef struct {
    Fixed version;
    Fixed FontRevision;
    ULONG checkSumAdjustment;
    ULONG MagicNumber;
    USHORT Flags;
    USHORT unitsPerEm;
    LONGDATETIME created;
    LONGDATETIME modified;
    SHORT xMin;
    SHORT yMin;
    SHORT xMax;
    SHORT yMax;
    USHORT macStyle;
    USHORT lowestRecPPEM;
    SHORT FontDirectionHint;
    SHORT indexToLocFormat;
    SHORT glyphDataFormat;
    SHORT PAD;
} head_table;

typedef struct {
    Fixed version;
    FWORD Ascender;
    FWORD Descender;
    FWORD LineGap;
    UFWORD advanceWidthMax;
    FWORD minLeftSideBearing;
    FWORD minRightSideBearing;
    FWORD xMaxExtent;
    SHORT caretSlopeRise;
    SHORT caretSlopeRun;
    SHORT caretOffset;
    SHORT reserved1;
    SHORT reserved2;
    SHORT reserved3;
    SHORT reserved4;
    SHORT metricDataFormat;
    USHORT numberOfHMetrics;
} hhea_table;

typedef struct {
    longHorMetric hMetrics[2];
} hmtx_table;

typedef struct {
    Fixed version;
    USHORT numGlyphs;
    USHORT maxPoints;
    USHORT maxContours;
    USHORT maxCompositePoints;
    USHORT maxCompositeContours;
    USHORT maxZones;
    USHORT maxTwilightPoints;
    USHORT maxStorage;
    USHORT maxFunctionDefs;
    USHORT maxInstructionDefs;
    USHORT maxStackElements;
    USHORT maxSizeOfInstructions;
    USHORT maxComponentElements;
    USHORT maxComponentDepth;
} maxp_table;

typedef struct {
    USHORT platformID;
    USHORT encodingID;
    USHORT languageID;
    USHORT nameID;
    USHORT length;
    USHORT offset;
} NameRecord;

typedef struct {
    USHORT format;
    USHORT count;
    USHORT stringOffset;
    NameRecord nameRecord[3];
} name_table;

typedef struct {
    USHORT version;
    SHORT xAvgCharWidth;
    USHORT usWeightClass;
    USHORT usWidthClass;
    USHORT fsType;
    SHORT ySubscriptXSize;
    SHORT ySubscriptYSize;
    SHORT ySubscriptXOffset;
    SHORT ySubscriptYOffset;
    SHORT ySuperscriptXSize;
    SHORT ySuperscriptYSize;
    SHORT ySuperscriptXOffset;
    SHORT ySuperscriptYOffset;
    SHORT yStrikeoutSize;
    SHORT yStrikeoutPosition;
    SHORT sFamilyClass;
    BYTE panose[10];
    ULONG ulUnicodeRange1;
    ULONG ulUnicodeRange2;
    ULONG ulUnicodeRange3;
    ULONG ulUnicodeRange4;
    CHAR achVendID[4];
    USHORT fsSelection;
    USHORT usFirstCharIndex;
    USHORT usLastCharIndex;
    SHORT sTypoAscender;
    SHORT sTypoDescender;
    SHORT sTypoLineGap;
    USHORT usWinAscent;
    USHORT usWinDescent;
    ULONG ulCodePageRange1;
    ULONG ulCodePageRange2;
    SHORT sxHeight;
    SHORT sCapHeight;
    USHORT usDefaultChar;
    USHORT usBreakChar;
    USHORT usMaxContent;
} OS2_table;

typedef struct {
    Fixed version;
    Fixed italicAngle;
    FWORD underlinePosition;
    FWORD underlineThickness;
    ULONG isFixedPitch;
    ULONG minMemType42;
    ULONG maxMemType42;
    ULONG minMemType1;
    ULONG maxMemType1;
} post_table;
