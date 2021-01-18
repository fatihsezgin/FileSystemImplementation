#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const int REG_FILE = 0;
const int DIRECTORY = 1;
const int NUMOFINODES = 32;
int curDirInodeNum = 0;
int curDirDataNum = 0;
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

//first bash utility we will write is 'ls'.
//Becuase we will use it for debuggind as well.

//the ls function will print all the directory/files 
// in the file system with proper indentation so that
//we can "see" the picture of the whole file system.
void ls();
void mkdir(char file_name[28]);
void mkfile(char file_name[28],char file_content[100]);
//this will only print the current directory contents
//every directory/file has it unique inode number 
//WHICH IS USED AS AN INDEX INTO THE INODE TABLE
void ls_cur(int curDirInodeNum);

void show_bit_in_sb_inode_bitmap(struct super_block sb){
	unsigned bits[sizeof(sb.inode_bitmap) * 8]; //32
	unsigned dividend = sb.inode_bitmap; //4
	unsigned divider = 2;
	int i = 0;
	int counter = sizeof(sb.inode_bitmap) * 8 ; //32
	while(counter > 0){
		unsigned result = dividend%divider;
		bits[i] = result;
		dividend = dividend/divider;
		counter--;
		i++;
	}
	i = (sizeof(sb.inode_bitmap) * 8) - 1; //31
	for(i;i >=0 ;i--){
		printf("%d",bits[i]);
	}
	printf("\n");
}

void show_bit_in_sb_datablock_bitmap(struct super_block sb, int datablock_loc){
	unsigned bits[sizeof(sb.data_bitmap[datablock_loc]) * 8]; //32
	unsigned dividend = sb.data_bitmap[datablock_loc]; //4
	unsigned divider = 2;
	int i = 0;
	int counter = sizeof(sb.data_bitmap[datablock_loc]) * 8 ; //32
	while(counter > 0){
		unsigned result = dividend%divider;
		bits[i] = result;
		dividend = dividend/divider;
		counter--;
		i++;
	}
	i = (sizeof(sb.data_bitmap[datablock_loc]) * 8) - 1; //31
	for(i;i >=0 ;i--){
		printf("%d",bits[i]);
	}
	printf("\n");
}

int next_empty_sb_inode_bitmap(struct super_block sb){
	unsigned bits[sizeof(sb.inode_bitmap) * 8]; //32
	unsigned dividend = sb.inode_bitmap; //4
	unsigned divider = 2;
	int i = 0;
	int counter = sizeof(sb.inode_bitmap) * 8 ; //32
	while(counter > 0){
		unsigned result = dividend%divider;
		bits[i] = result;
		dividend = dividend/divider;
		counter--;
		i++;
	}
	unsigned size = (sizeof(sb.inode_bitmap) * 8); //32
	counter = -1;
	for(i = 0;i < size  ;i++){
		if(bits[i] == 0){
			counter = i;
			break;
		}
	}
	return counter;
}

int next_empty_sb_datablock_bitmap(struct super_block sb, int datablock_loc){
	unsigned bits[sizeof(sb.data_bitmap[datablock_loc]) * 8]; //32
	unsigned dividend = sb.data_bitmap[datablock_loc]; //4
	unsigned divider = 2;
	int i = 0;
	int counter = sizeof(sb.data_bitmap[datablock_loc]) * 8 ; //32
	while(counter > 0){
		unsigned result = dividend%divider;
		bits[i] = result;
		dividend = dividend/divider;
		counter--;
		i++;
	}
	unsigned size = (sizeof(sb.data_bitmap[datablock_loc]) * 8); //32
	counter = -1;
	for(i = 0;i < size  ;i++){
		if(bits[i] == 0){
			counter = i;
			break;
		}
	}
	return counter;
}

void set_bit(int *bitmap, int bit_num){
	*bitmap |= 1 << bit_num; 
}

