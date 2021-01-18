#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "structures.h"


char name[28];
int curDirInodeNum = 0;

struct dir_Entry_for_mkfile{
    char name [28];  
    char biography [200];
    unsigned int inodeNumber;
};

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
void ls_cur(int inodeNum);
void mkdir(int currentDir, char name[28]);
void mkfile(int currentDir, char name[28]);
void get(int bitmap);
int getBit(int bitnum, int n);
int setBit(int bitnum, int bitmap);

int main(){
    char command[32];
    while (1){
        printf("> ");
        scanf("%s",command);
        if(strcmp(command,"ls") == 0){
            ls_cur(curDirInodeNum);
        }else if(strcmp(command, "mkdir") == 0){
            scanf("%s",name);
            mkdir(curDirInodeNum, name);
        }else if(strcmp(command, "mkfile")== 0){
            scanf("%s", name);
            mkfile(curDirInodeNum, name);
        }else if (strcmp(command,"lspfs")==0){
            printf("you typed lspfs");
        }else if (strcmp(command, "exit") == 0){
            exit(0);
        }
        
    }
    return 0;
}

void ls_cur(int inodeNum){
    struct inodeStructure root;
    struct dirEntry entry;
    FILE *sfs = fopen("sfs.bin", "r");
    // just past the super block, that is we are at the beginning
    // of the inode table where the first inode structure contains,
    // the root directory.
    fseek(sfs, sizeof(struct SuperBlock) + inodeNum * sizeof(struct inodeStructure), SEEK_SET);
    fread(&root,sizeof(struct inodeStructure), 1, sfs);
    int numOfEntries = root.size / sizeof(struct dirEntry);
    fseek(sfs, sizeof(struct SuperBlock) + NUMBEROFINODES * sizeof(struct inodeStructure), SEEK_SET);
    
    while (fread(&entry, sizeof(struct dirEntry), 1, sfs)){
        printf("%s %u \n", entry.name , entry.inodeNumber);
    }
}

void mkfile(int currentDir, char name[28]){
    struct SuperBlock superBlock;
    struct inodeStructure dir;
    struct dirEntry entry;
    int inum = currentDir; // currentDir is a global variable, in mkdir scope it is reassigned into inum variable. 
    
    FILE *sfs = fopen("sfs.bin","r+");
    fread(&superBlock, sizeof(struct SuperBlock), 1, sfs); //read the super block

    bool flag = false;
    int datablock = 0;
    int datablockNumber = 0;
    int datablockBitmap;

    for (datablock = 0; datablock < 10; datablock++){ 
        while(datablockNumber < 32){
            if(getBit(datablock,superBlock.dataBitmap[datablock]) == 0){// finds the first zero bit in the dataBitmap[dataBlock]
                flag = true;
                datablockBitmap = setBit(datablockNumber, superBlock.dataBitmap[datablock]); // dataBlockBitmap is carries the new SuperBlock.dataBitmap
                break;
            }
            datablockNumber++;
        }
        if(flag) break; // if the dataBitmap has updated break the loop.
    }
    datablock = 0;

    int inode_bit;
    int inode =0;
    for (inode =0; inode< 32; inode++){
        if(getBit(inode,superBlock.inodeBitmap) == 0){ // find the first zero bit in the SuperBlock inodeBitmap
            inode_bit = setBit(inode, superBlock.inodeBitmap); // update the SuperBlock inodeBitmap and named as inode_bit
            break;
        }
    }
    superBlock.inodeBitmap = inode_bit; // updating the inodeBitmap in SuperBlock 
    superBlock.dataBitmap[0] = datablockBitmap; // updating the dataBlockBitmap in SuperBlock
    fseek(sfs, 0, SEEK_SET);
    fwrite(&superBlock, sizeof(struct SuperBlock), 1, sfs); // write the updated SuperBlock

    struct inodeStructure dirInode; 
    dirInode.type = DIRECTORY; // since the operation is mkdir the type is DIRECTORY
    dirInode.size = sizeof(struct   dirEntry) * 2; // memory allocation for . and ..
    dirInode.dataBlockIndices[datablock] = datablockNumber; // allocates the dataBlockIndises
    
    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * inode, SEEK_SET); // jump to the inodeStructure that the new directory will be written
    fwrite(&dirInode, sizeof(dirInode), 1, sfs); // write the dirInode to the Inode-Table


    struct dir_Entry_for_mkfile file;
	strcpy(file.name, "fatih");
    strcpy(file.biography, "Hello my dear friends. I am Fatih. I am 22 years old and I am a student of istanbul aydin university");
	file.inodeNumber = inode; // points the current directory inode

    // Jumps to the beginning of the datablock
    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * NUMBEROFINODES, SEEK_SET );
    fwrite(&file, sizeof(struct dir_Entry_for_mkfile), 1, sfs);

    // This is just for testing purpose.
    struct dir_Entry_for_mkfile testFile;
    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * NUMBEROFINODES, SEEK_SET );
    while(fread(&testFile, sizeof(struct dir_Entry_for_mkfile),1 ,sfs) == 1){
        printf("%s %u\n", testFile.name, testFile.inodeNumber);
    }
    
    fclose(sfs);
}

