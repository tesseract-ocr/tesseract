#!/usr/bin/env python3

# Create C/C++ code for two lookup tables.

import math

# Size of static tables.
kTableSize = 4096
# Scale factor for float arg to int index.
kScaleFactor = 256.0

print("// Generated code with lookup tables")
print('#include "functions.h"')
print("namespace tesseract {")

print("const double TanhTable[] = {")
for i in range(kTableSize):
    print("  %a," % math.tanh(i / kScaleFactor))
print("};")

print("const double LogisticTable[] = {")
for i in range(kTableSize):
    print("  %a," % (1 / (1 + math.exp(-i / kScaleFactor))))
print("};")
print("}  // namespace tesseract.")
