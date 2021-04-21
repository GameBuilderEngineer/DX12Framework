#include "PMDActor.h"

void* PMDActor::Transform::operator new(size_t size)
{
	return _aligned_malloc(size, 16);
}