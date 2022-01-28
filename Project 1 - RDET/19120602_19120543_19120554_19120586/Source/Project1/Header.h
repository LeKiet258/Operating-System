#pragma once
#include <stdio.h>
#include <Windows.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <dos.h>
#include <stack>
#include <vector>
#include <sstream>
#include <conio.h>
using namespace std;

struct Root {
    BYTE fileName[8];
    BYTE extension[3];
    BYTE fileAttributes;
    BYTE reserved;
    BYTE createTime_ms;
    //---nếu là entry phụ: nhửng thuộc tính dưới đây nằm trong file name
    BYTE createTime[2];
    BYTE createDate[2];
    BYTE accessedDate[2];
    BYTE highCluster[2];
    BYTE modifiedTime[2];
    BYTE modifiedDate[2];
    //---
    BYTE lowCluster[2];
    //---nếu là entry phụ: thì đây là 4 ký tự trong file name
    BYTE sizeofFile[4];
};

struct FEntity {
    vector<BYTE> name;
    BYTE attribute;
    int startingCluster;
    vector<int> claimedClusters;
    vector<int> claimedSectors;
    int fsize; // bytes
    vector<BYTE> data;
};

//------------------------------ Function Prototype ------------------------------
int hextodec(string s);
int sectorToDec(BYTE sector[512], int start, int count_bytes);
void readBootSectorFat32(BYTE sector[512]);
void readBootSectorNTFS(BYTE sector[512]);

//read boot sector
void ReadSector(LPCWSTR drive, int readPoint, BYTE sector[512]);
//mode=0 -> entry chính, mode=1 -> entry phụ
void readFileName(Root root, FEntity& f, int mode = 0);
vector<DWORD> readFat(BYTE bootSector[512], HANDLE fat32_disk);
//read both RDET & SDET
vector<FEntity> readXdet(BYTE bootSector[512], HANDLE fat32_disk, vector<DWORD> fat, int xdetPos, vector<int> info);
void printXdet(vector<FEntity>);
string convertExtension(vector<BYTE> extension);
//read data of txt
void readData(HANDLE fat32_disk, int Sc, int readPos, FEntity& file);

//UI
void clrscr();
int menu();
int subMenu(vector<FEntity> rdetEntities, BYTE bootSector[512], HANDLE fat32_disk, vector<DWORD> fat, FEntity fentity, vector<int> info);

//------------------------------ Function definition ------------------------------
int hextodec(string s)
{
    char* p;
    long n = strtoul(s.c_str(), &p, 16);
    return n;
}

int sectorToDec(BYTE sector[512], int start, int count_bytes)  //Hàm đọc ngược sector và chuyển về hệ 10 với vị trí bắt đầu và số byte muốn đọc
{
    stringstream hexa;
    string s;
    for (int i = 0; i < count_bytes; i++)
    {
        hexa << hex << setfill('0') << setw(2) << int(sector[start + count_bytes - 1 - i]);
    }
    s = hexa.str();
    return hextodec(s);
}

void readBootSectorFat32(BYTE sector[512]) {
    cout << "\tOS version: ";
    for (int i = 0; i < 8; i++)
        cout << char(int(sector[3 + i]));

    cout << "\n\tNumber of sectors/fat: " << int(sector[13]);
    cout << "\n\tNumber of sectors/cluster: " << sectorToDec(sector, 13, 1);
    cout << "\n\tNumber of bytes/sector: " << sectorToDec(sector, 11, 2);
    cout << "\n\tNumber of FAT table(s): " << sectorToDec(sector, 16, 1);
    cout << "\n\tNumber of entries in 1 FAT table: " << sectorToDec(sector, 17, 2);
    cout << "\n\tVolume type: " << int(sector[21]);
    cout << "\n\tNumber of sectors/track: " << sectorToDec(sector, 24, 2);
    cout << "\n\tNumber of heads: " << sectorToDec(sector, 26, 2);
    cout << "\n\tBPB_HiddSec:  " << sectorToDec(sector, 28, 4);
    cout << "\n\tBPB_FATSz32: " << sectorToDec(sector, 36, 4);
    cout << "\n\tExit flag: " << sectorToDec(sector, 40, 2);
    cout << "\n\tFAT32 verison: " << sectorToDec(sector, 42, 2);
    cout << "\n\tFirst cluster index in RDET: " << sectorToDec(sector, 44, 4);
    cout << "\n\tBPB_FSInfor: " << sectorToDec(sector, 48, 2);
    cout << "\n\tBPB_BkBootSec: " << sectorToDec(sector, 50, 2);
    cout << "\n\tReserved: " << sectorToDec(sector, 52, 12);
    cout << "\n\tBPB_DrvNum: " << sectorToDec(sector, 64, 1);
    cout << "\n\tBPB_Reservedl: " << sectorToDec(sector, 65, 1);
    cout << "\n\tBPB_BootSig: " << sectorToDec(sector, 66, 1);
    cout << "\n\tVolume serial number: " << sectorToDec(sector, 67, 4);
    cout << "\n\tFAT type: ";
    for (int i = 0; i < 8; i++) 
        cout << char(sector[82 + i]);
}

