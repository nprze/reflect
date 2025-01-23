#ifdef VLD_ENABLED
#include <vld.h>
#endif // VLD_ENABLED

int main() {
	int* leak = new int[10];
	for (int l = 0; l < 10;l++) {
		leak[l] = l;
	}
}