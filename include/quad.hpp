#ifndef SQUI_SQUARE_HPP
#define SQUI_SQUARE_HPP
#include "array"
#include "span"
#include "vertex.hpp"

namespace squi {
	class Quad {
	public:
		enum class TextureType : uint32_t {
			NoTexture = 0,
			Texture = 1,
			Text = 2
		};
	private:

		std::array<Vertex, 4> vertices{};
		VertexData data;
		GLuint textureId;

	public:
		struct Args {
			glm::vec2 pos;
			glm::vec2 size;
			glm::vec2 offset;
			glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
			glm::vec4 borderColor = {0.0f, 0.0f, 0.0f, 1.0f};
			glm::f32 borderRadius;
			glm::f32 borderSize;
			glm::u32 textureId;
			TextureType textureType = TextureType::NoTexture;
			glm::vec4 textureUv;
		};

		Quad(const Args &args);

		[[nodiscard]] std::span<const Vertex> getVertices() const;
		[[nodiscard]] const VertexData &getData() const;

		void setId(int id);

		void setTextureIndex(int id);
		[[nodiscard]] TextureType getTextureType() const;
		[[nodiscard]] const GLuint &getTextureId() const;

		void setPos(glm::vec2 pos);
		void setSize(glm::vec2 size);
		void setColor(glm::vec4 color);
		void setBorderColor(glm::vec4 color);
		void setBorderRadius(float radius);
		void setBorderSize(float size);
	};
}// namespace squi

#endif