#include <stdint.h>
#include "uart.h"
#include "minixs_head.h"
#include "minixs_main.h"



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

int minix_init()
{
	
	init_frame_pointer(); // 初始化帧指针
	uart_puts("init_frame_pointer returned ok\n");
	//kinfo_t *pre_init(int argc, char **argv)
	uart_puts("fake pre_init returned ok\n");
	init_x1_zero(); // 初始化x1寄存器为0
	uart_puts("init_x1_zero returned ok\n");
	//void kmain(kinfo_t *local_cbi)
	uart_puts("fake kmain with kinfo starting\n");
	kmain();/* x0 holds kinfo_t ptr */
	/*code should not reached here , cause when a operating system runs,
	kmain is the main function of that OS, it will working without issue to return here*/
	tmp_delay(900000); // delay about 500ms
	uart_puts("we are alive ok just FYI\n");
	return 0;
}
