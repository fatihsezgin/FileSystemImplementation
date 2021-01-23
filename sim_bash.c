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

int main(){
    char command[32];
    while (1){
        printf("> ");
        scanf("%s",command);
        if(strcmp(command,"ls") == 0){
            ls_cur(curDirInodeNum, tab);
        }else if(strcmp(command,"cd") == 0){
            scanf("%s", name);
            cd(name, curDirInodeNum);
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

void cd(char name[28], int inum){
    char str1[28];
    char str2[28];
    int ret;
    bool flag = false;
    struct inodeStructure inostr;
    struct dirEntry entry;
    
    FILE *sfs = fopen("sfs.bin", "r");
    fseek(sfs, sizeof(struct SuperBlock), SEEK_SET);
    fseek(sfs, sizeof(struct inodeStructure)*inum, SEEK_CUR);
    fread(&inostr, sizeof(struct inodeStructure), 1, sfs);
    int i;
    for(i = 0; i < inostr.size / 32; i++){
        fseek(sfs,sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * 32 + 512 * inostr.dataBlockIndices[0] + i * sizeof(struct dirEntry), SEEK_SET);
        fread(&entry,sizeof(struct dirEntry),1,sfs);
        if(inostr.type == DIRECTORY){
            printf("name: %s type: %d\n", entry.name,inostr.type);
            strcpy(str1,entry.name);
            strcpy(str2, name);
            ret = strcmp(str1,str2);
            if(ret == 0) {
                previousDir = curDirInodeNum;
                curDirInodeNum = entry.inodeNumber;
                flag = true;
            }
            if(strcmp(name,"..") == 0){
                fseek(sfs, sizeof(struct SuperBlock), SEEK_SET);
                fseek(sfs, sizeof(struct inodeStructure)*inum, SEEK_CUR);
                fread(&inostr, sizeof(struct inodeStructure), 1, sfs);
                fseek(sfs,sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * 32 + 512 * inostr.dataBlockIndices[0] + 1 * sizeof(struct dirEntry), SEEK_SET);
                fread(&entry,sizeof(struct dirEntry),1,sfs);
                previousDir = entry.inodeNumber;
                curDirInodeNum = entry.inodeNumber;
                fclose(sfs);
                return;
            }
        }
    }
    fclose(sfs);
    if(!flag)
        printf("%s: No such file or directory\n", name);
}

void ls_cur(int inodeNum, int tab){
    struct inodeStructure inostr;
    struct dirEntry entry;
    FILE *sfs = fopen("sfs.bin", "r");
    // just past the super block, that is we are at the beginning
    // of the inode table where the first inode structure contains,
    // the root directory.
    fseek(sfs, sizeof(struct SuperBlock), SEEK_SET);
    fseek(sfs, sizeof(struct inodeStructure) * inodeNum, SEEK_CUR);
    fread(&inostr,sizeof(struct inodeStructure), 1, sfs);
    tab++;
    int i;
    for(i = 0; i < inostr.size / 32; i++){
        fseek(sfs,sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * 32 + 512 * inostr.dataBlockIndices[0] + i * sizeof(struct dirEntry), SEEK_SET);
        fread(&entry,sizeof(struct dirEntry),1,sfs);
        if(strcmp(entry.name,".") != 0 && strcmp(entry.name,"..") != 0){
            if(inostr.type == DIRECTORY){
                printf("%*s%s", tab * 5,"",entry.name);
                printf("\t\t%d\n",inodeNum);
                ls_cur(entry.inodeNumber, tab);
            }
        }
    }
    tab--;
    fclose(sfs);
}

void mkfile(int currentDir, char name[28]){
    struct SuperBlock superBlock;
    struct inodeStructure inostr;
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
    int inode;
    for (inode = 0; inode< 32; inode++){
        if(getBit(inode,superBlock.inodeBitmap) == 0){ // find the first zero bit in the SuperBlock inodeBitmap
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
    fwrite(&dirInode, sizeof(dirInode),1, sfs);

    struct inodeStructure testInode;
    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * inode, SEEK_SET);
    fread(&testInode, sizeof(testInode), 1, sfs);
    printf("test typee %d", testInode.type);

    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * 32 + 512 * (datablock * datablockNumber + datablockNumber), SEEK_SET);
    
    
    struct dirEntry file;
    fseek(sfs, sizeof(struct SuperBlock), SEEK_SET);
    fseek(sfs, sizeof(struct inodeStructure) * inum, SEEK_CUR);
    fread(&inostr, sizeof(struct inodeStructure), 1, sfs);
    strcpy(file.name,name);
    file.inodeNumber = inode;
    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * 32 + 512 * inostr.dataBlockIndices[0] + (inostr.size / 32) * sizeof(struct dirEntry), SEEK_SET);
    fwrite(&file, sizeof(file), 1, sfs);
    
    fseek(sfs,sizeof(struct SuperBlock),SEEK_SET);
    fseek(sfs,sizeof(struct inodeStructure) * inum,SEEK_CUR);
    fread(&inostr,sizeof(inostr),1,sfs);
    inostr.size += 32;

    fseek(sfs,sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * inum,SEEK_SET);
    fwrite(&inostr, sizeof(inostr), 1, sfs);

    superBlock.inodeBitmap= inode_bit;
    superBlock.dataBitmap[datablock] = datablockBitmap;
    fseek(sfs,0,SEEK_SET);
    fwrite(&superBlock,sizeof(superBlock),1,sfs);

    fclose(sfs);
}

void mkdir(int currentDir, char name[28]){
    struct SuperBlock superBlock;
    struct inodeStructure inostr;
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
    
    struct inodeStructure dirInode;
    dirInode.type = DIRECTORY; // 1
    dirInode.size = 32 *2;
    dirInode.dataBlockIndices[datablock] = datablockNumber;
    
    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * inode, SEEK_SET);
    fwrite(&dirInode, sizeof(dirInode),1, sfs);

    struct dirEntry dot;
    strcpy(dot.name, ".");
    dot.inodeNumber = inode;
    
    struct dirEntry dotdot;
    strcpy(dotdot.name,"..");
    dotdot.inodeNumber = inum;

    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * 32 + 512 * (datablock * datablockNumber + datablockNumber), SEEK_SET);
    fwrite(&dot, sizeof(dot), 1, sfs);
    fwrite(&dotdot, sizeof(dotdot), 1, sfs);
    
    struct dirEntry dir;
    fseek(sfs, sizeof(struct SuperBlock), SEEK_SET);
    fseek(sfs, sizeof(struct inodeStructure) * inum, SEEK_CUR);
    fread(&inostr, sizeof(struct inodeStructure), 1, sfs);
    strcpy(dir.name,name);
    dir.inodeNumber = inode;
    fseek(sfs, sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * 32 + 512 * inostr.dataBlockIndices[0]
     + (inostr.size / 32) * sizeof(struct dirEntry), SEEK_SET);
    fwrite(&dir, sizeof(dir), 1, sfs);
    
    fseek(sfs,sizeof(struct SuperBlock),SEEK_SET);
    fseek(sfs,sizeof(struct inodeStructure) * inum,SEEK_CUR);
    fread(&inostr,sizeof(inostr),1,sfs);
    inostr.size += 32;

    fseek(sfs,sizeof(struct SuperBlock) + sizeof(struct inodeStructure) * inum,SEEK_SET);
    fwrite(&inostr, sizeof(inostr), 1, sfs);

    superBlock.inodeBitmap= inode_bit;
    superBlock.dataBitmap[datablock] = datablockBitmap;
    fseek(sfs,0,SEEK_SET);
    fwrite(&superBlock,sizeof(superBlock),1,sfs);

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