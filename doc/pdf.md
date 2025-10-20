Design notes from Ken Sharp, with light editing.

We think one solution is a font with a single glyph (.notdef) and a
CIDToGIDMap which maps all the CIDs to 0. That map would then be
stored as a stream in the PDF file, and when flat compressed should
be pretty small. The font, of course, will be approximately the same
size as the one you currently use.

I'm working on such a font now, the CIDToGIDMap is trivial, you just
create a stream object which contains 128k bytes (2 bytes per possible
CID and your CIDs range from 0 to 65535) and where you currently have
`"/CIDToGIDMap /Identity"` you would have `"/CIDToGIDMap <object> 0 R"`.

Note that if, in future, you were to use a different (ie not 2 byte)
CMap for character codes you could trivially extend the CIDToGIDMap.

The following is an explanation of how some of the font stuff works,
this may be too simple for you in which case please accept my
apologies, its hard to know how much knowledge someone has. You can
skip all this anyway, its just for information.

The font embedded in a PDF file is usually intended just to be
rendered, but extensions allow for at least some ability to locate (or
copy) text from a document. This isn't something which was an original
goal of the PDF format, but its been retro-fitted, presumably due to
popular demand.

To do this reliably the PDF file must contain a ToUnicode CMap, a
device for mapping character codes to Unicode code points. If one of
these is present, then this will be used to convert the character
codes into Unicode values. If its not present then the reader will
fall back through a series of heuristics to try and guess the
result. This is, as you would expect, prone to failure.

This doesn't concern you of course, since you always write a ToUnicode
CMap, so because you are writing the text in text rendering mode 3 it
would seem that you don't really need to worry about this, but in the
PDF spec you cannot have an isolated ToUnicode CMap, it has to be
attached to a font, so in order to get even copy/paste to work you
need to define a font.

This is what leads to problems, tools like pdfwrite assume that they
are going to be able to (or even have to) modify the font entries, so
they require that the font being embedded be valid, and to be honest
the font Tesseract embeds isn't valid (for this purpose).


To see why lets look at how text is specified in a PDF file:

`(Test) Tj`

Now that looks like text but actually it isn't. Each of those bytes is
a 'character code'. When it comes to rendering the text a complex
sequence of events takes place, which converts the character code into
'something' which the font understands. Its entirely possible via
character mappings to have that text render as 'Sftu'

For simple fonts (PostScript type 1), we use the character code as the
index into an Encoding array (256 elements), each element of which is
a glyph name, so this gives us a glyph name. We then consult the
CharStrings dictionary in the font, that's a complex object which
contains pairs of keys and values, you can use the key to retrieve a
given value. So we have a glyph name, we then use that as the key to
the dictionary and retrieve the associated value. For a type 1 font,
the value is a glyph program that describes how to draw the glyph.

For CIDFonts, its a little more complicated. Because CIDFonts can be
large, using a glyph name as the key is unreasonable (it would also
lead to unfeasibly large Encoding arrays), so instead we use a 'CID'
as the key. CIDs are just numbers.

But.... We don't use the character code as the CID. What we do is use
a CMap to convert the character code into a CID. We then use the CID
to key the CharStrings dictionary and proceed as before. So the 'CMap'
is the equivalent of the Encoding array, but its a more compact and
flexible representation.

Note that you have to use the CMap just to find out how many bytes
constitute a character code, and it can be variable. For example you
can say if the first byte is 0x00->0x7f then its just one byte, if its
0x80->0xf0 then its 2 bytes and if its 0xf0->0xff then its 3 bytes. I
have seen CMaps defining character codes up to 5 bytes wide.

Now that's fine for 'PostScript' CIDFonts, but its not sufficient for
TrueType CIDFonts. The thing is that TrueType fonts are accessed using
a Glyph ID (GID) (and the LOCA table) which may well not be anything
like the CID. So for this case PDF includes a CIDToGIDMap. That maps
the CIDs to GIDs, and we can then use the GID to get the glyph
description from the GLYF table of the font.

So for a TrueType CIDFont, character-code->CID->GID->glyf-program.

Looking at the PDF file I was supplied with we see that it contains
text like :

`<0x0075> Tj`

So we start by taking the character code (117) and look it up in the
CMap. Well you don't supply a CMap, you just use the Identity-H one
which is predefined. So character code 117 maps to CID 117. Then we
use the CIDToGIDMap, again you don't supply one, you just use the
predefined 'Identity' map. So CID 117 maps to GID 117. But the font we
were supplied with only contains 116 glyphs.

Now for Latin that's not a huge problem, you can just supply a bigger
font. But for more complex languages that *is* going to be more of a
problem. Either you need to supply a font which contains glyphs for
all the possible CID->GID mappings, or we need to think laterally.

Our solution using a TrueType CIDFont is to intervene at the
CIDToGIDMap stage and convert all the CIDs to GID 0. Then we have a
font with just one glyph, the .notdef glyph at GID 0. This is what I'm
looking into now.

It would also be possible to have a 'PostScript' (ie type 1 outlines)
CIDFont which contained 1 glyph, and a CMap which mapped all character
codes to CID 0. The effect would be the same.

Its possible (I haven't checked) that the PostScript CIDFont and
associated CMap would be smaller than the TrueType font and associated
CIDToGIDMap.

--- in a followup ---

OK there is a small problem there, if I use GID 0 then Acrobat gets
upset about it and complains it cannot extract the font. If I set the
CIDToGIDMap so that all the entries are 1 instead, it's happy. Totally
mad......
