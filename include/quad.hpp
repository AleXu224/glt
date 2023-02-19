#ifndef SQUI_SQUARE_HPP
#define SQUI_SQUARE_HPP
#include "array"
#include "span"
#include "vertex.hpp"

namespace squi {
	class Quad {
		std::array<Vertex, 4> vertices{};
		/**
		 * Quad data for the vertex shader
		 * The format is the following:
		 * colorR  	    colorG     colorB  colorA
		 * borderR 	    borderG    borderB borderA
		 * posX    	    posY       sizeX   sizeY
		 * borderRadius borderSize texId   0
		 * 
		 */
		glm::mat4 data;

	public:
		Quad(
			glm::vec2 position,
			glm::vec2 size,
			glm::vec4 color,
			float borderRadius = 0.0f,
			float borderSize = 0.0f,
			glm::vec4 borderColor = {0.0f, 0.0f, 0.0f, 1.0f});

		std::span<const Vertex> getVertices() const;
		const glm::mat4 &getData() const;

		void setId(int id);

		void setTextureId(int id);

		void setPos(glm::vec2 pos);
		void setSize(glm::vec2 size);
		void setColor(glm::vec4 color);
		void setBorderColor(glm::vec4 color);
		void setBorderRadius(float radius);
		void setBorderSize(float size);
	};
}// namespace squi

#endif