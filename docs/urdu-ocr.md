\# Urdu OCR with Tesseract



This document provides practical notes for using Tesseract OCR with Urdu text.



\## Language Pack

To use Urdu OCR, install the Urdu trained data.



Run OCR with:tesseract image.png output -l urd



For mixed Urdu and English:tesseract image.png output -l urd+eng



\## Font Considerations

\- Tesseract works better with \*\*Naskh\*\* fonts

\- \*\*Nastaliq\*\* fonts may produce broken words or incorrect spacing



\## Image Preparation Tips

\- Convert image to grayscale

\- Increase contrast

\- Remove noise if possible

\- Avoid skewed or rotated scans

\- Printed text works better than handwritten text



\## Common Issues

\- Words may break incorrectly

\- Right-to-left text order issues

\- Diacritics may be missing



\## Notes

Urdu OCR accuracy depends heavily on image quality and font style.

