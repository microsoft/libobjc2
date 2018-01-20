#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
#define LIBOBJC2_POINTER_WIDTH __SIZEOF_POINTER__
#define LIBOBJC2_LONG_WIDTH __SIZEOF_LONG__
#elif defined(__LP64__)
#define LIBOBJC2_POINTER_WIDTH 8
#define LIBOBJC2_LONG_WIDTH 8
#else // defined(__LP64__)
#define LIBOBJC2_POINTER_WIDTH 4
#define LIBOBJC2_LONG_WIDTH 4
#endif // SIZEOF_POINTER/LONG, __LP64__

#if LIBOBJC2_POINTER_WIDTH == 8 && LIBOBJC2_LONG_WIDTH == 8
// 64-bit (non-windows)
#define DTABLE_OFFSET  64
#define SMALLOBJ_BITS  3
#define SHIFT_OFFSET   0
#define DATA_OFFSET    8
#define SLOT_OFFSET    32
#elif LIBOBJC2_POINTER_WIDTH == 8 && LIBOBJC2_LONG_WIDTH == 4
// 64-bit (windows)
#define DTABLE_OFFSET  56
#define SMALLOBJ_BITS  3
#define SHIFT_OFFSET   0
#define DATA_OFFSET    8
#define SLOT_OFFSET    32
#else
// 32-bit (all other targets)
#define DTABLE_OFFSET  32
#define SMALLOBJ_BITS  1
#define SHIFT_OFFSET   0
#define DATA_OFFSET    8
#define SLOT_OFFSET    16
#endif
#define SMALLOBJ_MASK  ((1<<SMALLOBJ_BITS) - 1)
