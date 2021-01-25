#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "structures.h"

char name[28];
int tab = 0;
int curDirInodeNum = 0;
int previousDir = 0;

// First bash utility we will write is 'ls'.
// Because we will use it for debugging as well.
//

// ls function will print all the directory/files in the file system
// with proper indentation so that we can "see" the picture of the whole
// file system.
//void ls();

// this will only print the current directory contents
// every directory/file has it unique inode number
// which used as an index into the inode table.
void ls_cur(int inodeNum, int tab);
void cd(char name[28], int inum);
void mkdir(int currentDir, char name[28]);
void mkfile(int currentDir, char name[28]);
void get(int bitmap);
int getBit(int bitnum, int n);
int setBit(int bitnum, int bitmap);

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
        // if the type is directory
        //if (inostr.type == DIRECTORY)
        //{
        printf("name: %s type: %d\n", entry.name, inostr.type);
        strcpy(str1, entry.name);
        strcpy(str2, name);
        ret = strcmp(str1, str2); // comparing the name that is given as parameter and the entry name in the inostr
        if (ret == 0)
        {
            fseek(sfs, sizeof(struct SuperBlock), SEEK_SET);
            fseek(sfs, sizeof(struct inodeStructure)*entry.inodeNumber, SEEK_CUR);
            fread(&inostr, sizeof(struct inodeStructure), 1, sfs);
            printf("name: %s type: %d inode_number %d\n", entry.name,inostr.type,entry.inodeNumber);
            if(inostr.type == DIRECTORY){
                previousDir = curDirInodeNum;
                curDirInodeNum = entry.inodeNumber;
                flag = true;
            }
            // if both names are equal
            // previousDir = curDirInodeNum; // to store the previous directory
            // directory will be change and the global pointer of the directory, should be equal with the entry.inodeNumber
            //curDirInodeNum = entry.inodeNumber;
            //flag = true;
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
            fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * 32 
                            + 512 * inostr.dataBlockIndices[0] + 1 * sizeof(struct dirEntry), SEEK_SET);
            // read the entry
            fread(&entry, sizeof(struct dirEntry), 1, sfs);
            previousDir = entry.inodeNumber; // change the pointer of the 
            curDirInodeNum = entry.inodeNumber;
            fclose(sfs);
            return;
        }
        //}
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
                printf("\t\t%d\n", inodeNum); // print the inode number
                ls_cur(entry.inodeNumber, tab); // function call for the recursive
            }
        }
    }
    tab--;
    fclose(sfs);
}

