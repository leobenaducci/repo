
class IStream
{
public:
	static const int SeekSet, SeekCur, SeekEnd;

	virtual int Seek(int Offset, int Origin) = 0;
	virtual int Tell() = 0;
	virtual int Size() = 0;

	virtual int Read(int NumBytes, void* Data) = 0;
};

#ifdef WIN32

class IWriteStream
{
public:
	IWriteStream() {}
	virtual ~IWriteStream() {}

	virtual int Write(int NumBytes, const void* Data) = 0;
};

#endif