#ifndef ORL_PACKED

#if defined __WATCOMC__
#define ORL_PACKED _Packed
#elif defined _MSC_VER
#define ORL_PACKED _declspec(align(1))
#elif defined __GNUC__
#define ORL_PACKED __attribute__((__packed__))
#else
#error NO PACKING
#define ORL_PACKED
#endif

#endif