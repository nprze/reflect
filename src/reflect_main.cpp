#ifdef VLD_ENABLE
#include <vld.h>
#endif // VLD_ENABLE
#include <iostream>
#include "utils/profiling_label.h"
int main() {
	RFCT_PROFILE_FUNCTION();
	// test vld and nvtx
	int* leak = new int[10];
	for (int l = 0; l < 10;l++) {
		leak[l] = l;
	}
	int a;
	std::cin >> a;
	return a;
}