/*
CSC139 
Fall 2019
First Assignment
Papageorgacopoulos, Dimitrios
Section #02
OSs Tested on: such as Linux
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>

// Size of shared memory block
// Pass this to ftruncate and mmap
#define SHM_SIZE 4096

// Global pointer to the shared memory block
// This should receive the return value of mmap
// Don't change this pointer in any function
void* gShmPtr;

// You won't necessarily need all the functions below
void Producer(int, int, int);
void InitShm(int, int);
void SetBufSize(int);
void SetItemCnt(int);
void SetIn(int);
void SetOut(int);
void SetHeaderVal(int, int);
int GetBufSize();
int GetItemCnt();
int GetIn();
int GetOut();
int GetHeaderVal(int);
void WriteAtBufIndex(int, int);
int ReadAtBufIndex(int);
int GetRand(int, int);


int main(int argc, char* argv[])
{
    pid_t pid;
    int bufSize; // Bounded buffer size
    int itemCnt; // Number of items to be produced
    int randSeed; // Seed for the random number generator 

    if(argc != 4){
		printf("Invalid number of command-line arguments\n");
		exit(1);
    }
	bufSize = atoi(argv[1]);
	itemCnt = atoi(argv[2]);
	randSeed = atoi(argv[3]);
	
	//checks the validity of the command-line arguments
	if(bufSize < 2)
	{
		fprintf(stderr, "Insufficient buffer size, must be at least 2\n");
		exit(1);
	}
	if(bufSize > 1000)
	{
		fprintf(stderr, "Buffer size too large, must be at less than 1000");
	}
	
	if(itemCnt < 0)
	{
		fprintf(stderr, "itemCnt cannot be negative, exiting\n");
		exit(1);
	}
	if(randSeed < 0)
	{
		fprintf(stderr, "Invalid Seed\n");
		exit(1);
	}

    // Function that creates a shared memory segment and initializes its header
    InitShm(bufSize, itemCnt);        

	/* fork a child process */ 
	pid = fork();
	if (pid < 0) { /* error occurred */
		fprintf(stderr, "Fork Failed\n");
		exit(1);
	}
	else if (pid == 0) { /* child process */
		printf("Launching Consumer \n");
		execlp("./consumer","consumer",NULL);
	}
	else { /* parent process */
		/* parent will wait for the child to complete */
		printf("Starting Producer\n");
		
		// The function that actually implements the production
        Producer(bufSize, itemCnt, randSeed);
		
		printf("Producer done and waiting for consumer\n");
		wait(NULL);		
		printf("Consumer Completed\n");
	}
    
    return 0;
}

void InitShm(int bufSize, int itemCnt)
{
	int in = 0;
	int out = 0;
	int shared_mem;
	const char *name = "OS_HW1_DimitriosP"; // Name of shared memory object to be passed to shm_open
	
	// Write code here to create a shared memory block and map it to gShmPtr
	// Use the above name.
	// **Extremely Important: map the shared memory block for both reading and writing 
	// Use PROT_READ | PROT_WRITE
	shared_mem = shm_open(name, O_CREAT | O_RDWR, 0666);
	ftruncate(shared_mem, SHM_SIZE);
	gShmPtr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,  shared_mem, 0);
	
	// Write code here to set the values of the four integers in the header
	// Just call the functions provided below, like this
	SetBufSize(bufSize);
	SetItemCnt(itemCnt);
	SetIn(in);
	SetOut(out);
}

void Producer(int bufSize, int itemCnt, int randSeed)
{
    int in = 0;
    int out = 0;
    int i = 0;
    in = GetIn();
	out = GetOut();
	int item = 0;
	srand(randSeed);
	
	while(i <= itemCnt)
	{
		item = GetRand(0, 3000);
		while(((in + 1) % bufSize) == GetOut());
	
		printf("Producing Item %d with value %d at Index %d\n", i, item, in);
		WriteAtBufIndex(in, item);
		
		in = ((in + 1) % bufSize);
		i++;
		SetIn(in);
	}

	printf("Producer Completed\n");
}

// Set the value of shared variable "bufSize"
void SetBufSize(int val)
{
        SetHeaderVal(0, val);
}

// Set the value of shared variable "itemCnt"
void SetItemCnt(int val)
{
        SetHeaderVal(1, val);
}

// Set the value of shared variable "in"
void SetIn(int val)
{
        SetHeaderVal(2, val);
}

// Set the value of shared variable "out"
void SetOut(int val)
{
        SetHeaderVal(3, val);
}

// Set the value of the ith value in the header
void SetHeaderVal(int i, int val)
{
        void* ptr = gShmPtr + i*sizeof(int);
        memcpy(ptr, &val, sizeof(int));
}

// Get the value of shared variable "bufSize"
int GetBufSize()
{       
        return GetHeaderVal(0);
}

// Get the value of shared variable "itemCnt"
int GetItemCnt()
{
        return GetHeaderVal(1);
}

// Get the value of shared variable "in"
int GetIn()
{
        return GetHeaderVal(2);
}

// Get the value of shared variable "out"
int GetOut()
{             
        return GetHeaderVal(3);
}

// Get the ith value in the header
int GetHeaderVal(int i)
{
        int val;
        void* ptr = gShmPtr + i*sizeof(int);
        memcpy(&val, ptr, sizeof(int));
        return val;
}

// Write the given val at the given index in the bounded buffer 
void WriteAtBufIndex(int indx, int val)
{
        // Skip the four-integer header and go to the given index 
        void* ptr = gShmPtr + 4*sizeof(int) + indx*sizeof(int);
	    memcpy(ptr, &val, sizeof(int));
}

// Read the val at the given index in the bounded buffer
int ReadAtBufIndex(int indx)
{
        int val;

        // Skip the four-integer header and go to the given index
        void* ptr = gShmPtr + 4*sizeof(int) + indx*sizeof(int);
        memcpy(&val, ptr, sizeof(int));
        return val;
}

// Get a random number in the range [x, y]
int GetRand(int x, int y)
{
	int r = rand();
	r = x + r % (y-x+1);
    return r;
}
