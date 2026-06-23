#include "File.hpp"

#include <cstdio>

#include <Error.hpp>

Buffer fileReadData(const char *path) {
    if (path == NULL) {
        Panic("fileReadData: path is NULL");
    }

    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        return Buffer ();
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return Buffer ();
    }

    size_t fileSize = static_cast<size_t>(ftell(fp));
    if (fileSize == (size_t)-1L) {
        fclose(fp);
        return Buffer ();
    }
    
    Buffer buffer = Buffer (fileSize);

    rewind(fp);
    size_t bytesCopied = fread(buffer.data(), 1, fileSize, fp);
    if ((bytesCopied < fileSize) && (ferror(fp) != 0)) {
        fclose(fp);
        return Buffer ();
    }

    fclose(fp);
    return buffer;
}

bool fileWriteData(const char *path, const Buffer &buffer) {
    if (path == NULL) {
        Panic("fileWriteData: path is NULL");
    }
    if (!buffer.check()) {
        Panic("fileWriteData: buffer is invalid");
    }

    FILE *fp = fopen(path, "wb");
    if (fp == NULL) {
        return false;
    }

    size_t bytesCopied = fwrite(buffer.data(), 1, buffer.get_size(), fp);
    if ((bytesCopied < buffer.get_size()) && (ferror(fp) != 0)) {
        fclose(fp);
        return false;
    }

    fclose(fp);
    return true;
}
