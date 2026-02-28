#include <stdint.h>

#define REG32(addr) (*((volatile uint32_t *)(addr)))

// delay function match with rk3568 1.8GHz
// count 90000000 is about 500ms ?
static void delay(uint64_t count) {
    for (volatile uint64_t i = 0; i < count; i++);
}

// GPIO init set GPIO0_C7 output mode
static void gpio_led_init(void) {
    REG32(0xfdd6000C)= 0xFFFF0080;                                                    
    REG32(0xfdd60004)= 0xFFFF0080;
}

// LED on set GPIO0_C7 low
static void led_on(void) {
    REG32(0xfdd6000C)= 0xFFFF0000;
}

// LED on set GPIO0_C7 high
static void led_off(void) {
    REG32(0xfdd6000C)= 0xFFFF0080;
}

int main(int argc, char *argv[])
{
    gpio_led_init();
    while (1) {
        delay(90000000);  // 延时约500ms（可根据实际主频调整）
        led_on();
        delay(90000000);  // 延时约500ms（可根据实际主频调整）
        led_off();
    }
    return 0;
}