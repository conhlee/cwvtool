#include "WAVProc.hpp"

#include <Error.hpp>

#define IDENTIFIER_TO_U32(char1, char2, char3, char4) ( \
    ((u32)(char4) << 24) | ((u32)(char3) << 16) |       \
    ((u32)(char2) <<  8) | ((u32)(char1) <<  0)         \
)

enum {
    /* Master */
    WAV_CHUNK_RIFF = IDENTIFIER_TO_U32('R','I','F','F'),
    /* Format */
    WAV_CHUNK_FMT  = IDENTIFIER_TO_U32('f','m','t',' '),
    /* Data */
    WAV_CHUNK_DATA = IDENTIFIER_TO_U32('d','a','t','a'),
};

#define WAVE_MAGIC IDENTIFIER_TO_U32('W','A','V','E')

enum {
    WAV_FORMAT_PCM      = 1,
    WAV_FORMAT_FLOAT    = 3,
    WAV_FORMAT_A_LAW    = 6,
    WAV_FORMAT_MU_LAW   = 7
};

typedef struct __attribute__((packed)) {
    /* Master (RIFF) Chunk */
    u32 riffChunkType; /* Compare to WAVE_CHUNK_RIFF. */
    u32 riffChunkSize; /* The file size can be calculated from this value by adding
                        * the size of riffChunkType and riffChunkSize (effectively 8).
                        */
    u32 waveMagic; /* Compare to WAVE_MAGIC. */
} WavFileHeader;

static inline u32 WavCalcFileSize(const void *wavData) {
    const WavFileHeader *fileHeader = static_cast<const WavFileHeader *>(wavData);
    return fileHeader->riffChunkSize + 8;
}

/* Chunk type is WAVE_CHUNK_FMT. */
typedef struct __attribute__((packed)) {
    u16 format; /* See WAVE_FORMAT enums. */

    u16 channelCount;
    u32 sampleRate;

    u32 dataRate; // Bytes per second (sampleRate * sampleSize * channelCount)

    u16 blockSize; // sampleSize * channelCount

    u16 bitsPerSample; // 8 * sampleSize
} WavFmtChunk;

/* Chunk type is WAVE_CHUNK_DATA. */
typedef struct __attribute__((packed)) {
    u8 data[1];
} WavDataChunk;

typedef struct __attribute__((packed)) {
    u32 type; /* See WAVE_CHUNK enums. */
    u32 size; /* Size of chunk body. */
} WavChunkHeader;

u32 WavCalcChunkSize(const void *chunkData) {
    const WavChunkHeader *chunkHeader = reinterpret_cast<const WavChunkHeader *>(
        static_cast<const u8 *>(chunkData) - sizeof(WavChunkHeader)
    );
    return chunkHeader->size;
}

/* NOTE: WavFindChunk will panic if the specified chunk is not found. */
template <typename T>
const T *WavFindChunk(const void *wavData, u32 type) {
    const WavChunkHeader *chunksStart = reinterpret_cast<const WavChunkHeader *>(
        static_cast<const u8 *>(wavData) + sizeof(WavFileHeader));
    const WavChunkHeader *chunksEnd = reinterpret_cast<const WavChunkHeader *>(
        static_cast<const u8 *>(wavData) + WavCalcFileSize(wavData)
    );
    
    const WavChunkHeader *currentChunk = chunksStart;
    while ((currentChunk + 1) <= chunksEnd) {
        if (currentChunk->type == type) {
            return (const T *)(currentChunk + 1);
        }
        currentChunk = reinterpret_cast<const WavChunkHeader *>(
            reinterpret_cast<const u8 *>(currentChunk + 1) +
            ((currentChunk->size + 1) & ~1)
        );
    }

    const char *typeChars = (const char *)&type;
    Panic(
        "WAVSound: chunk '%c%c%c%c' not found",
        typeChars[0], typeChars[1], typeChars[2], typeChars[3]
    );
}

