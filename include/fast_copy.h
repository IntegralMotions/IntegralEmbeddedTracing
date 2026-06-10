#pragma once
#include <cstdint>
#include <cstring>

#if defined(__arm__)
static inline __attribute__((always_inline)) void fast_copy(uint8_t* dest, const uint8_t* src, uint32_t size) {
    switch(size) {
    case 1: {
        uint32_t tmp;
        __asm__ volatile ("ldrb %0, [%1]" : "=r"(tmp) : "r"(src));
        __asm__ volatile ("strb %0, [%1]" : : "r"(tmp), "r"(dest) : "memory");
        break;
    }
    case 2: {
        uint32_t tmp;
        __asm__ volatile ("ldrh %0, [%1]" : "=r"(tmp) : "r"(src));
        __asm__ volatile ("strh %0, [%1]" : : "r"(tmp), "r"(dest) : "memory");
        break;
    }
    case 4: {
        uint32_t tmp;
        __asm__ volatile ("ldr %0, [%1]" : "=r"(tmp) : "r"(src));
        __asm__ volatile ("str %0, [%1]" : : "r"(tmp), "r"(dest) : "memory");
        break;
    }
    case 8: {
        uint32_t t1, t2;
        __asm__ volatile ("ldr %0, [%1]" : "=r"(t1) : "r"(src));
        __asm__ volatile ("ldr %0, [%1, #4]" : "=r"(t2) : "r"(src));
        __asm__ volatile ("str %0, [%1]" : : "r"(t1), "r"(dest) : "memory");
        __asm__ volatile ("str %0, [%1, #4]" : : "r"(t2), "r"(dest) : "memory");
        break;
    }
    case 16: {
        uint32_t t1, t2, t3, t4;
        __asm__ volatile ("ldr %0, [%1]"     : "=r"(t1) : "r"(src));
        __asm__ volatile ("ldr %0, [%1, #4]"  : "=r"(t2) : "r"(src));
        __asm__ volatile ("ldr %0, [%1, #8]"  : "=r"(t3) : "r"(src));
        __asm__ volatile ("ldr %0, [%1, #12]" : "=r"(t4) : "r"(src));
        __asm__ volatile ("str %0, [%1]"     : : "r"(t1), "r"(dest) : "memory");
        __asm__ volatile ("str %0, [%1, #4]"  : : "r"(t2), "r"(dest) : "memory");
        __asm__ volatile ("str %0, [%1, #8]"  : : "r"(t3), "r"(dest) : "memory");
        __asm__ volatile ("str %0, [%1, #12]" : : "r"(t4), "r"(dest) : "memory");
        break;
    }
    default:
        memcpy(dest, src, size);
        break;
    }
}
#else
static inline void fast_copy(uint8_t* dest, const uint8_t* src, uint32_t size) {
    switch(size) {
    case 1:
        *dest = *src;
        break;
    case 2:
        *reinterpret_cast<uint16_t*>(dest) = *reinterpret_cast<const uint16_t*>(src);
        break;
    case 4:
        *reinterpret_cast<uint32_t*>(dest) = *reinterpret_cast<const uint32_t*>(src);
        break;
    case 8:
        *reinterpret_cast<uint64_t*>(dest) = *reinterpret_cast<const uint64_t*>(src);
        break;
    case 16: {
        auto s = reinterpret_cast<const uint64_t*>(src);
        auto d = reinterpret_cast<uint64_t*>(dest);
        d[0] = s[0];
        d[1] = s[1];
        break;
    }
    default:
        memcpy(dest, src, size);
    }
}
#endif
