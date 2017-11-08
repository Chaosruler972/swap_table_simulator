#include "mem_sim.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
//====================================================================
//Public header for the memory simulation module.
//====================================================================
 
#define VIRTUAL_RAM  4096
#define PHYSICAL_RAM  512
#define PAGES_NUM  128
#define PAGE_SIZE 32
#define FRAME_NUM 16
// one more var to save the data while swaping
 
/**
* metadata type for the memory simulation library. this type is opaqe -
* struct specification is subject to internal implementation.
*/
 

/******************************************* structs defining *************************************///////////////
typedef struct page_descriptor
{
    unsigned int valid; //is page mapped to frame
    unsigned int permission; //READ or READ-WRITE
    unsigned int dirty; //has the page changed
    unsigned int frame; //number of frame if page mapped
} page_descriptor_t;
 
 
 
struct sim_database
{
    page_descriptor_t* page_table; //pointer to page table
    char* swapfile_name; //name of swap file
    int swapfile_fd; //swap file fd
    char* executable_name; //name of executable file
    int executable_fd; //executable file fd
    int txt_lim;
    int data_lim;
    int bss_lim;
    int hit;
    int miss;
    int load;
    int store;
    int address_high;
    int illegal_acc;
};

 


/******************************************* Functions call *************************************///////////////
void swap_out(sim_database_t *sim_db , int fd, int page_num);
void swap_in(sim_database_t *sim_db , int fd, int frame_num, int page_num);
int random_frame();
int check_frames();
int test_info(sim_database_t *sim_db, unsigned int address, unsigned char* p_char);
int memory_full(sim_database_t *sim_db);
int page_handler(sim_database_t *sim_db, page_descriptor_t page_t, unsigned short address, int s);



/******************************************* global varaibles init *************************************///////////////
int Bitmap[FRAME_NUM]; // the frames in the main memory
int Framemap[FRAME_NUM];
static char mainMemory[PHYSICAL_RAM];
char* swapf="swap";


sim_database_t* vm_constructor(char* executable, int text_size, int data_size, int bss_size)
{
    // initialize virtual system
    //* return: pointer to main data structure
 
    // check if we have premissons for file
    // or if the file exists
   if(executable == NULL || access(executable,F_OK)==-1 || access(executable,R_OK)==-1 ) // checks access and filename
   {
    perror("Unable to access file!\n");
    return NULL;    
   }
   else
   {
    sim_database_t* sys = (sim_database_t*) malloc (sizeof(sim_database_t)); // allocate system struct
    if(sys == NULL)
    {
        perror("Memory allocation error!\n");
        return NULL;        
    }
    sys->hit=0;
    sys->miss=0;
    sys->load=0;
    sys->store=0;
    sys->address_high=0;
    sys->illegal_acc=0;
    sys->page_table = (page_descriptor_t*) malloc (sizeof(page_descriptor_t)*PAGES_NUM); // allocate pagetable struct
    if(sys->page_table == NULL)
    {
        perror("Memory allocation error!\n");
        return NULL;
    }
    int i;
    for(i=0; i<PAGES_NUM; i++) // reset pagetable to default and write permission set
    {
        sys->page_table[i].valid = 0;
        if(i>=text_size)
        {
            sys->page_table[i].permission = 0;
        }
        else
        {
            sys->page_table[i].permission = 1;
        }
        sys->page_table[i].frame = 0;
        sys->page_table[i].dirty = 0;    
    }
    sys->executable_name = (char*) malloc (strlen(executable)+1); // allocates filename string
    if(sys->executable_name == NULL)
    {
        free(sys->page_table);
        free(sys);
        perror("Memory allocation error!\n");
        return NULL;
    }
    strcpy( sys->executable_name ,executable);// copies filename to struct
    sys->executable_fd = open(executable, O_RDONLY); // checks executable file open
    if(sys->executable_fd < 0)
    {
	free(sys->executable_name);
	free(sys->swapfile_name);
	free(sys->page_table);
	free(sys);
	return NULL;
    }
    int swapDesc = open(swapf, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWOTH);
    if(swapDesc < 0) // checks if swap file opening failed
    {
        free(sys->page_table);
        free(sys);
        perror("swap-file opening error!\n");
        return NULL;
    }
    sys->swapfile_name = (char*) malloc (strlen(swapf) + 1); // allocate swapfile name string
    if(sys->swapfile_name == NULL)
    {
	close(sys->executable_fd);
        free(sys->page_table);
        free(sys);
        perror("Memory allocation error!\n");
        return NULL;
    }   
    strcpy(sys->swapfile_name,swapf); // copies swapfile name to struct
    sys->swapfile_fd=swapDesc;
    sys->txt_lim =text_size;
    sys->data_lim = sys->txt_lim +data_size;
    sys->bss_lim=sys->data_lim + bss_size;
    srand(time(NULL)); // SEED create for random in the rest of the code
    return sys; 
   }
   return NULL;
}
 


