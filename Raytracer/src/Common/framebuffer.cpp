#include "stdafx.h"
#include "framebuffer.h"

FrameBufferTexture::FrameBufferTexture()
	:	Width(0),
		Height(0),
		Mips(0),
		NumSamples(1),
		TextureTarget(GL_TEXTURE_2D),
		Format(0),
		InternalFormat(0),
		TextureObject(0)
{
}

FrameBufferTexture::~FrameBufferTexture()
{
	//Release();
}

void FrameBufferTexture::Bind(uint32_t TexUnit, bool bCompare)
{
	glActiveTexture(GL_TEXTURE0 + TexUnit);
	glBindTexture(GetTextureTarget(), GetTextureObject());

	if(Format == GL_DEPTH_COMPONENT)
		glTexParameteri( GetTextureTarget(), GL_TEXTURE_COMPARE_MODE, bCompare ? GL_COMPARE_R_TO_TEXTURE : GL_NONE);
}

void FrameBufferTexture::BindImage(uint32_t TexUnit, bool bWriteOnly, int Level)
{
	glBindImageTexture(TexUnit, GetTextureObject(), Level, false, 0, bWriteOnly ? GL_WRITE_ONLY : GL_READ_WRITE, InternalFormat);
}

void FrameBufferTexture::Release()
{
	if (TextureObject != 0)
		glDeleteTextures(1, &TextureObject);
}

