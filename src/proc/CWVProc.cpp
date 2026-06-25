#include "CWVProc.hpp"

#include <Error.hpp>

#include <Macro.hpp>
#include <cstring>

/*
 * demov1.0 7100ae7448
 */

static const u16 s_cwvLUT[256] = {
    0x0,     0x2,     0x4,     0x8,
    0xD,     0x13,    0x1B,    0x24,
    0x2F,    0x3C,    0x4B,    0x5C,
    0x70,    0x85,    0x9D,    0xB7,
    0xD4,    0xF4,    0x116,   0x13B,
    0x163,   0x18E,   0x1BC,   0x1ED,
    0x222,   0x259,   0x294,   0x2D2,
    0x314,   0x359,   0x3A1,   0x3EE,
    0x43E,   0x491,   0x4E9,   0x544,
    0x5A3,   0x606,   0x66D,   0x6D9,
    0x748,   0x7BC,   0x833,   0x8AF,
    0x930,   0x9B4,   0xA3D,   0xACB,
    0xB5D,   0xBF4,   0xC8F,   0xD2F,
    0xDD3,   0xE7D,   0xF2B,   0xFDE,
    0x1096,  0x1153,  0x1214,  0x12DB,
    0x13A7,  0x1478,  0x154E,  0x1629,
    0x1709,  0x17EF,  0x18DA,  0x19CA,
    0x1AC0,  0x1BBB,  0x1CBC,  0x1DC2,
    0x1ECD,  0x1FDE,  0x20F5,  0x2212,
    0x2334,  0x245B,  0x2589,  0x26BC,
    0x27F5,  0x2934,  0x2A79,  0x2BC4,
    0x2D15,  0x2E6C,  0x2FC9,  0x312B,
    0x3294,  0x3404,  0x3579,  0x36F5,
    0x3876,  0x39FE,  0x3B8D,  0x3D21,
    0x3EBD,  0x405E,  0x4206,  0x43B4,
    0x4569,  0x4725,  0x48E7,  0x4AAF,
    0x4C7F,  0x4E54,  0x5031,  0x5214,
    0x53FE,  0x55EF,  0x57E7,  0x59E5,
    0x5BEB,  0x5DF7,  0x600A,  0x6224,
    0x6446,  0x666E,  0x689D,  0x6AD3,
    0x6D11,  0x6F55,  0x71A1,  0x73F4,
    0x764E,  0x78B0,  0x7B18,  0x7D88,
    0x8000,  0x8278,  0x84E8,  0x8750,
    0x89B2,  0x8C0C,  0x8E5F,  0x90AB,
    0x92EF,  0x952D,  0x9763,  0x9992,
    0x9BBA,  0x9DDC,  0x9FF6,  0xA209,
    0xA415,  0xA61B,  0xA819,  0xAA11,
    0xAC02,  0xADEC,  0xAFCF,  0xB1AC,
    0xB381,  0xB551,  0xB719,  0xB8DB,
    0xBA97,  0xBC4C,  0xBDFA,  0xBFA2,
    0xC143,  0xC2DF,  0xC473,  0xC602,
    0xC78A,  0xC90B,  0xCA87,  0xCBFC,
    0xCD6C,  0xCED5,  0xD037,  0xD194,
    0xD2EB,  0xD43C,  0xD587,  0xD6CC,
    0xD80B,  0xD944,  0xDA77,  0xDBA5,
    0xDCCC,  0xDDEE,  0xDF0B,  0xE022,
    0xE133,  0xE23E,  0xE344,  0xE445,
    0xE540,  0xE636,  0xE726,  0xE811,
    0xE8F7,  0xE9D7,  0xEAB2,  0xEB88,
    0xEC59,  0xED25,  0xEDEC,  0xEEAD,
    0xEF6A,  0xF022,  0xF0D5,  0xF183,
    0xF22D,  0xF2D1,  0xF371,  0xF40C,
    0xF4A3,  0xF535,  0xF5C3,  0xF64C,
    0xF6D0,  0xF751,  0xF7CD,  0xF844,
    0xF8B8,  0xF927,  0xF993,  0xF9FA,
    0xFA5D,  0xFABC,  0xFB17,  0xFB6F,
    0xFBC2,  0xFC12,  0xFC5F,  0xFCA7,
    0xFCEC,  0xFD2E,  0xFD6C,  0xFDA7,
    0xFDDE,  0xFE13,  0xFE44,  0xFE72,
    0xFE9D,  0xFEC5,  0xFEEA,  0xFF0C,
    0xFF2C,  0xFF49,  0xFF63,  0xFF7B,
    0xFF90,  0xFFA4,  0xFFB5,  0xFFC4,
    0xFFD1,  0xFFDC,  0xFFE5,  0xFFED,
    0xFFF3,  0xFFF8,  0xFFFC,  0xFFFE
};

static inline u8 calcLUTSample(s16 value) {
    u32 best = 0;
    u32 bestDiff = ABS((s16)s_cwvLUT[0] - value);

    if (bestDiff == 0) {
        return best;
    }

    for (u32 i = 1; i < ARRAY_LENGTH(s_cwvLUT); i++) {
        u32 diff = ABS((s16)s_cwvLUT[i] - value);

        if (diff == 0) {
            return i;
        }
        else if (diff < bestDiff) {
            bestDiff = diff;
            best = i;
        }
    }

    return best;
}

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

            u8 lutSample = calcLUTSample(diff);
            dest[index] = lutSample;

            prevSample += (s16)s_cwvLUT[lutSample];
        }
    }
}
