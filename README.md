# swap_table_simulator

A software to simulate demand paging and memory management limited to one process (therefore one file and one page table)
the software also supports "heap and stack" simulation but won't simulate "free" from "heap and stack"


## How to install:
	extract the files and move them to a spceified folder, open terminal and use cd command to move to that folder,
	make sure files named mem_sim.c and mem_sim.c and makefile are there and than  type "make", 
	a file main should be created - this is the program executable
## How to run:
	after installing, go to previously mentioned folder that conists the executable file aforementioned, run it by typing
	"./main --filename here--" while "--filename here--" actually has the program file name (in this example, program_mem)
## How to uninstall
	run "make clean"
# THE main.c ACTS AS A TESTER TO THE SIMULATOR
Functions:
	
	void swap_out(sim_database_t *sim_db , int fd, int page_num);
		A function to write content of a page into a specific file.
		
	input: sim_database_t *sim_db : a pointer to the simulated memory database, int fd -> the file descriptor of the file that 
	content will be written to, int pagenum : which content will be written [which page]
	output: nothing
	
	
	
	void swap_in(sim_database_t *sim_db , int fd, int frame_num, int page_num);
		A function to read the content from a specific file into memory to fill a specific page into a specific frame
	
	input: sim_database_t *sim_db : a pointer to the simulated memory database, int fd -> the file descriptor of the file that
	content will be read from, int frame_num -> the frame number which the content will be written to on the memory, int page_num :
	the page number that will be read from the file
	output: nothing
	
	
	
	int random_frame();
		A function to generate a random number between 0 and 15 to get a random frame, used in a function to determine which frame
		should be dropped from memory when memory is full
		
		input: nothing
		output: a random number between 0 and 15 out of the seed from time(NULL)
		
		
		
	int check_frames();
		A function to determine if the memory is full by checking the Bitmap array
		
		input: nothing
		output: if there's an empty frame, than it returns the number of that frame, if all the frames are taken, it returns -1
	
	
	int test_info(sim_database_t *sim_db, unsigned int address, unsigned char* p_char);
		A function to check if the input data to that function is valid for operation in any way
		
		input: sim_database_t *sim_db : a pointer to the simlulated memory database, unsigned int address: the address that
		we are going to operate on, unsigned char* p_char : a char pointer that should anything be loaded it will be there
		output: if either the sim_database_t pointer or the unsigned char* pointer is NULL, it returns 0, it also returns 0
		if the address is above 4096 which is too high of an address to operate on, on this example at least, it returns 1 otherwise
		
		
	int memory_full(sim_database_t *sim_db);
			A function to handle the memory once it knows it's full and is called when there's a frame in need, the function uses random_frame()
			to generate a frame to be dropped, drops it and saves it in SWAP, than it returns the frame that it handled as a value
			
			input: sim_database_t *sim_db : a pointer to the simulated memory database
			output: a random number between 0 and 15 which would be the frame that was freed (and dropped) from the memory
			
			
	int page_handler(sim_database_t *sim_db, page_descriptor_t page_t, unsigned short address, int s);
			A function to handle the memory call of either load or store command accordingily using the last variable loaded and uses the different functions
			to help manage and test the info provided into it
			
			input: sim_database_t *sim_db : a pointer to the simulated memory database, page_descriptor_t page_t : the page table that
			is beind handled, unsigned short address : the address that is being worked on, int s : is 1 when store command was sent, 0 when load command was sent
			output: upon success it returns 1, if it failed for any reason, it returns 0

			
	sim_database_t* vm_constructor(char *executable, int text_size, int data_size, int bss_size);			
			A function to allocate and construct a simulated memory database on the heap of this process
			
			input: char* executable - the name of the executable file name, int text_size - the amount of pages that the text code segment
			takes, int data_size - the amount of pages the data segment takes, int bss_size - the amount of pages bss_size takes ....
			bss_size + text_size + data_size should add up to 64 (exactly!)
			output: if the allocation was success and the file opening was successfull among opening a new file named swap for the program to run on simulation
			it should return 1, 0 otherwise
		
		
	int vm_load(sim_database_t *sim_db, unsigned short address, unsigned char *p_char);	
			A function to read data from specific address on our simulated memory and put it on allocated unsigned char pointer
			uses the demand paging and functions aforementioned
			
			input: sim_database_t *sim_db - the simulated memory database, unsigned short address - the virtual address that
			should be loaded, unsigned char *p_char - an allocated char address which the info will be provided into
			output: if load was successfull, it returns 1, 0 otherwise
			
			
	int vm_store(sim_database_t *sim_db, unsigned short address, unsigned char value);
			A function to store data to specific address on our simulated memory, uses the demand paging and functions aforementioned
			
			input: sim_database_t *sim_db - the simulated memory database, unsigned short address - the virtual address
			that should be loaded, unsigned char value - the value that should be written into the aforementioned virtual address
			output: if store was succesfull, it returns 1, 0 otherwise
			
			
			void vm_destructor(sim_database_t *sim_db);
				A function to take care of freeing allocated memory for our memory database, closing all the files we opened
				to "destroy" the simulated memory database before closing the software
				
			input: sim_database_t *sim_db - the simulated memory database
			output: nothing
			
			
	void vm_print(sim_database_t* sim_db);
		A function to print satistics into the STDOUT_FILE
				
		input: sim_database_t *sim_db - the simulated memory database
		output: (to the STDOUT_FILE) satistics (which would be the amount of hits, misses, load calls, store calls and out of range address called)
		the memory divided into frames and each frame would call on the page that it's holding
		
		