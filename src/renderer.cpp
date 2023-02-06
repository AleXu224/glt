#include "renderer.hpp"

using namespace squi;

Renderer *Renderer::instance = nullptr;

auto vertexShader = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec4 aColor;
        layout (location = 2) in vec2 aUV;
        layout (location = 3) in vec2 aSize;
        layout (location = 4) in float aBorderRadius;
        layout (location = 5) in float aBorderSize;
        layout (location = 6) in vec4 aBorderColor;
        layout (location = 7) in float z;

        uniform mat4 uProjectionMatrix;

        out vec4 vColor;
        out vec2 vUV;
        out vec2 vSize;
        out float vBorderRadius;
        out float vBorderSize;
        out vec4 vBorderColor;

        void main()
        {
            vColor = aColor;
            vUV = aUV;
            vSize = aSize;
            vBorderRadius = aBorderRadius;
            vBorderSize = aBorderSize;
            vBorderColor = aBorderColor;
            gl_Position = vec4(uProjectionMatrix * vec4(aPos, z, 1.0));
        }
    )";
auto fragmentShader = R"(
        #version 330 core
        out vec4 FragColor;

        in vec4 vColor;
        in vec2 vUV;
        in vec2 vSize;
        in float vBorderRadius;
        in float vBorderSize;
        in vec4 vBorderColor;

        // Credit https://www.shadertoy.com/view/ldfSDj
        float udRoundBox( vec2 p, vec2 b, float r )
        {
            return length(max(abs(p)-b+r,0.0))-r;
        }

        void main()
        {
            vec2 coords = (vUV * vSize) - 0.5;
            vec2 halfRes = (0.5 * vSize) - 0.5;
            float borderRadius = min(vBorderRadius, min(halfRes.x, halfRes.y));
            float b = udRoundBox(coords - halfRes, halfRes, borderRadius);
            vec4 outColor = vColor;
            if (b > -vBorderSize && vBorderSize > 0.0) {
                outColor = mix(outColor, vBorderColor, smoothstep(0.0, 1.0, b + vBorderSize));
            }
            FragColor = mix(outColor, vec4(0.0, 0.0, 0.0, 0.0), smoothstep(0.0, 1.0, b));
        }
    )";

auto textVertexShader = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec2 aUv;
    layout (location = 2) in vec2 aSize;
    layout (location = 3) in vec4 aColor;

    uniform mat4 uProjectionMatrix;

    out vec2 vUv;
    out vec2 vSize;
    out vec4 vColor;

    void main()
    {
        vUv = aUv;
        vSize = aSize;
        vColor = aColor;
        gl_Position = vec4(uProjectionMatrix * vec4(aPos, 0.99, 1.0));
    }
)";

auto textFragmentShader = R"(
    #version 330 core
    out vec4 FragColor;

    in vec2 vUv;
    in vec2 vSize;
    in vec4 vColor;

    uniform sampler2D uTexture;

    void main()
    {
        float dist = texture2D(uTexture, vUv.st).r;
        float width = 0.75 * fwidth(dist);
        float alpha = smoothstep(0.5-width, 0.5+width, dist);
        FragColor = vec4(vColor.rgb, alpha * vColor.a);
    }
)";

Renderer::Renderer() : shader(vertexShader, fragmentShader), textShader(textVertexShader, textFragmentShader) {
	// projection matrix to identity
	this->projectionMatrix = glm::mat4(1.0f);
	// Scale to 800x600 and flip y axis
	projectionMatrix[0][0] = 2.0f / 800.0f;
	projectionMatrix[1][1] = -2.0f / 600.0f;
	// Translate to topleft corner
	projectionMatrix[3][0] = -1.0f;
	projectionMatrix[3][1] = 1.0f;
    

	batches.push_back(Batch());

	textBatches.push_back(TextBatch("segoe-semibold.ttf "));
	textBatches.front().createQuads("REEEEEEEEEEEEEEEEEEEEEE", {10.0f, 10.0f}, 15.0f, {1.0f, 1.0f, 1.0f, 1.0f});
}

Renderer::~Renderer() {
	// glDeleteProgram(this->shaderProgram);
}

Renderer *Renderer::get() {
	if (instance == nullptr) {
		instance = new Renderer();
	}
	return instance;
}

std::tuple<std::span<Vertex>, std::span<unsigned int>, unsigned int, unsigned int>
Renderer::addVertex() {
	for (unsigned int i = 0; i < batches.size(); i++) {
		if (!batches[i].isFull()) {
			auto [vertices, indices, index] = batches[i].addVertex();
			return {vertices, indices, i, index};
		}
	}
	batches.push_back(Batch());
	auto [vertices, indices, index] = batches.back().addVertex();
	return {vertices, indices, batches.size() - 1, index};
}

void Renderer::removeVertex(unsigned int batchIndex, unsigned int vertexIndex) {
	batches[batchIndex].removeVertex(vertexIndex);
	if (batches[batchIndex].isEmpty()) {
		batches.erase(batches.begin() + batchIndex);
	}
}

void Renderer::render() {
	shader.use();
	shader.setUniform("uProjectionMatrix", projectionMatrix);
	for (auto &batch: batches) {
		batch.render();
	}

	textShader.use();
    shader.setUniform("uTexture", 0);
	shader.setUniform("uProjectionMatrix", projectionMatrix);
	for (auto &batch: textBatches) {
		batch.render();
	}
}

void Renderer::updateScreenSize(int width, int height) {
	projectionMatrix[0][0] = 2.0f / width;
	projectionMatrix[1][1] = -2.0f / height;
}