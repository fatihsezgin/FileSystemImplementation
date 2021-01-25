#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "structures.h"

char name[28];
int tab = 0;            // for visualization while the printing the directory entries
int curDirInodeNum = 0; // global variable that keeps the current inode number
int previousDir = 0;    // global variable that keeps the previous inode number

void ls_cur(int inodeNum, int tab);         // recursively prints the dirEntries in the console.
void cd(char name[28], int inum);           // changes the directory to given directory name
void mkdir(int currentDir, char name[28]);  // creates a new directory with given name
void mkfile(int currentDir, char name[28]); // creates a new file with given name
void get(int bitmap);                       // prints the given bitmap to the console.
int getBit(int bitnum, int bitmap);         // get the given bitmap's given bitnum
int setBit(int bitnum, int bitmap);         // sets the given bitmap's given bitnum to 1

int main()
{
    char command[32];
    while (1)
    {
        printf("> ");
        scanf("%s", command);
        if (strcmp(command, "ls") == 0)
        {
            ls_cur(curDirInodeNum, tab);
        }
        else if (strcmp(command, "cd") == 0)
        {
            scanf("%s", name);
            cd(name, curDirInodeNum);
        }
        else if (strcmp(command, "mkdir") == 0)
        {
            scanf("%s", name);
            mkdir(curDirInodeNum, name);
        }
        else if (strcmp(command, "mkfile") == 0)
        {
            scanf("%s", name);
            mkfile(curDirInodeNum, name);
        }
        else if (strcmp(command, "lspfs") == 0)
        {
            printf("you typed lspfs");
        }
        else if (strcmp(command, "exit") == 0)
        {
            exit(0);
        }
    }
    return 0;
}

/********************************************/ /**
* The cd function changes the directory if there is a directory within given
* @param name. If the searching fails, it prints the related error string.
*
* @param name the name that will be searched in the directory.
* @param inum same as the global curDirInodeNum.
* 
* @returns none
***********************************************/
void cd(char name[28], int inum)
{
    char str1[28];
    char str2[28];
    int ret;
    bool flag = false;
    struct inodeStructure inostr;
    struct dirEntry entry;

    // opening the sfs.bin file in the read mode
    FILE *sfs = fopen("sfs.bin", "r");
    // jumping the corresponding inode structure
    fseek(sfs, sizeof(struct SuperBlock), SEEK_SET);
    fseek(sfs, sizeof(struct inodeStructure) * inum, SEEK_CUR);
    fread(&inostr, sizeof(struct inodeStructure), 1, sfs);
    // end of jumping inode structure

    int i;
    // there is (inostr.size / 32) times directory in the inodeStructure which called inostr
    for (i = 0; i < inostr.size / 32; i++)
    {
        // jumping through the data block section, that stores the directories.
        fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * 32 + 512 * inostr.dataBlockIndices[0] + i * sizeof(struct dirEntry), SEEK_SET);
        fread(&entry, sizeof(struct dirEntry), 1, sfs);

        printf("name: %s type: %d\n", entry.name, inostr.type);
        strcpy(str1, entry.name);
        strcpy(str2, name);
        ret = strcmp(str1, str2); // comparing the name that is given as parameter and the entry name in the inostr
        if (ret == 0)
        {
            fseek(sfs, sizeof(struct SuperBlock), SEEK_SET);                         // skip the superblock
            fseek(sfs, sizeof(struct inodeStructure) * entry.inodeNumber, SEEK_CUR); // skip the inodeStructure * entry.inodeNumber
            fread(&inostr, sizeof(struct inodeStructure), 1, sfs);                   // read the inodeStructure
            printf("name: %s type: %d inode_number %d\n", entry.name, inostr.type, entry.inodeNumber);
            if (inostr.type == DIRECTORY)
            {
                previousDir = curDirInodeNum;       // set previous directory as current directory inode number
                curDirInodeNum = entry.inodeNumber; // set current directory inode number as entry.inodeNumber
                flag = true;                        //set the flag is true to control a file or folder is existed or not or check the type of entry is file or directory.
            }
        }
        if (strcmp(name, "..") == 0) // if the user input is (..) which indicates the previous directory
        {
            // pass the super block, we are beginning of the inodeStructure
            // that contains the entries in current directory
            fseek(sfs, sizeof(struct SuperBlock), SEEK_SET);
            fseek(sfs, sizeof(struct inodeStructure) * inum, SEEK_CUR);
            fread(&inostr, sizeof(struct inodeStructure), 1, sfs);
            // end of the jumping to the inodeStructure

            // pass the whole super and inodeStructure, and pass through before the dirEntry we are currently in
            fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * 32 + 512 * inostr.dataBlockIndices[0] + 1 * sizeof(struct dirEntry), SEEK_SET);
            // read the entry
            fread(&entry, sizeof(struct dirEntry), 1, sfs);
            previousDir = entry.inodeNumber;
            curDirInodeNum = entry.inodeNumber;
            fclose(sfs);
            return;
        }
    }
    fclose(sfs);
    if (!flag)
        printf("%s: No such file or directory\n", name);
}
/********************************************/ /**
* This function prints the directory items, in our case dirEntries recursively
*
* @param inodeNum represents the inodeNumber that user is currently in
* @param tab is used for visualization in the ls command
* 
* @returns none
***********************************************/
void ls_cur(int inodeNum, int tab)
{
    struct inodeStructure inostr;
    struct dirEntry entry;
    FILE *sfs = fopen("sfs.bin", "r");
    // just past the super block, that is we are at the beginning
    // of the inode table where the inode structure contains,
    // the current directory.
    fseek(sfs, sizeof(struct SuperBlock), SEEK_SET);
    fseek(sfs, sizeof(struct inodeStructure) * inodeNum, SEEK_CUR);
    fread(&inostr, sizeof(struct inodeStructure), 1, sfs);
    // end of the jumping
    tab++;
    int i;
    // there are (inostr.size / 32) items in the directory
    for (i = 0; i < inostr.size / 32; i++)
    {
        //iterating through the dirEntry
        fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * 32 + 512 * inostr.dataBlockIndices[0] + i * sizeof(struct dirEntry), SEEK_SET);

        fread(&entry, sizeof(struct dirEntry), 1, sfs);
        // if the entry name is not equal to (.) and (..)
        if (strcmp(entry.name, ".") != 0 && strcmp(entry.name, "..") != 0)
        {
            if (inostr.type == DIRECTORY) // if the entry type is directory
            {
                printf("%*s%s", tab * 5, "", entry.name); // print the entry name
                printf("\t\t%d\n", inodeNum);             // print the inode number
                ls_cur(entry.inodeNumber, tab);           // function call for the recursive
            }
        }
    }
    tab--;
    fclose(sfs); // closing the file
}

