#ifndef SQUI_RENDERER_HPP
#define SQUI_RENDERER_HPP

#include "array"
#include "vertex.hpp"
#include "batch.hpp"
#include "textBatch.hpp"
#include "shader.hpp"
#include "list"

namespace squi {
    class Renderer {
        static Renderer *instance;
        std::vector<Batch> batches{};
        std::list<TextBatch> textBatches{};
        Shader shader;
        Shader textShader;
        glm::mat4 projectionMatrix;

        Renderer();
        ~Renderer();
        
    public:
        static Renderer *get();

        std::tuple<std::span<Vertex>, std::span<unsigned int>, unsigned int, unsigned int>
        addVertex();

        void removeVertex(unsigned int batchIndex, unsigned int vertexIndex);

        void render();

        void updateScreenSize(int width, int height);
    };
}

#endif