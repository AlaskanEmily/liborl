#ifndef ORL_PACKED

#if defined __WATCOMC__
#define ORL_PACKED _Packed
#define ORL_PACKED_STRUCT ORL_PACKED struct
#elif defined _MSC_VER
#define ORL_PACKED _declspec(align(1))
#define ORL_PACKED_STRUCT ORL_PACKED struct
#elif defined __GNUC__
#define ORL_PACKED __attribute__((__packed__))
#define ORL_PACKED_STRUCT struct ORL_PACKED
#else
#error NO PACKING
#define ORL_PACKED
#define ORL_PACKED_STRUCT struct
#endif

#endif