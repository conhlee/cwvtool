#include "CWVProc.hpp"

#include <cstdio>
#include <cstring>

#include <Error.hpp>

#include <Macro.hpp>

#include "CWVProc_LUT.inc.cpp"

typedef struct __attribute__((packed)) {
    u32 flags;
    u32 sampleRate;
    u32 sampleCount;

    u32 unk0C; /* Usually 2 */
    u32 loopStart;
    u32 loopEnd;
    f32 volume;
    f32 pitch; /* Usually 1.0f */
    u32 pan; /* Usually 0.0f */
    u32 unk24; /* Usually 2 */
    u32 unk28;
    u32 unk2C;
    f32 unk30; /* Usually 1.0f */
    u32 unk34;
    u32 unk38;
    u32 unk3C;
    u32 unk40;
    u32 unk44;
    u32 unk48;

    char name[0xB4];
} CWVFooter;
STRUCT_SIZE_ASSERT(CWVFooter, 0x100);

CWVSound::CWVSound(void) :
    mSampleRate(0), mSampleCount(0), mChannelCount(0), mSampleData(NULL),
    mLoopStart(0), mLoopEnd(0), mVolume(1.0f), mPitch(1.0f), mPan(1.0f)
{
    mName[0] = '\0';
}

/*
 * demov1.0 71004ebb10
 */

CWVSound::CWVSound(const Buffer &data) {
    if (!data.check()) {
        Panic("CWVSound ctor: data buffer is invalid");
    }

    if (data.get_size() < sizeof(CWVFooter)) {
        Panic("CWVSound ctor: data buffer is too small (0x%x < 0x%x)", data.get_size(), sizeof(CWVFooter));
    }

    const u8 *samples = data.data<u8>();
    const CWVFooter *footer = data.data<const CWVFooter>(data.get_size() - sizeof(CWVFooter));

    if ((footer->flags >> 1) != 1) {
        Panic("CWVSound ctor: CWV flagbits are bad (%08X)", footer->flags);
    }

    bool stereo = (footer->flags & 1) != 0;
    mChannelCount = stereo ? 2 : 1;

    mSampleCount = footer->sampleCount;
    mSampleRate = footer->sampleRate;

    u32 sampleDataSize = mSampleCount * sizeof(s16) * mChannelCount;
    mSampleData = new s16[sampleDataSize / sizeof(s16)];

    for (u32 i = 0; i < mChannelCount; i++) {
        s16 prevSample = 0;
        for (u32 j = 0; j < mSampleCount; j++) {
            s16 sample = (s16)s_cwvLUT[samples[(j * mChannelCount) + i]];

            s16 finalSample = sample + prevSample;
            prevSample = finalSample;

            mSampleData[(j * mChannelCount) + i] = finalSample;
        }
    }

    u32 nameLength = 0;
    for (u32 i = 0; i < sizeof(footer->name); i++) {
        if (footer->name[i] == '\0') break;
        nameLength++;
    }

    mLoopStart = footer->loopStart;
    mLoopEnd = footer->loopEnd;
    mVolume = footer->volume;
    mPitch = footer->pitch;
    mPan = footer->pan;

    memcpy(mName, footer->name, nameLength);
    mName[nameLength] = '\0';
}

CWVSound::CWVSound(const s16 *sampleData, u32 sampleCount, u16 channelCount, u32 sampleRate) :
    mSampleCount(sampleCount), mChannelCount(channelCount), mSampleRate(sampleRate),
    mLoopStart(0), mLoopEnd(0), mVolume(1.0f), mPitch(1.0f), mPan(1.0f)
{
    u32 sampleDataSize = sampleCount * sizeof(s16) * channelCount;
    mSampleData = new s16[sampleDataSize / sizeof(s16)];
    memcpy(mSampleData, sampleData, sampleDataSize);

    mName[0] = '\0';
}

CWVSound::~CWVSound(void) {
    if (mSampleData != NULL) {
        delete[] mSampleData;
        mSampleData = NULL;
    }
}

CWVSound &CWVSound::operator=(CWVSound &&other) {
    if (this != &other) {
        if (mSampleData != NULL) {
            delete[] mSampleData;
        }
        mSampleRate = other.mSampleRate;
        mSampleCount = other.mSampleCount;
        mChannelCount = other.mChannelCount;
        mSampleData = other.mSampleData;
        memcpy(mName, other.mName, sizeof(mName));

        other.mSampleRate = 0;
        other.mSampleCount = 0;
        other.mChannelCount = 0;
        other.mSampleData = NULL;
        other.mName[0] = '\0';
    }
    return *this;
}

void CWVSound::setName(const char *name) {
    u32 nameLength = strlen(name);
    memcpy(mName, name, MIN(nameLength, sizeof(mName) - 1));
    mName[sizeof(mName) - 1] = '\0';
}

Buffer CWVSound::build(void) {
    u32 sampleDataSize = sizeof(u8) * mSampleCount * mChannelCount;

    u32 fileSize = (sampleDataSize + sizeof(CWVFooter) + 0xFFF) & ~0xFFF;

    u32 footerOffset = fileSize - sizeof(CWVFooter); 

    Buffer buffer (fileSize);

    u8 *samples = buffer.data<u8>();
    doEncode(samples);

    CWVFooter *footer = buffer.data<CWVFooter>(footerOffset);
    footer->flags = (1 << 1) | ((mChannelCount == 2) ? 1 : 0);

    footer->sampleRate = mSampleRate;
    footer->sampleCount = mSampleCount;

    footer->unk0C = 2;
    footer->loopStart = mLoopStart;
    footer->loopEnd = mLoopEnd;
    footer->volume = mVolume;
    footer->pitch = mPitch;
    footer->pan = mPan;
    footer->unk24 = 2;
    footer->unk30 = 1.0f;

    u32 nameLength = strlen(mName);
    memcpy(footer->name, mName, MIN(nameLength, sizeof(footer->name)));

    return buffer;
}

void CWVSound::doEncode(u8 *dest) {
    for (u32 i = 0; i < mChannelCount; i++) {
        s32 prevSample = 0;
        for (u32 j = 0; j < mSampleCount; j++) {
            u32 index = (j * mChannelCount) + i;

            s32 diff = mSampleData[index] - prevSample;

            if (diff > (s16)0x7FFF) {
                diff = (s16)0x7FFF;
            }
            else if (diff < (s16)0x8000) {
                diff = (s16)0x8000;
            }

            u8 lutSample = s_cwvLUTInv[(u16)diff];
            dest[index] = lutSample;

            prevSample += (s16)s_cwvLUT[lutSample];
        }
    }
}
