
#ifdef WINDOWS_BUILD
#ifdef VLD_ENABLE
#include <vld.h>
#endif // VLD_ENABLE
#include <iostream>
#include "app.h"
using namespace rfct;

int main() {
	reflectApplication();
	return 0;
}
#endif	