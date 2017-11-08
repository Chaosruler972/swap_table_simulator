#include "mem_sim.c"

//define map
/*
#define PAGE_SIZE 32
#define PAGE_NUM 128
#define FRAME_NUM 16
*/
#define SUCCESS "TEST SUCCESS\n"
#define FAIL "TEST FAILED\n"
int main(int argc, char* argv[])
{
    if(argv[1] == 0)
    {
	perror("Missing executable name\n");
	return 0;
    }
    unsigned char c='z';
    int i;
    int old;
    int grade=0;
    char str[PAGE_SIZE];
    sim_database_t* db = vm_constructor(argv[1], 10,30,24);
    printf("Write to lower than code section RAM TEST\n");
    old = db->illegal_acc; // replace db->illegal_acc with your global variable to count how many store attempts there were on page on CODE segment
    if(vm_store(db,0,c)!=old - db->illegal_acc)
    {
	printf("%s",FAIL);
    }
    else
    {
	printf("%s",SUCCESS);
	grade+=10;
    }
    printf("Load heap/stack memory before allocation TEST\n");
    old = db->address_high; // replace db->address_high with your global variable to count how many load before store attemts there were on pages number >=64 (seg fault)
    if(vm_load(db,PAGE_SIZE * 64,&c) != old - db->address_high)
    {
	printf("%s",FAIL);
    }
    else
    {
	printf("%s",SUCCESS);
	grade+=10;
    }
    printf("Page table test\n");
    old=1;
    for(i=0; i<PAGES_NUM;i++)
    {
	if(db->page_table[i].valid!=0 || db->page_table[i].frame !=0 || db->page_table[i].dirty!=0)
		old=0;
	if(i<10 && db->page_table[i].permission!=1)
		old=0;
	if(i>=10 && db->page_table[i].permission!=0)
		old=0;
    }
    if(!old)
    {
	printf("%s",FAIL);
    }
    else
    {
	printf("%s",SUCCESS);
        grade+=10;
    }
    old=1;
    printf("load's content memory test\n");
    vm_load(db,0,&c);
    vm_load(db,PAGE_SIZE * 63,&c);
    strncpy(str,mainMemory,PAGE_SIZE); // if your RAM pointer [the array which has all the unsigned char's which represents memory] is named different, replace it with mainMemory
    str[32]='\0';
    if(strcmp(str,"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")!=0)
 	old=0;
    strncpy(str,mainMemory + PAGE_SIZE,PAGE_SIZE); // if your RAM pointer [the array which has all the unsigned char's which represents memory] is named different, replace it with mainMemory
    str[32]='\0';
    if(strcmp(str,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")!=0)
	old=0;
    if(!old)
	printf("%s",FAIL);
    else
    {
	printf("%s",SUCCESS);
	grade+=10;
    }

    printf("heap allocation & test\n");
    old=1;
    char avi[13];
    strncpy(avi,"AVI HAMANIAK",12);
    avi[12]='\0';
    for(i=0; i<12;i++)
    {
	vm_store(db,65*PAGE_SIZE+i,avi[i]);
    }
    if(strcmp(avi,mainMemory + 2*PAGE_SIZE)!=0 || db->page_table[65].dirty!=1) // if your RAM pointer [the array which has all the unsigned char's which represents memory] is named different, replace it with mainMemory
	printf("%s",FAIL);
    else
    {
	printf("%s",SUCCESS);
	grade+=20;
    }
    printf("Sorry avi ;)\n");

    printf("bitframe test\n");
    for(i=0; i<64;i++)
	vm_load(db,i*PAGE_SIZE,&c);
    for(i=0; i<FRAME_NUM;i++)
    {
	if(Bitmap[i]==0)
		old=0;
    }
    if(!old)
	printf("%s",FAIL);
    else
    {
	printf("%s",SUCCESS);
	grade+=20;
    }
    printf("Load/Store functions call TEST\n");
    if(db->load!=66 || db->store!=12) // replace db->load with a global variable counting your vm_load calls and db->store with your global variable counting store calls or replace them both with a single global variable and equal it to 80 [ if(globalVar!=80) ]
	printf("%s",FAIL);
    else
    {
	printf("%s",SUCCESS);
	grade+=20;
    }
    printf("YOUR GRADE IS %d\n",grade);

   // vm_print(db);
    vm_destructor(db);
    return 0;
}
