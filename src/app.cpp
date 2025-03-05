#include "app.h"
#include "renderer_p\renderer.h"
rfct::reflectApplication::reflectApplication(RFCT_NATIVE_WINDOW_ANDROID RFCT_NATIVE_WINDOW_ANDROID_VAR):
    m_Renderer(std::make_unique<renderer>(RFCT_NATIVE_WINDOW_ANDROID_VAR))
{
#ifdef WINDOWS_BUILD
    renderer::getRen().render();
	renderer::getRen().showWindow();
	while (renderer::getRen().getWindow().pollEvents())
	{
		renderer::getRen().render();
	}
#endif
}

void rfct::reflectApplication::render() {
    renderer::getRen().render();
}
