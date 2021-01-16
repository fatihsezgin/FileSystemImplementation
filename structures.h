#define NUMBEROFINODES 32
#define REG_FILE 0;
#define DIRECTORY 1;

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