WAVSound::WAVSound(void) :
    mSampleRate(0), mSampleCount(0), mChannelCount(0),
    mSampleData(NULL)
{}

WAVSound::WAVSound(const Buffer &data) {
    if (!data.check()) {
        Panic("WAVSound ctor: data buffer is invalid");
    }

    const WavFileHeader *fileHeader = data.data<const WavFileHeader>();

    if (fileHeader->riffChunkType != WAV_CHUNK_RIFF) {
        Panic("WAVSound ctor: WAV RIFF chunk type is nonmatching");
    }
    if (fileHeader->waveMagic != WAVE_MAGIC) {
        Panic("WAVSound ctor: WAV WAVE magic is nonmatching");
    }

    u32 expectedSize = WavCalcFileSize(data.data());
    if (expectedSize > data.get_size()) {
        u32 diff = expectedSize - data.get_size();
        Panic("WAVSound ctor: WAV buffer is missing %u bytes of data", diff);
    }

    const WavFmtChunk *fmtChunk = WavFindChunk<WavFmtChunk>(data.data(), WAV_CHUNK_FMT);
    const WavDataChunk *dataChunk = WavFindChunk<WavDataChunk>(data.data(), WAV_CHUNK_DATA);

    u16 sampleFormat = fmtChunk->format;
    u16 bitsPerSample = fmtChunk->bitsPerSample;

    switch (sampleFormat) {
    case WAV_FORMAT_PCM:
        if ((bitsPerSample != 16) && (bitsPerSample != 24)) {
            Panic(
                "WAVSound ctor: %u-bit PCM is not supported (expected 32-bit FLOAT, "
                "16-bit PCM, or 24-bit PCM)",
                bitsPerSample
            );
        }
        break;
    case WAV_FORMAT_FLOAT:
        if (bitsPerSample != 32) {
            Panic(
                "WAVSound ctor: %u-bit FLOAT is not supported (expected 32-bit FLOAT, "
                "16-bit PCM, or 24-bit PCM)",
                bitsPerSample
            );
        }
        break;
    case WAV_FORMAT_A_LAW:
        Panic("WAVSound ctor: A-LAW format is unsupported");
    case WAV_FORMAT_MU_LAW:
        Panic("WAVSound ctor: MU-LAW format is unsupported");
    default:
        Panic("WAVSound ctor: WAV format %u is unknown", sampleFormat);
    }

    mChannelCount = fmtChunk->channelCount;
    if ((mChannelCount != 1) && (mChannelCount != 2)) {
        Panic("WAVSound ctor: channel count of %u is not supported", mChannelCount);
    }

    mSampleRate = fmtChunk->sampleRate;

    mSampleCount = WavCalcChunkSize(dataChunk) / (bitsPerSample / 8) / mChannelCount;

    /* Process sample data now .. */

    u32 totalSampleCount = mSampleCount * mChannelCount;
    mSampleData = new s16[totalSampleCount];

    /* 32-bit FLOAT sample format */
    if (sampleFormat == WAV_FORMAT_FLOAT) {
        const f32 *src = reinterpret_cast<const f32 *>(dataChunk->data);
        for (u32 i = 0; i < totalSampleCount; i++) {
            f32 sample = src[i] * 32768.0f;

            sample += 0.5f;
            if (sample > 32767.0f) {
                sample = 32767.0f;
            }
            else if (sample < -32768.0f) {
                sample = -32768.0f;
            }
            mSampleData[i] = static_cast<s16>(sample);
        }
    }
    /* 16-bit PCM sample format; straight copy */
    else if ((fmtChunk->format == WAV_FORMAT_PCM) && (fmtChunk->bitsPerSample == 16)) {
        memcpy(mSampleData, dataChunk->data, sizeof(s16) * totalSampleCount);
    }
    /* 24-bit PCM sample format */
    else if ((fmtChunk->format == WAV_FORMAT_PCM) && (fmtChunk->bitsPerSample == 24)) {
        const u8 *src = reinterpret_cast<const u8 *>(dataChunk->data);
        for (u32 i = 0; i < totalSampleCount; i++) {
            s32 sample = ((s32)(src[i * 3 + 0]) << 8)  | 
                         ((s32)(src[i * 3 + 1]) << 16) | 
                         ((s32)(src[i * 3 + 2]) << 24);
            sample >>= 8;

            if (sample > 32767) {
                sample = 32767;
            }
            else if (sample < -32768) {
                sample = -32768;
            }
            mSampleData[i] = static_cast<s16>(sample);
        }
    }
    else {
        Panic("WAVSound ctor: no resampling condition met");
    }
}

