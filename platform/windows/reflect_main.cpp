#ifdef VLD_ENABLE
#include <vld.h>
#endif // VLD_ENABLE
#include "app.h"
#include "assets/assets_utils.h"
using namespace rfct;

// entry point on windows
int main() {
	srand(static_cast<unsigned>(time(nullptr)));
	rfct::SetAssetsPath(std::string(""));
	reflectApplication();
	return 0;
}