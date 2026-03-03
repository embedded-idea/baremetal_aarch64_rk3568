#include "uart.h"
#include "minix_head.h"
#include <minix/param.h>
#include <minix_main.h>

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
	//set_machine_id(bootargs);
	//bsp_ser_init();
	// Get our own copy boot params pointed to by ebx.
	// Here we find out whether we should do serial output.
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
