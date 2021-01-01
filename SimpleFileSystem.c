#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
    If I use a byte, how many inode structures can I have?
    8 inode structures.
    Let's have an int (32 bits) for the size of the inode bitmap.
    Therefore, we can have at most 32 files/directories in tyhe simple file system.

    Let's assume that each file/directory can have at most 10 data blocks and each data block gas 512 bytes in it.

    We need 10*32 = 320 is the number of data blocks. We need 320 bits for the data blocks.
*/

const int REG_FILE = 0;
const int DIRECTORY = 1;

struct SuperBlock
{
    int inodeBitmap;    //will tell us an inodeStructure is used or free
    int dataBitmap[10]; //will tell us a data block is used of free
};

// Metadata
struct inodeStructure
{
    int type;
    int size;
    int dataBlockIndices[10];
};

// Every directory in the file system has name and inode number
// in which assosiated with each other.
struct dirEntry
{
    char name[28];
    unsigned int inodeNumber;
};

void initialize_sb(struct SuperBlock sb)
{
    sb.inodeBitmap = 0;
    for (int i = 0; i < 10; i++)
        sb.dataBitmap[i] = 0;
}

int main()
{
    struct SuperBlock sb;
    sb.inodeBitmap = 32;
    initialize_sb(sb);

    struct inodeStructure root;
    root.type = DIRECTORY;
    root.size = sizeof(struct dirEntry) * 2;
    for (int i = 0; i < 10; i++)
    {
        root.dataBlockIndices[i] = 0;
    }

    // mark superblock inode and data bitmap because
    // an inode and a data block is being allocated for the
    // root directory. Mark (put 1) in the inode bitmap (bit 0)
    // and data bitmap (index 0, bit 0 to ) denote usage.
    sb.inodeBitmap = 1;
    sb.dataBitmap[0] = 1;

    printf("size of dirEntry: %lu\n", sizeof(struct dirEntry));

    //The root directory has a special feature, which having the same inode
    //numbers in dot and dotdot. In the root directory, the current and the
    //previous directories are the same.
    struct dirEntry dotdot;
    strcpy(dotdot.name, "..");
    dotdot.inodeNumber = 0;

    struct dirEntry dot;
    strcpy(dot.name, ".");
    dot.inodeNumber = 0;

    root.dataBlockIndices[0] = dotdot.inodeNumber; //Previous
    root.dataBlockIndices[1] = dot.inodeNumber; // current

    printf("%s\n ", dotdot.name);
    printf("%u\n", dotdot.inodeNumber);

    FILE *sfs = fopen("sfs.bin", "w+");
    fwrite(&sb, sizeof(sb), 1, sfs);
    
    fwrite(&dotdot, sizeof( struct dirEntry), 1, sfs);
    fwrite(&dot, sizeof( struct dirEntry), 1, sfs);
    
    fclose(sfs);

    // Reading //

    FILE *infile;
    struct dirEntry entry;

    /*** open the sfs file ***/
    infile = fopen("sfs.bin", "r");
    if (infile == NULL)
    {
        fprintf(stderr, "\nError opening sfs.bin\n\n");
        exit(1);
    }
    /*  
        Fseek is mandatory for able to read the dirEnrties in the file
        With the fseek the cursor location will be set 
        cur = beginning of the file + size of struct SuperBlock
    */
    if (fseek(infile, sizeof(struct SuperBlock), SEEK_SET) < 1){
        printf("seek is succesfull\n");
    }

    /* 
        Reading the dirEntries in sfs.bin file.
    */
    while (fread(&entry, sizeof(struct dirEntry), 1, infile))
        printf("Name = %s \n", entry.name);

    //write the first inode structure for the root to the file
    // (sfs.bin), remember this file contains your file system
    // (its meta data and actual data).

    // Jump(!) to the beginning of the data block to write the two entries
    // (dot and dotdot) the first data block.
    return 0;
}