#pragma once

class FrameBufferTexture
{
public:
	FrameBufferTexture();
	FrameBufferTexture(const FrameBufferTexture&) = delete;
	~FrameBufferTexture();

	GLuint GetTextureObject(bool bFrameBuffer = false) const { return bFrameBuffer ? TextureObject : TextureObjectView; }
	GLuint GetTextureTarget(bool bFrameBuffer = false) const { return bFrameBuffer ? TextureTarget : TextureTargetView; }

	void Bind(uint32_t TexUnit, bool bCompare = false);
	void BindImage(uint32_t TexUnit, bool bWriteOnly = true, int Level = 0);

	void SetSize(const GLuint InWidth, const GLuint InHeight, const GLuint InNumSamples = 1) 
	{
		Width = InWidth; 
		Height = InHeight; 
		Mips = NumSamples > 1 ? 1 : (GLuint) std::ceilf( std::log2f( std::fmaxf( (float)InWidth, (float)InHeight) ) );
		NumSamples = InNumSamples; 
		TextureTarget = NumSamples > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		TextureTargetView = TextureTarget;
		_Refresh(); 
	}
	
	void SetCubeSize(const GLuint InSize)
	{
		Width = InSize; 
		Height = InSize; 
		NumSamples = 1; 
		Mips = (GLuint) std::ceilf( std::log2f( (float)InSize ) );
		TextureTarget = GL_TEXTURE_2D_ARRAY;
		TextureTargetView = GL_TEXTURE_CUBE_MAP;
		_Refresh(); 
	}

	void SetFormat(const GLenum InFormat, const GLenum InInternalFormat) 
	{
		Format = InFormat; 
		InternalFormat = InInternalFormat; 
		_Refresh(); 
	}

	void SetFilters(const GLenum MinFilter, const GLenum MagFilter);

	void GenerateMipmaps();
	void Clear(glm::vec4 Color);

	void Release();

private:

	void _Refresh();
	bool _IsValid() const;

	GLuint Width, Height, Mips;
	GLuint NumSamples;
	GLenum TextureTarget, TextureTargetView;
	GLenum Format, InternalFormat;

	GLuint TextureObject, TextureObjectView;

	friend class FrameBuffer;
};

class FrameBuffer
{
public:
	enum class Type
	{
		COLOR0 = 0,
		COLOR1,
		COLOR2,
		COLOR3,
		COLOR4,
		COLOR5,
		COLOR6,
		COLOR7,
		DEPTH,
	};

	FrameBuffer();
	~FrameBuffer();

	static void BindBackBuffer(size_t Width, size_t Height);
	void Bind();
	void BindRead(uint32_t Level = 0);

	int GetFBO() const { return FramebufferObject.size() > 0 ? FramebufferObject[0] : 0; }

	void SetBuffer(const Type Buffer, const FrameBufferTexture* Texture);


	void GetViewportSize(GLuint* Width, GLuint* Height) const { *Width = ViewportWidth; *Height = ViewportHeight; }

	static void Blit(const GLuint Width, const GLuint Height, const GLuint X = 0, const GLuint Y = 0);

private:
	void BindLevel(uint32_t Level);

	void Create(uint32_t Levels);
	void Release();

	bool _IsValid() const;

	GLuint ViewportWidth, ViewportHeight;

	std::vector<GLuint> FramebufferObject;
	GLuint ColorBuffers[8];
	GLuint DepthBuffer;

	GLuint DrawBuffers[8];
	GLuint NumDrawBuffers = 0;
};
