#include "app.h"
#include "renderer_p\renderer.h"
rfct::reflectApplication::reflectApplication()
{
	renderer* ren = new renderer();
	renderer::getRen().render();
	renderer::getRen().showWindow();
	while (renderer::getRen().getWindow().pollEvents())
	{
		renderer::getRen().render();
	}
	delete ren;
}
