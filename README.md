# cwvtool

CWVTool is a tool for encoding & decoding CWV sound files used in Rhythm Heaven Groove.

## about CWV

CWV (presumably meaning Compressed WaVe), is a file format storing sample data in some custom form of ADPCM, information such as channel count, sample rate and sample count, and some additonal data (loop points, volume, pitch, and pan).

This ADPCM utilises a LUT (lookup table) to convert from the ADPCM sample to a delta PCM sample. This LUT is non-linear and offers higher precision / accuracy to delta values which are closer to zero. This LUT (and its inverse) can be found in [src/proc/CWVProc_LUT.inc.cpp](src/proc/CWVProc_LUT.inc.cpp).

## building

The ninja build system is required. To build, simply run `ninja` in the root directory.