int check_frames()
{
	int i;
	for(i=0; i< sizeof(Bitmap)/sizeof(Bitmap[0]); i++) // goes through the entire frame array
	{
		if(Bitmap[i] == 0) // if a frame is empty
			return i; // return it
	}
	return -1; // return if frame array is full
}

int random_frame()
{
	int r = rand()% FRAME_NUM ; // fetch random number between 0 and 15
	return r;
}

int test_info(sim_database_t *sim_db, unsigned int address, unsigned char* p_char)
{
	if(sim_db == 0 || p_char==0)
	{
		perror("invalid sim_db or unallocated p_char pointer\n");
		return 0;
	}
	if(address >= PAGES_NUM*PAGE_SIZE)
	{
		perror("address too high!\n");
		sim_db->address_high++;
		return 0;
	}
	return 1;

}

void swap_out(sim_database_t *sim_db , int fd, int page_num)
{
		int frame = sim_db->page_table[page_num].frame; // fetch frame number that I need to read
		
		if(page_num < sim_db->txt_lim) // checks permission
		{
			sim_db->page_table[page_num].valid = 0;
			sim_db->page_table[page_num].dirty = 0;
			sim_db->page_table[page_num].frame = 0;
			Bitmap[frame]=0;
			return;
		}
		
		char str[PAGE_SIZE]; 
		int i;
		for(i=0; i<PAGE_SIZE;i++)
			str[i] = mainMemory[(frame*PAGE_SIZE)+i];
		sim_db->page_table[page_num].valid = 0; // sets valid bit to 0 since it's not in memory anymore
		sim_db->page_table[page_num].frame = 0;
		//sim_db->page_table[page_num].dirty = 1;
		Bitmap[frame]=0; // cleans frame since it's not in memory anymore
		lseek(fd, PAGE_SIZE * page_num, SEEK_SET); // optionally SEEK_CUR SEEK_END
		write(fd,str,PAGE_SIZE); // writes into file
}

int memory_full(sim_database_t *sim_db)
{
	int out_frame = random_frame(); // fetchs a random frame
	int out_page = Framemap[out_frame]; // gets the relevent page data
	if(sim_db->page_table[out_page].dirty == 0) // if the page wasn't changed
	{
		Bitmap[out_frame]=0; // emulate frame clean and return
		sim_db->page_table[out_page].valid=0;
		sim_db->page_table[out_page].frame=0;
		return out_frame;
	}
	swap_out(sim_db, sim_db->swapfile_fd ,out_page); // clean frame and save the page
	return out_frame;
	
}


void swap_in(sim_database_t *sim_db , int fd, int frame, int page_num)
{
	char str[PAGE_SIZE];
	Bitmap[frame]=1; // sets frame as taken
	Framemap[frame]=page_num; // updates frame data to RAM
	sim_db->page_table[page_num].frame = frame; // updates pagetable to relevent frame
	sim_db->page_table[page_num].valid = 1; // sets the page to be in memory upon search
	lseek(fd,PAGE_SIZE * page_num, SEEK_SET);
	int i;
	read(fd,str,PAGE_SIZE); // read the relevent page data from file
	for(i=0; i<PAGE_SIZE;i++)
	{
		mainMemory[(frame*PAGE_SIZE) + i] = (char) 0;
	}
	for(i=0; i<PAGE_SIZE;i++)
	{
		mainMemory[(frame*PAGE_SIZE) + i]=str[i];
	}
}

