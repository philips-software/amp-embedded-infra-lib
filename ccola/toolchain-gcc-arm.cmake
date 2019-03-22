include(CMakeForceCompiler)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CCOLA_GCCARM_VERSION "5.4 2016q3")
set(CMAKE_C_COMPILER "C:/Program Files (x86)/GNU Tools ARM Embedded/${CCOLA_GCCARM_VERSION}/bin/arm-none-eabi-gcc.exe")
set(CMAKE_CXX_COMPILER "C:/Program Files (x86)/GNU Tools ARM Embedded/${CCOLA_GCCARM_VERSION}/bin/arm-none-eabi-g++.exe")

set(CMAKE_ASM_COMPILER "C:/Program Files (x86)/GNU Tools ARM Embedded/${CCOLA_GCCARM_VERSION}/bin/arm-none-eabi-as.exe" CACHE FILEPATH as)
set(CMAKE_AR "C:/Program Files (x86)/GNU Tools ARM Embedded/${CCOLA_GCCARM_VERSION}/bin/arm-none-eabi-gcc-ar.exe" CACHE FILEPATH ar)
set(CMAKE_RANLIB "C:/Program Files (x86)/GNU Tools ARM Embedded/${CCOLA_GCCARM_VERSION}/bin/arm-none-eabi-gcc-ranlib.exe" CACHE FILEPATH ranlib)
set(CMAKE_NM "C:/Program Files (x86)/GNU Tools ARM Embedded/${CCOLA_GCCARM_VERSION}/bin/arm-none-eabi-gcc-rm.exe" CACHE FILEPATH nm)

set(CCOLA_LD_TOOL "C:/Program Files (x86)/GNU Tools ARM Embedded/${CCOLA_GCCARM_VERSION}/bin/arm-none-eabi-ld.exe")
set(CCOLA_APP_SIZE_TOOL "C:/Program Files (x86)/GNU Tools ARM Embedded/${CCOLA_GCCARM_VERSION}/bin/arm-none-eabi-size.exe")
set(CCOLA_OBJ_COPY_TOOL "C:/Program Files (x86)/GNU Tools ARM Embedded/${CCOLA_GCCARM_VERSION}/bin/arm-none-eabi-objcopy.exe")
set(CCOLA_OBJ_DUMP_TOOL "C:/Program Files (x86)/GNU Tools ARM Embedded/${CCOLA_GCCARM_VERSION}/bin/arm-none-eabi-objdump.exe")
set(CCOLA_DEBUG_TOOL "C:/Program Files (x86)/GNU Tools ARM Embedded/${CCOLA_GCCARM_VERSION}/bin/arm-none-eabi-gdb.exe")
