#ifdef VLD_ENABLE
#include <vld.h>
#endif // VLD_ENABLE
#include "app.h"
#include "assets/assets_manager.h"
using namespace rfct;

// entry point on windows
int main() {
	AssetsManager::get().init("");
	reflectApplication();
	return 0;
}