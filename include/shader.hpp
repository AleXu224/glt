#ifndef SQUI_SHADER_HPP
#define SQUI_SHADER_HPP

#include "glad/glad.h"
#include "glm/glm.hpp"

namespace squi {
	class Shader {
		GLuint vertexShader;
		GLuint fragmentShader;
		GLuint shaderProgram;

	public:
		Shader(const char *vertexShaderSource, const char *fragmentShaderSource);
		~Shader();

		void use();

		void setUniform(const char *name, int value);
		void setUniform(const char *name, float value);
		void setUniform(const char *name, glm::vec2 value);
		void setUniform(const char *name, glm::vec3 value);
		void setUniform(const char *name, glm::vec4 value);
		void setUniform(const char *name, glm::mat3 value);
		void setUniform(const char *name, glm::mat4 value);
	};
}// namespace squi

#endif