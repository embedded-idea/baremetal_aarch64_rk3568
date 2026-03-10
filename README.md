# 1) baremetal_aarch64_rk3568
This is baremetal fun project for aarch64 processor rk3568 lubancat2 board

# 2) first you could try to run in a qemu-system-aarch64 with cortex-a57, cause there is no cortex-a55 which is used by rk3568 chip.i am sorry about that. after you lunch following , you could actaully use ctrl+a, c to go into a command mode, then run command info registers, looking at register you set usually X13, you are feel free to use any registers as you like. pretty much this is your first step to verify start.s and link.ld.
qemu-system-aarch64 -M virt -cpu cortex-a57 -m 128M -nographic -device loader,file=./build/min.img,addr=0x40100000 -device loader,addr=0x40100000,cpu-num=0


# 3) move to real board, if you already have uboot running on your board , it is really cool , because you are able to utilize uboot to set gpio toggle your led manually in my lubancat2 board just verify if it works, there is one user led gpio which is GPIO 0, group C, pin7 which is led GPIO0_C7,rockchip gpio register is kind of weird,GPIO0 has 32 gpio control ability, but it grouped by A(7-0) B(15-8) low , C(0-7) D(0-7) high. 

# 4) in uboot , please try to toggle this manually you will see your led on and off controlled by you , you should feel so happy about it
md 0xfdd60008(Direction of (GPIO0_A and GPIO0_B))
md 0xfdd6000C(Direction of (GPIO0_C and GPIO0_D))
mw 0xfdd6000C 0xFFFF0080(GPIO0_C7 set to output)

md 0xfdd60000(DATA of (GPIO0_A and GPIO0_B))
md 0xfdd60004(DATA of (GPIO0_C and GPIO0_D))
LED OFF:
mw 0xfdd60004 0xFFFF0080(GPIO0_C7 DATA high, led should off)
LED ON:
mw 0xfdd60004 0xFFFF0080(GPIO0_C7 DATA low, led should off)

# 5) we can not do it manually all the time , so I decide to write a program toggle it like flash led on and off. about 1 second interval. This is my first commit will add more code.
git checkout a1fa8787214263470c190cd5b81853f340038625
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

=> fatload  mmc 1:1 0x40100000 min.bin
reading min.bin
8608 bytes read in 4 ms (2.1 MiB/s)

=> go 0x40100000
## Starting application at 0x40100000 ...
Hello-ARMv8-A (AArch64) Baremetal Program
UART: 1500000 8N1
press any key and enter to continue
hinit_frame_pointer returned ok
fake pre_init returned ok
init_x1_zero returned ok
kmain with kinfo starting
we are alive ok just FYI
minix_init returned

# 6) lets put some knowledge here , let me explain the following files after you compile uboot
uboot.img(2copied of uboot.itb(has many bins))
u-boot                  ELF file   
u-boot.cfg.configs      config file
u-boot.dump             u-boot dump text instructions  
u-boot-nodtb.bin        u-boot bin without dtb 
u-boot.srec             srec format of u-boot
u-boot.bin              if need device tree it same as u-boot-dtb.bin if no need device tree same as u-boot-nodtb.bin     
u-boot.dtb              u-boot dtb file          
u-boot.lds              u-boot link file
u-boot-nodtb.bin.digest sha256sum key value in binary of u-boot-nodtb.bin
u-boot.sym              u-boot symbol file
u-boot.cfg              u-boot #define   
u-boot-dtb.bin          u-boot with device tree bin      
u-boot.map              u-boot map file
u-boot-nodtb.bin.gz     gzip file of u-boot-nodtb.bin

just give a quick ramp up on uboot compile flow
1) each folder which joins the compilation, it will generate a built-in.o file

2) all built-in.o will be linked by u-boot.lds to a elf format u-boot

3) objcopy will strip out all the symbols of u-boot to u-boot-nodtb.bin which is a bin file

4) generate dtb file from dts file, dts file will be compiled by dtc and pack dtb to u-boot.dtb

5) combine u-boot-nodtb.bin and u-boot.dtb to u-boot-dtb.bin

6) just copy u-boot-dtb.bin to u-boot.bin

pretty much the stand process of result bin file is u-boot.bin, which should contain 2 parts , u-boot binary code and u-boot device tree blob.

# 7) in our case , let me show you the details of the u-boot generate process.
repos which you need as presiqusite
https://github.com/LubanCat/u-boot.git(8f53f800da2c25d0c6ba414fb45902a01675703a)
https://github.com/LubanCat/rkbin.git(74213af1e952c4683d2e35952507133b61394862)
https://github.com/rockchip-linux/rkdeveloptool.git(304f073752fd25c854e1bcf05d8e7f925b1f4e14)
git@github.com:embedded-idea/baremetal_aarch64_rk3568.git

