#include "stdafx.h"

#ifndef SHADER_EXTENSION
#define SHADER_EXTENSION ".hlsl"
#endif

#include <tuple>
#include <fstream>
#include <streambuf>
#include <iterator>
#include <filesystem>

namespace ShaderErrors
{
	static GLchar ErrorString[1024];
	static GLsizei ErrorLen;
	static std::string CurrentShader;
}

Shader::Shader() 
	:	ComputeLocalSize(0, 0, 0)
{
}

Shader::~Shader()
{
	Discard();
}

void Shader::Discard()
{
	for (auto it : ShaderVariations)
	{
		auto shader = it.second;
		if (shader.VertexShader != 0)
			glDeleteShader(shader.VertexShader);
		if (shader.TessControlShader != 0)
			glDeleteShader(shader.TessControlShader);
		if (shader.GeometryShader != 0)
			glDeleteShader(shader.GeometryShader);
		if (shader.TessEvalShader != 0)
			glDeleteShader(shader.TessEvalShader);
		if (shader.FragmentShader != 0)
			glDeleteShader(shader.FragmentShader);
		if (shader.ComputeShader != 0)
			glDeleteShader(shader.ComputeShader);
		if (shader.Program != 0)
			glDeleteProgram(shader.Program);

		shader.Parameters.clear();
	}

	ShaderVariations.clear();
}

void Shader::Use(uint32_t Flags)
{
	auto it = ShaderVariations.find(Flags);

	if(it == ShaderVariations.end())
	{
		printf("Shader variation %i was not found\n", Flags);
		return;
	}
	//if(TessControlShader != 0)
	//	glPatchParameteri(GL_PATCH_VERTICES, 3);

	CurrentShaderVariation = Flags;

	glUseProgram(it->second.Program);
}

void Shader::Dispatch(GLint global_x, GLint global_y, GLint global_z, uint32_t Flags, std::function<void(Shader*, uint32_t)> SetParameters)
{
	Use(Flags);

	if(SetParameters)
		SetParameters(this, Flags);

	glDispatchCompute((global_x + ComputeLocalSize.x - 1) / ComputeLocalSize.x,
					  (global_y + ComputeLocalSize.y - 1) / ComputeLocalSize.y,
					  (global_z + ComputeLocalSize.z - 1) / ComputeLocalSize.z);
}

void Shader::ClearFlags()
{
	FlagStrings.clear();
}

void Shader::AddFlag(uint32_t FlagId, const std::string& Definition)
{
	FlagStrings[FlagId] += std::string("#define ") + Definition + "\n";
}

void Shader::AddCommonDefinition(const std::string& Definition)
{
	CommonDefinitions += std::string("#define ") + Definition + "\n";
}

void Shader::Compile(GLenum ShaderType, const std::string& Src, uint32_t Flags)
{
	const GLchar* ShaderStr[3] = {};
	GLint ShaderLen[3] = {};

	GLhandleARB NewShader;
	GLhandleARB* pNewShader;

	auto it = &ShaderVariations[Flags];

	switch (ShaderType)
	{
	case GL_VERTEX_SHADER:
		pNewShader = &it->VertexShader;
		break;
	case GL_TESS_CONTROL_SHADER:
		pNewShader = &it->TessControlShader;
		break;
	case GL_GEOMETRY_SHADER:
		pNewShader = &it->GeometryShader;
		break;
	case GL_TESS_EVALUATION_SHADER:
		pNewShader = &it->TessEvalShader;
		break;
	case GL_FRAGMENT_SHADER:
		pNewShader = &it->FragmentShader;
		break;
	case GL_COMPUTE_SHADER:
		pNewShader = &it->ComputeShader;
		break;
	default:
		printf("Shader::Compile -> Invalid Shader Type = 0x%x\n", ShaderType);
		return;
	}

	if (ShaderType == GL_COMPUTE_SHADER)
	{
		int TotalSize = ComputeLocalSize.x * ComputeLocalSize.y * ComputeLocalSize.z;
		if (TotalSize <= 0 || TotalSize > 128)
		{
			printf("Shader::Compile -> Invalid ComputeLocalSize %i\n", TotalSize);
			return;
		}
	}

	it->Parameters.clear();

	if (*pNewShader != 0)
	{
		glDeleteShader(*pNewShader);
		*pNewShader = 0;
	}

	if (it->Program != 0)
	{
		glDeleteProgram(it->Program);
		it->Program = 0;
	}

	std::string Definitions;

	Definitions += "#version 440 compatibility\n";
	Definitions += "#extension GL_NV_shader_buffer_load : enable\n";
	Definitions += "#extension GL_ARB_bindless_texture : enable\n";
	Definitions += "#extension GL_ARB_shader_group_vote : enable\n";	

	//Definitions += "#pragma optionNV(strict on)\n";

	Definitions += FlagStrings[Flags];
	Definitions += CommonDefinitions;

	if (ShaderType == GL_COMPUTE_SHADER)
	{
		static char buf[512];
		sprintf_s(buf, "#define DISPATCH_SIZE layout(local_size_x = %i, local_size_y = %i, local_size_z = %i) in;\nDISPATCH_SIZE;\n", ComputeLocalSize.x, ComputeLocalSize.y, ComputeLocalSize.z);
		Definitions += buf;
	}

	ShaderStr[0] = (GLchar*)Definitions.c_str();
	ShaderLen[0] = (GLint)Definitions.length();

	ShaderStr[1] = (GLchar*)"";
	ShaderLen[1] = (GLint)0;

	if(ShaderType != GL_COMPUTE_SHADER)
	{
		std::ifstream CommonShader(GetResourcesPath() + "common" SHADER_EXTENSION, std::ios_base::in | std::ios_base::binary);
		if (CommonShader.is_open())
		{
			static std::string CommonSrc { std::istreambuf_iterator<char>(CommonShader), std::istreambuf_iterator<char>() };
			ShaderStr[1] = (GLchar*)CommonSrc.c_str();
			ShaderLen[1] = (GLint)CommonSrc.length();
		}
	}

	ShaderStr[2] = (GLchar*)Src.c_str();
	ShaderLen[2] = (GLint)Src.length();

	NewShader = glCreateShader(ShaderType);
	glShaderSource(NewShader, 3, ShaderStr, ShaderLen);
	glCompileShader(NewShader);

	GLint bCompiled;
	glGetObjectParameterivARB(NewShader, GL_COMPILE_STATUS, &bCompiled);
	if (!bCompiled)
	{
		glGetShaderInfoLog(NewShader, sizeof(ShaderErrors::ErrorString), &ShaderErrors::ErrorLen, ShaderErrors::ErrorString);
		printf("Error compiling shader %s: ", ShaderErrors::CurrentShader.c_str());
		printf(ShaderErrors::ErrorString);
		printf("\n");

		return;
	}
	
	*pNewShader = NewShader;	
}


