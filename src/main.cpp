#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "proc/CWVProc.hpp"
#include "proc/WAVProc.hpp"

#include <File.hpp>

#include <Error.hpp>

#include <Macro.hpp>

static const char PROJ_VERSION_SUPPORT[] = "1.0";

int main(int argc, const char **argv) {
    if (argc < 3) {
        printf("CWVtool v1.0\n");
        printf("CWVtool was built " __DATE__ " " __TIME__ "\n\n");

        printf("usage: %s [path to cwv] [path to wav]\n", argv[0]);
        return 1;
    }

    const char *pathToCWV = NULL;
    const char *pathToWAV = NULL;

    for (s32 i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if ((arg[0] == '-') && (arg[1] == '-')) {
            arg += 2;
            {
                arg -= 2;
                Warn("unknown argument \"%s\" will be ignored", arg);
            }
        }
        else {
            if (pathToCWV == NULL) {
                pathToCWV = arg;
            }
            else if (pathToWAV == NULL) {
                pathToWAV = arg;
            }
            else {
                Warn("additional argument \"%s\" will be ignored", arg);
            }
        }
    }

    Buffer cwvBuffer = fileReadData(pathToCWV);
    if (!cwvBuffer.check()) {
        Panic("main: failed to load CWV at path \"%s\"", pathToCWV);
    }
    CWVSound cwvSound (cwvBuffer);

    Warn("Sample Count : %u", cwvSound.calcSampleCount());
    Warn("Channel Count : %u", cwvSound.getChannelCount());
    Warn("Sample Rate : %u", cwvSound.getSampleRate());
    
    WAVSound wavSound (
        cwvSound.getSampleData(), cwvSound.calcSampleCount(),
        cwvSound.getChannelCount(), cwvSound.getSampleRate()
    );
    Buffer wavBuffer = wavSound.build();

    if (!fileWriteData(pathToWAV, wavBuffer)) {
        Panic("main: failed to write WAV to path \"%s\"", pathToWAV);
    }
}