#pragma once

#include "../gl_core_3_3.h"				// OpenGL binding

#include <glm/glm.hpp>				// For Uniform get,set
#include <glm/gtc/type_ptr.hpp>		// For Uniform get,set

#include <unordered_map>
#include <string>

namespace jge
{
	/**
    * Simple abstraction of a OpenGL shader program consisting of vertex and fragmentshader.
    * Sharing single shaders across shaderprograms is not supported.
    */
	class ShaderProgram
	{
	public:
		ShaderProgram();
		~ShaderProgram();

		bool Create(GLuint vertShader, GLuint fragShader);
		bool IsValid() const;
		void UseProgram();
		bool RegisterUniform(const std::string& name);

		void UpdateUniform(const std::string& name, int i);
		void UpdateUniform(const std::string& name, float f);
		void UpdateUniform(const std::string& name, const glm::vec2& vec2);
		void UpdateUniform(const std::string& name, const glm::vec3& vec3);
		void UpdateUniform(const std::string& name, const glm::mat3& mat3);
		void UpdateUniform(const std::string& name, const glm::vec4& vec4);
		void UpdateUniform(const std::string& name, const glm::mat4& mat4);

		static GLuint CreateShader(GLenum type, const char* source, unsigned int length);
	private:
		void RegisterUniforms();
		
		GLuint m_programId;
		GLuint m_vertexShaderId;
		GLuint m_fragmentShaderId;

		bool m_isInitialized;
		bool m_isValid;

		// Cannot use const char ptrs here as they are not treated
		// as string by the map template. 
		std::unordered_map<std::string, GLuint> m_uniformMap;
	};
}