WAVSound::WAVSound(const s16 *sampleData, u32 sampleCount, u16 channelCount, u32 sampleRate) {
    u32 sampleDataSize = sampleCount * sizeof(s16) * channelCount;
    mSampleData = new s16[sampleDataSize / sizeof(s16)];
    memcpy(mSampleData, sampleData, sampleDataSize);

    mSampleCount = sampleCount;
    mChannelCount = channelCount;
    mSampleRate = sampleRate;
}

WAVSound::~WAVSound(void) {
    if (mSampleData != NULL) {
        delete[] mSampleData;
        mSampleData = NULL;
    }
}

WAVSound &WAVSound::operator=(WAVSound &&other) {
    if (this != &other) {
        if (mSampleData != NULL) {
            delete[] mSampleData;
        }

        mSampleRate = other.mSampleRate;
        mSampleCount = other.mSampleCount;
        mChannelCount = other.mChannelCount;
        mSampleData = other.mSampleData;

        other.mSampleRate = 0;
        other.mSampleCount = 0;
        other.mChannelCount = 0;
        other.mSampleData = NULL;
    }
    return *this;
}

Buffer WAVSound::build(void) {
    u32 sampleDataSize = sizeof(s16) * mSampleCount * mChannelCount;

    u32 fileSize = sizeof(WavFileHeader);

    u32 fmtChunkOff = fileSize;
    fileSize += sizeof(WavChunkHeader) + sizeof(WavFmtChunk);

    u32 dataChunkOff = fileSize;
    fileSize += sizeof(WavChunkHeader) + sampleDataSize;

    Buffer buffer (fileSize);

    WavFileHeader *fileHeader = buffer.data<WavFileHeader>();

    fileHeader->riffChunkType = WAV_CHUNK_RIFF;
    fileHeader->riffChunkSize = fileSize - 8;

    fileHeader->waveMagic = WAVE_MAGIC;

    WavChunkHeader *chunkHeader = buffer.data<WavChunkHeader>(fmtChunkOff);
    chunkHeader->type = WAV_CHUNK_FMT;
    chunkHeader->size = sizeof(WavFmtChunk);

    WavFmtChunk *fmtChunk = buffer.data<WavFmtChunk>(fmtChunkOff + sizeof(WavChunkHeader));
    fmtChunk->format = WAV_FORMAT_PCM;
    fmtChunk->channelCount = mChannelCount;
    fmtChunk->sampleRate = mSampleRate;
    fmtChunk->dataRate = sizeof(s16) * mSampleRate * mChannelCount;
    fmtChunk->blockSize = sizeof(s16) * mChannelCount;
    fmtChunk->bitsPerSample = sizeof(s16) * 8;

    chunkHeader = buffer.data<WavChunkHeader>(dataChunkOff);
    chunkHeader->type = WAV_CHUNK_DATA;
    chunkHeader->size = sampleDataSize;

    void *dataChunk = buffer.data<WavChunkHeader>(dataChunkOff + sizeof(WavChunkHeader));

    memcpy(dataChunk, mSampleData, sampleDataSize);

    return buffer;
}
