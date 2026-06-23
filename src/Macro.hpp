#ifndef MACRO_HPP
#define MACRO_HPP

#include <Type.hpp> // IWYU pragma: keep

#include <bit> // IWYU pragma: keep

// Array length

#define ARRAY_LENGTH(array) ( (size_t)(sizeof((array)) / sizeof((array)[0])) )

// Alignment

#define ALIGN_DOWN_2(value) ( (value) & ~(2 - 1) )
#define ALIGN_UP_2(value)   ( ((value) + 2 - 1) & ~(2 - 1) )

#define ALIGN_DOWN_4(value) ( (value) & ~(4 - 1) )
#define ALIGN_UP_4(value)   ( ((value) + 4 - 1) & ~(4 - 1) )

#define ALIGN_DOWN_8(value) ( (value) & ~(8 - 1) )
#define ALIGN_UP_8(value)   ( ((value) + 8 - 1) & ~(8 - 1) )

#define ALIGN_DOWN_16(value) ( (value) & ~(16 - 1) )
#define ALIGN_UP_16(value)   ( ((value) + 16 - 1) & ~(16 - 1) )

#define ALIGN_DOWN_32(value) ( (value) & ~(32 - 1) )
#define ALIGN_UP_32(value)   ( ((value) + 32 - 1) & ~(32 - 1) )

#define ALIGN_DOWN_64(value) ( (value) & ~(64 - 1) )
#define ALIGN_UP_64(value)   ( ((value) + 64 - 1) & ~(64 - 1) )

// Byteswap

#define BYTESWAP_64 __builtin_bswap64
#define BYTESWAP_32 __builtin_bswap32
#define BYTESWAP_16 __builtin_bswap16

#define BYTESWAP_FLOAT(x) (                         \
    std::bit_cast<f32, u32>(                        \
        BYTESWAP_32(std::bit_cast<u32, f32>((x)))   \
    )                                               \
)

// Maxmin

#define MAX(a, b) ( (a) > (b) ? (a) : (b) )
#define MIN(a, b) ( (a) < (b) ? (a) : (b) )

// Literal string & array length

#define ARR_LIT_LEN(larray) ( \
    sizeof(larray) / sizeof((larray)[0]) + \
    0 * sizeof(char[1 - 2 * !!(sizeof(larray) / sizeof((larray)[0]) == 0)]) \
)

// No (const) char* please!!
#define STR_LIT_LEN(lstring) ( ARR_LIT_LEN(lstring) - 1 )

// Assertion

#define STRUCT_SIZE_ASSERT(struct, expectSize) \
    _Static_assert(sizeof(struct) == expectSize, "sizeof " #struct " is mismatched")

#endif // MACRO_HPP