rkdeveloptool is the upgrade tool for maskrom mode of rk3568 lubancat2 board
rkbin has more bin which required by bootloaders to generate miniloadedr(rk356x_spl_loader_v1.23.114.bin)
u-boot real u-boot source code which supports rk3568 lubancat2
baremetal_aarch64_rk3568 is our bootloader

NOTE:***

1) generate a miniloader with the following commands for rk3568 lubancat2 board 
cd rkbin/
./tools/boot_merger /home/mzhang/home_work/minix/lubancat2/bootloader/rkbin/RKBOOT/RK3568MINIALL.ini

cat /home/mzhang/home_work/minix/lubancat2/bootloader/rkbin/RKBOOT/RK3568MINIALL.ini
[CHIP_NAME]
NAME=RK3568
[VERSION]
MAJOR=1
MINOR=1
[CODE471_OPTION]
NUM=1
Path1=bin/rk35/rk3568_ddr_1560MHz_v1.23.bin
Sleep=1
[CODE472_OPTION]
NUM=1
Path1=bin/rk35/rk356x_usbplug_v1.17.bin
[LOADER_OPTION]
NUM=2
LOADER1=FlashData
LOADER2=FlashBoot
FlashData=bin/rk35/rk3568_ddr_1560MHz_v1.23.bin
FlashBoot=bin/rk35/rk356x_spl_v1.14.bin
[OUTPUT]
PATH=rk356x_spl_loader_v1.23.114.bin
[SYSTEM]
NEWIDB=true
[FLAG]
471_RC4_OFF=true
RC4_OFF=true

will generate a file rk356x_spl_loader_v1.23.114.bin its is a miniloader could be download and run during MASKROM mode(rk356x_spl_loader_v1.23.114.bin should have 3 parts(ddr init data,usbplugxxxx,miniloaderxxxx from rockchip)

2) generate the following content to file "parameter.txt" as gpt table information used by "rkdeveloptool gpt parameter.txt", there is a partition name uboot ,which from 0x4000 of emmc, size is 0x2000.
sudo ./rkdeveloptool gpt parameter.txt

FIRMWARE_VER: 1.0
MACHINE_MODEL: RK3588
MACHINE_ID: 007
MANUFACTURER: RK3588
MAGIC: 0x5041524B
ATAG: 0x00200800
MACHINE: 0xffffffff
CHECK_MASK: 0x80
PWR_HLD: 0,0,A,0,1
TYPE: GPT
GROW_ALIGN: 0
CMDLINE: mtdparts=:0x00002000@0x00004000(uboot),0x00002000@0x00006000(misc),0x00020000@0x00008000(boot),0x00040000@0x00028000(recovery),0x00010000@0x00068000(backup),0x01c00000@0x00078000(rootfs),0x00040000@0x01c78000(oem),-@0x01cb8000(userdata:grow)
uuid:rootfs=614e0000-0000-4b53-8000-1d28000054a9
uuid:boot=7A3F0000-0000-446A-8000-702F00006273

cd u-boot
./make.sh CROSS_COMPILE=/home/mzhang/home_work/minix/lubancat2/sdk/gen_sdk/prebuilts/gcc/linux-x86/aarch64/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu- rk3568
commands will compile and packing everything

uboot has 2 parts , which are compiling and packing
3) compile uboot, it is just very stand process of uboot which we just talked about above.


4) create a u-boot.its file 

arch/arm/mach-rockchip/make_fit_atf.sh -t 0x08400000 -c gzip   > u-boot.its

u-boot.its example:
/*
 * Copyright (C) 2020 Rockchip Electronic Co.,Ltd
 *
 * Simple U-boot fit source file containing ATF/OP-TEE/U-Boot/dtb/MCU
 */

/dts-v1/;

