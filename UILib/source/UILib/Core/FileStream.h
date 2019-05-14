
class ReadStreamFile : public IStream
{
public:
	ReadStreamFile(const char* FileName);
	virtual ~ReadStreamFile();

	int Seek(int Offset, int Origin) override;
	int Tell() override;
	int Size() override;

	int Read(int NumBytes, void* Data) override;

protected:
	void* m_pFile;
};

#if WIN32

class WriteStreamFile : public IWriteStream
{
public:
	WriteStreamFile(const char* FileName);
	virtual ~WriteStreamFile();
	
	int Write(int NumBytes, const void* Data) override;

protected:
	void* m_pFile;
};

#endif