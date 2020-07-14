#include <iostream>
#include <vector>

#include <Windows.h>

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

int main()
{
	HMODULE library = LoadLibraryA("lib");
	FARPROC proc = GetProcAddress(
		library,
		"");

}
