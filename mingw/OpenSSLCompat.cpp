/*
 * OpenSSL's prebuilt MinGW libraries were built for MSVCRT and import the
 * _timezone data symbol directly. UCRT exposes the same state through the
 * __timezone accessor, so provide the import-pointer symbol expected by the
 * static library when DC++ is built with a UCRT-based MinGW toolchain.
 */

#include <time.h>

#if defined(__MINGW32__) && defined(_UCRT)

#if defined(__x86_64__)
#define OPENSSL_TIMEZONE_IMPORT "__imp__timezone"
#elif defined(__i386__)
#define OPENSSL_TIMEZONE_IMPORT "__imp___timezone"
#endif

#ifdef OPENSSL_TIMEZONE_IMPORT
extern "C" {
long* opensslTimezoneImport __asm__(OPENSSL_TIMEZONE_IMPORT) = __timezone();
}
#endif

#endif