/********************************************/ /**
* This function creates a file with given name to current directory
*
* @param currentDir represents the inodeNumber that user is currently in
* @param name directory name
* 
* @returns none
***********************************************/
void mkfile(int currentDir, char name[28])
{
    struct SuperBlock superBlock;
    struct inodeStructure inostr;
    struct dirEntry entry;
    int inum = currentDir; // currentDir is a global variable, in mkdir scope it is reassigned into inum variable.

    FILE *sfs = fopen("sfs.bin", "r+");
    fread(&superBlock, sizeof(struct SuperBlock), 1, sfs); //read the super block

    bool flag = false;
    int datablock = 0;       //datablock represents the index of the non-used datablock
    int datablockNumber = 0; //datablockNumber represents the first non-used bit in the datablock.
    int datablockBitmap;     // it will used for the replacing the datablockbitmap with the newly generated one.

    // datablock will be iterating through datablock.size which is 10.
    for (datablock = 0; datablock < 10; datablock++)
    {
        // iterating the datablock number
        while (datablockNumber < 32)
        {
            // get the first non-used bit in the datablock index in the superblock.databitmap array
            if (getBit(datablockNumber, superBlock.dataBitmap[datablock]) == 0)
            { // finds the first zero bit in the dataBitmap[dataBlock]
                flag = true;
                datablockBitmap = setBit(datablockNumber, superBlock.dataBitmap[datablock]); // set the bit to 1
                get(datablockBitmap);                                                        // prints the bitmap for testing purposes
                break;
            }
            datablockNumber++; // if the datablockNumber is not available
        }
        if (flag)
            break; // if the dataBitmap has updated break the loop.
    }
    printf("%d  %d --> datablock, datablockNumber\n", datablock, datablockNumber);

    int inode_bit; // inodebitmap
    int inode = 0; // index of the inodebitmap
    // iterate through the inodebitmap
    for (inode = 0; inode < 32; inode++)
    {
        if (getBit(inode, superBlock.inodeBitmap) == 0)        // find the first available index
        {                                                      // find the first zero bit in the SuperBlock inodeBitmap
            inode_bit = setBit(inode, superBlock.inodeBitmap); // update the SuperBlock inodeBitmap and named as inode_bit
            get(inode_bit);                                    // print the inode for testing purposes
            break;
        }
    }

    struct inodeStructure dirInode;                         // initialize inodeStructure
    dirInode.type = REG_FILE;                               // 0 // set the type of entry as regular file
    dirInode.size = 64;                                     //set the size of regular file
    dirInode.dataBlockIndices[datablock] = datablockNumber; // set the datablock number

    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * inode, SEEK_SET);
    // skip the file as the size of superblock and inodeStructure multiplied by inode
    fwrite(&dirInode, sizeof(dirInode), 1, sfs);
    //write the dirInode in the file.
    struct inodeStructure testInode; //initialize inodeStructure for testing
    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * inode, SEEK_SET);
    // skip file as the size of superblock and inodeStructure multiplied by inode
    fread(&testInode, sizeof(testInode), 1, sfs);
    // read the testInode from the file
    printf("test typee %d", testInode.type);

    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * 32 + 512 * (datablock * datablockNumber + datablockNumber), SEEK_SET);

    struct dirEntry file; // initializing the file
    // jumping the corresponding inodeStructure
    fseek(sfs, sizeof(struct SuperBlock), SEEK_SET);
    fseek(sfs, sizeof(struct inodeStructure) * inum, SEEK_CUR);
    fread(&inostr, sizeof(struct inodeStructure), 1, sfs);
    // end of jump
    strcpy(file.name, name);  // sets the file name to the given name
    file.inodeNumber = inode; // sets the inodeNumber to founded inode in the databitmap

    // jumping the beginning of the data section for te writing dirEntry
    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * 32 + 512 * inostr.dataBlockIndices[0] + (inostr.size / 32) * sizeof(struct dirEntry), SEEK_SET);
    fwrite(&file, sizeof(file), 1, sfs); // writing the dir

    // jump the inode structure for updating the inode structure size
    fseek(sfs, sizeof(struct SuperBlock), SEEK_SET);
    fseek(sfs, sizeof(struct inodeStructure) * inum, SEEK_CUR);
    fread(&inostr, sizeof(inostr), 1, sfs);
    inostr.size += 32; // inode structure size is updated

    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * inum, SEEK_SET);
    fwrite(&inostr, sizeof(inostr), 1, sfs); // writing the updated inode structure

    // updating the both inode and data bitmaps
    superBlock.inodeBitmap = inode_bit;
    superBlock.dataBitmap[datablock] = datablockBitmap;
    fseek(sfs, 0, SEEK_SET);
    fwrite(&superBlock, sizeof(superBlock), 1, sfs); // updating the both inode and data bitmaps

    fclose(sfs); // updating the both inode and data bitmaps
}
/********************************************/ /**
* This function creates a new directory with the given @param name
* in the given @param currentDir location.
*
* @param currentDir  represents the inodeNumber that user is currently in
* @param name name of the directory
* 
* @returns none
***********************************************/
void mkdir(int currentDir, char name[28])
{
    struct SuperBlock superBlock;
    struct inodeStructure inostr;
    struct dirEntry entry;
    int inum = currentDir; // currentDir is a global variable, in mkdir scope it is reassigned into inum variable.

    FILE *sfs = fopen("sfs.bin", "r+");
    fread(&superBlock, sizeof(struct SuperBlock), 1, sfs); //read the super block

    bool flag = false;
    int datablock = 0;       //datablock represents the index of the non-used datablock
    int datablockNumber = 0; //datablockNumber represents the first non-used bit in the datablock.
    int datablockBitmap;     // it will used for the replacing the datablockbitmap with the newly generated one.

    // datablock will be iterating through datablock.size which is 10.
    for (datablock = 0; datablock < 10; datablock++)
    {
        // iterating the datablock number
        while (datablockNumber < 32)
        {
            // get the first non-used bit in the datablock index in the superblock.databitmap array
            if (getBit(datablockNumber, superBlock.dataBitmap[datablock]) == 0)
            { // finds the first zero bit in the dataBitmap[dataBlock]
                flag = true;
                datablockBitmap = setBit(datablockNumber, superBlock.dataBitmap[datablock]); // set the bit to 1
                get(datablockBitmap);                                                        // prints the bitmap for testing purposes
                break;
            }
            datablockNumber++; // if the datablockNumber is not available
        }
        if (flag)
            break; // if the dataBitmap has updated break the loop.
    }
    printf("%d  %d --> datablock, datablockNumber\n", datablock, datablockNumber);

    int inode_bit; // inodebitmap
    int inode = 0; // index of the inodebitmap
    // iterate through the inodebitmap
    for (inode = 0; inode < 32; inode++)
    {
        if (getBit(inode, superBlock.inodeBitmap) == 0)        // find the first available index
        {                                                      // find the first zero bit in the SuperBlock inodeBitmap
            inode_bit = setBit(inode, superBlock.inodeBitmap); // update the SuperBlock inodeBitmap and named as inode_bit
            get(inode_bit);                                    // print the inode for testing purposes
            break;
        }
    }

    struct inodeStructure dirInode;                         // Initiliazing inodeStructure
    dirInode.type = DIRECTORY;                              // set the type of directory to 1
    dirInode.size = 32 * 2;                                 // set the size 64 for (.) and (..)
    dirInode.dataBlockIndices[datablock] = datablockNumber; // set the datablock array

    // jump the inode structure that directory will be written
    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * inode, SEEK_SET);
    // write the dirInode
    fwrite(&dirInode, sizeof(dirInode), 1, sfs);

    // writing the dot
    struct dirEntry dot;
    strcpy(dot.name, ".");
    dot.inodeNumber = inode;

    // writing the dot dot
    struct dirEntry dotdot;
    strcpy(dotdot.name, "..");
    dotdot.inodeNumber = inum;

    // jumping the beginning of the data block section
    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * 32 + 512 * (datablock * datablockNumber + datablockNumber), SEEK_SET);
    // writing the dot and dot dot
    fwrite(&dot, sizeof(dot), 1, sfs);
    fwrite(&dotdot, sizeof(dotdot), 1, sfs);

    // initializing a dirEntry struct to fill the directory
    struct dirEntry dir;
    // jumping the corresponding inodeStructure
    fseek(sfs, sizeof(struct SuperBlock), SEEK_SET);
    fseek(sfs, sizeof(struct inodeStructure) * inum, SEEK_CUR);
    fread(&inostr, sizeof(struct inodeStructure), 1, sfs);
    // end of the jumping
    strcpy(dir.name, name);  // sets the dir name to the given name
    dir.inodeNumber = inode; // sets the inodeNumber to founded inode in the databitmap
    // jumping the beginning of the data section for te writing dirEntry
    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * 32 + 512 * inostr.dataBlockIndices[0] + (inostr.size / 32) * sizeof(struct dirEntry), SEEK_SET);
    fwrite(&dir, sizeof(dir), 1, sfs); // writing the dir

    // jump the inode structre for updating the inode structure size
    fseek(sfs, sizeof(struct SuperBlock), SEEK_SET);
    fseek(sfs, sizeof(struct inodeStructure) * inum, SEEK_CUR);
    fread(&inostr, sizeof(inostr), 1, sfs);
    inostr.size += 32; // inode structure size is updated

    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * inum, SEEK_SET);
    fwrite(&inostr, sizeof(inostr), 1, sfs); // writing the updated inode structure

    // updating the both inode and data bitmaps
    superBlock.inodeBitmap = inode_bit;
    superBlock.dataBitmap[datablock] = datablockBitmap;
    fseek(sfs, 0, SEEK_SET);
    fwrite(&superBlock, sizeof(superBlock), 1, sfs); // writing the updated super block
    fclose(sfs);                                     // closing the file
}

/********************************************/ /**
* Prints the bitmap
*
* @param bitmap to be printed.
* 
* @returns None
***********************************************/
void get(int bitmap)
{
    int count = 0;
    for (int i = 31; i >= 0; i--)
    {
        count++;
        int k = bitmap >> i;
        if (k & 1)
            printf("1");
        else
            printf("0");
    }
    printf("\n");
}

/********************************************/ /**
* Right shift the bitmap, then perform and operand with 1.
*
* @param bitnum source bitmap
* @param bitmap index of the compared int
* 
* @returns first zero bit index.
***********************************************/
int getBit(int bitnum, int bitmap)
{
    return (bitmap >> bitnum) & 1;
}
/********************************************/ /**
* XOR the bitmap and bitnum. XOR sets the bit if and only if the operands are different.
*
* @param bitnum index of the bitmap
* @param bitmap source bitmap to be updated
* 
* @returns first zero bit index.
***********************************************/
int setBit(int bitnum, int bitmap)
{
    return bitmap ^ (1 << bitnum);
}