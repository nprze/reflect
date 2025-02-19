#include "app.h"
#include "renderer_p\renderer.h"
rfct::reflectApplication::reflectApplication()
{
	renderer::ren.showWindow();
	while (renderer::ren.getWindow().pollEvents())
	{
		renderer::ren.render();
	}
}