/ {
	description = "FIT Image with ATF/OP-TEE/U-Boot/MCU";
	#address-cells = <1>;

	images {

		uboot {
			description = "U-Boot";
			data = /incbin/("u-boot-nodtb.bin.gz");
			type = "standalone";
			arch = "arm64";
			os = "U-Boot";
			compression = "gzip";
			load = <0x00a00000>;
			digest {
				value = /incbin/("./u-boot-nodtb.bin.digest");
				algo = "sha256";
			};
			hash {
				algo = "sha256";
			};
		};
		atf-1 {
			description = "ARM Trusted Firmware";
			data = /incbin/("./bl31_0x00040000.bin.gz");
			type = "firmware";
			arch = "arm64";
			os = "arm-trusted-firmware";
			compression = "gzip";
			load = <0x00040000>;
			hash {
				algo = "sha256";
			};
			digest {
				value = /incbin/("./bl31_0x00040000.bin.digest");
				algo = "sha256";
			};
		};
		atf-2 {
			description = "ARM Trusted Firmware";
			data = /incbin/("./bl31_0xfdcc1000.bin");
			type = "firmware";
			arch = "arm64";
			os = "arm-trusted-firmware";
			compression = "none";
			load = <0xfdcc1000>;
			hash {
				algo = "sha256";
			};
		};
		atf-3 {
			description = "ARM Trusted Firmware";
			data = /incbin/("./bl31_0x0005c000.bin");
			type = "firmware";
			arch = "arm64";
			os = "arm-trusted-firmware";
			compression = "none";
			load = <0x0005c000>;
			hash {
				algo = "sha256";
			};
		};
		atf-4 {
			description = "ARM Trusted Firmware";
			data = /incbin/("./bl31_0xfdcce000.bin");
			type = "firmware";
			arch = "arm64";
			os = "arm-trusted-firmware";
			compression = "none";
			load = <0xfdcce000>;
			hash {
				algo = "sha256";
			};
		};
		atf-5 {
			description = "ARM Trusted Firmware";
			data = /incbin/("./bl31_0xfdcd0000.bin");
			type = "firmware";
			arch = "arm64";
			os = "arm-trusted-firmware";
			compression = "none";
			load = <0xfdcd0000>;
			hash {
				algo = "sha256";
			};
		};
		atf-6 {
			description = "ARM Trusted Firmware";
			data = /incbin/("./bl31_0x0005a000.bin");
			type = "firmware";
			arch = "arm64";
			os = "arm-trusted-firmware";
			compression = "none";
			load = <0x0005a000>;
			hash {
				algo = "sha256";
			};
		};
		optee {
			description = "OP-TEE";
			data = /incbin/("tee.bin.gz");
			type = "firmware";
			arch = "arm64";
			os = "op-tee";
			compression = "gzip";
			
			load = <0x8400000>;
			digest {
				value = /incbin/("./tee.bin.digest");
				algo = "sha256";
			};
			hash {
				algo = "sha256";
			};
		};
		fdt {
			description = "U-Boot dtb";
			data = /incbin/("./u-boot.dtb");
			type = "flat_dt";
			arch = "arm64";
			compression = "none";
			hash {
				algo = "sha256";
			};
		};
	};

	configurations {
		default = "conf";
		conf {
			description = "rk3568-evb";
			rollback-index = <0x0>;
			firmware = "atf-1";
			loadables = "uboot", "atf-2", "atf-3", "atf-4", "atf-5", "atf-6", "optee";
			
			fdt = "fdt";
			signature {
								algo = "sha256,rsa2048";
				
				key-name-hint = "dev";
				sign-images = "firmware", "loadables", "fdt";
			};
		};
	};
};

5) create u-boot.itb(u-boot.its is source file for packing a FIT format mutilp program bin, its a concept of the u-boot packing, it purpose is make 1 container contains more bins , it will be reconigzed by rk356x_spl_loader_v1.23.114.bin)

./tools/mkimage -f u-boot.its -E -p 0x1200 fit/uboot.itb -v 0

6) 2times
cat fit/uboot.itb >> uboot.img
truncate -s %4096K uboot.img
cat fit/uboot.itb >> uboot.img
truncate -s %4096K uboot.img


7) flash uboot.img to partion name "uboot"
sudo ./rkdeveloptool wlx uboot uboot.img

8) reset board here you go.

# 7) if we could like to direct boot from our booloader min.bin,please do following. get all files from u-boot folder.
u-boot-nodtb.bin.gz
u-boot-nodtb.bin.digest
bl31_0x00040000.bin.gz
bl31_0x00040000.bin.digest
bl31_0x0005c000.bin
bl31_0x0005a000.bin
bl31_0xfdcc1000.bin  
bl31_0xfdcd0000.bin
bl31_0xfdcce000.bin  
u-boot.dtb           
tee.bin.gz
tee.bin.digest
mkimage 
min.its

./mkimage -f min.its -E -p 0x1200 min.itb -v 0
min.itb      

cat min.itb >> min.img
truncate -s %4096K uboot.img
cat min.itb >> min.img
truncate -s %4096K min.img

sudo ./rkdeveloptool wlx uboot min.img





###################################################
###################################################
###################################################
# porting minix is huge big work for arm64
cp ./obj.evbearm-el/destdir.evbearm-el/usr/include/ ../lubancat2/bootloader/baremetal_aarch64_rk3568/src/ -r
cp ./minix3os_learning/minix ../lubancat2/bootloader/baremetal_aarch64_rk3568/src/ -r


