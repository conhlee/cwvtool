#ifndef LIB_API_H
#define LIB_API_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus

#include <Buffer.hpp>

#include <proc/WAVProc.hpp>
#include <proc/CWVProc.hpp>

extern "C" {
#endif

void* CWV_decode(void* cwvData, size_t size, uint32_t loopTimes = 0, bool applyVolume = false, bool applyPitch = false, bool applyPan = false);
void* CWV_encode (
    void* wavData, size_t size, const char* name = "",
    bool loopEnable = false, s32 loopStart = -1, s32 loopEnd = -1,
    f32 volume = 1.0f, f32 pitch = 1.0f, f32 pan = 0.0f
);

#ifdef __cplusplus
}

namespace CWV {
    WAVSound decode(CWVSound &cwvSound, u32 loopTimes = 0, bool applyVolume = false, bool applyPitch = false, bool applyPan = false);
    CWVSound encode(
        WAVSound &wavSound, const char* name = "",
        bool loopEnable = false, s32 loopStart = -1, s32 loopEnd = -1,
        f32 volume = 1.0f, f32 pitch = 1.0f, f32 pan = 0.0f
    );

    Buffer decode(Buffer &cwvBuffer, u32 loopTimes = 0, bool applyVolume = false, bool applyPitch = false, bool applyPan = false);
    Buffer encode(
        Buffer &wavBuffer, const char* name = "",
        bool loopEnable = false, s32 loopStart = -1, s32 loopEnd = -1,
        f32 volume = 1.0f, f32 pitch = 1.0f, f32 pan = 0.0f
    );
};

#endif

#endif