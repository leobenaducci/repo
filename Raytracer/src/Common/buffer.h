#pragma once

class Buffer
{
public:
	Buffer(GLenum usage = GL_STATIC_READ);
	Buffer(const Buffer&) = delete;
	~Buffer();

	void UploadData(const void* Data, size_t DataSize);

	GLuint GetBufferId() const { return BufferId; }
	GLuint64EXT GetBufferAddress() const;

protected:
	GLuint BufferId;
	GLenum BufferUsage;
};
