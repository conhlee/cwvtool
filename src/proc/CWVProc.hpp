#ifndef PROC_CWV_PROC_HPP
#define PROC_CWV_PROC_HPP

#include <Type.hpp>

#include <Buffer.hpp>

#include <proc/WAVProc.hpp>

class CWVSound {
public:
    CWVSound(void);
    CWVSound(const Buffer &data); 
    CWVSound(const s16 *sampleData, u32 sampleCount, u16 channelCount, u32 sampleRate);
    ~CWVSound(void);
    
    CWVSound &operator=(CWVSound &&other);

    u32 getSampleRate(void) const { return mSampleRate; }
    void setSampleRate(u32 sampleRate) { mSampleRate = sampleRate; }

    u16 getChannelCount(void) const { return mChannelCount; }

    u32 calcSampleCount(void) const { return mSampleCount; }

    const s16 *getSampleData(void) const { return mSampleData; }
    s16 *getSampleData(void) { return mSampleData; }

    bool getLoopEnabled(void) const { return mLoopLength != 0; }

    u32 getLoopStart(void) const { return mLoopStart; }
    void setLoopStart(u32 loopStart) { mLoopStart = loopStart; }

    u32 getLoopLength(void) const { return mLoopLength; }
    void setLoopLength(u32 loopLength) { mLoopLength = loopLength; }

    f32 getVolume(void) const { return mVolume; }
    void setVolume(f32 volume) { mVolume = volume; }

    f32 getPitch(void) const { return mPitch; }
    void setPitch(f32 pitch) { mPitch = pitch; }

    f32 getPan(void) const { return mPan; }
    void setPan(f32 pan) { mPan = pan; }
    
    const char *getName(void) const { return mName; }
    void setName(const char *name);

    Buffer build(void);

private:
    void doEncode(u8 *dest);

private:
    u32 mSampleRate;
    u32 mSampleCount;
    u16 mChannelCount;
    s16 *mSampleData;

    u32 mLoopStart;
    u32 mLoopLength;
    f32 mVolume;
    f32 mPitch;
    f32 mPan;
    char mName[0xB4 + 1];
};

#endif // PROC_CWV_PROC_HPP
