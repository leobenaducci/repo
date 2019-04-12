#include "stdafx.h"
#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

Texture Texture::WhiteTexture;
Texture Texture::BlackTexture;
Texture Texture::IdentityNormalMapTexture;

void Texture::MakeRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	Width = 1;
	Height = 1;
	Mips = 1;
	TextureTarget = GL_TEXTURE_2D;
	Format = GL_RGBA;
	InternalFormat = GL_RGBA8;
		
	unsigned char data[4] = { r,g,b,a };
	glGenTextures(1, &TextureObject);

	Bind(0);
	glTexImage2D(TextureTarget, 0, InternalFormat, Width, Height, 0, Format, GL_UNSIGNED_BYTE, data);
	SetFilters(GL_POINT, GL_POINT);

	TextureHandle = glGetTextureHandleARB(TextureObject);
	glMakeTextureHandleResidentARB(TextureHandle);
}

Texture::Texture()
	: Width(0),
	Height(0),
	Mips(0),
	TextureTarget(GL_TEXTURE_2D),
	Format(0),
	InternalFormat(0),
	TextureObject(0),
	TextureHandle(0)
{
}

Texture::~Texture()
{
	Release();
}

void Texture::Bind(uint32_t TexUnit) const
{
	glActiveTexture(GL_TEXTURE0 + TexUnit);
	glBindTexture(TextureTarget, GetTextureObject());
	glGetError();
}

void Texture::Release()
{
	if(TextureHandle != 0)
		glMakeTextureHandleNonResidentARB(TextureHandle);
	TextureHandle = 0;
	if (TextureObject != 0)
		glDeleteTextures(1, &TextureObject);
	TextureObject = 0;
}

void Texture::SetFilters(const GLenum MinFilter, const GLenum MagFilter)
{
	Bind(0);
	glTexParameteri(TextureTarget, GL_TEXTURE_MIN_FILTER, MinFilter);
	glTexParameteri(TextureTarget, GL_TEXTURE_MAG_FILTER, MagFilter);

	glTexParameteri(TextureTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(TextureTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(TextureTarget, GL_TEXTURE_WRAP_R, GL_REPEAT);

	glTexParameteri(TextureTarget, GL_TEXTURE_MAX_LEVEL, Mips-1);
	glGetError();
}


void Texture::GenerateMipmaps()
{
	Bind(0);
	glGenerateMipmap(TextureTarget);
	Mips = glm::max((GLuint)std::log2(Width), (GLuint)std::log2(Height));
}

bool Texture::_IsValid() const
{
	return TextureObject != 0;
}

void Texture::Load(const std::string& fileName, Texture& NewTex)
{
	bool bHDR = stbi_is_hdr(fileName.c_str());

	int x,y,n;
	unsigned char *data = bHDR ? (unsigned char*)stbi_load_16(fileName.c_str(), &x, &y, &n, 4) : (unsigned char*)stbi_load(fileName.c_str(), &x, &y, &n, 4);
	n = 4;

	if (data == nullptr)
		return;

	NewTex.Release();
	NewTex.FileName = fileName;

	NewTex.TextureTarget = GL_TEXTURE_2D;
	NewTex.Width = x;
	NewTex.Height = y;
	NewTex.Format = n == 3 ? GL_RGB : GL_RGBA;
	NewTex.InternalFormat = bHDR ? (n == 3 ? GL_RGB32F : GL_RGBA32F) : (n == 3 ? GL_COMPRESSED_SRGB_S3TC_DXT1_EXT : GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT);

	glGenTextures(1, &NewTex.TextureObject);
	NewTex.Bind(0);

	glTexImage2D(NewTex.TextureTarget, 0, NewTex.InternalFormat, NewTex.Width, NewTex.Height, 0, NewTex.Format, bHDR ? GL_FLOAT : GL_UNSIGNED_BYTE, data);
	NewTex.GenerateMipmaps();
	NewTex.SetFilters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

	NewTex.TextureHandle = glGetTextureHandleARB(NewTex.TextureObject);
	glMakeTextureHandleResidentARB(NewTex.TextureHandle);

	stbi_image_free(data);

	glGetError();
}


void Texture::LoadCube(const std::string& folder, const std::string& ext, Texture& NewTex)
{
	const char* fileNames[] = { "posx", "negx", "posy", "negy", "posz", "negz" };
	int x, y, n;

	NewTex.Release();
	NewTex.FileName = folder;

	if (stbi_info((folder + fileNames[0] + ext).c_str(), &x, &y, &n) != 1)
		return;

	NewTex.TextureTarget = GL_TEXTURE_CUBE_MAP;
	NewTex.Width = x;
	NewTex.Height = y;
	NewTex.Format = n == 3 ? GL_RGB : GL_RGBA;
	NewTex.InternalFormat = n == 3 ? GL_COMPRESSED_SRGB_S3TC_DXT1_EXT : GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;

	glGenTextures(1, &NewTex.TextureObject);
	NewTex.Bind(0);
	glGetError();

	for (int i = 0; i < 6; i++)
	{
		unsigned char *data = stbi_load((folder + fileNames[i] + ext).c_str(), &x, &y, &n, n);

		if (data == nullptr)
			continue;

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, NewTex.InternalFormat, NewTex.Width, NewTex.Height, 0, NewTex.Format, GL_UNSIGNED_BYTE, data);
		glGetError();

		stbi_image_free(data);
	}

	NewTex.GenerateMipmaps();
	glGetError();
	NewTex.SetFilters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	glGetError();
	NewTex.TextureHandle = glGetTextureHandleARB(NewTex.TextureObject);
	glGetError();
	glMakeTextureHandleResidentARB(NewTex.TextureHandle);
	glGetError();
}
