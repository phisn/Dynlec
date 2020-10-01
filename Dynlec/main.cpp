/*
#include "DynlecCaller.h"
#include "DynlecLibraries.h"

#include <iostream>
#include <vector>

class Volume
{
public:
	static std::vector<Volume> Volumes()
	{
		std::vector<Volume> volumes;

		HANDLE handle = Dynlec::Call<Dynlec::FindFirstVolumeA>(
			volumes.emplace_back().path,
			MAX_PATH);

		while (Dynlec::Call<Dynlec::FindNextVolumeA>(
			handle,
			volumes.emplace_back().path,
			MAX_PATH));

		volumes.pop_back();

#ifdef _DEBUG
		if (GetLastError() != ERROR_NO_MORE_FILES)
		{
			std::cout << "FindNextVolume failed: " << GetLastError() << std::endl;
		}
#endif

		Dynlec::Call<Dynlec::FindVolumeClose>(handle);

		return volumes;
	}

	Volume()
	{
	}

	const char* getPath() const
	{
		return path;
	}

private:
	char path[MAX_PATH];
};
#include <Windows.h>
*/
#include <iostream>
int main()
{
	/*
	std::vector<Volume> volumes = Volume::Volumes();

	for (Volume& volume : volumes)
		std::cout << "V: " << volume.getPath() << std::endl;

	TOKEN_PRIVILEGES privileges;
	HANDLE token;

	privileges.Privileges[0].Luid;
	*/
#define MACRO()

	int i = 0;
	MACRO(++i);
	std::cout << i << std::endl;
	/*
	BOOLEAN enabled;
	std::cout << "rtlap_r: " << std::hex << (unsigned long) Dynlec::Call<Dynlec::RtlAdjustPrivilege>(
		19,
		TRUE,
		FALSE,
		&enabled) << std::endl;
	std::cout << "ntss_r: " << std::hex << (unsigned long) Dynlec::Call<Dynlec::NtShutdownSystem>(
		Dynlec::ShutdownPowerOff) << std::endl;
		*/
}
