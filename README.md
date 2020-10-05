# tesseract-ocr with visible_pdf_image

This is a modified version of Tesseract. The official upstream project is here: https://github.com/tesseract-ocr/tesseract

The modifications in the visible_pdf_image branch enable the user to input both a "cleaned" image to be used for OCR and a "visible" image that the user will see in the output PDF. Cleaning an image helps OCR engines by removing background colors and patterns, sharpening text, increasing contrast, etc. The process usually makes the image look terrible to humans, so the idea with these patches is to give us the best of both worlds.

To clean an image for OCR, I suggest using textcleaner from Fred's ImageMagick Scripts: http://www.fmwconcepts.com/imagemagick/textcleaner/

For the visible image you can use the original copy, but I suggest using a compressed version instead to save space. The only requirement is that the dimensions of the "cleaned" and "visible" images are the same.

Once you've built the visible_pdf_image branch along with the other Tesseract dependencies, just add `--visible-pdf-image <image>` to the arguments. For example:

	tesseract -l eng --visible-pdf-image compressed.webp cleaned.pnm out pdf

I've found this to be very helpful when creating searchable PDFs from scanned documents, receipts, etc. YMMV. Here's the original feature request upstream: https://github.com/tesseract-ocr/tesseract/issues/210
