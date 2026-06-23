#ifndef FILE_HPP
#define FILE_HPP

#include <Buffer.hpp>

/* Returns NULL buffer on failure. */
Buffer fileReadData(const char *path);

/* Returns true on success, false on failure. */
bool fileWriteData(const char *path, const Buffer &buffer);

#endif // FILE_HPP
