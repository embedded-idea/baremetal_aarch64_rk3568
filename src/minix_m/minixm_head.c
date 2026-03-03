#include "uart.h"
#include "minixm_head.h"
#include <minix/param.h>
#include <minix_main.h>
#include <minix/board.h>


// delay function match with rk3568 , u need find out frequency of your cpu and adjust the count to get about 500ms delay, this is just for testing
// count is about 500ms ?
static void tmp_delay(uint64_t count) {
    for (volatile uint64_t i = 0; i < count; i++);
}

// 初始化ARMv8的FP寄存器（x29）为0，适配RK3568
void init_frame_pointer(void) {
    __asm__ volatile (
        "mov x29, #0\n"  // 将帧指针x29（FP）置0
        "isb sy\n"       // 指令同步屏障，确保修改立即生效（裸机推荐加）
        :
        :
        : "x29", "memory"// 必须声明x29被修改，否则编译器可能优化出错
    );
}

void init_x1_zero(void) {
    __asm__ volatile (
        "mov x1, #0\n"  // 将帧指针x1置0
        "isb sy\n"       // 指令同步屏障，确保修改立即生效（裸机推荐加）
        :
        :
        : "x1", "memory"// 必须声明x1被修改，否则编译器可能优化出错
    );
}

kinfo_t kinfo;
struct kmessages kmessages;
struct machine machine; /* pre init stage machine */


/**
 *
 * The following function combines a few things together
 * that can well be done using standard libc like strlen/strstr
 * and such but these are not available in pre_init stage. 
 *
 * The function expects content to be in the form of space separated
 * key value pairs.
 * param content the contents to search in
 * param key the key to find (this *should* include the key/value delimiter)
 * param value a pointer to an initialized char * of at least value_max_len length
 * param value_max_len the maximum length of the value to store in value including
 *       the end char
 *
**/
int find_value(char * content,char * key,char *value,int value_max_len){

	char *iter,*keyp;
	int key_len,content_len,match_len,value_len;

	/* return if the input is invalid */
	if  (key == NULL || content == NULL || value == NULL) {
		return 1;
	}

	/* find the key and content length */
	key_len = content_len =0;
	for(iter = key ; *iter != '\0'; iter++, key_len++);
	for(iter = content ; *iter != '\0'; iter++, content_len++);

	/* return if key or content length invalid */
	if (key_len == 0 || content_len == 0) {
		return 1;
	}

	/* now find the key in the contents */
	match_len =0;
	for (iter = content ,keyp=key; match_len < key_len && *iter != '\0' ; iter++) {
		if (*iter == *keyp) {
			match_len++;
			keyp++;
			continue;
		} 
		/* The current key does not match the value , reset */
		match_len =0;
		keyp=key;
	}

	if (match_len == key_len) {
		printf("key found at %d %s\n", match_len, &content[match_len]);
		value_len = 0;
		/* copy the content to the value char iter already points to the first 
		   char value */
		while(*iter != '\0' && *iter != ' ' && value_len  + 1< value_max_len) {
			*value++ = *iter++;
			value_len++;
		}
		*value='\0';
		return 0;
	}
	return 1; /* not found */
}

/* 
 * During low level init many things are not supposed to work
 * serial being one of them. We therefore can't rely on the
 * serial to debug. POORMANS_FAILURE_NOTIFICATION can be used
 * before we setup our own vector table and will result in calling
 * the bootloader's debugging methods that will hopefully show some
 * information like the currnet PC at on the serial.
 */
#define POORMANS_FAILURE_NOTIFICATION  asm volatile("svc #00\n")

/* use the passed cmdline argument to determine the machine id */
void set_machine_id(char *cmdline)
{

	char boardname[20];
	memset(boardname,'\0',20);
	if (find_value(cmdline,"board_name=",boardname,20)){
		/* we expect the bootloader to pass a board_name as argument
		 * this however did not happen and given we still are in early
		 * boot we can't use the serial. We therefore generate an interrupt
		 * and hope the bootloader will do something nice with it */
		POORMANS_FAILURE_NOTIFICATION;
	}  
	machine.board_id = get_board_id_by_short_name(boardname);

	if (machine.board_id ==0){
		/* same thing as above there is no safe escape */
		POORMANS_FAILURE_NOTIFICATION;
	}
}



kinfo_t *pre_init(int argc, char **argv)
{
	char *bootargs;
	// This is the main "c" entry point into the kernel. It gets called
	//   from minix_init
	   
	// Clear BSS
	// memset(&_edata, 0, (u32_t)&_end - (u32_t)&_edata);
    // memset(&_kern_unpaged_edata, 0, (u32_t)&_kern_unpaged_end - (u32_t)&_kern_unpaged_edata);

	// we get called in a c like fashion where the first arg
	// is the program name (load address) and the rest are
	// arguments. by convention the second argument is the
	//  command line 
	if (argc != 2) {
		uart_puts("Invalid number of arguments to pre_init. Expected 2, got ");
		uart_puts('0'+argc);
		uart_puts("\n");
		return NULL;
	}

	bootargs = argv[1];
	set_machine_id(bootargs);
	//bsp_ser_init();// we want mmu map uart controller base address

	//Get our own copy boot params pointed to by ebx.
	//Here we find out whether we should do serial output.
	//get_parameters(&kinfo, bootargs);

	// Make and load a pagetable that will map the kernel
	// to where it should be; but first a 1:1 mapping so
	// this code stays where it should be.
	//dcache_clean(); // clean the caches 
	//pg_clear();
	//pg_identity(&kinfo);
	//kinfo.freepde_start = pg_mapkernel();
	//pg_load();
	//vm_enable_paging();

	// Done, return boot info so it can be passed to kmain(). 
	return &kinfo;
}


int minix_init(void)
{
	kinfo_t *kinfo=NULL;
	/* Kernel is mapped high now and ready to go, with
	 * the boot info pointer returned by pre_init in r0.
	 * Set the highly mapped stack and initialize it.
	 *
	 * Afther that call kmain with r0 still pointing to boot info
	 */
	init_frame_pointer(); // 初始化帧指针
	uart_puts("init_frame_pointer returned ok\n");
	//kinfo_t *pre_init(int argc, char **argv)
	kinfo = pre_init(2, "hello,world"); // 这里模拟传递参数，实际情况可能需要根据启动方式调整
	if (kinfo == NULL) {
		uart_puts("pre_init returned NULL\n");
		return -1;//pre_init fails
	}
	uart_puts("pre_init returned ok\n");
	init_x1_zero(); // 初始化x1寄存器为0
	uart_puts("init_x1_zero returned ok\n");

	//void kmain(kinfo_t *local_cbi)
	uart_puts("kmain with kinfo starting\n");
	kmain(kinfo);/* x0 holds kinfo_t ptr */
	/* not reached */
	while(1)
	{	
		tmp_delay(900000); // delay about 500ms
		uart_puts("we are alive ok just FYI\n");
	}
	return 0;
}
