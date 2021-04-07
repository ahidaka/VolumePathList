#pragma once

#include <Windows.h>
#include <stdio.h>

#define BUFFER_LENGTH ((MAX_PATH + 1) * sizeof(WCHAR))
#define NUM_DRIVE (26)

class VolumePathList {

public:
	VolumePathList();
	~VolumePathList();

	PWSTR GetPath(_Out_ PCWSTR Letter);

private:
	HRESULT Initialize();
	HRESULT GetVolumePaths(
		_Out_ PWSTR Path,
		_In_ PWCHAR VolumeName
	);

	PWSTR  LetterBuffer = NULL;
	PWSTR  DriveList = NULL; // [NUM_DRIVE][BUFFER_LENGTH];
};
