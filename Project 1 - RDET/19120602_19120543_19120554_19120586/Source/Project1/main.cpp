#include "Header.h"

int main(int argc, char** argv) {
    int option = -1;
    char keyboard = ' ';
    
    while (option != 3) {
        clrscr();
        option = menu();

        //FAT32
        if (option == 1) {
            BYTE bootSector[512];
            wstring fat32Drive = L"";
            wstringstream writer;

            //input
            cout << "Enter FAT32 drive (E/G/...): ";
            wcin >> fat32Drive;
            writer << L"\\\\.\\" << fat32Drive << L":";
            fat32Drive = writer.str();
            LPCWSTR lpcwstr = fat32Drive.c_str();
            //cout << endl << "--------------------------------------------------" << endl << endl;

            //read FAT32 
            ReadSector(lpcwstr, 0, bootSector);

            //các thông số quan trọng
            int Sb = bootSector[15] << 8 | bootSector[14];  //sector vùng bootsector
            int Nf = bootSector[16];                        // số bảng FAT
            int Sc = bootSector[13];                        //số sector cho 1 cluster
            int Sr = 0;                                     //Fat32 -> Sr=0
            int Sf = bootSector[23] << 8 | bootSector[22];  //số sector cho 1 bảng FAT, khởi tạo là xx16 - 2 byte
            if (Sf == 0x00) { //xx24 - 4 byte
                Sf = bootSector[39] << 8 | bootSector[38];
                Sf = Sf << 8 | bootSector[37];
                Sf = Sf << 8 | bootSector[36];
            }

            vector<int> info = { Sb, Nf, Sf, Sr, Sc };
            int rdetPos = Sb + Nf * Sf;
            HANDLE fat32_disk = NULL;
            fat32_disk = CreateFile(lpcwstr,    // drive to open
                GENERIC_READ,                     // Access mode
                FILE_SHARE_READ,                  // Share Mode
                NULL,                             // Security Descriptor
                OPEN_EXISTING,                    // How to create
                0,                                // File attributes
                NULL);                            // Handle to template
            vector<DWORD> fat = readFat(bootSector, fat32_disk);
            vector<FEntity> rdetEntities = readXdet(bootSector, fat32_disk, fat, rdetPos, info);
            vector<string> message = { "1. Read boot sector\n", "2. Display RDET\n", "3. Back to menu\n", "Your choice: " };
            int fatOption = -1;

            while (fatOption != 3) {
                clrscr();
                for (int i = 0; i < message.size(); ++i)
                    cout << message[i];
                cin >> fatOption;

                switch (fatOption) {
                case 1: //read boot sector
                    clrscr();
                    readBootSectorFat32(bootSector);
                    cout << "\nPress any key to escape";
                    keyboard = _getch();
                    break;
                case 2://read RDET + SDET
                    int subMenuOption = subMenu(rdetEntities, bootSector, fat32_disk, fat, rdetEntities[0], info);
                    if (subMenuOption != 3) {
                        cout << "\nPress any key to escape";
                        keyboard = _getch();
                    }
                    break;
                }
            }
            CloseHandle(fat32_disk);
        }
        //NTFS
        else if (option == 2) {
            clrscr();
            wstring ntfsDrive = L"";
            wstringstream writer;
            LPCWSTR lpcwstr1;

            //read input
            cout << "Enter NTFS drive (E/G/...): ";
            wcin >> ntfsDrive;
            writer << L"\\\\.\\" << ntfsDrive << L":";
            ntfsDrive = writer.str();
            lpcwstr1 = ntfsDrive.c_str();
            //process ntfs drive
            BYTE ntfs[512];
            ReadSector(lpcwstr1, 0, ntfs);
            readBootSectorNTFS(ntfs);

            cout << "\nPress any key to escape";
            keyboard = _getch();
        }
    }

    return 0;
}