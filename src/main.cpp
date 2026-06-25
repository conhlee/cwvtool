#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>

#include "proc/CWVProc.hpp"
#include "proc/WAVProc.hpp"

#include <File.hpp>

#include <Error.hpp>

#include <Macro.hpp>

enum {
    MODE_NONE,
    MODE_DECODE,
    MODE_ENCODE
};

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("CWVtool v1.2\n");
        printf("CWVtool was built " __DATE__ " " __TIME__ "\n\n");

        printf("usage for decode: %s decode <path to cwv> [path to wav]\n", argv[0]);
        printf("                  [--apply-volume] [--apply-pitch] [--apply-pan]\n");
        printf("                  [--loop-times=<times>]\n");
        printf("usage for encode: %s encode <path to wav> [path to cwv]\n", argv[0]);
        printf("                  [--volume=<vol>] [--pitch=<pitch>] [--pan=<pan>]\n");
        printf("                  [--name=<name>]\n");
        printf("                  [--loop-start=<offset>] [--loop-end=<offset>]\n");
        printf("                                             ^ note: negative offset\n");
        printf("                                                     is allowed here\n");
        return 1;
    }

    s32 mode = MODE_NONE;
    char *pathToCWV = NULL;
    char *pathToWAV = NULL;

    bool decApplyVolume = false;
    bool decApplyPitch = false;
    bool decApplyPan = false;

    u32 decLoopTimes = 0;

    bool encLoopEnable = false;
    s32 encLoopStart = 0;
    s32 encLoopEnd = -1;
    f32 encVolume = 1.0f;
    f32 encPitch = 1.0f;
    f32 encPan = 0.0f;
    const char *encName = "";

    for (s32 i = 1; i < argc; i++) {
        char *arg = argv[i];
        if ((arg[0] == '-') && (arg[1] == '-')) {
            arg += 2;
            if (strcasecmp(arg, "apply-volume") == 0) {
                decApplyVolume = true;
            }
            else if (strcasecmp(arg, "apply-pitch") == 0) {
                decApplyPitch = true;
            }
            else if (strcasecmp(arg, "apply-pan") == 0) {
                decApplyPan = true;
            }
            else if (strncasecmp(arg, "loop-times=", STR_LIT_LEN("loop-times=")) == 0) {
                const char *strLoopTimes = arg + STR_LIT_LEN("loop-times=");
                decLoopTimes = strtol(strLoopTimes, NULL, 0);
            }
            else if (strncasecmp(arg, "loop-start=", STR_LIT_LEN("loop-start=")) == 0) {
                const char *strLoopStart = arg + STR_LIT_LEN("loop-start=");
                encLoopStart = strtol(strLoopStart, NULL, 0);
                encLoopEnable = true;
            }
            else if (strncasecmp(arg, "loop-end=", STR_LIT_LEN("loop-end=")) == 0) {
                const char *strLoopEnd = arg + STR_LIT_LEN("loop-end=");
                encLoopEnd = strtol(strLoopEnd, NULL, 0);
                encLoopEnable = true;
            }
            else if (strncasecmp(arg, "volume=", STR_LIT_LEN("volume=")) == 0) {
                const char *strVolume = arg + STR_LIT_LEN("volume=");
                encVolume = strtof(strVolume, NULL);
            }
            else if (strncasecmp(arg, "pitch=", STR_LIT_LEN("pitch=")) == 0) {
                const char *strPitch = arg + STR_LIT_LEN("pitch=");
                encPitch = strtof(strPitch, NULL);
            }
            else if (strncasecmp(arg, "pan=", STR_LIT_LEN("pan=")) == 0) {
                const char *strPan = arg + STR_LIT_LEN("pan=");
                encPan = strtof(strPan, NULL);
            }
            else if (strncasecmp(arg, "name=", STR_LIT_LEN("name=")) == 0) {
                encName = arg + STR_LIT_LEN("name=");
            }
            else {
                arg -= 2;
                Warn("unknown argument \"%s\" will be ignored", arg);
            }
        }
        else {
            if (mode == MODE_NONE) {
                if (strcasecmp(arg, "decode") == 0) {
                    mode = MODE_DECODE;
                }
                else if (strcasecmp(arg, "encode") == 0) {
                    mode = MODE_ENCODE;
                }
                else {
                    Panic("main: unknown mode argument \"%s\"", arg);
                }
            }
            else if (mode == MODE_DECODE) {
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
            else if (mode == MODE_ENCODE) {
                if (pathToWAV == NULL) {
                    pathToWAV = arg;
                }
                else if (pathToCWV == NULL) {
                    pathToCWV = arg;
                } 
                else {
                    Warn("additional argument \"%s\" will be ignored", arg);
                }
            }
        }
    }

    if (mode == MODE_DECODE) {
        char *wavPath = pathToWAV;
        if (wavPath == NULL) {
            wavPath = new char[strlen(pathToCWV) + STR_LIT_LEN(".wav") + 1];
            sprintf(wavPath, "%s.wav", pathToCWV);
        }

        Buffer cwvBuffer = fileReadData(pathToCWV);
        if (!cwvBuffer.check()) {
            Panic("main: failed to load CWV at path \"%s\"", pathToCWV);
        }
        CWVSound cwvSound (cwvBuffer);

        printf("-- Decode CWV \"%s\" --\n", pathToCWV);
        printf("Name: %s\n", cwvSound.getName());

        f32 seconds = static_cast<f32>(cwvSound.calcSampleCount()) / cwvSound.getSampleRate();
        printf("Sample Count: %u (%f seconds)\n", cwvSound.calcSampleCount(), seconds);

        printf("Channel Count: %u\n", cwvSound.getChannelCount());
        printf("Sample Rate: %u\n", cwvSound.getSampleRate());

        printf("Loop: ");
        if (cwvSound.getLoopEnabled()) {
            printf("(start=%u, end=%u)\n", cwvSound.getLoopStart(), cwvSound.getLoopEnd());
        }
        else {
            printf("(none)\n");
        }

        printf("Volume: %f%s\n", cwvSound.getVolume(), decApplyVolume ? " (Applied)" : "");
        printf("Pitch: %f%s\n", cwvSound.getPitch(), decApplyPitch ? " (Applied)" : "");
        printf("Pan: %f%s\n", cwvSound.getPan(), decApplyPan ? " (Applied)" : "");
        
        WAVSound wavSound;
        if (decLoopTimes > 0) {
            u32 loopStart, loopEnd;
            if (cwvSound.getLoopEnabled()) {
                loopStart = cwvSound.getLoopStart();
                loopEnd = cwvSound.getLoopEnd();
            }
            else {
                loopStart = 0;
                loopEnd = cwvSound.calcSampleCount();
            }

            u32 loopLength = loopEnd - loopStart;
            u32 finalLength = loopStart + (loopLength * (decLoopTimes + 1));
            
            const s16 *source = cwvSound.getSampleData();

            s16 *sampleData = new s16[finalLength * cwvSound.getChannelCount()];

            if (loopStart != 0) {
                memcpy(sampleData, source, loopStart * sizeof(s16) * cwvSound.getChannelCount());
            }
            for (u32 i = loopStart; i < finalLength; i += loopLength) {
                memcpy(
                    sampleData + (i * cwvSound.getChannelCount()),
                    source + (loopStart * cwvSound.getChannelCount()),
                    loopLength * sizeof(s16) * cwvSound.getChannelCount()
                );
            }

            wavSound = WAVSound (
                sampleData, finalLength, 
                cwvSound.getChannelCount(), cwvSound.getSampleRate()
            );

            delete[] sampleData;
        }
        else {
            wavSound = WAVSound(
                cwvSound.getSampleData(), cwvSound.calcSampleCount(),
                cwvSound.getChannelCount(), cwvSound.getSampleRate()
            );
        }

        /* TODO: confirm accuracy */
        f32 pan = cwvSound.getPan();
        if (decApplyPitch && (pan != 0.0f) && (cwvSound.getChannelCount() == 2)) {
            f32 p = (pan + 1.0f) * 0.7853982f;

            f32 vol[2];
            vol[0] = cosf(p);
            vol[1] = sinf(p);
            
            vol[0] = MAX(MIN(vol[0], 1.0f), 0.0f);
            vol[1] = MAX(MIN(vol[1], 1.0f), 0.0f);

            s16 *sampleData = wavSound.getSampleData();
            for (u32 i = 0; i < wavSound.calcSampleCount(); i += wavSound.getChannelCount()) {
                for (u32 j = 0; j < wavSound.getChannelCount(); j++) {
                    s32 sample = static_cast<s32>(static_cast<f32>(sampleData[i]) * vol[j]);
                    if (sample > (s16)0x7FFF) {
                        sample = (s16)0x7FFF;
                    }
                    else if (sample < (s16)0x8000) {
                        sample = (s16)0x8000;
                    }
                    sampleData[i] = static_cast<s16>(sample);
                }
            }
        }

        f32 volume = cwvSound.getVolume();
        if (decApplyVolume && (volume != 1.0f)) {
            s16 *sampleData = wavSound.getSampleData();
            u32 sampleCount = wavSound.calcSampleCount() * wavSound.getChannelCount();
            for (u32 i = 0; i < sampleCount; i++) {
                s32 sample = static_cast<s32>(static_cast<f32>(sampleData[i]) * volume);
                if (sample > (s16)0x7FFF) {
                    sample = (s16)0x7FFF;
                }
                else if (sample < (s16)0x8000) {
                    sample = (s16)0x8000;
                }
                sampleData[i] = static_cast<s16>(sample);
            }
        }

        f32 pitch = cwvSound.getPitch();
        if (decApplyPitch && (pitch != 1.0f)) {
            f32 newRate = static_cast<f32>(wavSound.getSampleRate()) * pitch;
            wavSound.setSampleRate(static_cast<u32>(newRate));
        }

        Buffer wavBuffer = wavSound.build();

        if (!fileWriteData(wavPath, wavBuffer)) {
            Panic("main: failed to write WAV to path \"%s\"", wavPath);
        }

        if (pathToWAV == NULL) {
            delete[] wavPath;
        }
    }
    else if (mode == MODE_ENCODE) {
        char *cwvPath = pathToCWV;
        if (cwvPath == NULL) {
            cwvPath = new char[strlen(pathToWAV) + STR_LIT_LEN(".cwv") + 1];
            sprintf(cwvPath, "%s.cwv", pathToWAV);
        }

        Buffer wavBuffer = fileReadData(pathToWAV);
        if (!wavBuffer.check()) {
            Panic("main: failed to load WAV at path \"%s\"", pathToWAV);
        }
        WAVSound wavSound (wavBuffer);

        CWVSound cwvSound (
            wavSound.getSampleData(), wavSound.calcSampleCount(),
            wavSound.getChannelCount(), wavSound.getSampleRate()
        );

        if (encLoopEnable) {
            if (encLoopEnd < 0) {
                encLoopEnd = wavSound.calcSampleCount() + encLoopEnd + 1;
            }

            cwvSound.setLoopStart(encLoopStart);
            cwvSound.setLoopEnd(encLoopEnd);
        }

        cwvSound.setVolume(encVolume);
        cwvSound.setPitch(encPitch);
        cwvSound.setPan(encPan);

        cwvSound.setName(encName);

        Buffer cwvBuffer = cwvSound.build();

        if (!fileWriteData(cwvPath, cwvBuffer)) {
            Panic("main: failed to write CWV to path \"%s\"", cwvPath);
        }

        if (pathToCWV == NULL) {
            delete[] cwvPath;
        }
    }
}