int page_handler(sim_database_t *sim_db, page_descriptor_t page_t, unsigned short address, int s)
{
	int page = address / PAGE_SIZE; // fetch page number
	int valid = page_t.valid; // checks the valid bit
	if(valid) // V==1
	{
		sim_db->hit++;
		return 1;
	}
	else // V==0
	{
		sim_db->miss++;
		if(sim_db->page_table[page].permission) // P == 1
		{
			// READ ONLY
			int frame = check_frames(); // do I have a clean frame?
			if(frame == -1) // if I don't, clean and get one
				frame=memory_full(sim_db);
			swap_in(sim_db,sim_db->executable_fd,frame,page); // copy the page into memory
			return 1;
		}
		else // P == 0
		{
			// RD\RW
			if(sim_db->page_table[page].dirty) // D == 1
			{
				// page is dirty - default data was changed
				int frame = check_frames(); // ...
				if(frame == -1) // ..
					frame=memory_full(sim_db); // ..
				swap_in(sim_db,sim_db->swapfile_fd,frame,page); // ..
				return 1;
			}
			else // D == 0
			{
				if(page >= sim_db->bss_lim) // if BSS/Data == 0
				{
					if(s)
					{
						// store function
						int frame = check_frames(); //..
						if(frame == -1) // ..
							frame=memory_full(sim_db); // ..
						sim_db->page_table[page].valid = 1; // ..
						sim_db->page_table[page].dirty = 1;
						Bitmap[frame]=1;
						Framemap[frame]=page;
						sim_db->page_table[page].frame = frame;
						int i;
						char str[PAGE_SIZE];
						for(i=0; i<PAGE_SIZE;i++)
							str[i] = (char) 0;
						strncpy(mainMemory + frame*PAGE_SIZE,str,PAGE_SIZE);
						return 1;
					}
					else
					{
						sim_db->address_high++;
						perror("unable to load unallocated address\n");
						return 0;
					}
				}
				else // BSS/Data == 1
				{
					int frame = check_frames();
					if(frame == -1)
						frame=memory_full(sim_db);
					swap_in(sim_db,sim_db->executable_fd,frame,page);
				}
			}
		}
	}
	return 1;
}


int vm_load(sim_database_t *sim_db, unsigned short address, unsigned char *p_char)
{
	
	if(test_info(sim_db,address,p_char) == 0)
		return -1;
	int page = address / PAGE_SIZE;
	int offset = address % PAGE_SIZE;
	if(page_handler(sim_db,sim_db->page_table[page],address,0) == 0)
	{
		perror("page handling failed.\n");
		return -1;
	}
	int frame = sim_db->page_table[page].frame;
	p_char[0] = (unsigned char) mainMemory[(frame * PAGE_SIZE)+offset ];
	sim_db->load++;
	return 0;
}


void vm_destructor(sim_database_t *sim_db)
{
	if(sim_db == 0) // checks if there's anything to free or access 
	{
		perror("There's no sim_db to destroy\n");
		return;
	}
	close(sim_db->swapfile_fd); // closes strcut file table
	close(sim_db->executable_fd);
	free(sim_db->page_table); // frees page table
	free(sim_db->swapfile_name); // frees relevent strings
	free(sim_db->executable_name);
	free(sim_db); // frees system memory
	sim_db=0; // sets as null
	return;
}

int vm_store(sim_database_t *sim_db, unsigned short address, unsigned char value)
{
	unsigned char c;
	if(test_info(sim_db,address,&c) == 0)
		return -1;
	int page = address / PAGE_SIZE;
	if( page < sim_db->txt_lim)
	{
		sim_db->illegal_acc++;
		perror("Unable to store in that address, this is Code segment!\n");
		return -1;
	}
	int offset = address % PAGE_SIZE;
	if(page_handler(sim_db,sim_db->page_table[page],address,1) == 0)
	{
		perror("page handle failed!\n");
		return -1;
	}
	int frame = sim_db->page_table[page].frame;
	mainMemory[(frame*PAGE_SIZE) + offset] = (char) value;
	sim_db->page_table[page].dirty=1; // data was changed, mark page as dirty
	sim_db->store++;
	return 0;
}

void vm_print(sim_database_t* sim_db)
{
	if(sim_db==0)
		return;
	int i;
       	printf("i\tvalid\t‫‪permission‬‬\tdirty\tframe\n");
	for(i=0; i<PAGES_NUM;i++)
		printf("%d\t%d\t\t%d\t%d\t%d\n",i,sim_db->page_table[i].valid,sim_db->page_table[i].permission,sim_db->page_table[i].dirty,sim_db->page_table[i].frame);
	printf("the name of the swap file is %s and it's file descriptor is %d\n",sim_db->swapfile_name,sim_db->swapfile_fd);
	printf("the name of the swap file is %s\n",sim_db->executable_name);
	for(i=0; i<FRAME_NUM; i++)
	{
		printf("Frame number %d : ",i);
		int j;
		for(j=0; j<PAGE_SIZE;j++)
		{
			printf("%c",mainMemory[i*PAGE_SIZE+j]);
		}
		printf("\t(page number %d)",Framemap[i]);
		printf("\n");
	}
	printf("Satistics:\nThe number of load and store calls is %d\nThe number of hits is %d\nThe number of misses is %d\nThe number of too high address sent is %d\nThe number of illegal premission is %d\n",sim_db->load+sim_db->store,sim_db->hit,sim_db->miss,sim_db->address_high,sim_db->illegal_acc);
	
	
}


