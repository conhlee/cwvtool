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
    u16 getChannelCount(void) const { return mChannelCount; }

    /* NOTE: returns samples per channel. */
    u32 calcSampleCount(void) const { return mSampleCount; }

    const s16 *getSampleData(void) const { return mSampleData; }

    Buffer build(void);

private:
    u32 mSampleRate;
    u32 mSampleCount;
    u16 mChannelCount;
    s16 *mSampleData;
    char *mName;
};

#endif // PROC_CWV_PROC_HPP
