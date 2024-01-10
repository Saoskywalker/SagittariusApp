#限制最低cmake版本
cmake_minimum_required(VERSION 3.12)

#cmake工程名字 
project (SagittariusApp)

#设置C编译器特性
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -s USE_SDL=2 -s USE_SDL_MIXER=2 -s USE_SDL_IMAGE=2 -s USE_SDL_NET=2")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")

#宏定义, 类似gcc的-D
add_definitions(-D_USER_DEBUG)
add_definitions(-DLCD_MODEL_TM080SDH01)

#定义变量
set(ARCH x64)
set(MACH mingw)
set(BOARD pc)

include_directories(arch/x64/public/include)
include_directories(arch/x64/mingw/include)

include_directories(arch/${ARCH}/${MACH}/port/include)

set(SRC_C ${SRC_C} arch/${ARCH}/${MACH}/port/MTF_io.c)
set(SRC_C ${SRC_C} arch/${ARCH}/${MACH}/port/file_operate_hal.c)
set(SRC_C ${SRC_C} arch/${ARCH}/${MACH}/port/delay.c)
set(SRC_C ${SRC_C} arch/${ARCH}/${MACH}/port/framebuffer_port.c)
set(SRC_C ${SRC_C} arch/${ARCH}/${MACH}/port/audio_port.c)
set(SRC_C ${SRC_C} arch/${ARCH}/${MACH}/port/touch_port.c)
set(SRC_C ${SRC_C} arch/${ARCH}/${MACH}/port/PWM_port.c)
set(SRC_C ${SRC_C} arch/${ARCH}/${MACH}/port/GPIO_port.c)
set(SRC_C ${SRC_C} arch/${ARCH}/${MACH}/port/ROM_port.c)
set(SRC_C ${SRC_C} arch/${ARCH}/${MACH}/port/uart_port.c)
set(SRC_C ${SRC_C} arch/${ARCH}/${MACH}/port/timer_port.c)
set(SRC_C ${SRC_C} arch/${ARCH}/${MACH}/port/system_port.c)

include_directories(board/${BOARD}/include)

#board src
set(SRC_C ${SRC_C} board/${BOARD}/licence.c)
set(SRC_C ${SRC_C} board/${BOARD}/system_board.c)

#设置头文件路径, 相当于gcc的-I
include_directories(SagittariusUI/include)
include_directories(hal/include)
include_directories(lib/libx/include)
include_directories(lib/UI)
include_directories(lib/PICTURE)
include_directories(lib/TOUCH)
include_directories(lib/TEXT)
include_directories(lib/crypto)
include_directories(lib/FreeModbus_Slave_Master_v16/port)
include_directories(lib/FreeModbus_Slave_Master_v16/modbus/rtu)
include_directories(lib/ComPort)
include_directories(lib/music_library)
include_directories(lib/debug_library)
include_directories(lib/DW_Lib)

#hal src
set(SRC_C ${SRC_C} hal/beep.c)
# set(SRC_C ${SRC_C} hal/dma_pool.c)

# usr library src
# set(SRC_C ${SRC_C} lib/libx/fifo.c)
set(SRC_C ${SRC_C} lib/PICTURE/bmp.c)
set(SRC_C ${SRC_C} lib/PICTURE/gif.c)
set(SRC_C ${SRC_C} lib/PICTURE/tjpgd.c)
set(SRC_C ${SRC_C} lib/PICTURE/lodepng.c)
set(SRC_C ${SRC_C} lib/PICTURE/piclib.c)
set(SRC_C ${SRC_C} lib/TOUCH/touch.c)
set(SRC_C ${SRC_C} lib/TOUCH/ts_calibrate_common.c)
set(SRC_C ${SRC_C} lib/TEXT/text.c)
set(SRC_C ${SRC_C} lib/TEXT/text_rect.c)
set(SRC_C ${SRC_C} lib/TEXT/font.c)
set(SRC_C ${SRC_C} lib/UI/UI_engine.c)
set(SRC_C ${SRC_C} lib/UI/lcd.c)
set(SRC_C ${SRC_C} lib/crypto/crc32.c)
set(SRC_C ${SRC_C} lib/crypto/sha256.c)
# set(SRC_C ${SRC_C} lib/ComPort/dgus2Com.c)
set(SRC_C ${SRC_C} lib/ComPort/ComPort.c)
# set(SRC_C ${SRC_C} lib/ComPort/MTF_ComProtocol.c)
# set(SRC_C ${SRC_C} lib/ComPort/T5UIC2_agreement.c)
set(SRC_C ${SRC_C} lib/FreeModbus_Slave_Master_v16/modbus/rtu/mbcrc.c)
set(SRC_C ${SRC_C} lib/music_library/music_play.c)
set(SRC_C ${SRC_C} lib/debug_library/my_assert.c)
set(SRC_C ${SRC_C} lib/DW_Lib/DW_FileDecodeLib.c)
set(SRC_C ${SRC_C} lib/DW_Lib/dwDriver.c)
set(SRC_C ${SRC_C} lib/DW_Lib/dwDriveD2.c)