void readBootSectorNTFS(BYTE sector[512])
{
    cout << "\tSystem ID:  ";
    for (int i = 3; i < 11; i++) cout << char(int(sector[i]));
    cout << "\n\tSo byte cua Sector: " << dec << sectorToDec(sector, 11, 2);
    cout << "\n\tSo sector cua cluster: " << dec << sectorToDec(sector, 13, 2);
    cout << "\n\tLoai dia: " << hex << setfill('0') << setw(2) << int(sector[21]);
    cout << "\n\tSo sector cua volume: " << dec << sectorToDec(sector, 40, 3);
    cout << "\n\tVi tri MFT: " << sectorToDec(sector, 48, 8);
    cout << "\n\tVi tri ban sao MFT: " << sectorToDec(sector, 56, 8);
    cout << "\n\tSo cluster cua MFT record: " << sectorToDec(sector, 64, 1);
}

void ReadSector(LPCWSTR drive, int readPoint, BYTE sector[512])
{
    int retCode = 0;
    DWORD bytesRead;
    HANDLE device = NULL;

    device = CreateFile(drive,    // Drive to open
        GENERIC_READ,           // Access mode
        FILE_SHARE_READ | FILE_SHARE_WRITE,        // Share Mode
        NULL,                   // Security Descriptor
        OPEN_EXISTING,          // How to create
        0,                      // File attributes
        NULL);                  // Handle to template

    // Open Error
    if (device == INVALID_HANDLE_VALUE) {
        cout << "Error when opening the disk";
        return;
    }

    SetFilePointer(device, readPoint, NULL, FILE_BEGIN);//Set a Point to Read

    if (!ReadFile(device, sector, 512, &bytesRead, NULL))
        cout << "Cannot read disk";
    //else {
    //    for(int i=0; i<512; ++i)
    //        cout << hex << setfill('0') << setw(2) << (int)sector[i] << " ";
    //}
}

void readData(HANDLE fat32_disk, int Sc, int readPos, FEntity& file) {
    BYTE byteRoot[512];
    DWORD dwBytesRead;
    DWORD dwFilePointer = SetFilePointer(fat32_disk, (512 * readPos), NULL, FILE_BEGIN);
    int nSector = file.claimedClusters.size() * Sc;

    if (dwFilePointer != INVALID_SET_FILE_POINTER) {
        BOOL noMoreData = FALSE;
        int i = 0;
        for (i = 0; i < nSector; i++) {
            if (!ReadFile(fat32_disk, byteRoot, 512, &dwBytesRead, NULL))
                cout << "Error when reading Data.\n";

            // check entry có trống
            if (byteRoot[0] == 0x00) {
                return;
            }

            for (int i = 0; i < 512; i++) {
                if (byteRoot[i] == 0x00) {
                    noMoreData = true;
                    break;
                }
                file.data.push_back(byteRoot[i]);
            }
            if (noMoreData) break;
        }
    }
    else
        cout << "INVALID_SET_FILE_POINTER";
}

