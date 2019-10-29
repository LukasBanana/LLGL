#!/bin/sh
xcrun -sdk macosx metal -c FillBufferByte4.metal -o FillBufferByte4.air
xcrun -sdk macosx metallib FillBufferByte4.air -o FillBufferByte4.metallib
python GenerateByteArray.py FillBufferByte4.metallib