#mp3 library
include_directories(lib/Mp3Lib/real)
include_directories(lib/Mp3Lib/pub)
set(SRC_C ${SRC_C} lib/Mp3Lib/real/bitstream.c)
set(SRC_C ${SRC_C} lib/Mp3Lib/real/buffers.c)
set(SRC_C ${SRC_C} lib/Mp3Lib/real/dct32.c)
set(SRC_C ${SRC_C} lib/Mp3Lib/real/dequant.c)
set(SRC_C ${SRC_C} lib/Mp3Lib/real/dqchan.c)
set(SRC_C ${SRC_C} lib/Mp3Lib/real/huffman.c)
set(SRC_C ${SRC_C} lib/Mp3Lib/real/hufftabs.c)
set(SRC_C ${SRC_C} lib/Mp3Lib/real/imdct.c)
set(SRC_C ${SRC_C} lib/Mp3Lib/real/polyphase.c)
set(SRC_C ${SRC_C} lib/Mp3Lib/real/scalfact.c)
set(SRC_C ${SRC_C} lib/Mp3Lib/real/stproc.c)
set(SRC_C ${SRC_C} lib/Mp3Lib/real/subband.c)
set(SRC_C ${SRC_C} lib/Mp3Lib/real/trigtabs.c)
set(SRC_C ${SRC_C} lib/Mp3Lib/mp3_play.c)
set(SRC_C ${SRC_C} lib/Mp3Lib/mp3dec.c)
set(SRC_C ${SRC_C} lib/Mp3Lib/mp3tabs.c)

#usr src
set(SRC_C ${SRC_C} SagittariusUI/Sagittarius_global.c)
set(SRC_C ${SRC_C} SagittariusUI/Sagittarius_timer.c)
set(SRC_C ${SRC_C} SagittariusUI/cJSON.c)
set(SRC_C ${SRC_C} SagittariusUI/cJSON_extend.c)
# set(SRC_C ${SRC_C} SagittariusUI/TestBoard.c)
set(SRC_C ${SRC_C} SagittariusUI/MTF_HMI.c)
set(SRC_C ${SRC_C} SagittariusUI/file_type.c)
set(SRC_C ${SRC_C} user_src/main.c)

#c++ source files
set(SRC_CXX ${SRC_CXX} arch/x64/mingw/Serial.cpp)

#设置库文件路径, 相当于gcc的-L
# link_directories(lib)

#添加子cmakelists文件
# add_subdirectory(lvgl)
# add_subdirectory(lv_drivers)

#扫描指定文件并添加到编译文件目录
# file(GLOB MY_SOURCES "./*.c")
# set(SOURCES ${MY_SOURCES})

#设置带编译文件, 可包括C/C++
# set(SRC_C main.c)
# set(SRC_C ${SRC_C} test_sdl.c)

#编译生成名为index可执行文件
add_executable(index ${SRC_C})
# add_executable(index ${SRC_C} ${SRC_CXX})

#按输入参数选择
# if(NOT LVGL_CHOSEN_DEMO)
#     set(LVGL_CHOSEN_DEMO lv_demo_widgets)
# endif()

#设置某个文件特性
# set_source_files_properties(main.c PROPERTIES COMPILE_FLAGS -DCHOSEN_DEMO=${LVGL_CHOSEN_DEMO})

#设置可执行文件后缀
set(CMAKE_EXECUTABLE_SUFFIX ".html")
# set(CMAKE_EXECUTABLE_SUFFIX ".exe")

#index文件需要使用到的库名称, PS:使用GCC规则, 缩写名, 如libSDL2.so, 则为SDL2
# target_link_libraries(index
#     gcc mingw32 SDL2main SDL2 winmm stdc++
# )

#设置运行文件特性
#按要求生成合适的HTML文件
set_target_properties(index PROPERTIES LINK_FLAGS "--shell-file ${PROJECT_SOURCE_DIR}/arch/x64/emscripten/tools/emsdk_shell.html -s SINGLE_FILE=0 -s FORCE_FILESYSTEM=1")
# set_target_properties(index PROPERTIES LINK_FLAGS "../ico/project_ico.res -mwindows")
