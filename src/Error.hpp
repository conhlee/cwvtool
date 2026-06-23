#ifndef ERROR_HPP
#define ERROR_HPP

void Warn(const char *fmt, ...);
void Error(const char *fmt, ...);

void Panic(const char *fmt, ...) __attribute__((noreturn));

#endif // ERROR_HPP
