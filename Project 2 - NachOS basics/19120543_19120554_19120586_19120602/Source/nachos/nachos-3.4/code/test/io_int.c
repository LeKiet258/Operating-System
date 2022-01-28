#include "syscall.h"

int main()
{
	int x;
	PrintString("Nhap vao 1 so: ");
	x = ReadInt();
	PrintString("Output: ");
	PrintInt(x);
	
	return 0;
}

