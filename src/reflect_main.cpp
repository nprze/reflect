#ifdef VLD_ENABLE
#include <vld.h>
#endif // VLD_ENABLE
#include <iostream>
#include "renderer_p/renderer.h"
using namespace rfct;
int main() {
	renderer::ren.run();
	return 0;
}