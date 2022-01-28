#include "syscall.h"

int main() {
	int i;
	char c;
	PrintString("\n--ASCII--\n");
	for (i = 0; i < 127; i++) {
		PrintInt(i);
		PrintString("\t");
		c = i;
		PrintChar(c);
		PrintString("\n");
	}

	return 0;
}
