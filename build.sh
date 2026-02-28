#!/bin/bash
echo "init env vars..."

CROSS_COMPILE=./tools/prebuilts/gcc/linux-x86/aarch64/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-
CC=${CROSS_COMPILE}gcc
AS=${CROSS_COMPILE}as
LD=${CROSS_COMPILE}ld
OBJCOPY=${CROSS_COMPILE}objcopy
OBJDUMP=${CROSS_COMPILE}objdump 

echo "init env vars completed..."

function clean() {
  echo "clean..."
  rm -rf ./build
}

function check(){
    CHECK_DIR="./tools/prebuilts"
    if [ -d "${CHECK_DIR}" ]; then
        echo "folder ${CHECK_DIR} is there，skip tar xvf prebuilts_gcc.tar.gz"
    else
        echo "folder ${CHECK_DIR} not there，run tar xvf prebuilts_gcc.tar.gz："
        cd tools
        tar xvf prebuilts_gcc.tar.gz
        cd ..
    fi
}

function build() {
  echo "compiling..."
  c_flag="-g -O0 -ffreestanding -nostdlib"
  boot_src="boot"
  src_src="src"

  build_dir="build"
  mkdir -p ${build_dir}

  mkdir -p "${build_dir}/${boot_src}"
  ${CC} ${c_flag} -c ${boot_src}/start.s -o "${build_dir}/${boot_src}/_start.o"


  for src_file in ${src_src}/*.c; do
    dirpath=$(dirname "$src_file")
    mkdir -p "${build_dir}/${dirpath}"
    ${CC} ${c_flag} -c "$src_file" -o "${build_dir}/${src_file%.c}.o"
  done

  echo "linking..."
  ${CC} ${c_flag} -T./boot/link.ld $(find ${build_dir} -type f -name "*.o") -o "${build_dir}/min.elf"

  echo "copy binary..."
  ${OBJCOPY} -O binary "${build_dir}/min.elf" "${build_dir}/min.img"

  echo "disassembling..."
  ${OBJDUMP} -S "${build_dir}/min.elf" > "${build_dir}/min.disasm"
  cat ${build_dir}/min.disasm
}

function start() {
  echo "start ..."
  echo "nothing happening now ,but will later"
}



if [ "$1" == "run" ]; then
    echo "doing clean"
    clean
    echo "doing check"
    check
    echo "doing build"
    build
    echo "start run"
    start run
elif [ "$1" == "build" ]; then
    echo "doing clean"
    clean
    echo "doing check"
    check
    echo "doing build"
    build
elif [ "$1" == "clean" ]; then
    echo "doing clean"
    clean
else
    echo "Usage: demo.sh {run|clean|build}"
fi


