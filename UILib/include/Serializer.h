
class ISerializer
{
public:
	virtual void operator >> (signed char& val) = 0;
	virtual void operator >> (unsigned char& val) = 0;
	virtual void operator >> (signed short& val) = 0;
	virtual void operator >> (unsigned short& val) = 0;
	virtual void operator >> (signed int& val) = 0;
	virtual void operator >> (unsigned int& val) = 0;
	virtual void operator >> (float& val) = 0;
	virtual void operator >> (wchar_t* val) = 0;
	virtual void operator >> (char* val) = 0;
};

class SerializeFromStream : public ISerializer
{
public:
	SerializeFromStream(class IStream* ReadStream);
	~SerializeFromStream();

	void operator >> (signed char& val) override;
	void operator >> (unsigned char& val) override;
	void operator >> (signed short& val) override;
	void operator >> (unsigned short& val) override;
	void operator >> (signed int& val) override;
	void operator >> (unsigned int& val) override;
	void operator >> (float& val) override;
	void operator >> (wchar_t* val) override;
	void operator >> (char* val) override;

protected:
	class IStream* Stream;

private:
	SerializeFromStream();
};

#ifdef WIN32

class SerializeToStream : public ISerializer
{
public:
	SerializeToStream(class IWriteStream* WriteStream);
	~SerializeToStream();

	void operator >> (signed char& val) override;
	void operator >> (unsigned char& val) override;
	void operator >> (signed short& val) override;
	void operator >> (unsigned short& val) override;
	void operator >> (signed int& val) override;
	void operator >> (unsigned int& val) override;
	void operator >> (float& val) override;
	void operator >> (wchar_t* val) override;
	void operator >> (char* val) override;

protected:
	class IWriteStream* Stream;

private:
	SerializeToStream();
};

#endif