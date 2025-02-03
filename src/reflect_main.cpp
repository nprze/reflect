#ifdef VLD_ENABLE
#include <vld.h>
#endif // VLD_ENABLE
#include <iostream>
#include "platform_p/windows/windows_window.h"
using namespace rfct;
int main() {
	windowsWindow window(100,100, "reflect");
	window.show();
	while (window.pollEvents());
	return 0;
}