#include "Dynlec.h"
#include "DynlecLibraries.h"

#include <iostream>
#include <vector>

class Volume
{
public:
	static std::vector<Volume> Volumes()
	{
		std::vector<Volume> volumes;

		HANDLE handle = DL::Call<DL::FindFirstVolumeA>(
			volumes.emplace_back().path,
			MAX_PATH);

		while (DL::Call<DL::FindNextVolumeA>(
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

		DL::Call<DL::FindVolumeClose>(handle);

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

int main()
{
	std::vector<Volume> volumes = Volume::Volumes();

	for (Volume& volume : volumes)
		std::cout << "V: " << volume.getPath() << std::endl;
}
