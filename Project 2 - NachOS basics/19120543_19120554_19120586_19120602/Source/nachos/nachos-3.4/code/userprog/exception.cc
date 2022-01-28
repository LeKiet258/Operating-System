// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
// Input: - User space address (int)
// - Limit of buffer (int)
// Output:- Buffer (char*)
// Purpose: chuyen doi vung nho tu user mode -> kernel mode
char* User2System(int virtAddr, int limit)
{
	int i;// index
	int oneChar;
	char* kernelBuf = NULL;
	kernelBuf = new char[limit + 1];//need for terminal string

	if (kernelBuf == NULL)
		return kernelBuf;
	memset(kernelBuf, 0, limit + 1);
	//printf("\n Filename u2s:");
	for (i = 0; i < limit; i++)
	{
		machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;
		//printf("%c",kernelBuf[i]);
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}

// Input: - User space address (int)
// - Limit of buffer (int)
// - Buffer (char[])
// Output:- Number of bytes copied (int)
// Purpose: chuyen doi vung nho tu kernel mode -> user mode 
int System2User(int virtAddr, int len, char* buffer)
{
	if (len < 0) return -1;
	if (len == 0)return len;
	int i = 0;
	int oneChar = 0;
	do {
		oneChar = (int)buffer[i];
		machine->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < len && oneChar != 0);
	return i;
}

void IncreasePC()
{
	machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg)); //ghi de thanh ghi truoc thanh thanh ghi hien tai 
    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg)); //ghi de thanh ghi hien tai thanh thanh ghi tiep theo
    machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4); //ghi de thanh ghi tiep theo thanh thanh ghi tiep theo nua
}

