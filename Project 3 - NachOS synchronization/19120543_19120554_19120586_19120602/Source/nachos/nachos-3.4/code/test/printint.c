#include "syscall.h"

int main()
{
	PrintInt(12); //in dung cac so trong [-2147483648, 2147483647], ngoai doan nay in so 0
	Halt();
	return 0;
}

