#define _CRT_SECURE_NO_WARNINGS 1

#include <IStreams.h>

#include "stdio.h"
#include "FileStream.h"

const int IStream::SeekSet = SEEK_SET, IStream::SeekCur = SEEK_CUR, IStream::SeekEnd = SEEK_END;

///////////////////////////////////////////////////////
///ReadStreamFile
ReadStreamFile::ReadStreamFile(const char* FileName)
	: IStream()
{
	m_pFile = (void*)fopen(FileName, "rb");
}

ReadStreamFile::~ReadStreamFile()
{
	if(m_pFile)
		fclose((FILE*)m_pFile);
}

int ReadStreamFile::Seek(int Offset, int Origin)
{
	if(m_pFile)
		return fseek((FILE*)m_pFile, Offset, Origin);

	return -1;
}

int ReadStreamFile::Tell()
{
	if(m_pFile)
		return ftell((FILE*)m_pFile);

	return -1;
}

int ReadStreamFile::Size()
{
	if(m_pFile)
	{
		int Cur = ftell((FILE*)m_pFile);
		fseek((FILE*)m_pFile, 0, SEEK_END);
		int Size = ftell((FILE*)m_pFile);
		fseek((FILE*)m_pFile, Cur, SEEK_SET);

		return Size;
	}

	return -1;
}

int ReadStreamFile::Read(int NumBytes, void* Data)
{
	if(m_pFile)
		return fread(Data, 1, NumBytes, (FILE*)m_pFile);

	return -1;
}

#ifdef WIN32

///////////////////////////////////////////////////////
///WriteStreamFile
WriteStreamFile::WriteStreamFile(const char* FileName)
	: IWriteStream()
{
	m_pFile = (void*)fopen(FileName, "wb");
}

WriteStreamFile::~WriteStreamFile()
{
	if(m_pFile)
		fclose((FILE*)m_pFile);
}

int WriteStreamFile::Write(int NumBytes, const void* Data)
{
	if(m_pFile)
		return fwrite(Data, 1, NumBytes, (FILE*)m_pFile);

	return -1;
}

#endif