void FrameBufferTexture::_Refresh()
{
	if (TextureObject != 0)
		glDeleteTextures(1, &TextureObject);
	TextureObject = 0;

	if (Width == 0 || Height == 0)
		return;
	if (Format == 0 || InternalFormat == 0)
		return;

	glGenTextures(1, &TextureObject);
	glBindTexture(TextureTarget, TextureObject);

	if(TextureTargetView == GL_TEXTURE_2D || TextureTargetView == GL_TEXTURE_2D_MULTISAMPLE)
	{
		if(NumSamples > 1)
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NumSamples, InternalFormat, Width, Height, true);
		else
			glTexStorage2D(GL_TEXTURE_2D, (GLsizei)Mips, InternalFormat, Width, Height);

		TextureObjectView = TextureObject;
	}
	else if(TextureTargetView == GL_TEXTURE_CUBE_MAP)
	{
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, (GLsizei)Mips, InternalFormat, Width, Height, 6);

		glGenTextures(1, &TextureObjectView);
		glTextureView(TextureObjectView, TextureTargetView, TextureObject, InternalFormat, 0, Mips, 0, 6);
	}

	if(Format == GL_DEPTH_COMPONENT)
	{
		glTexParameteri( GetTextureTarget(), GL_TEXTURE_MAX_LEVEL, 0 );
		glTexParameteri( GetTextureTarget(), GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GetTextureTarget(), GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GetTextureTarget(), GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);
		glTexParameteri( GetTextureTarget(), GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTexParameteri( GetTextureTarget(), GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
	}
	else
	{
		glTexParameteri(GetTextureTarget(), GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glTexParameteri(GetTextureTarget(), GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri(GetTextureTarget(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GetTextureTarget(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GetTextureTarget(), GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}
}

void FrameBufferTexture::SetFilters(const GLenum MinFilter, const GLenum MagFilter)
{
	Bind(0);

	glTexParameteri(GetTextureTarget(), GL_TEXTURE_MIN_FILTER, MinFilter );
	glTexParameteri(GetTextureTarget(), GL_TEXTURE_MAG_FILTER, MagFilter );

	glGetError();
}


void FrameBufferTexture::GenerateMipmaps()
{
	Bind(0);
	glGenerateMipmap(GetTextureTarget());
}

void FrameBufferTexture::Clear(glm::vec4 Color)
{
	glClearTexImage(GetTextureObject(), 0, Format, GL_FLOAT, &Color);
}


bool FrameBufferTexture::_IsValid() const
{
	return TextureObject != 0;
}

/***/
FrameBuffer::FrameBuffer()
	:	ViewportWidth(0),
		ViewportHeight(0),
		FramebufferObject(0),
		DepthBuffer(0)
{
	for(size_t i = 0; i < sizeof(ColorBuffers)/sizeof(ColorBuffers[0]); ColorBuffers[i++]=0);
	for(size_t i = 0; i < sizeof(DrawBuffers)/sizeof(DrawBuffers[0]); DrawBuffers[i++]=0);
}

FrameBuffer::~FrameBuffer()
{
	Release();
}

void FrameBuffer::Bind()
{
	BindLevel(0);
}

void FrameBuffer::BindLevel(uint32_t Level)
{
	if (!_IsValid())
	{
		printf("Framebuffer::Bind error: invalid framebuffer\n");
		return;
	}

	if(FramebufferObject.size() <= Level)
		return;

	glViewport(0,0, ViewportWidth >> Level, ViewportHeight >> Level);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FramebufferObject[Level]);

	int err =	glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if(err != GL_FRAMEBUFFER_COMPLETE)
		printf("glCheckFramebufferStatus Error %x\n", err);

	glDrawBuffers(NumDrawBuffers, DrawBuffers);
}

void FrameBuffer::BindRead(uint32_t Level)
{
	if (!_IsValid())
	{
		printf("Framebuffer::Bind error: invalid framebuffer\n");
		return;
	}

	if(FramebufferObject.size() <= Level)
		return;

	glBindFramebuffer(GL_READ_FRAMEBUFFER, FramebufferObject[Level]);
}

void FrameBuffer::BindBackBuffer(size_t Width, size_t Height)
{
	glViewport(0, 0, Width, Height);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	//GLuint BackBufferDrawBuffers[] { GL_COLOR_ATTACHMENT0 };
	//glDrawBuffers(1, BackBufferDrawBuffers);
}

void FrameBuffer::Blit(const GLuint Width, const GLuint Height, const GLuint X, const GLuint Y)
{
	glBlitFramebuffer(X, Y, Width, Height, X, Y, Width, Height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

void FrameBuffer::Create(uint32_t Levels)
{
	Release();

	if(FramebufferObject.size() <= Levels)
	{
		FramebufferObject.resize(Levels);
		glGenFramebuffers(Levels, &FramebufferObject[0]);
	}
}

void FrameBuffer::Release()
{
	for(auto it : FramebufferObject)
		glDeleteFramebuffers(1, &it);
	FramebufferObject.empty();
}

void FrameBuffer::SetBuffer(const Type Buffer, const FrameBufferTexture* Texture)
{
	GLenum AttachmentType;

	if(Texture != nullptr)
	{
		if(FramebufferObject.size() == 0)
			Create(Texture->Mips);

		ViewportWidth = Texture->Width;
		ViewportHeight = Texture->Height;
	}

	if (Buffer == Type::DEPTH)
	{
		AttachmentType = GL_DEPTH_ATTACHMENT;
		DepthBuffer = AttachmentType;
	}
	else
	{
		AttachmentType = GL_COLOR_ATTACHMENT0 + ((int)Buffer - (int)Type::COLOR0);
		ColorBuffers[((int)Buffer - (int)Type::COLOR0)] = AttachmentType;
	}

	for(size_t i = 0; i < FramebufferObject.size(); i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferObject[i]);

		if(Texture == nullptr)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, AttachmentType, GL_TEXTURE_2D, 0, i);
		}
		else if(Texture->GetTextureTarget() == GL_TEXTURE_CUBE_MAP)
		{
//			glFramebufferTexture2D(GL_FRAMEBUFFER, AttachmentType, GL_TEXTURE_CUBE_MAP_POSITIVE_X, Texture->GetTextureObject(), i);
			for(int layer = 0; layer < 6; layer++)
				glFramebufferTextureLayer(GL_FRAMEBUFFER, AttachmentType, Texture->GetTextureObject(true), i, layer);
		}
		else
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, AttachmentType, Texture->GetTextureTarget(true), Texture->GetTextureObject(true), i);
		}
	}

	NumDrawBuffers = 0;
	for(int i = 0; i < 8; i++)
	{
		DrawBuffers[i] = 0;

		if(ColorBuffers[i] != 0)
			DrawBuffers[NumDrawBuffers++] = GL_COLOR_ATTACHMENT0+i;
	}
}

bool FrameBuffer::_IsValid() const
{
	return FramebufferObject.size() > 0;
}
