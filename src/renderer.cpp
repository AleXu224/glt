#include "renderer.hpp"

using namespace squi;

std::unique_ptr<Renderer> Renderer::instance = nullptr;

auto vertexShader = R"(
        #version 450 core
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
    // TODO: Borders do not work with border radius = 0
auto fragmentShader = R"(
        #version 450 core
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

        uniform sampler2D uTexture[32];

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
            outColor *= vColor.a;
            float bSize = vBorderSize;
            if (b > -bSize && bSize > 0.0) {
                vec4 borderColor = vBorderColor;
                borderColor *= vBorderColor.a;
                outColor = mix(outColor, vBorderColor, smoothstep(0.0, 1.0, b + bSize));
            }
            FragColor = mix(outColor, vec4(0.0), smoothstep(0.0, 1.0, b));
        }

        vec4 getTextureColor() {
            switch (vTextureId) {
                case 0:
                    return texture(uTexture[0], vTexUV);
                case 1:
                    return texture(uTexture[1], vTexUV);
                case 2:
                    return texture(uTexture[2], vTexUV);
                case 3:
                    return texture(uTexture[3], vTexUV);
                case 4:
                    return texture(uTexture[4], vTexUV);
                case 5:
                    return texture(uTexture[5], vTexUV);
                case 6:
                    return texture(uTexture[6], vTexUV);
                case 7:
                    return texture(uTexture[7], vTexUV);
                case 8:
                    return texture(uTexture[8], vTexUV);
                case 9:
                    return texture(uTexture[9], vTexUV);
                case 10:
                    return texture(uTexture[10], vTexUV);
                case 11:
                    return texture(uTexture[11], vTexUV);
                case 12:
                    return texture(uTexture[12], vTexUV);
                case 13:
                    return texture(uTexture[13], vTexUV);
                case 14:
                    return texture(uTexture[14], vTexUV);
                case 15:
                    return texture(uTexture[15], vTexUV);
            }
        }

        void TextQuad() {
            // float alpha = getTextureColor().r;
            FragColor = vColor * vColor.a * getTextureColor().r;
            // outColor *= vColor.a * alpha;
            // outColor *= alpha;
            // FragColor = outColor;
            // FragColor = vec4(vColor.rgb, getTextureColor().r * vColor.a);
            // FragColor = vec4(vColor.rgb, texture2D(uTexture[vTextureId], vTexUV).r * vColor.a);
        }

        void main()
        {
            // NoTextureQuad();
            if (vTextureType == 0) {
                NoTextureQuad();
            } else if (vTextureType == 2) {
                TextQuad();
            } else {
                FragColor = vec4(1.0, 0.0, 1.0, 1.0);
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

Renderer::Renderer() : shader(vertexShader, fragmentShader) {
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
	batch.addQuad(quad, shader);
}

void Renderer::render() {
	shader.use();
	shader.setUniform("uProjectionMatrix", projectionMatrix);
    constexpr uint32_t textureCount = 32;
    std::array<int, textureCount> textureSlots = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};
    shader.setUniform("uTexture", textureSlots.data(), textureCount);
	batch.render(shader);

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