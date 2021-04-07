#include <windows.h>
#include <stdio.h>
#include "VolumePathList.h"

VolumePathList::VolumePathList()
{
    (VOID) Initialize();
}

VolumePathList::~VolumePathList()
{
    ;
}

PWSTR VolumePathList::GetPath(_Out_ PCWSTR Letter)
{
    INT index = Letter[0] - L'A';

    if (Letter == NULL) {
        return NULL;
    }

    return (index >= 0 && index < NUM_DRIVE ?
        &DriveList[index * BUFFER_LENGTH] : NULL);
}

HRESULT VolumePathList::GetVolumePaths(
    _Out_ PWSTR Path,
    _In_ PWCHAR VolumeName
)
{
    DWORD  CharCount = MAX_PATH + 1;
    PWCHAR Names = NULL;
    PWCHAR NameIdx = NULL;
    BOOL   Success = FALSE;
    HRESULT result = S_OK;

    if (!Path) {
        return ERROR_INVALID_PARAMETER;
    }

    while (TRUE) {
        Names = (PWCHAR) new BYTE[BUFFER_LENGTH];

        if (!Names) {
        
            printf("ERROR_OUTOFMEMORY\n");
            return ERROR_OUTOFMEMORY;
        }

        Success = GetVolumePathNamesForVolumeNameW(
            VolumeName, Names, CharCount, &CharCount
        );

        if (Success) {
            break;
        }

        if (GetLastError() != ERROR_MORE_DATA) {
            break;
        }

        //  Try again with the
        //  new suggested size.
        delete[] Names;
        Names = NULL;
    }

    if (Success) {
        //  Display the various paths.
        for (NameIdx = Names;
            NameIdx[0] != L'\0';
            NameIdx += wcslen(NameIdx) + 1) {
            //wprintf(L"  %s", NameIdx);

            wcscpy_s(Path, BUFFER_LENGTH / 2, NameIdx);

        }
    }

    if (Names != NULL) {

        delete[] Names;
        Names = NULL;
    }

    return result;
}

HRESULT VolumePathList::Initialize()
{
    DWORD  CharCount = 0;
    WCHAR  DeviceName[MAX_PATH] = L"";
    DWORD  Error = ERROR_SUCCESS;
    HANDLE FindHandle = INVALID_HANDLE_VALUE;
    BOOL   Found = FALSE;
    size_t Index = 0;
    BOOL   Success = FALSE;
    WCHAR  VolumeName[MAX_PATH] = L"";

    FindHandle = FindFirstVolumeW(VolumeName, ARRAYSIZE(VolumeName));

    if (FindHandle == INVALID_HANDLE_VALUE) {
        Error = GetLastError();
        wprintf(L"FindFirstVolumeW failed with error code %d\n", Error);
        return 1;
    }

    LetterBuffer = (PWSTR)HeapAlloc(
        GetProcessHeap(),
        0, //HEAP_ZERO_MEMORY,
        BUFFER_LENGTH
    );
    if (LetterBuffer == NULL) {
        Error = GetLastError();
        wprintf(L"Cannot allocate LetterBuffer, size=%llu err=%lu\n", BUFFER_LENGTH, Error);
        return ERROR_OUTOFMEMORY;
    }

    DriveList = (PWSTR)HeapAlloc(
            GetProcessHeap(),
            HEAP_ZERO_MEMORY,
            (NUM_DRIVE * BUFFER_LENGTH)
    );
    if (DriveList == NULL) {
        Error = GetLastError();
        wprintf(L"Cannot allocate LetterBuffer, size=%llu err=%lu\n",
            (NUM_DRIVE * BUFFER_LENGTH), Error);

        return ERROR_OUTOFMEMORY;
    }

    while (TRUE) {
        //  Skip the \\?\ prefix and remove the trailing backslash.
        Index = wcslen(VolumeName) - 1;

        if (VolumeName[0] != L'\\' ||
            VolumeName[1] != L'\\' ||
            VolumeName[2] != L'?' ||
            VolumeName[3] != L'\\' ||
            VolumeName[Index] != L'\\') {
        
            Error = ERROR_BAD_PATHNAME;
            wprintf(L"FindFirstVolumeW/FindNextVolumeW returned a bad path: %s\n", VolumeName);
            break;
        }

        //
        //  QueryDosDeviceW does not allow a trailing backslash,
        //  so temporarily remove it.
        VolumeName[Index] = L'\0';

        CharCount = QueryDosDeviceW(&VolumeName[4], DeviceName, ARRAYSIZE(DeviceName));

        VolumeName[Index] = L'\\';

        if (CharCount == 0) {
        
            Error = GetLastError();
            wprintf(L"QueryDosDeviceW failed with error code %d\n", Error);
            break;
        }

        wprintf(L"%s\n", DeviceName);
        //wprintf(L"\nVolume name: %s", VolumeName);
        //wprintf(L"Paths:");

        LetterBuffer[0] = L'\0';
        GetVolumePaths(LetterBuffer, VolumeName);
        wprintf(L"Paths:<%ws>\n", LetterBuffer);

        do {
            INT index = LetterBuffer[0] - L'A';

            if (index >= 0) {
                wcscpy_s(&DriveList[index * BUFFER_LENGTH], BUFFER_LENGTH / 2, DeviceName);
                wprintf(L"**%d %ws<%ws>\n", index, LetterBuffer, DeviceName);
            }
        }
        while (0);

        //
        //  Move on to the next volume.
        Success = FindNextVolumeW(FindHandle, VolumeName, ARRAYSIZE(VolumeName));

        if (!Success) {
        
            Error = GetLastError();

            if (Error != ERROR_NO_MORE_FILES) {
            
                wprintf(L"FindNextVolumeW failed with error code %d\n", Error);
                break;
            }

            Error = ERROR_SUCCESS;
            break;
        }
    }

    FindVolumeClose(FindHandle);
    FindHandle = INVALID_HANDLE_VALUE;

    return S_OK;
}
