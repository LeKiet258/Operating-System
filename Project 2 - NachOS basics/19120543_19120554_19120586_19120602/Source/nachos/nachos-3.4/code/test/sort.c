//Cai dat thuat toan Bubble Sort

#include "syscall.h"

void AscendingSort(int arr[], int n) {
	int i, j, tmp;

	for (i = 0; i < n-1; i++) 
	{
		for (j = 0; j < n-i-1; j++)
		{
			if (arr[j] > arr[j+1])
			{
				//swap(arr[j], arr[j+1])
				tmp = arr[j];
				arr[j] = arr[j+1];
				arr[j+1] = tmp;
			}
		}
	}
}

void DescendingSort(int arr[], int n) {
	int i, j, tmp;

	for (i = 0; i < n-1; i++) 
	{
		for (j = 0; j < n-i-1; j++)
		{
			if (arr[j] < arr[j+1])
			{
				//swap(arr[j], arr[j+1])
				tmp = arr[j];
				arr[j] = arr[j+1];
				arr[j+1] = tmp;
			}
		}
	}
}

int main() {
	int n, i, number, sortType;
	int A[100];
	
	// Nhap so luong phan tu n	
	PrintString("\nNhap so luong phan tu (1 <= n <= 100): ");
	n = ReadInt();

	// Neu n ko hop le
	if (n < 1 || n > 100)
	{
		PrintString("n phai thuoc [1,100]\n");
		return 0;
	}

	// Nhap thu tu sap xep	
	PrintString("\nNhap thu tu sap xep (1: tang dan; 2: giam dan): ");
	sortType = ReadInt();

	// Neu sortType ko hop le
	if (sortType != 1 && sortType != 2)
	{
		PrintString("Thu tu sap xep nhap vao khong hop le\n");
		return 0;
	}
	
	// Nhap cac phan tu
	PrintString("\nNhap cac phan tu:\n");
	for (i = 0; i < n; i++) {
		PrintString(" [");
		PrintInt(i + 1);
		PrintString("]: ");
		number = ReadInt(); // neu nhap vao so vi pham ReadInt thi number = 0
		A[i] = number;
	}

	if (sortType == 1) // Sap xep tang dan
		AscendingSort(A, n);
	else // Sap xep giam dan
		DescendingSort(A, n);

	PrintString("\nMang da sap xep:\n ");
	// In mang da sap xep
	for (i = 0; i < n; i++) {
		PrintInt(A[i]);
		PrintString(" ");
	}
	PrintString("\n");

	return 0;
}

