#include <stdio.h>
#include <stdlib.h>
#include "objc/runtime.h"

static void objc_enumeration_mutation_fallback(id object)
{
	fprintf(stderr, "Mutation occured during enumeration.");
	abort();
}

void (*_objc_enumeration_mutation)(id object) = &objc_enumeration_mutation_fallback;

// This function is exported as a weak symbol to enable GNUstep or some other
// framework to replace it trivially. On platforms with linkers that cannot handle weak exports,
// the objc_enumeration_mutation hook is the preferred override point.
void __attribute__((weak)) objc_enumerationMutation(id obj)
{
	_objc_enumeration_mutation(obj);
}