//mode=0 -> entry chính, mode=1 -> entry phụ
void readFileName(Root root, FEntity& f, int mode) {
    if (mode == 0) {
        for (int i = 0; i < 8; ++i)
            f.name.push_back(root.fileName[i]);

        if (root.extension[0] != 0x20) {
            f.name.push_back('.');
            for (int i = 0; i < 3; i++) {
                f.name.push_back(root.extension[i]);
            }
        }
    }
    else {
        for (int i = 1; i < 8; ++i) {
            f.name.push_back(root.fileName[i]);
        }
            
        for (int i = 0; i < 3; ++i) {
            f.name.push_back(root.extension[i]);
        }

        f.name.push_back(root.createTime[0]);
        f.name.push_back(root.createDate[0]);
        f.name.push_back(root.accessedDate[0]);
        f.name.push_back(root.highCluster[0]);
        f.name.push_back(root.modifiedTime[0]);
        f.name.push_back(root.modifiedDate[0]);

        for (int i = 0; i < 4; ++i) {
            f.name.push_back(root.sizeofFile[i]);
        }
    }
}

void printXdet(vector<FEntity> a) {
    for (int i = 0; i < a.size(); ++i) {
        cout << i + 1 << ". File/Folder name:  ";
        for (int j = 0; j < a[i].name.size(); ++j)
            cout << a[i].name[j];

        cout << "\nFile attribute: ";
        if (a[i].attribute & 0x01)
            cout << "Read Only File\n";
        else if (a[i].attribute & 0x02)
            cout << "Hidden File\n";
        else if (a[i].attribute & 0x04)
            cout << "System File\n";
        else if (a[i].attribute & 0x08)
            cout << "Volume Label\n";
        else if (a[i].attribute & 0x10)
            cout << "Directory\n";
        else if (a[i].attribute & 0x20)
            cout << "Archive\n";
        
        cout << "Starting cluster: " << a[i].startingCluster << endl;

        cout << "Claimed clusters: ";
        for (int j = 0; j < a[i].claimedClusters.size(); ++j) 
            cout << a[i].claimedClusters[j] << " ";

        cout << "\nClaimed sectors: ";
        for (int j = 0; j < 3; ++j)
            cout << a[i].claimedSectors[j] << " ";
        cout << "... " << a[i].claimedSectors.back();
        
        cout << "\nFile size: " << a[i].fsize << " byte";
        cout << "\n\n";
    }
}

vector<DWORD> readFat(BYTE bootSector[512], HANDLE fat32_disk) {
    int fatPos = bootSector[15] << 8 | bootSector[14]; //Sb, xxxE - 2 byte
    DWORD dwFilePointer = SetFilePointer(fat32_disk, (512 * fatPos), NULL, FILE_BEGIN);
    BYTE* byteRoot = new BYTE[512];
    DWORD dwBytesRead;
    vector<DWORD> fat;

    if (!ReadFile(fat32_disk, byteRoot, 512, &dwBytesRead, NULL))
        cout << "Error in Reading FAT32 disk.\n";
    else {
        for (int i = 0; i < 512; ++i) {
            DWORD group = 0;
            for (int j = i + 3; j >= i; --j)
                group = group << 8 | byteRoot[j];
            i += 3;
            fat.push_back(group);
        }
    }
    return fat;
}

