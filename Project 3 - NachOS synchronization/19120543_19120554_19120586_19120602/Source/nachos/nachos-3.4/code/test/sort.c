/* sort.c 
 *    Test program to sort a large number of integers.
 *
 *    Intention is to stress virtual memory system.
 *
 *    Ideally, we could read the unsorted array off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"

void BubbleSort(int A[], int n) {
	int i, tmp;
	int m = 0;
	int flag;
	flag = 1;
	while(flag == 1) {
		flag = 0;
		for (i = n - 1; i >= m; i--) {
			if(A[i - 1] > A[i]) {
	      			tmp = A[i - 1];
	      			A[i - 1] = A[i];
	      			A[i] = tmp;
				flag = 1;
			}
		}
	}
}

int main() {
	int n, i, number;
	int A[100];
	
	// Nhap so luong phan tu n	
	PrintString("\nNhap so luong phan tu: ");
	n = ReadInt();
	
	// Nhap cac phan tu
	for (i = 0; i < n; i++) {
		PrintString(" [");
		PrintInt(i + 1);
		PrintString("]: ");
		number = ReadInt();
		if (number == 0) {
			PrintString(" -> Hay nhap so nguyen !\n");
			i--;
			continue;
		} else {
			A[i] = number;
		}
	}

	// Sap xep tang dan
	BubbleSort(A, n);
	
	PrintString("\nMang da sap xep:\n ");
	// In mang da sap xep
	for (i = 0; i < n; i++) {
		PrintInt(A[i]);
		PrintString(" ");
	}
	PrintString("\n");

	return 0;
}

