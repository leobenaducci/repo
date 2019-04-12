#pragma once

class Texture
{
protected:
	void MakeRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

public:
	static const Texture& White() { if (!WhiteTexture._IsValid()) { WhiteTexture.MakeRGBA(255, 255, 255, 255); } return WhiteTexture; }
	static const Texture& Black() { if (!BlackTexture._IsValid()) { BlackTexture.MakeRGBA(0, 0, 0, 255); }  return BlackTexture; }
	static const Texture& IdentityNormalMap() { if (!IdentityNormalMapTexture._IsValid()) { IdentityNormalMapTexture.MakeRGBA(127, 127, 255, 255); }  return IdentityNormalMapTexture; }

	Texture();
	Texture(const Texture&) = delete;
	~Texture();

	const std::string& GetFileName() const { return FileName; }
	GLuint GetTextureTarget() const { return TextureTarget; }
	GLuint GetTextureObject() const { return TextureObject; }
	GLuint64 GetTextureHandle() const { return TextureHandle; }

	void Bind(uint32_t TexUnit) const;

	void SetFilters(const GLenum MinFilter, const GLenum MagFilter);

	void GenerateMipmaps();

	void Release();

	static void Load(const std::string& fileName, Texture& NewTex);
	static void LoadCube(const std::string& folder, const std::string& ext, Texture& NewTex);

private:
	
	static Texture WhiteTexture;
	static Texture BlackTexture;
	static Texture IdentityNormalMapTexture;

	bool _IsValid() const;

	std::string FileName;
	GLuint Width, Height, Mips;
	GLenum TextureTarget;
	GLenum Format, InternalFormat;

	GLuint TextureObject;
	GLuint64 TextureHandle;

	friend class FrameBuffer;
};
