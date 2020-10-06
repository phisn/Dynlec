#include "Dynlec/DynlecLoader.h"

class SomeTestLibrary
	:
	public Dynlec::Library
{
public:
	SomeTestLibrary()
		:
		Dynlec::Library(binary, sizeof(binary) / sizeof(*binary))
	{
	}

private:
	static constexpr char binary[] =
	{
		0x00
		// binary data
	};
};
