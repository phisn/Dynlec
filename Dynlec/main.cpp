#include "Dynlec.h"
#include "DynlecQuick.h"

#include <iostream>
#include <vector>

/*
class Volume
{
public:
	static std::vector<Volume> Volumes()
	{
		std::vector<Volume> volumes;

		volumes.emplace_back();
		Volume& volume = volumes.back();

		HANDLE handle = FindFirstVolumeA(
			volume.path,
			MAX_PATH);

		while (true)
		{
			// FindNextVolumeW(
			//	handle,);
		}

		FindVolumeClose(handle);

		return volumes;
	}

	Volume()
	{
	}

private:
	char path[MAX_PATH];
};
*/

namespace Dynlec
{
	DYNLEC_QUICK_LIBR(User32Library, "User32.dll");
	DYNLEC_QUICK_FUNC(
		MessageBoxA,
		User32Library,
		"MessageBoxA",
		int(__stdcall*)(HWND, LPCSTR, LPCSTR, UINT));
}

int main()
{
	DL::Call<DL::MessageBoxA>((HWND) NULL, "Hello world", "Title", MB_OK);
}
