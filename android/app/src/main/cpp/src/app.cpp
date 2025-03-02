#include "app.h"
#include "renderer_p\renderer.h"
#include <android/native_window.h>
rfct::reflectApplication::reflectApplication(ANativeWindow* windowArg)
{
    renderer rend = renderer(windowArg);
	renderer::ren->render();
	renderer::ren->showWindow();
	while (renderer::ren->getWindow().pollEvents())
	{
		renderer::ren->render();
	}
}
