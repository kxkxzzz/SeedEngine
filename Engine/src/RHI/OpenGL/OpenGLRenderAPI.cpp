// OpenGLRenderAPI.cpp
#include "RHI/OpenGL/OpenGLRenderAPI.h"

#include <glad/gl.h>

namespace seed {

void OpenGLRenderAPI::Init() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    // cubemap 跨面采样时在接缝处插值，否则光滑金属球上会看到六个面的拼接线
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void OpenGLRenderAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    glViewport(x, y, width, height);
}

void OpenGLRenderAPI::SetClearColor(const glm::vec4& color) {
    glClearColor(color.r, color.g, color.b, color.a);
}

void OpenGLRenderAPI::Clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderAPI::DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray,
                                  uint32_t indexCount) {
    vertexArray->Bind();
    uint32_t count = indexCount == 0 ? vertexArray->GetIndexBuffer()->GetCount() : indexCount;
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
}

}  // namespace seed
