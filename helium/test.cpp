// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This is the main file for the core front-end to Helium. It is rather
// simple, and does not provide any debugging information.
// The detected text is output to a file called "text.dat".
//
// Please note, that for image loading, Leptonica is required!
//
// Local includes
#include "color.h"
#include "debugging.h"
#include "helium_image.h"
#include "heliumbinarizer.h"
#include "heliumtextdetector.h"
#include "textareas.h"
#include "textrecognition.h"

// C includes
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace helium;

#define FAILIF(cond, msg...) do {                        	 \
        if (cond) { 	                                	 \
	        fprintf(stderr, "%s(%d): ", __FILE__, __LINE__); \
        	        fprintf(stderr, ##msg);                  \
                	exit(1);                                 \
        }                                                	 \
} while(0)

int main(int argc, char** argv) {

	const char *infile, *outfile, *lang;
	void *buffer;
	struct stat s;
	int x, y, ifd;

	FAILIF(argc != 6, 
           "%s infile xres yres outfile lang\n", *argv);

	infile = argv[1];
	FAILIF(sscanf(argv[2], "%d", &x) != 1, "could not parse x!\n");
	FAILIF(sscanf(argv[3], "%d", &y) != 1, "could not parse y!\n");
	outfile = argv[4];
	lang = argv[5];

	printf("input file %s\n", infile);
	ifd = open(infile, O_RDONLY);
	FAILIF(ifd < 0, "open(%s): %s\n", infile, strerror(errno));
	FAILIF(fstat(ifd, &s) < 0, "fstat(%d): %s\n", ifd, strerror(errno));
	printf("file size %lld\n", s.st_size);
	buffer = mmap(NULL, s.st_size * 2 / 3, PROT_READ, MAP_PRIVATE, ifd, 0);
	FAILIF(buffer == MAP_FAILED, "mmap(): %s\n", strerror(errno));
	printf("infile mmapped at %p\n", buffer);

	printf("generating image object\n");
    GrayMap gray(x, y, (uint8 *)buffer);
    Image image = Image::FromGrayMap(gray);
    FAILIF(!image.Valid(), "error while loading image file!");
    
    // Run text detector
    printf("running text detector\n");
    HeliumTextDetector detector;
    detector.SetDefaultParameters();
    detector.DetectText(image);
    
    // Setup binarizer
    printf("setting up binarizer...\n");
    HeliumBinarizer binarizer(image);
    binarizer.AddClusters(detector.GetClusters());

    // Run OCR
    printf("OCRing (language %s)...\n", lang);
    TextAreas text;
    TextRecognition::Init("/sdcard/", 
                          lang,
                          "/sdcard/tessdata/ratings");

    TextRecognition::RecognizeUsingBinarizer(&binarizer, text);
    
    // Output Text
    printf("writing to output file %s\n", outfile);
    text.WriteDatFile(outfile);
    
    printf("done\n");
    return 0;
};
