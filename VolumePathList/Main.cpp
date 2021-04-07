#include <Windows.h>
#include <stdio.h>
#include "VolumePathList.h"

int main()
{
	VolumePathList* vpList = new VolumePathList();

	wprintf(L"C:<%ws>", vpList->GetPath(L"C"));

	return 0;
}