void cd(char file_name[28]){
	FILE *sfs = fopen("sfs.bin","r");
	struct inode_st InodeSt;
	struct dir_ent Entry;
	int counter = curDirDataNum;
	int datablocknum = 0;
	int is_Directory = -1;
	int is_found = 0;
	int flag = 1;
	//int temp_datablocknum = curDirDataNum;
	fseek(sfs, sizeof(struct super_block)+32*sizeof(struct inode_st) + counter * sizeof(struct dir_ent), SEEK_SET);
	while(fread(&Entry,sizeof(struct dir_ent),1,sfs)){
		printf("%s %d\n", Entry.name,Entry.inode_no);
		fseek(sfs,sizeof(struct super_block) + Entry.inode_no * sizeof(struct inode_st),SEEK_SET);
		fread(&InodeSt,sizeof(struct inode_st),1,sfs);
		datablocknum = InodeSt.data_block_indices[0];
		//printf("%d --> InodeSt.data_block_indices[0]\n",datablocknum);
		fseek(sfs,sizeof(struct super_block) + NUMOFINODES * sizeof(struct inode_st) + datablocknum * sizeof(struct dir_ent),SEEK_SET);
		//printf("data_block_indices[0] --> %d\n",InodeSt.data_block_indices[0]);
		if(strcmp(Entry.name, file_name) == 0){
			if(strcmp(file_name,"..") == 0){
				is_Directory = 1;
				is_found = 1;
				curDirInodeNum = Entry.inode_no;
				curDirDataNum = datablocknum;
			}
			else {
				struct dir_ent temp_Entry;
				while(fread(&temp_Entry,sizeof(struct dir_ent),1,sfs)){
					//printf("%s %d--> temp_Entry.name, temp_Entry.inode_no\n",temp_Entry.name,temp_Entry.inode_no);
					if(temp_Entry.inode_no == curDirInodeNum){
						//printf("%d --> curDirInodeNum\n",curDirInodeNum);
						printf("%d --> InodeSt.type\n",InodeSt.type);
						if(InodeSt.type == DIRECTORY){
							is_Directory = 1;
							is_found = 1;
							curDirInodeNum = Entry.inode_no;
							curDirDataNum = datablocknum;
						} else {
							printf("Not Directory\n");
							is_found = 1;
							is_Directory = 0;
						}
						flag = 0;
						break; 
					}
				}	
			}
			flag = 0;
			break;
		}
		if(flag == 0) break;
		counter++;
		fseek(sfs, sizeof(struct super_block)+32*sizeof(struct inode_st) + counter * sizeof(struct dir_ent), SEEK_SET);
	}
	
	if(is_found == 1){
		if(is_Directory == 0){
			printf("File is not a directory\n");
		}
	}else {
		printf("File not Found\n");
	}
	
	fclose(sfs);	
}

/*void cd(char file_name[28]){
	FILE *sfs = fopen("sfs.bin","r");
	struct inode_st InodeSt;
	struct dir_ent Entry;
	int counter = curDirDataNum;
	int datablocknum = 0;
	int is_Directory = -1;
	int is_found = 0;
	
	fseek(sfs, sizeof(struct super_block)+32*sizeof(struct inode_st) + counter * sizeof(struct dir_ent), SEEK_SET);
	while(fread(&Entry,sizeof(struct dir_ent),1,sfs)){
		printf("%s %d\n", Entry.name,Entry.inode_no);
		fseek(sfs,sizeof(struct super_block) + Entry.inode_no * sizeof(struct inode_st),SEEK_SET);
		fread(&InodeSt,sizeof(struct inode_st),1,sfs);
		datablocknum = InodeSt.data_block_indices[0];
		printf("%d --> InodeSt.data_block_indices[0]\n",datablocknum);
		fseek(sfs,sizeof(struct super_block) + NUMOFINODES * sizeof(struct inode_st) + datablocknum * sizeof(struct dir_ent),SEEK_SET);
		printf("data_block_indices[0] --> %d\n",InodeSt.data_block_indices[0]);
		if(strcmp(Entry.name, file_name) == 0){
			is_found = 1;
			if(InodeSt.type == DIRECTORY){
				is_Directory = 1;
				curDirInodeNum = Entry.inode_no;
				curDirDataNum = datablocknum;
			} else {
				is_Directory = 0;
			}
			break; 
		}
		counter++;
		fseek(sfs, sizeof(struct super_block)+32*sizeof(struct inode_st) + counter * sizeof(struct dir_ent), SEEK_SET);
	}
	
	if(is_found == 1){
		if(is_Directory == 0){
			printf("File is not a directory");
		}
	}else {
		printf("File not Found\n");
	}
	
	fclose(sfs);	
}
*/

