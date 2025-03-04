#include "app.h"
#include "renderer_p\renderer.h"
#include <android/native_window.h>
rfct::reflectApplication::reflectApplication(ANativeWindow* windowArg):
    m_Renderer(std::make_unique<renderer>(windowArg))
{
}

void rfct::reflectApplication::render() {
    renderer::ren->render();
}
