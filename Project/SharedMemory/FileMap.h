#pragma once
#include <Windows.h>
#include <iostream>

struct FileMap
{
	const std::string name = "FILEMAP";
	const size_t size = 200 * 1024;

	HANDLE object;
	void* view = nullptr;

	FileMap()
	{
		object = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, (DWORD)0, (DWORD)size, name.c_str());

		if (object == NULL)
			return;

		view = MapViewOfFile(object, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	}

	~FileMap()
	{
		UnmapViewOfFile((LPCVOID)view);
		CloseHandle(object);
	}
};