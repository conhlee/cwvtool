#include <cmath>

#include "Macro.hpp"

#include "Buffer.hpp"

#include "proc/CWVProc.hpp"
#include "proc/WAVProc.hpp"

#include "API.h"

namespace CWV {
    Buffer decode(Buffer &cwvBuffer, u32 loopTimes, bool applyVolume, bool applyPitch, bool applyPan) {
        CWVSound cwvSound (cwvBuffer);

        WAVSound wavSound;
        if (loopTimes > 0) {
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
            u32 finalLength = loopStart + (loopLength * (loopTimes + 1));
            
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
        if (applyPan && (pan != 0.0f) && (cwvSound.getChannelCount() == 2)) {
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
        if (applyVolume && (volume != 1.0f)) {
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
        if (applyPitch && (pitch != 1.0f)) {
            f32 newRate = static_cast<f32>(wavSound.getSampleRate()) * pitch;
            wavSound.setSampleRate(static_cast<u32>(newRate));
        }

        return wavSound.build();
    }

    Buffer encode(Buffer &wavBuffer, const char* name, bool loopEnable, s32 loopStart, s32 loopEnd, f32 volume, f32 pitch, f32 pan) {
        WAVSound wavSound (wavBuffer);

        CWVSound cwvSound (
            wavSound.getSampleData(), wavSound.calcSampleCount(),
            wavSound.getChannelCount(), wavSound.getSampleRate()
        );

        if (loopEnable) {
            if (loopEnd < 0) {
                loopEnd = wavSound.calcSampleCount() + loopEnd + 1;
            }

            cwvSound.setLoopStart(loopStart);
            cwvSound.setLoopEnd(loopEnd);
        }

        cwvSound.setVolume(volume);
        cwvSound.setPitch(pitch);
        cwvSound.setPan(pan);

        cwvSound.setName(name);

        return cwvSound.build();
    }
}

extern "C" void* CWV_decode(void* cwvData, size_t size, uint32_t loopTimes, bool applyVolume, bool applyPitch, bool applyPan) {
    Buffer cwvBuffer = Buffer(size);
    memcpy(cwvBuffer.data(), cwvData, size);

    Buffer wavBuffer = CWV::decode(cwvBuffer, loopTimes, applyVolume, applyPitch, applyPan);

    void* wavData = malloc(wavBuffer.get_size());
    memcpy(wavData, wavBuffer.data(), wavBuffer.get_size());

    return wavData;
}

extern "C" void* CWV_encode(void* wavData, size_t size, const char* name, bool loopEnable, s32 loopStart, s32 loopEnd, f32 volume, f32 pitch, f32 pan) {
    Buffer wavBuffer = Buffer(size);
    memcpy(wavBuffer.data(), wavData, size);

    Buffer cwvBuffer = CWV::encode(wavBuffer, name, loopEnable, loopStart, loopEnd, volume, pitch, pan);

    void* cwvData = malloc(cwvBuffer.get_size());
    memcpy(cwvData, cwvBuffer.data(), cwvBuffer.get_size());

    return cwvData;
}