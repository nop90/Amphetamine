#include "File.hpp"

CFile::CFile()
{}

CFile::~CFile()
{}

boolVar	CFile::OpenDataFile(char *name)
{
	fileRef = fopen(name, "rb");
	return fileRef != NULL;
}


void	CFile::CloseDataFile()
{
	fclose(fileRef);
}

void	CFile::SetFilePos(long offset)
{
	fseek(fileRef, offset, SEEK_SET);
}

long	CFile::ReadData(void *data, long size)
{
	return fread(data, size, 1, fileRef);
}