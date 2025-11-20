#ifdef VLD_ENABLE
#include <vld.h>
#endif // VLD_ENABLE
#include "app.h"
#include "assets/assets_utils.h"
using namespace rfct;

// entry point on windows
int main() {
	rfct::setAssetsPath(std::string(""));
	reflectApplication();
	return 0;
}