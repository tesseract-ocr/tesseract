#include "baseapi.h"
#include "leptonica/allheaders.h"

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

class BitReader {
    private:
        uint8_t const* data;
        size_t size;
        size_t shift;
    public:
        BitReader(const uint8_t* data, size_t size) :
            data(data), size(size), shift(0)
        { }

        int Read(void) {
            if ( size == 0 ) {
                return 0;
            }

            const int ret = ((*data) >> shift) & 1;

            shift++;
            if ( shift >= 8 ) {
                shift = 0;
                data++;
                size--;
            }

            return ret;
        }
};

tesseract::TessBaseAPI *api = nullptr;

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    (void)argc;
    (void)argv;


#ifndef TESSDATA_PREFIX
#error TESSDATA_PREFIX must be defined
#else
    if ( setenv("TESSDATA_PREFIX", TESSDATA_PREFIX, 1) != 0 ) {
        printf("Setenv failed\n");
        abort();
    }
#endif

    api = new tesseract::TessBaseAPI();
    if ( api->Init(nullptr, "eng") != 0 ) {
        printf("Cannot initialize API\n");
        abort();
    }

    /* Silence output */
    api->SetVariable("debug_file", "/dev/null");

    return 0;
}


static PIX* createPix(BitReader& BR, const size_t width, const size_t height) {
    Pix* pix = pixCreate(width, height, 1);

    if ( pix == nullptr ) {
        printf("pix creation failed\n");
        abort();
    }

    for (size_t i = 0; i < width; i++) {
        for (size_t j = 0; j < height; j++) {
            pixSetPixel(pix, i, j, BR.Read());
        }
    }

    return pix;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    BitReader BR(data, size);

    auto pix = createPix(BR, 100, 100);

    api->SetImage(pix);

    char* outText = api->GetUTF8Text();

    pixDestroy(&pix);
    delete[] outText;

    return 0;
}
