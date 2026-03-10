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
  echo "compiling preparing..."
  c_flag="-g -O0 -ffreestanding -nostdlib"
  l_flag="-lc -lgcc"
  BOOT_SRC="boot"
  SRC_SRC="src"
  MINIX_S="minix_s"
  CODE_DEFINE=""

  HEADER_INCLUDE="-I./${SRC_SRC} -I./${SRC_SRC}/${MINIX_S}"


  build_dir="build"
  mkdir -p ${build_dir}

  mkdir -p "${build_dir}/${BOOT_SRC}"
  echo "compiling... ${CC} ${c_flag} -c ${BOOT_SRC}/start.s -o "${build_dir}/${BOOT_SRC}/_start.o""
  ${CC} ${c_flag} -c ${BOOT_SRC}/start.s -o "${build_dir}/${BOOT_SRC}/_start.o"

  #compile src
  for src_file in ${SRC_SRC}/*.c; do
    dirpath=$(dirname "$src_file")
    mkdir -p "${build_dir}/${dirpath}"
    echo "compiling... ${CC} ${c_flag} ${HEADER_INCLUDE} ${CODE_DEFINE} -c "$src_file" -o "${build_dir}/${src_file%.c}.o""
    ${CC} ${c_flag} ${HEADER_INCLUDE} ${CODE_DEFINE} -c "$src_file" -o "${build_dir}/${src_file%.c}.o"
  done

  #compile src/minix_s src files
  for src_file in ${SRC_SRC}/${MINIX_S}/*.c; do
    dirpath=$(dirname "$src_file")
    mkdir -p "${build_dir}/${dirpath}"
    echo "compiling... ${CC} ${c_flag} ${HEADER_INCLUDE} ${CODE_DEFINE} -c "$src_file" -o "${build_dir}/${src_file%.c}.o""
    ${CC} ${c_flag} ${HEADER_INCLUDE} ${CODE_DEFINE} -c "$src_file" -o "${build_dir}/${src_file%.c}.o"
  done

  echo "linking...${CC} ${c_flag} ${l_flag} -T./boot/link.ld $(find ${build_dir} -type f -name "*.o") -o "${build_dir}/min.elf""
  ${CC} ${c_flag} ${l_flag} -T./boot/link.ld $(find ${build_dir} -type f -name "*.o") -o "${build_dir}/min.elf"

  echo "copy binary...${OBJCOPY} -O binary "${build_dir}/min.elf" "${build_dir}/min.bin""
  ${OBJCOPY} -O binary "${build_dir}/min.elf" "${build_dir}/min.bin"

  echo "disassembling...${OBJDUMP} -S "${build_dir}/min.elf" > "${build_dir}/min.disasm""
  ${OBJDUMP} -S "${build_dir}/min.elf" > "${build_dir}/min.disasm"
  cat ${build_dir}/min.disasm
}
function pack() {
  echo "packing..."
  
  cd rockchipbins_uboottoolbins
  rm -rf min.bin min.bin.digest min.bin.gz min.img min.itb
  cp ../build/min.bin ./

  openssl dgst -sha256 -binary -out min.bin.digest min.bin
	COMPRESS_CMD="gzip -kf9"
  MIN_SZ=`ls -l min.bin | awk '{ print $5 }'`
  if [ ${MIN_SZ} -gt 0 ]; then
    ${COMPRESS_CMD} min.bin 
  else
    touch min.bin.digest
  fi

  ./mkimage -f min.its -E -p 0x1200 min.itb -v 0
  cat min.itb >> min.img
  truncate -s %4096K min.img
  cat min.itb >> min.img
  truncate -s %4096K min.img
  cd ..
  # TODO
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
    echo "doing pack"
   pack
elif [ "$1" == "clean" ]; then
    echo "doing clean"
    clean
else
    echo "Usage: demo.sh {run|clean|build}"
fi