void mkfile(int currentDir, char name[28])
{
    struct SuperBlock superBlock;
    struct inodeStructure inostr;
    struct dirEntry entry;
    int inum = currentDir; // currentDir is a global variable, in mkdir scope it is reassigned into inum variable.

    FILE *sfs = fopen("sfs.bin", "r+");
    fread(&superBlock, sizeof(struct SuperBlock), 1, sfs); //read the super block

    bool flag = false;
    int datablock = 0;       //db_array
    int datablockNumber = 0; //db_num
    int datablockBitmap;     //db_bit

    for (datablock = 0; datablock < 10; datablock++)
    {
        printf("buraya girdi\n");
        while (datablockNumber < 32)
        {
            printf("buraya da girdi\n");
            if (getBit(datablockNumber, superBlock.dataBitmap[datablock]) == 0)
            { // finds the first zero bit in the dataBitmap[dataBlock]
                flag = true;
                datablockBitmap = setBit(datablockNumber, superBlock.dataBitmap[datablock]);
                printf("after set data bitmap: \n");
                get(datablockBitmap);
                break;
            }
            datablockNumber++;
        }
        if (flag)
            break; // if the dataBitmap has updated break the loop.
    }
    printf("%d  %d --> datablock, datablockNumber\n", datablock, datablockNumber);

    int inode_bit;
    int inode;
    for (inode = 0; inode < 32; inode++)
    {
        if (getBit(inode, superBlock.inodeBitmap) == 0)
        {                                                      // find the first zero bit in the SuperBlock inodeBitmap
            inode_bit = setBit(inode, superBlock.inodeBitmap); // update the SuperBlock inodeBitmap and named as inode_bit
            printf("after set inode bitmap: \n");
            get(inode_bit);
            break;
        }
    }

    struct inodeStructure dirInode;
    dirInode.type = REG_FILE; // 0
    dirInode.size = 64;
    dirInode.dataBlockIndices[datablock] = datablockNumber;

    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * inode, SEEK_SET);
    fwrite(&dirInode, sizeof(dirInode), 1, sfs);

    struct inodeStructure testInode;
    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * inode, SEEK_SET);
    fread(&testInode, sizeof(testInode), 1, sfs);
    printf("test typee %d", testInode.type);

    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * 32 + 512 * (datablock * datablockNumber + datablockNumber), SEEK_SET);

    struct dirEntry file;
    fseek(sfs, sizeof(struct SuperBlock), SEEK_SET);
    fseek(sfs, sizeof(struct inodeStructure) * inum, SEEK_CUR);
    fread(&inostr, sizeof(struct inodeStructure), 1, sfs);
    strcpy(file.name, name);
    file.inodeNumber = inode;
    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * 32 + 512 * inostr.dataBlockIndices[0] + (inostr.size / 32) * sizeof(struct dirEntry), SEEK_SET);
    fwrite(&file, sizeof(file), 1, sfs);

    fseek(sfs, sizeof(struct SuperBlock), SEEK_SET);
    fseek(sfs, sizeof(struct inodeStructure) * inum, SEEK_CUR);
    fread(&inostr, sizeof(inostr), 1, sfs);
    inostr.size += 32;

    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * inum, SEEK_SET);
    fwrite(&inostr, sizeof(inostr), 1, sfs);

    superBlock.inodeBitmap = inode_bit;
    superBlock.dataBitmap[datablock] = datablockBitmap;
    fseek(sfs, 0, SEEK_SET);
    fwrite(&superBlock, sizeof(superBlock), 1, sfs);

    fclose(sfs);
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
    int datablock = 0; //datablock represents the index of the non-used datablock
    int datablockNumber = 0; //datablockNumber represents the first non-used bit in the datablock.
    int datablockBitmap; // it will used for the replacing the datablockbitmap with the newly generated one.

    // datablock will be iterating through datablock.size which is 10.
    for (datablock = 0; datablock < 10; datablock++)
    {
        // iterating the int
        while (datablockNumber < 32)
        {
            printf("buraya da girdi\n");
            if (getBit(datablockNumber, superBlock.dataBitmap[datablock]) == 0)
            { // finds the first zero bit in the dataBitmap[dataBlock]
                flag = true;
                datablockBitmap = setBit(datablockNumber, superBlock.dataBitmap[datablock]);
                printf("after set data bitmap: \n");
                get(datablockBitmap);
                break;
            }
            datablockNumber++;
        }
        if (flag)
            break; // if the dataBitmap has updated break the loop.
    }
    printf("%d  %d --> datablock, datablockNumber\n", datablock, datablockNumber);

    int inode_bit;
    int inode = 0;
    for (inode = 0; inode < 32; inode++)
    {
        if (getBit(inode, superBlock.inodeBitmap) == 0)
        {                                                      // find the first zero bit in the SuperBlock inodeBitmap
            inode_bit = setBit(inode, superBlock.inodeBitmap); // update the SuperBlock inodeBitmap and named as inode_bit
            printf("after set inode bitmap: \n");
            get(inode_bit);
            break;
        }
    }

    struct inodeStructure dirInode;
    dirInode.type = DIRECTORY; // 1
    dirInode.size = 32 * 2;
    dirInode.dataBlockIndices[datablock] = datablockNumber;

    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * inode, SEEK_SET);
    fwrite(&dirInode, sizeof(dirInode), 1, sfs);

    struct dirEntry dot;
    strcpy(dot.name, ".");
    dot.inodeNumber = inode;

    struct dirEntry dotdot;
    strcpy(dotdot.name, "..");
    dotdot.inodeNumber = inum;

    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * 32 + 512 * (datablock * datablockNumber + datablockNumber), SEEK_SET);
    fwrite(&dot, sizeof(dot), 1, sfs);
    fwrite(&dotdot, sizeof(dotdot), 1, sfs);

    struct dirEntry dir;
    fseek(sfs, sizeof(struct SuperBlock), SEEK_SET);
    fseek(sfs, sizeof(struct inodeStructure) * inum, SEEK_CUR);
    fread(&inostr, sizeof(struct inodeStructure), 1, sfs);
    strcpy(dir.name, name);
    dir.inodeNumber = inode;
    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * 32 + 512 * inostr.dataBlockIndices[0] + (inostr.size / 32) * sizeof(struct dirEntry), SEEK_SET);
    fwrite(&dir, sizeof(dir), 1, sfs);

    fseek(sfs, sizeof(struct SuperBlock), SEEK_SET);
    fseek(sfs, sizeof(struct inodeStructure) * inum, SEEK_CUR);
    fread(&inostr, sizeof(inostr), 1, sfs);
    inostr.size += 32;

    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * inum, SEEK_SET);
    fwrite(&inostr, sizeof(inostr), 1, sfs);

    superBlock.inodeBitmap = inode_bit;
    superBlock.dataBitmap[datablock] = datablockBitmap;
    fseek(sfs, 0, SEEK_SET);
    fwrite(&superBlock, sizeof(superBlock), 1, sfs);

    fclose(sfs);
}

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
// Right shift the bitmap, then perform and operand with 1. It returns the first zero bit.
int getBit(int bitnum, int n)
{
    return (n >> bitnum) & 1;
}
// XOR the bitmap and bitnum. XOR sets the bit if and only if the operands are different.
int setBit(int bitnum, int bitmap)
{
    return bitmap ^ (1 << bitnum);
}