#include "mem_init.h"

Memory memory;

template <typename T>
static constexpr auto relativeToAbsolute( int* address ) noexcept
{
	return reinterpret_cast< T >(reinterpret_cast< char* >(address + 1) + *address);
}

#define FIND_PATTERN(type, ...) \
reinterpret_cast<type>(findPattern(__VA_ARGS__))

void Memory::initialize() noexcept {
	itemSchema = relativeToAbsolute<decltype(itemSchema)>( FIND_PATTERN( int*, "client", "\xE8????\x0F\xB7\x0F", 1 ) );
}