void Shader::Link(uint32_t Flags)
{
	if(ShaderVariations.find(Flags) == ShaderVariations.end())
		return;

	auto it = &ShaderVariations[Flags];

	it->Parameters.clear();

	if (it->Program != 0)
		glDeleteProgram(it->Program);

	it->Program = glCreateProgram();

	if (it->ComputeShader != 0)
		glAttachShader(it->Program, it->ComputeShader);
	else
	{
		if (it->VertexShader != 0)
			glAttachShader(it->Program, it->VertexShader);
		if (it->TessControlShader != 0)
			glAttachShader(it->Program, it->TessControlShader);
		if (it->GeometryShader != 0)
			glAttachShader(it->Program, it->GeometryShader);
		if (it->TessEvalShader != 0)
			glAttachShader(it->Program, it->TessEvalShader);
		if (it->FragmentShader != 0)
			glAttachShader(it->Program, it->FragmentShader);
	}

	glLinkProgram(it->Program);

	GLint bLinked;
	glGetProgramiv(it->Program, GL_LINK_STATUS, &bLinked);
	if (!bLinked)
	{
		glGetProgramInfoLog(it->Program, sizeof(ShaderErrors::ErrorString), &ShaderErrors::ErrorLen, ShaderErrors::ErrorString);
		printf("Error linking shader: %s -> ", ShaderErrors::CurrentShader.c_str());
		printf(ShaderErrors::ErrorString);
		printf("\n");

		glDeleteProgram(it->Program);
		it->Program = 0;

		return;
	}

	_ParseParameters(it);
}

void Shader::_ParseParameters(Shader::ShaderVariation* it)
{
	int Num = -1;
	glGetProgramiv(it->Program, GL_ACTIVE_UNIFORMS, &Num);

	for (int i = 0; i < Num; ++i)
	{
		static char name[512];
		int namelen = -1, num = -1;
		GLenum type = GL_ZERO;

		/* Get the name of the ith Uniform */
		glGetActiveUniform(it->Program,
			static_cast<GLuint>(i),
			sizeof(name)-1,
			&namelen,
			&num,
			&type,
			name);
		name[namelen] = 0;

		/* Get the location of the named uniform */
		GLuint location = glGetUniformLocation(it->Program, name);

		it->Parameters[std::string(name)] = location;
	}
}

#include <direct.h>

std::string get_working_path()
{
	char temp[260];
	return (_getcwd(temp, sizeof(temp)) ? std::string(temp) : std::string(""));
}

void Shader::CompileFiles(const std::string& FileName, const std::vector<std::string>& Includes)
{
	Discard();

	typedef std::tuple< GLenum, std::string> ShaderTuple;
	static std::vector< ShaderTuple > ShaderTypes  
	{ 
		ShaderTuple { GL_VERTEX_SHADER, "vert" },
		ShaderTuple { GL_TESS_CONTROL_SHADER, "tcs" },
		ShaderTuple { GL_GEOMETRY_SHADER, "geom" },
		ShaderTuple { GL_TESS_EVALUATION_SHADER, "tes" },
		ShaderTuple { GL_FRAGMENT_SHADER, "frag" },
		ShaderTuple { GL_COMPUTE_SHADER, "compute" },
	};
	
	FlagStrings[0] += "";

	std::string IncludesSrc;
	for (auto Include : Includes)
	{
		//printf("Open (%s) %s\n", get_working_path().c_str(), (FileName + "." + std::get<1>(it) + SHADER_EXTENSION).c_str());
		std::ifstream file(Include + SHADER_EXTENSION, std::ios_base::in | std::ios_base::binary);

		if (file.is_open())
		{
			const std::string src{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
			IncludesSrc += "#line 1\n";
			IncludesSrc += src;
			IncludesSrc += '\n';
		}
	}
	IncludesSrc += "#line 1\n";

	for(auto Flags : FlagStrings)
	{
		for (auto it : ShaderTypes)
		{
			//printf("Open (%s) %s\n", get_working_path().c_str(), (FileName + "." + std::get<1>(it) + SHADER_EXTENSION).c_str());
			ShaderErrors::CurrentShader = FileName + "." + std::get<1>(it) + SHADER_EXTENSION;
			std::ifstream file(FileName + "." + std::get<1>(it) + SHADER_EXTENSION, std::ios_base::in | std::ios_base::binary);

			if (file.is_open())
			{
				const std::string src{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
				Compile(std::get<0>(it), IncludesSrc + src, Flags.first);
			}
		}

		ShaderErrors::CurrentShader = FileName;

		Link(Flags.first);

		ShaderErrors::CurrentShader = "";
	}
}
