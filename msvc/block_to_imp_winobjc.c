#include "objc/runtime.h"
#include "../visibility.h"

/* These are stub implementations of the block->imp->block API functions.
 * Until we can emit the block trampolines at build time (like iOS appears to),
 * we'll have to live without these functions.
 */

IMP imp_implementationWithBlock(void *block)
{
	return NULL;
}

void *imp_getBlock(IMP anImp)
{
	return NULL;
}

BOOL imp_removeBlock(IMP anImp)
{
	return NO;
}

char *block_copyIMPTypeEncoding_np(void*block)
{
	return NULL;
}

PRIVATE void init_trampolines(void)
{
}
