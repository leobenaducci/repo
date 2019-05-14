#include <Serializer.h>
#include <IStreams.h>
#include <assert.h>

#ifdef BIG_ENDIAN
void swap_short(void* val)
{
	*(unsigned short*)val = (((*(unsigned short*)val) & 0x00ff) << 8) |
							(((*(unsigned short*)val) & 0xff00) >> 8);
}

void swap_int(void* val)
{
	*(unsigned int*)val =	(((*(unsigned int*)val) & 0x000000ff) << 24) |
							(((*(unsigned int*)val) & 0x0000ff00) << 8)  |
							(((*(unsigned int*)val) & 0x00ff0000) >> 8)  |
							(((*(unsigned int*)val) & 0xff000000) >> 24);
}

void swap_qword(void* val)
{
	*(unsigned long long*)val = (((*(unsigned long long*)val) & 0x00000000000000ff) << 56) |
								(((*(unsigned long long*)val) & 0x000000000000ff00) << 40) |
								(((*(unsigned long long*)val) & 0x0000000000ff0000) >> 24) |
								(((*(unsigned long long*)val) & 0x00000000ff000000) >> 8)  |
								(((*(unsigned long long*)val) & 0x000000ff00000000) << 8)  |
								(((*(unsigned long long*)val) & 0x0000ff0000000000) >> 24) |
								(((*(unsigned long long*)val) & 0x00ff000000000000) >> 40) |
								(((*(unsigned long long*)val) & 0xff00000000000000) << 56);
}

#endif

///////////////////////////////////
// SerializeFromStream
SerializeFromStream::SerializeFromStream(class IStream* ReadStream)
{
	assert(ReadStream);
	Stream = ReadStream;
}

SerializeFromStream::~SerializeFromStream()
{
}

void SerializeFromStream::operator >> (signed char& val)
{
	Stream->Read(1, &val);
}

void SerializeFromStream::operator >> (unsigned char& val)
{
	Stream->Read(1, &val);
}

void SerializeFromStream::operator >> (signed short& val)
{
	Stream->Read(2, &val);
	#ifdef BIG_ENDIAN
		swap_short(&val);
	#endif
}

void SerializeFromStream::operator >> (unsigned short& val)
{
	Stream->Read(2, &val);
	#ifdef BIG_ENDIAN
		swap_short(&val);
	#endif
}

void SerializeFromStream::operator >> (signed int& val)
{
	Stream->Read(4, &val);
	#ifdef BIG_ENDIAN
		swap_int(&val);
	#endif
}

void SerializeFromStream::operator >> (unsigned int& val)
{
	Stream->Read(4, &val);
	#ifdef BIG_ENDIAN
		swap_int(&val);
	#endif
}

void SerializeFromStream::operator >> (float& val)
{
	Stream->Read(4, &val);
	#ifdef BIG_ENDIAN
		swap_int(&val);
	#endif
}

void SerializeFromStream::operator >> (wchar_t* val)
{
	int i = 0;
	do { Stream->Read(2, &val[i]); i++; } while(val[i-1] != 0);
}

void SerializeFromStream::operator >> (char* val)
{
	int i = 0;
	do { Stream->Read(1, &val[i]); i++; } while(val[i-1] != 0);
}

#ifdef WIN32

///////////////////////////////////
// SerializeToStream
SerializeToStream::SerializeToStream(IWriteStream* WriteStream)
{
	assert(WriteStream);
	Stream = WriteStream;
}

SerializeToStream::~SerializeToStream()
{
}

void SerializeToStream::operator >> (signed char& val)
{	
	Stream->Write(1, &val);
}

void SerializeToStream::operator >> (unsigned char& val)
{
	Stream->Write(1, &val);
}

void SerializeToStream::operator >> (signed short& val)
{
	Stream->Write(2, &val);
}

void SerializeToStream::operator >> (unsigned short& val)
{
	Stream->Write(2, &val);
}

void SerializeToStream::operator >> (signed int& val)
{
	Stream->Write(4, &val);
}

void SerializeToStream::operator >> (unsigned int& val)
{
	Stream->Write(4, &val);
}

void SerializeToStream::operator >> (float& val)
{
	Stream->Write(4, &val);
}

void SerializeToStream::operator >> (wchar_t* val)
{
	int i = 0;
	do { Stream->Write(2, &val[i]); i++; } while(val[i-1] != 0);
}

void SerializeToStream::operator >> (char* val)
{
	int i = 0;
	do { Stream->Write(1, &val[i]); i++; } while(val[i-1] != 0);
}

#endif
