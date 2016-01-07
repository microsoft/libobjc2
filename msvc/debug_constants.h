#ifndef DBG_SKIP
// The line number 0xF00F00 (here in decimal) is a signal to the debugger that the following line should be skipped.
#define DBG_SKIP 15732480
#endif
#ifndef DBG_FOLLOW
// The line number 0xFEEFEE (in decimal below) signals to the debugger that the line should be skipped, but its jump targets should not be.
#define DBG_FOLLOW 16707566
#endif