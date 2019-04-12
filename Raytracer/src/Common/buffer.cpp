#include "stdafx.h"
#include "buffer.h"

Buffer::Buffer(GLenum usage) : BufferUsage(usage), BufferId(0)
{
}

Buffer::~Buffer()
{
	if(BufferId != 0)
		glDeleteBuffers(1, &BufferId);
}

void Buffer::UploadData(const void* Data, size_t DataSize)
{
	if(BufferId == 0)
		glGenBuffers(1, &BufferId);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, BufferId);
	glBufferData(GL_SHADER_STORAGE_BUFFER, DataSize, Data, BufferUsage);
	glMakeBufferResidentNV(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	
	glGetError();
}

GLuint64EXT Buffer::GetBufferAddress() const
{
	if (BufferId == 0)
		return 0;
		
	GLuint64EXT BufferAddr;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, BufferId);
	glGetBufferParameterui64vNV(GL_SHADER_STORAGE_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &BufferAddr);

	return BufferAddr;
}

