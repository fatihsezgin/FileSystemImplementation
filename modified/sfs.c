#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const int REG_FILE = 0;
const int DIRECTORY = 1;
const int NUMOFINODES = 32;
//if I use a byte, how many inode structures can I have? 8 inode structures. Let's have an int (32 bits) for the size of the inode bitmap. Therefore, we can have at most 32 files/directories in the simple file system.
//Let's assume that each file/directory can have at most 10 data blocks and each data block has 512 btyes in it.
//We need 10 * 32 = 320 is the number of data blocks. We need 320 bits for the data blocks. 
struct super_block{
    int inode_bitmap;
    int data_bitmap[10];
};
struct inode_st{
    int type;
    int size;
    int data_block_indices[10];
};
//in my mind a directory entry is 
//an atomic pair (do not separate them). Therefore, I do put them together in the code together.
struct dir_ent{
  char name [28];  
  unsigned int inode_no;
};

int show_bit_in_sb_inode_bitmap(struct super_block sb){
	unsigned bits[sizeof(sb.inode_bitmap) * 8];
	unsigned dividend = sb.inode_bitmap;
	unsigned divider = 2;
	int i = 0;
	int counter = sizeof(sb.inode_bitmap) * 8 ;
	while(counter > 0){
		unsigned result = dividend%divider;
		bits[i] = result;
		dividend = dividend/divider;
		counter--;
		i++;
	}
	i = (sizeof(sb.inode_bitmap) * 8) - 1;
	for(i;i >=0 ;i--){
		printf("%d",bits[i]);
	}
	printf("\n");
}

void initialize_sb(struct super_block sb){
    sb.inode_bitmap = 0;
    int i;
    for(i = 0; i < 10; i++){
        sb.data_bitmap[i] = 0;
    }
}
int main() {
    printf("Hello, world!\n");
    struct super_block sb;
    initialize_sb(sb);
    
    struct inode_st root;
    root.type = DIRECTORY;
    root.size = sizeof(struct dir_ent)*2;
    int i;
    for(i = 0; i < 10; i++)
        root.data_block_indices[i] = 0;
    
    //mark superblock inode and data
    //bitmap because an inode and a data
    //block is being allocated for the
    //root directory. Mark (put 1) 
    //in the inode bitmap (bit 0) and
    //data bitmap (index 0, bit 0) to 
    //denote usage
    sb.inode_bitmap = 1;
    sb.data_bitmap[0] = 3;
    
    printf("size of dir. entry: %lu\n",sizeof(struct dir_ent));
    
    struct dir_ent dot;
    strcpy(dot.name, ".");
    dot.inode_no = 0;
    struct dir_ent dotdot;
    strcpy(dotdot.name, "..");
    dotdot.inode_no = 0;
    
    printf("dotdot.name: %s\n", dotdot.name);
    
    FILE *sfs = fopen("sfs.bin", "w+");
    fwrite(&sb,sizeof(sb),1, sfs);
	//write the first inode structure (for the root) to the file	
	//(sfs.bin, remember this file contains your file system (its
	//meta data and actual data).
	//Jump(!) to the beginning of the data block to write the two 
	//entries (dot and dotdot) the first data block.
	fseek(sfs,sizeof(struct super_block),SEEK_SET);
	fwrite(&root,sizeof(root),1,sfs);
	
	show_bit_in_sb_inode_bitmap(sb);
	
	fseek(sfs,sizeof(struct super_block) + NUMOFINODES * sizeof(struct inode_st),SEEK_SET);
	fwrite(&dot,sizeof(dot),1,sfs);
	fwrite(&dotdot, sizeof(dotdot),1,sfs);
	fclose(sfs);
	
	FILE *sfs1 = fopen("sfs.bin","r+");
	fseek(sfs1,sizeof(struct super_block) + NUMOFINODES * sizeof(struct inode_st),SEEK_SET);
	struct dir_ent curr_ent;
	while(fread(&curr_ent,sizeof(struct dir_ent),1,sfs1)){
		printf("name : %s\n",curr_ent);
	}
	fclose(sfs);
    return 0;
}


  
