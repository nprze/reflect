#include "app.h"
#include "renderer_p\renderer.h"
rfct::reflectApplication::reflectApplication()
{
	renderer::ren.render();
	renderer::ren.showWindow();
	while (renderer::ren.getWindow().pollEvents())
	{
		renderer::ren.render();
	}
}