vector<FEntity> readXdet(BYTE bootSector[512], HANDLE fat32_disk, vector<DWORD> fat, int xdetPos, vector<int> info) {
    vector<FEntity> res;
    DWORD dwFilePointer;
    DWORD dwBytesRead;
    Root stRoot;
    BYTE byteRoot[512];
    memset(&byteRoot, 0, 512);
    
    // info = {Sb, Nf, Sf, Sr, Sc}
    int Sb = info[0], Nf = info[1], Sf = info[2], Sr = info[3], Sc = info[4];

    if (fat32_disk != NULL) {
        dwFilePointer = SetFilePointer(fat32_disk, (512 * xdetPos), NULL, FILE_BEGIN);

        if (dwFilePointer != INVALID_SET_FILE_POINTER) {
            BOOL bNoEntry = FALSE;
            do {
                if (!ReadFile(fat32_disk, byteRoot, 512, &dwBytesRead, NULL))
                    cout << "Error in Reading Root Entry.\n";
                else {
                    BYTE* pByteRoot = byteRoot;

                    // đọc mỗi entry (1 entry = 32 byte)
                    for (int i = 0; i < (512 / 32); i++) {
                        FEntity f;
                        memcpy(&stRoot, pByteRoot, 32);
                        bool hasSubEntry = 0;

                        // check entry có trống
                        if (stRoot.fileName[0] == 0x00) {
                            bNoEntry = true;
                            break;
                        }
                        // check entry có bị xóa 
                        if (stRoot.fileName[0] == 0xE5) {
                            pByteRoot += 32;
                            continue;
                        }
                        //check entry phụ
                        if (stRoot.fileAttributes == 0x0F) {
                            stack<Root> subEntries;
                            hasSubEntry = 1;
                            FEntity ftmp;

                            //lưu entries phụ
                            do {
                                subEntries.push(stRoot);
                                pByteRoot += 32;
                                i++;
                                memcpy(&stRoot, pByteRoot, 32);
                            } while (stRoot.fileAttributes == 0x0F);

                            //lấy tên từ entries phụ
                            while (!subEntries.empty()) {
                                Root subEntry = subEntries.top();
                                subEntries.pop();
                                readFileName(subEntry, ftmp, 1);
                            }
                            f = ftmp;
                        }

                        //file name
                        if (!hasSubEntry) {
                            readFileName(stRoot, f);
                        }

                        // check thuộc tính trạng thái
                        if (stRoot.fileAttributes & 0x01)
                            f.attribute = 0x01;
                        else if (stRoot.fileAttributes & 0x02)
                            f.attribute = 0x02;
                        else if (stRoot.fileAttributes & 0x04)
                            f.attribute = 0x04;
                        else if (stRoot.fileAttributes & 0x08)
                            f.attribute = 0x08;
                        else if (stRoot.fileAttributes & 0x10)
                            f.attribute = 0x10;
                        else if (stRoot.fileAttributes & 0x20)
                            f.attribute = 0x20;

                        // cluster bắt đầu
                        int highCluster = stRoot.highCluster[1] << 8 | stRoot.highCluster[0];
                        int  lowCluster = stRoot.lowCluster[1] << 8 | stRoot.lowCluster[0];
                        int startCluster = (highCluster << 8 | lowCluster);
                        f.startingCluster = startCluster;
                        // chiếm các cluster 
                        vector<int> claimedClusters = { startCluster };
                        int cluster = fat[startCluster]; // next cluster index

                        while (cluster < fat.size()) {
                            if (cluster != 0x0 && cluster != 0xffffffff && 
                                                  cluster != 0xfffffff && 
                                                  cluster != 0xffffff8)  
                                claimedClusters.push_back(cluster);
                            else 
                                break;
                            cluster = fat[cluster]; //update
                        }
                        f.claimedClusters = claimedClusters;

                        //chiếm các sector
                        vector<int> claimedSectors;
                        int claimedSector = 0;
                        for (int j = 0; j < claimedClusters.size(); ++j) {
                            claimedSector = (Sb + Nf * Sf + Sr) + Sc * (claimedClusters[j] - 2);
                            
                            for(int k=0; k<Sc;++k)
                                claimedSectors.push_back(claimedSector + k);
                        }
                        f.claimedSectors = claimedSectors;

                        // kích thước nội dung tập tin
                        int sz = 0;
                        for (int i = 3; i >= 0; --i)
                            sz = sz << 8 | stRoot.sizeofFile[i];
                        f.fsize = sz;

                        //update pByteRoot
                        pByteRoot += 32;
                        res.push_back(f);
                    } //end for
                } //end else: readFile = success
                if (bNoEntry)
                    break;
            } while (true);
        } //end if (dwFilePointer != INVALID_SET_FILE_POINTER)
        else
            cout << "INVALID_SET_FILE_POINTER";
    }
    else
        cout << "cant open handle";
    return res;
} //end function

int menu() {
    char keyboard = ' ';
    vector<string> options = {
        "1. FAT32",
        "2. NTFS",
        "3. Exit"
    };
    int option = -1;
    cout << "\tMENU\n";
    for (int i = 0; i < options.size(); ++i)
        cout << options[i] << endl;
    cout << "Your choice (1 -> 3): ";
    cin >> option;
    return option;
}