void ExceptionHandler(ExceptionType which)
{
	int type = machine->ReadRegister(2);
	int MaxFileLength = 32;
	int maxBuffer = 255;
	char* buffer; //doc ND tu man hinh va luu vao buffer
	buffer = new char[maxBuffer + 1]; //"+1" vi doc ca ki tu '\0'

	switch (which) {
	case NoException: //tra quyen dieu khien cho hdh
		//file "interrupt.h": enum MachineStatus {IdleMode, SystemMode, UserMode}; 
		interrupt->setStatus(SystemMode); 
		DEBUG('a', "\n Tra quyen dieu khien cho HDH.");
		return;

	case PageFaultException:
		printf("\nNo valid translation found.\n");
		interrupt->Halt();
		break;

	case ReadOnlyException:
		printf("\nWrite attempted to page marked \"read-only\".\n");
		interrupt->Halt();
		break;

	case BusErrorException:
		printf("\nTranslation resulted in an invalid physical address.\n");
		interrupt->Halt();
		break;

	case AddressErrorException:
		printf("\nUnaligned reference or one that was beyond the end of the address space.\n");
		interrupt->Halt();
		break;

	case OverflowException:
		printf("\nInteger overflow in add or sub.\n");
		interrupt->Halt();
		break;

	case IllegalInstrException:
		printf("\nUnimplemented or reserved instr\n");
		interrupt->Halt();
		break;

	case NumExceptionTypes:
		printf("\nNumExceptionTypes\n");
		interrupt->Halt();
		break;

	case SyscallException: //neu which la syscall thi...
		switch (type) { //coi thu syscall thuoc loai (type) nao
		case SC_Halt:
			DEBUG('a', "\n Shutdown, initiated by user program.");
			printf("\nShutdown, initiated by user program.\n");
			interrupt->Halt();
			return;

		case SC_ReadInt:
		{
			//doc input nguoi dung vao buffer, tra ve so ki tu doc duoc vao nBytes
			int nBytes = gSynchConsole->Read(buffer, maxBuffer);
			long long number = 0; // so luu ket qua tra ve cuoi cung
			int startIndex = (buffer[0] == '-'? 1 : 0); // Xac dinh so am hay so duong 
			bool flag = false; //check vong for duyet toi dau '.' chua
			bool notANum = false; //check co la so hop le

			// KT moi chu so co thuoc ['0', '9'], neu khong thi input khong hop le
			for (int i = startIndex; i < nBytes; i++)
			{
				//VD: 12.000 van chap nhan duoc
				if (buffer[i] == '.') 
				{
					flag = 1; //danh dau da duyet toi '.'
					for(int j=i+1; j < nBytes; j++)
					{
						//neu cac chu so sau dau '.' khong phai la cac chu so 0
						//VD: 12.001 thay vi 12.000
						if (buffer[j] != '0')
						{
							notANum = 1;
							break;
						}
					}
					
				}
				//neu chu so ko hop le (tru dau '.')
				else if (buffer[i] < '0' || buffer[i] > '9')
				{
					notANum = 1;
					break;
				}

				//neu ko la so hop le HOAC neu duyet toi dau '.' thi ket thuc ngay 
				if(notANum || flag) break; 

				//neu chu so hop le -> cong don vao bien number
				number = number * 10 + (buffer[i] - '0');
			}
			//neu khong la so hop le
			if(notANum)
			{
				machine->WriteRegister(2, 0); //xuat ra 0 neu input ko la so nguyen
				IncreasePC();
				delete buffer;
				return; //that bai
			}

			// neu la so am thi * -1;
			number = (buffer[0] == '-'? number * -1 : number);

			//int: (-2147483648, 2147483647] -> ngoai khoang nay tra ve 0
			if(number > 2147483647 || number < -2147483647)
			{
				machine->WriteRegister(2, 0);
				IncreasePC();
				delete buffer;
				return;
			}
				
			//int: (-2147483648 , 2147483647] -> trong khoang nay thi tra ve chinh no
			IncreasePC();			
			machine->WriteRegister(2, number);
			//IncreasePC();
			delete buffer;
			return; //thanh cong
		} 
		case SC_PrintInt:
		{
			int number = machine->ReadRegister(4); //doc input cua ham PrintInt(int x)
			bool isNegative = false;
			int nDigits = 0;
			int startIndex = 0;

			//neu number ngoai doan (-2147483648 , 2147483647] hoac so ko hop le
			if(number == 0)
			{
				gSynchConsole->Write("0", 1);
				delete buffer;
				IncreasePC();
				return;
			}
			
			//neu so la am thi tam thoi chuyen ve so duong de dem so chu so
			if (number < 0)
			{
				number *= -1; //chuyen ve thanh so duong
				startIndex = 1;
				isNegative = true;
			}

			//dem so chu so
			int tmp = number; 
			while (tmp)
			{
				nDigits++;
				tmp /= 10;	
			}

			//duyet nguoc de gan tung ki tu chu so cho buffer
			for (int i = startIndex + nDigits - 1; i >= startIndex; i--)
			{
				buffer[i] = (number % 10) + '0';
				number /= 10;
			}

			//neu la so am thi kich thuoc that buffer la nDigits + 1 do co dau '-'
			if(isNegative)
			{
				buffer[0] = '-';
				gSynchConsole->Write(buffer, nDigits + 1);
			}
			else //nguoc lai neu la so duong
			{
				gSynchConsole->Write(buffer, nDigits);
			}
			
			delete buffer;
			IncreasePC();
			return; 
		}
		case SC_ReadChar:
		{
			int nBytes = gSynchConsole->Read(buffer, maxBuffer);
	
			if(nBytes == 0) //Ky tu rong
			{
				printf("Ky tu rong!");
				DEBUG('a', "\nERROR: Ky tu rong!");
				machine->WriteRegister(2, 0); //tra ve 0: that bai
			}
			//chi lay ky tu dau neu nhap vao 1 chuoi
			else machine->WriteRegister(2, buffer[0]);
			
			delete buffer;
			IncreasePC();  
			return;
		}
		case SC_PrintChar:
		{
			// Doc ki tu tu thanh ghi
			char character = machine->ReadRegister(4);
			// In ki tu ra man hinh console
			gSynchConsole->Write(&character, 1);

			IncreasePC();
			return;
		}
		case SC_ReadString:
		{
			char* string;
			
			// Dia chi tham so truyen vao o thanh ghi r4
			int address = machine->ReadRegister(4);
			// Chieu dai chuoi o thanh ghi r5
			int length = machine->ReadRegister(5);

			// Sao chep chuoi tu vung nho UserSpace vao KernelSpace, chieu dai length
			string = User2System(address, length);
			// Doc chuoi tu console
			gSynchConsole->Read(string, length);
			// Sao chep chuoi ve lai UserSpace
			System2User(address, length, string);

			delete string;
			IncreasePC();
			return;
		}
		case SC_PrintString:
		{
			// Dia chi vung nho, chieu dai chuoi 
			int address;
			int length = 0;
			char* string;
			
			// Dia chi tham so truyen vao o thanh ghi r4
			address = machine->ReadRegister(4);
			// Sao chep chuoi tu vung nho vao KernelSpace, chieu dai tam thoi 255
			string = User2System(address, 255);

			// Lay do dai chuoi
			while (string[length] != '\0') length++;
			// In chuoi ra man hinh console
			gSynchConsole->Write(string, length + 1);

			delete string;
			IncreasePC();
			return;
		}

		case SC_Create:
		{
			int virtAddr;
			char* filename;
			DEBUG('a', "\n SC_Create call ...");
			DEBUG('a', "\n Reading virtual address of filename");
			// Lấy tham số tên tập tin từ thanh ghi r4
			virtAddr = machine->ReadRegister(4);
			DEBUG('a', "\n Reading filename.");
			// MaxFileLength là = 32
			filename = User2System(virtAddr, MaxFileLength + 1);
			if (filename == NULL)
			{
				printf("\n Not enough memory in system");
				DEBUG('a', "\n Not enough memory in system");
				machine->WriteRegister(2, -1); // tra ve loi (-1)
				delete filename;
				return;
			}
			DEBUG('a', "\n Finish reading filename.");
			if (!fileSystem->Create(filename, 0))
			{
				printf("\n Error create file '%s'", filename);
				machine->WriteRegister(2, -1); //that bai
				delete filename;
				return; 
			}
			machine->WriteRegister(2, 0); // thanh cong
			delete filename;
			return;
		}

		default: //neu la syscall khac -> increasePC va khong lam gi
			IncreasePC();
			break;
		} //ket thuc switch(type)

		break;
	} //ket thuc switch(which)
} //ket thuc function

