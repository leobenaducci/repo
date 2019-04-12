#pragma once

class Shader
{
	typedef std::map<std::string, GLint> ParameterMap;

public:
	Shader();
	Shader(const Shader&) = delete;
	~Shader();

	void Use(uint32_t Flags);

	void SetComputeLocalSize(GLint local_x, GLint local_y, GLint local_z) { ComputeLocalSize = glm::ivec3(local_x, local_y, local_z); }
	void Dispatch(GLint global_x, GLint global_y, GLint global_z, uint32_t Flags, std::function<void(Shader*, uint32_t)> SetParameters = nullptr);

	void ClearFlags();
	void AddFlag(uint32_t Flag, const std::string& Definition);
	void AddCommonDefinition(const std::string& Definition);
	void Compile(GLenum ShaderType, const std::string& Src, uint32_t Flags);
	void Link(uint32_t Flags);

	void CompileFiles(const std::string& FileName, const std::vector<std::string>& Includes = std::vector<std::string>());

	void SetParameter(const std::string& Name, const GLint& Value, const GLsizei Count = 1)
	{
		_CheckUsed();

		auto Parameters = ShaderVariations[CurrentShaderVariation].Parameters;
		auto it = Parameters.find(Name);
		if (it == Parameters.end()) return;

		glUniform1iv(it->second, Count, &Value);
	}

	void SetParameter(const std::string& Name, const GLuint& Value, const GLsizei Count = 1)
	{
		_CheckUsed();

		auto Parameters = ShaderVariations[CurrentShaderVariation].Parameters;
		auto it = Parameters.find(Name);
		if (it == Parameters.end()) return;

		glUniform1uiv(it->second, Count, &Value);
	}

	void SetParameter(const std::string& Name, const GLuint64EXT& Value, const GLsizei Count = 1)
	{
		_CheckUsed();

		auto Parameters = ShaderVariations[CurrentShaderVariation].Parameters;
		auto it = Parameters.find(Name);
		if (it == Parameters.end()) return;

		glUniformui64vNV(it->second, Count, &Value);
	}

	void SetParameter(const std::string& Name, const glm::ivec2& Value, const GLsizei Count = 1)
	{
		_CheckUsed();

		auto Parameters = ShaderVariations[CurrentShaderVariation].Parameters;
		auto it = Parameters.find(Name);
		if (it == Parameters.end()) return;

		glUniform2iv(it->second, Count, &Value.x);
	}

	void SetParameter(const std::string& Name, const GLfloat& Value, const GLsizei Count = 1)
	{
		_CheckUsed();

		auto Parameters = ShaderVariations[CurrentShaderVariation].Parameters;
		auto it = Parameters.find(Name);
		if (it == Parameters.end()) return;

		glUniform1fv(it->second, Count, &Value);
	}

	void SetParameter(const std::string& Name, const glm::vec2& Value, const GLsizei Count = 1)
	{
		_CheckUsed();

		auto Parameters = ShaderVariations[CurrentShaderVariation].Parameters;
		auto it = Parameters.find(Name);
		if (it == Parameters.end()) return;

		glUniform2fv(it->second, Count, &Value.x);
	}

	void SetParameter(const std::string& Name, const glm::vec3& Value, const GLsizei Count = 1)
	{
		_CheckUsed();

		auto Parameters = ShaderVariations[CurrentShaderVariation].Parameters;
		auto it = Parameters.find(Name);
		if (it == Parameters.end()) return;

		glUniform3fv(it->second, Count, &Value.x);
	}

	void SetParameter(const std::string& Name, const glm::vec4& Value, const GLsizei Count = 1)
	{
		_CheckUsed();

		auto Parameters = ShaderVariations[CurrentShaderVariation].Parameters;
		auto it = Parameters.find(Name);
		if (it == Parameters.end()) return;

		glUniform4fv(it->second, Count, &Value.x);
	}

	void SetParameter(const std::string& Name, const glm::mat4& Value, const GLsizei Count = 1)
	{
		_CheckUsed();

		auto Parameters = ShaderVariations[CurrentShaderVariation].Parameters;
		auto it = Parameters.find(Name);
		if (it == Parameters.end()) return;

		glUniformMatrix4fv(it->second, Count, GL_FALSE, (float*)&Value);
	}

private:

	struct ShaderVariation
	{
		GLhandleARB VertexShader;
		GLhandleARB TessControlShader;
		GLhandleARB GeometryShader;
		GLhandleARB TessEvalShader;
		GLhandleARB FragmentShader;
		GLhandleARB ComputeShader;
		GLhandleARB Program;

		ParameterMap Parameters;

		ShaderVariation() :
			VertexShader(0), 
			TessControlShader(0),
			GeometryShader(0), 
			TessEvalShader(0),
			FragmentShader(0),
			ComputeShader(0), 
			Program(0)
		{
		}
	};

	void _CheckUsed()
	{
		GLint CurrentProgram;

		glGetIntegerv(GL_CURRENT_PROGRAM, &CurrentProgram);

		//if (CurrentProgram != ShaderVariations[CurrentShaderVariation].Program)
		//	printf("Trying to set a parameter in a program that's not being used\n");
	}

	void _ParseParameters(Shader::ShaderVariation* it);

	void Discard();

	std::map<uint32_t, std::string> FlagStrings;
	std::string CommonDefinitions;

	std::map<uint32_t, ShaderVariation> ShaderVariations;
	uint32_t CurrentShaderVariation;

	glm::ivec3 ComputeLocalSize;
};