void lsrec(){
	struct dir_ent entry;
	struct inode_st root;
	FILE *sfs = fopen("sfs.bin", "r");
	int count = 0;
	int counter = curDirDataNum;
	fseek(sfs, sizeof(struct super_block)+32*sizeof(struct inode_st) + counter * sizeof(struct dir_ent), SEEK_SET);
	while(fread(&entry, sizeof(struct dir_ent), 1, sfs)){
		printf("%s %d\n", entry.name,entry.inode_no);
		counter++;
		count++;
	}
	fclose(sfs);
}

int main(){
	char command [32];
	while(1){
		printf("> ");
		scanf("%s", command);
		if(strcmp(command, "ls") == 0){
			ls();
		}
		else if(strcmp(command, "mkdir") == 0){
			char file_name[28];
			scanf("%s", &file_name);
			//printf("%s\n",file_name);
			mkdir(file_name);
		}
		else if(strcmp(command, "mkfile") == 0){
			char file_name[28];
			char file_content[100];
			scanf("%s",&file_name);
			mkfile(file_name,file_content);
		}
		else if(strcmp(command, "cd") == 0){
			char file_name[28];
			scanf("%s", &file_name);
			//scanf("%d", &curDirInodeNum);
			cd(file_name);
			printf("%d -> curDirInodeNum\n",curDirInodeNum);
			printf("%d -> curDirDataNum\n",curDirDataNum);
			//curDirDataNum = find_curDirDataNum(curDirInodeNum);
		}
		else if(strcmp(command, "lsrec") == 0){
			lsrec();
		}
		else if(strcmp(command, "exit") == 0){
			exit(0);
		}
	}
	
	return 0;
}

void ls (){
	struct dir_ent entry;
	struct inode_st root;
	FILE *sfs = fopen("sfs.bin", "r");
	//just past the super block, that is, we are at the beginning
	//of the inode table where the first inode structure
	//contains the meta data for the root directory
	//fseek(sfs, sizeof(struct super_block)+curDirInodeNum*sizeof(struct inode_st), SEEK_SET);
	//fread(&root, sizeof(struct inode_st), 1, sfs);
	//printf("size: %d\n", root.size);
	int counter = curDirDataNum;
	fseek(sfs, sizeof(struct super_block)+32*sizeof(struct inode_st) + counter * sizeof(struct dir_ent), SEEK_SET);
	while(fread(&entry, sizeof(struct dir_ent), 1, sfs)){
		printf("%s %d\n", entry.name,entry.inode_no);
		fseek(sfs,sizeof(struct super_block) + entry.inode_no * sizeof(struct inode_st),SEEK_SET);
		fread(&root,sizeof(struct inode_st),1,sfs);
		//printf("data_block_indices[0] --> %d\n",root.data_block_indices[0]);
		counter++;
		fseek(sfs, sizeof(struct super_block)+32*sizeof(struct inode_st) + counter * sizeof(struct dir_ent), SEEK_SET);
	}
	fclose(sfs);
}