void clrscr()
{
    CONSOLE_SCREEN_BUFFER_INFO	csbiInfo;
    HANDLE	hConsoleOut;
    COORD	Home = { 0,0 };
    DWORD	dummy;

    hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(hConsoleOut, &csbiInfo);

    FillConsoleOutputCharacter(hConsoleOut, ' ', csbiInfo.dwSize.X * csbiInfo.dwSize.Y, Home, &dummy);
    csbiInfo.dwCursorPosition.X = 0;
    csbiInfo.dwCursorPosition.Y = 0;
    SetConsoleCursorPosition(hConsoleOut, csbiInfo.dwCursorPosition);
}

int subMenu(vector<FEntity> rdetEntities, BYTE bootSector[512], HANDLE fat32_disk, vector<DWORD> fat, FEntity fentity, vector<int> info) {
    vector<string> message = { "      1. Display SDET\n" ,
                               "      2. Display a file's content\n" ,
                               "      3. Back\n",
                               "      Your choice: " };
    int Sb = info[0], Nf = info[1], Sf = info[2], Sr = info[3], Sc = info[4];
    char keyboard = ' ';

    while (1) {
        //display RDET
        clrscr();
        if (fentity.attribute == 0x08)
            cout << "RDET:\n";
        else {
            cout << "SDET of ";
            for (int i = 0; i < fentity.name.size(); ++i)
                cout << fentity.name[i];
            cout << endl;
        }
            
        printXdet(rdetEntities);
        cout << "\n";

        //print sub menu options
        for (int i = 0; i < message.size(); ++i)
            cout << message[i];
        int option = -1;
        cin >> option;

        if (option == 1) {
            int dirIndex = -1;
            FEntity fold;

            //input
            while (1) {
                cout << "\nEnter directory index (start from 1): ";
                cin >> dirIndex;

                if (!(dirIndex >= 1 && dirIndex <= rdetEntities.size() && rdetEntities[dirIndex-1].attribute == 0x10))
                    cout << "invalid index\n";
                else {
                    fold = rdetEntities[dirIndex-1];
                    break;
                }
            }
            int sdetPos = (Sb + Nf * Sf) + Sc * (fold.claimedClusters[0] - 2);
            vector<FEntity> sdetEntities = readXdet(bootSector, fat32_disk, fat, sdetPos, info);

            //valid input
            subMenu(sdetEntities, bootSector, fat32_disk, fat, fold, info);
        }
        else if (option == 2) {
            int fileIndex = -1;
            FEntity file;

            //input
            while (1) {
                cout << "\nEnter archive (file) index (start from 1): ";
                cin >> fileIndex;

                if (!(fileIndex >= 1 && fileIndex <= rdetEntities.size() && rdetEntities[fileIndex - 1].attribute == 0x20))
                    cout << "invalid index\n";
                else {
                    file = rdetEntities[fileIndex - 1];
                    //test
                    //tìm đuôi
                    vector<BYTE> ext;
                    for (int i = 0; i < file.name.size(); ++i) {
                        if (file.name[i] == '.') {
                            for(int j=i+1; j< file.name.size(); j++)
                                if(file.name[j] != 0x00 && file.name[j] != 0xff)
                                    ext.push_back(file.name[j]);
                            break;
                        }
                    }
                    
                    if (convertExtension(ext) == "TXT") {
                        int dataPos = Sb + Nf * Sf + Sc * (file.claimedClusters[0] - 2);
                        readData(fat32_disk, Sc, dataPos, file);

                        cout << "File's data:\n";
                        for (int i = 0; i < file.data.size(); ++i)
                            cout << file.data[i];
                    }
                    else 
                        cout << "Use other apps to open this file.";

                    cout << "\nPress any key to escape";
                    keyboard = _getch();
                    //cin >> keyboard;
                    break;
                }
            }
        }
        else if (option == 3)
            return 3;
    }
}

string convertExtension(vector<BYTE> extension) {
    stringstream writer;
    for (int i = 0; i < extension.size(); i++) {
        BYTE tmp = extension[i];
        if (extension[i] > 0x5A) //extension[i] > 'Z'
            tmp = extension[i] - 0x20;     // In hoa vd: t -> T
        writer << tmp;
    }
    return writer.str();
}
