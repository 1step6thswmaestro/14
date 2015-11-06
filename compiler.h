#if defined(__clang__)
#define PACKED __attribute__((packed))
#elif defined(__GNUC__)
#define PACKED __attribute__((packed))
#else
#define PACKED
#endif