void mkdir(char file_name[28]){
	FILE *sfs = fopen("sfs.bin","r+");
	struct super_block sb;
	fread(&sb,sizeof(struct super_block),1,sfs);
	int next_empty_sb_inode_bit = next_empty_sb_inode_bitmap(sb);
	//show_bit_in_sb_inode_bitmap(sb); // 1
	set_bit(&sb.inode_bitmap,next_empty_sb_inode_bit);
	//show_bit_in_sb_inode_bitmap(sb); // 2
	//show_bit_in_sb_datablock_bitmap(sb,0); //3
	
	int datablock = 0;
	int next_empty_sb_datablock_bit;
	
	//for(datablock = 0; datablock < 10 ; datablock++){
		next_empty_sb_datablock_bit = next_empty_sb_datablock_bitmap(sb,datablock);
		//if(next_empty_sb_datablock_bit != -1) break; 
	//}
	//show_bit_in_sb_datablock_bitmap(sb,0); //4
	set_bit(&sb.data_bitmap[0], next_empty_sb_datablock_bit);
	//show_bit_in_sb_datablock_bitmap(sb,0); //5
	set_bit(&sb.data_bitmap[0], next_empty_sb_datablock_bit + 1);
	//show_bit_in_sb_datablock_bitmap(sb,0); //6
	set_bit(&sb.data_bitmap[0], next_empty_sb_datablock_bit + 2);
	//show_bit_in_sb_datablock_bitmap(sb,0); //7
	
	printf("Inode : %d datablock : %d \n",next_empty_sb_inode_bit,next_empty_sb_datablock_bit);
	struct inode_st newDir;
	newDir.type = DIRECTORY;
	newDir.size = sizeof(struct dir_ent)*2;
	int i;
	
	for(i = 0; i < 10; i++)
        newDir.data_block_indices[i] = 0;
    newDir.data_block_indices[datablock] = next_empty_sb_datablock_bit;
    printf("%d %d-->  newDir.data_block_indices[datablock],datablock", newDir.data_block_indices[datablock],datablock);
    
    struct dir_ent dot;
	strcpy(dot.name, ".");
	dot.inode_no = next_empty_sb_inode_bit;

	struct dir_ent dotdot;
	strcpy(dotdot.name, "..");
	dotdot.inode_no = curDirInodeNum;
	
	struct dir_ent new_entry;
	strcpy(new_entry.name, file_name);
	new_entry.inode_no = next_empty_sb_inode_bit;
    
    fseek(sfs,0,SEEK_SET);
    fwrite(&sb, sizeof(struct super_block),1,sfs);
    
    fseek(sfs,sizeof(struct super_block) + next_empty_sb_inode_bit * sizeof(struct inode_st),SEEK_SET);
    fwrite(&newDir, sizeof(struct inode_st),1,sfs);
    
    fseek(sfs,sizeof(struct super_block) + NUMOFINODES * sizeof(struct inode_st) + next_empty_sb_datablock_bit * sizeof(struct dir_ent), SEEK_SET);
	fwrite(&new_entry, sizeof(struct dir_ent),1,sfs);
	fwrite(&dot,sizeof(struct dir_ent),1,sfs);
	fwrite(&dotdot, sizeof(struct dir_ent),1,sfs);
	
	fclose(sfs);	
}

void mkfile(char file_name[28],char file_content[100]){
	FILE *sfs = fopen("sfs.bin","r+");
	struct super_block sb;
	fread(&sb,sizeof(struct super_block),1,sfs);
	int next_empty_sb_inode_bit = next_empty_sb_inode_bitmap(sb);
	//show_bit_in_sb_inode_bitmap(sb); // 1
	set_bit(&sb.inode_bitmap,next_empty_sb_inode_bit);
	//show_bit_in_sb_inode_bitmap(sb); // 2
	//show_bit_in_sb_datablock_bitmap(sb,0); //3
	
	int datablock = 0;
	int next_empty_sb_datablock_bit;
	
	//for(datablock = 0; datablock < 10 ; datablock++){
		next_empty_sb_datablock_bit = next_empty_sb_datablock_bitmap(sb,datablock);
		//if(next_empty_sb_datablock_bit != -1) break; 
	//}
	
	//show_bit_in_sb_datablock_bitmap(sb,0); //4
	set_bit(&sb.data_bitmap[0], next_empty_sb_datablock_bit);
	//show_bit_in_sb_datablock_bitmap(sb,0); //7
	
	printf("Inode : %d datablock : %d \n",next_empty_sb_inode_bit,next_empty_sb_datablock_bit);
	struct inode_st newDir;
	newDir.type = REG_FILE;
	newDir.size = sizeof(struct dir_ent)*2;
	int i;
	
	for(i = 0; i < 10; i++)
        newDir.data_block_indices[i] = 0;
    newDir.data_block_indices[datablock] = next_empty_sb_datablock_bit;
    printf("%d %d-->  newDir.data_block_indices[datablock],datablock \n", newDir.data_block_indices[datablock],datablock);
	
	struct dir_ent new_entry;
	strcpy(new_entry.name, file_name);
	new_entry.inode_no = next_empty_sb_inode_bit;
    
    fseek(sfs,0,SEEK_SET);
    fwrite(&sb, sizeof(struct super_block),1,sfs);
    
    fseek(sfs,sizeof(struct super_block) + next_empty_sb_inode_bit * sizeof(struct inode_st),SEEK_SET);
    fwrite(&newDir, sizeof(struct inode_st),1,sfs);
    
    fseek(sfs,sizeof(struct super_block) + NUMOFINODES * sizeof(struct inode_st) + next_empty_sb_datablock_bit * sizeof(struct dir_ent), SEEK_SET);
	fwrite(&new_entry, sizeof(struct dir_ent),1,sfs);
	
	fclose(sfs);
	
}


void lsPFS(){
	
}
