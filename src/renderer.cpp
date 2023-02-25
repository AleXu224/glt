#include "renderer.hpp"

using namespace squi;

std::unique_ptr<Renderer> Renderer::instance = nullptr;

auto vertexShader = R"(
        #version 430 core
        layout (location = 0) in vec2 aUV;
        layout (location = 1) in vec2 aTexUV;
        layout (location = 2) in uint aID;

        struct VertexData {
            vec4 color;
            vec4 borderColor;
            vec2 pos;
            vec2 size;
            vec2 offset;
            float borderRadius;
            float borderSize;
            uint textureId;
            uint textureType;
        };

        layout (std430, binding = 3) buffer SSBO {
            VertexData data[1000];
        };

        uniform mat4 uProjectionMatrix;

        out vec4 vColor;
        out vec2 vUV;
        out vec2 vTexUV;
        out vec2 vSize;
        out float vBorderRadius;
        out float vBorderSize;
        out vec4 vBorderColor;
        out uint vTextureId;
        out uint vTextureType;

        void main()
        {
            VertexData quadData = data[aID];
            vColor = quadData.color;
            vUV = aUV;
            vTexUV = aTexUV;
            vSize = quadData.size;
            vBorderRadius = quadData.borderRadius;
            vBorderSize = quadData.borderSize;
            vBorderColor = quadData.borderColor;
            vTextureId = quadData.textureId;
            vTextureType = quadData.textureType;
            vec2 pos = quadData.offset + quadData.pos + (aUV * quadData.size);
            gl_Position = vec4(uProjectionMatrix * vec4(pos, 0.0, 1.0));
        }
    )";
auto fragmentShader = R"(
        #version 430 core
        out vec4 FragColor;

        in vec4 vColor;
        in vec2 vUV;
        in vec2 vTexUV;
        in vec2 vSize;
        in float vBorderRadius;
        in float vBorderSize;
        in vec4 vBorderColor;
        flat in uint vTextureId;
        flat in uint vTextureType;

        uniform sampler2D uTexture[16];

        // Credit https://www.shadertoy.com/view/ldfSDj
        float udRoundBox( vec2 p, vec2 b, float r )
        {
            return length(max(abs(p)-b+r,0.0))-r;
        }

        void NoTextureQuad() {
            vec2 coords = (vUV * vSize) - 0.5;
            vec2 halfRes = (0.5 * vSize) - 0.5;
            float borderRadius = min(vBorderRadius, min(halfRes.x, halfRes.y));
            float b = udRoundBox(coords - halfRes, halfRes, borderRadius);
            vec4 outColor = vColor;
            if (b > -vBorderSize && vBorderSize > 0.0) {
                outColor = mix(outColor, vBorderColor, smoothstep(0.0, 1.0, b + vBorderSize));
            }
            FragColor = mix(outColor, vec4(outColor.xyz, 0.0), smoothstep(0.0, 1.5, b));
        }

        void TextQuad() {
            FragColor = vec4(vColor.rgb, texture(uTexture[vTextureId], vTexUV).r * vColor.a);
        }

        void main()
        {
            if (vTextureType == 0) {
                NoTextureQuad();
            } else if (vTextureType == 2) {
                TextQuad();
            }
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
        float dist = texture(uTexture, vUv.st).r;
        float width = 0.75 * fwidth(dist);
        // float width = length(dFdx(dist) + dFdy(dist)) * 0.70710678118654757;
        float alpha = smoothstep(0.4-width, 0.4+width, dist);
        FragColor = vec4(vColor.rgb, alpha * vColor.a);
        // FragColor = vec4(vColor.rgb, texture(uTexture, vUv.st).r * vColor.a);
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

	// textBatches.push_back(TextBatch("C:\\Windows\\Fonts\\arial.ttf"));
	// textBatches.front().createQuads("Font looks really bad :(", {10.0f, 10.0f}, 12.0f, {1.0f, 1.0f, 1.0f, 1.0f});
}

Renderer::~Renderer() {
	// glDeleteProgram(this->shaderProgram);
}

Renderer &Renderer::getInstance() {
	if (!instance) {
		instance = std::make_unique<Renderer>();
	}
	return *instance;
}

void Renderer::addQuad(Quad &quad) {
	batch.addQuad(quad);
}

void Renderer::render() {
	shader.use();
	shader.setUniform("uProjectionMatrix", projectionMatrix);
	batch.render();

	// textShader.use();
	// shader.setUniform("uTexture", 0);
	// shader.setUniform("uProjectionMatrix", projectionMatrix);
	// for (auto &batch: textBatches) {
	// 	batch.render();
	// }
}

void Renderer::updateScreenSize(int width, int height) {
	projectionMatrix[0][0] = 2.0f / width;
	projectionMatrix[1][1] = -2.0f / height;
}