void mkdir(int currentDir, char name[28]){
    struct SuperBlock superBlock;
    struct inodeStructure dir;
    struct dirEntry entry;
    int inum = currentDir; // currentDir is a global variable, in mkdir scope it is reassigned into inum variable. 
    
    FILE *sfs = fopen("sfs.bin","r+");
    fread(&superBlock, sizeof(struct SuperBlock), 1, sfs); //read the super block

    bool flag = false;
    int datablock = 0; //db_array
    int datablockNumber = 0; //db_num
    int datablockBitmap; //db_bit

    for (datablock = 0; datablock < 10; datablock++){ 
        printf("buraya girdi\n");
        while(datablockNumber < 32){
            printf("buraya da girdi\n");
            if(getBit(datablockNumber,superBlock.dataBitmap[datablock]) == 0){// finds the first zero bit in the dataBitmap[dataBlock]
                flag = true;
                datablockBitmap = setBit(datablockNumber, superBlock.dataBitmap[datablock]);
                superBlock.dataBitmap[datablock] = datablockBitmap; // updating the dataBlockBitmap in SuperBlock
                datablockBitmap = setBit(datablockNumber + 1, superBlock.dataBitmap[datablock]);
                superBlock.dataBitmap[datablock] = datablockBitmap; // updating the dataBlockBitmap in SuperBlock
                datablockBitmap = setBit(datablockNumber + 2, superBlock.dataBitmap[datablock]);
                superBlock.dataBitmap[datablock] = datablockBitmap; // updating the dataBlockBitmap in SuperBlock
                 // dataBlockBitmap is carries the new SuperBlock.dataBitmap
                printf("after set data bitmap: \n");
                get(datablockBitmap);
                break;
            }
            datablockNumber++;
        }
        if(flag) break; // if the dataBitmap has updated break the loop.
    }
    printf("%d  %d --> datablock, datablockNumber\n",datablock,datablockNumber);

    int inode_bit;
    int inode =0;
    for (inode =0; inode< 32; inode++){
        if(getBit(inode,superBlock.inodeBitmap) == 0){ // find the first zero bit in the SuperBlock inodeBitmap
            inode_bit = setBit(inode, superBlock.inodeBitmap); // update the SuperBlock inodeBitmap and named as inode_bit
            printf("after set inode bitmap: \n");
            get(inode_bit);
            break;
        }
    }
    
    superBlock.inodeBitmap = inode_bit; // updating the inodeBitmap in SuperBlock 
    
    fseek(sfs, 0, SEEK_SET);
    fwrite(&superBlock, sizeof(struct SuperBlock), 1, sfs); // write the updated SuperBlock

    struct inodeStructure dirInode; 
    dirInode.type = DIRECTORY; // since the operation is mkdir the type is DIRECTORY
    dirInode.size = sizeof(struct dirEntry) * 2; // memory allocation for . and ..
    dirInode.dataBlockIndices[datablock] = datablockNumber; // allocates the dataBlockIndises
    
    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * inode, SEEK_SET); // jump to the inodeStructure that the new directory will be written
    fwrite(&dirInode, sizeof(dirInode), 1, sfs); // write the dirInode to the Inode-Table

    struct dirEntry dot;
	strcpy(dot.name, ".");
	dot.inodeNumber = inode; // points the current directory inode
    printf("dot inode : %d\n",inode );

	struct dirEntry dotdot;
	strcpy(dotdot.name, "..");
	dotdot.inodeNumber = inum; //points the previous directory inode
    printf("dotdot inode %d\n", inum);

    // Jumps to the beginning of the datablock
    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * NUMBEROFINODES + datablockNumber * sizeof(struct dirEntry)  , SEEK_SET);
    
    
    // this is the initialization of the directory
    struct dirEntry directory;
    strcpy(directory.name, name); //directory name copied with the name given with command.
    directory.inodeNumber = inode; //directyor inode variable should be same with the inode number which has been found previously, and indicates the first zero bit in inodeBitmap
    fwrite(&directory, sizeof(struct dirEntry), 1, sfs); // writing the directory to the sfs.bin file
    fwrite(&dot, sizeof(struct dirEntry), 1, sfs);
    fwrite(&dotdot, sizeof(struct dirEntry), 1, sfs);
    // This is just for testing purpose.
    struct dirEntry testDir;
    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * NUMBEROFINODES, SEEK_SET );
    while(fread(&testDir, sizeof(struct dirEntry),1 ,sfs) == 1){
        printf("%s %u\n", testDir.name, testDir.inodeNumber);
    }
    fclose(sfs);
}


void get(int bitmap) { 
    int count = 0;
    for (int i = 31; i >= 0; i--) { 
        count++;
        int k = bitmap >> i; 
        if (k & 1) 
            printf("1"); 
        else
            printf("0"); 
    }
    printf("\n");
}
int getBit(int bitnum, int n) { return (n >> bitnum) & 1; } 
int setBit(int bitnum, int bitmap) { return bitmap ^ (1 << bitnum); } // XOR the bitmap and bitnum. XOR sets the bit if and only if the operands are different.