#ifndef DEBUG_HEADER
#define DEBUG_HEADER

#include <cstdio>

#ifdef DEBUG
#define DBG(...) { __VA_ARGS__ }
#define DBG_LOG(fmt, ...) printf("[Debug] " fmt "\n", __VA_ARGS__)

#else
#define DBG(...)
#define DBG_LOG(fmt, ...)
#endif

#endif
