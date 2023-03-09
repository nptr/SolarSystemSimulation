#include "ShaderProgram.h"
#include <stdio.h>					// For reading the shader files
#include <stdexcept>

namespace jge
{
	ShaderProgram::ShaderProgram()
		: m_programId(0)
		, m_vertexShaderId(0)
		, m_fragmentShaderId(0)
		, m_isValid(false)
		, m_isInitialized(false)
	{
		m_uniformMap.clear();
	}

	bool ShaderProgram::Create(GLuint vertShader, GLuint fragShader)
	{
		assert(!m_isInitialized);
		assert(vertShader != 0);
		assert(fragShader != 0);

		m_programId = glCreateProgram();

		glAttachShader(m_programId, vertShader);
		glAttachShader(m_programId, fragShader);

		GLint isLinked = 0;
		glLinkProgram(m_programId);
		glGetProgramiv(m_programId, GL_LINK_STATUS, &isLinked);
		m_isValid = isLinked;
		m_isInitialized = true;

		if (m_isValid)
			RegisterUniforms();

		return m_isValid;
	}

	ShaderProgram::~ShaderProgram()
	{
		glDeleteProgram(m_programId);

		// Spec: A value of 0 for shader will be silently ignored.
		glDeleteShader(m_vertexShaderId);
		glDeleteShader(m_fragmentShaderId);
	}

	void ShaderProgram::UseProgram()
	{
		glUseProgram(m_programId);
	}


	bool ShaderProgram::IsValid() const
	{
		return m_isValid;
	}

	void ShaderProgram::RegisterUniforms()
	{
		const int bufferSize = 32;
		GLchar name[bufferSize];
		GLint count;
		GLint size;
		GLsizei len;
		GLenum type;

		glGetProgramiv(m_programId, GL_ACTIVE_UNIFORMS, &count);
		for (int i = 0; i < count; ++i)
		{
			glGetActiveUniform(m_programId, (GLuint)i, bufferSize, &len, &size, &type, name);
			GLuint location = glGetUniformLocation(m_programId, name);
			m_uniformMap[name] = location;
			//printf("uniform location %2d: %s\n", location, name);
		}
		//printf("\n");
	}

	bool ShaderProgram::RegisterUniform(const std::string& name)
	{
		GLuint location = glGetUniformLocation(m_programId, name.c_str());
		if (location >= 0)
		{
			m_uniformMap[name] = location;
			return true;
		}
		else return false;
	}

	void ShaderProgram::UpdateUniform(const std::string& name, int i)
	{
		const auto& f = m_uniformMap.find(name);
		if (f != m_uniformMap.end())
			glUniform1i(f->second, i);
	}
	void ShaderProgram::UpdateUniform(const std::string& name, float f)
	{
		const auto& fv = m_uniformMap.find(name);
		if (fv != m_uniformMap.end())
			glUniform1f(fv->second, f);
	}
	void ShaderProgram::UpdateUniform(const std::string& name, const glm::vec2& vec2)
	{
		const auto& f = m_uniformMap.find(name);
		if (f != m_uniformMap.end())
			glUniform2fv(f->second, 1, glm::value_ptr(vec2));
	}
	void ShaderProgram::UpdateUniform(const std::string& name, const glm::vec3& vec3)
	{
		const auto& f = m_uniformMap.find(name);
		if (f != m_uniformMap.end())
			glUniform3fv(f->second, 1, glm::value_ptr(vec3));
	}
	void ShaderProgram::UpdateUniform(const std::string& name, const glm::mat3& mat3)
	{
		const auto& f = m_uniformMap.find(name);
		if (f != m_uniformMap.end())
			glUniformMatrix3fv(f->second, 1, GL_FALSE, glm::value_ptr(mat3));
	}
	void ShaderProgram::UpdateUniform(const std::string& name, const glm::vec4& vec4)
	{
		const auto& f = m_uniformMap.find(name);
		if (f != m_uniformMap.end())
			glUniform4fv(f->second, 1, glm::value_ptr(vec4));
	}
	void ShaderProgram::UpdateUniform(const std::string& name, const glm::mat4& mat4)
	{
		const auto& f = m_uniformMap.find(name);
		if (f != m_uniformMap.end())
			glUniformMatrix4fv(f->second, 1, GL_FALSE, glm::value_ptr(mat4));
	}

	GLuint ShaderProgram::CreateShader(GLenum type, const char* source, unsigned int length)
	{
		assert(source != 0);
		assert(length > 0);

		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, (const GLchar**)&source, (const GLint*)&length);
		glCompileShader(shader);
		
		GLint shaderStatus;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderStatus);
		
		if (!shaderStatus)
		{
			GLint maxLength = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

			char* errorLog = new char[maxLength];
			glGetShaderInfoLog(shader, maxLength, &maxLength, errorLog);
			std::string errorMessage(errorLog);
			delete[] errorLog;
			glDeleteShader(shader);
			throw std::runtime_error(errorMessage);
		}

		return shader;
	}
}