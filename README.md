# baremetal_aarch64_rk3568
This is baremetal fun project for aarch64 processor rk3568 board

# first you could try to run in a qemu-system-aarch64 with cortex-a57, cause there is no cortex-a55 which is used by rk3568 chip.i am sorry about that. after you lunch following , you could actaully use ctrl+a, c to go into a command mode, then run command info registers, looking at register you set usually X13, you are feel free to use any registers as you like. pretty much this is your first step to verify start.s and link.ld.
qemu-system-aarch64 -M virt -cpu cortex-a57 -m 128M -nographic -device loader,file=./build/min.img,addr=0x40100000 -device loader,addr=0x40100000,cpu-num=0


# move to real board, if you already have uboot running on your board , it is really cool , because you are able to utilize uboot to set memory for gpio controller. in my lubancat2 board, there is one user led gpio which is GPIO 0, group C, pin7 which is led GPIO0_C7,rockchip gpio register is kind of weird,GPIO0 has 32 gpio control ability, but it grouped by A(7-0) B(15-8) low , C(0-7) D(0-7) high. 

# in uboot , please try to toggle this manually you will see your led on and off controlled by you , you should feel so happy about it
md 0xfdd60008(Direction of (GPIO0_A and GPIO0_B))
md 0xfdd6000C(Direction of (GPIO0_A and GPIO0_B))
mw 0xfdd6000C 0xFFFF0080(GPIO0_C7 set to output)

md 0xfdd60000(DATA of (GPIO0_A and GPIO0_B))
md 0xfdd60004(DATA of (GPIO0_C and GPIO0_D))
LED OFF:
mw 0xfdd60004 0xFFFF0080(GPIO0_C7 DATA high, led should off)
LED ON:
mw 0xfdd60004 0xFFFF0080(GPIO0_C7 DATA low, led should off)

# we could not just do it manually , so I decide to write a program toggle it like flash led on and off. about 1 second interval. This is my first commit will add more code.
step 1: build it
./build.sh build
step 2: make filesystem on micro sd card
sudo fdisk /dev/******* 
sudo mkfs.vfat /dev/*******
sudo mount /dev/***** /mnt
sudo build/min.img /media/mzhang/8B24-A79B/
sudo umount /media/mzhang/8B24-A79B/
step 3: insert micro sd card to lubancat2 board and run command in uboot

=> mmc list
dwmmc@fe2b0000: 1 (SD)
dwmmc@fe2c0000: 2
sdhci@fe310000: 0 (eMMC)

=> mmc dev 1
switch to partitions #0, OK
mmc1 is current device

=> fatls mmc 1:1 
      420   min.img
1 file(s), 0 dir(s)

=> fatload  mmc 1:1 0x40100000 min.img
reading min.img
420 bytes read in 3 ms (136.7 KiB/s)

=> go 0x40100000
## Starting application at 0x40100000 ...

all right you should see the green led flashing with half second interval.



