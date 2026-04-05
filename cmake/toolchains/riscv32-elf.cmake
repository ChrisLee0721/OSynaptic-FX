set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv32)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_COMPILER riscv64-unknown-elf-gcc)
set(CMAKE_AR riscv64-unknown-elf-ar)
set(CMAKE_RANLIB riscv64-unknown-elf-ranlib)

set(CMAKE_C_FLAGS_INIT "-march=rv32imac -mabi=ilp32 -ffreestanding")

