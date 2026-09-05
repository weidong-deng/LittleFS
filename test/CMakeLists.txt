cmake_minimum_required(VERSION 3.10)
project(LittleFS_Tests C)

# 设置 C 标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 编译选项
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -g -O0 --coverage)
    add_link_options(--coverage)
    # -mconsole 仅 MinGW：避免 Windows 下链接成 GUI 子系统
    if(WIN32 OR MINGW OR CYGWIN)
        add_link_options(-mconsole)
    endif()
endif()

# 测试环境：让 LFS_ASSERT 失败时退出而不是死循环
add_compile_definitions(LFS_ASSERT_TEST_FAIL=1)

# 包含目录
include_directories(
    Unity/src
    ../Littlefs
    ../Littlefs/port
    ../Littlefs/bd
    common
    stubs
)

# ==========================================
# 库目标
# ==========================================

# Unity 库
add_library(unity STATIC Unity/src/unity.c)

# LittleFS 核心库
add_library(lfs_core STATIC
    ../Littlefs/lfs.c
    ../Littlefs/lfs_util.c
)

# 块设备库
add_library(lfs_bd STATIC
    ../Littlefs/bd/lfs_rambd.c
    ../Littlefs/bd/lfs_emubd.c
)

# Stub 库（替代硬件端口层）
add_library(lfs_stubs STATIC
    stubs/lfs_flash_int_port_stub.c
    stubs/lfs_spi_flash_port_stub.c
)

# 应用层库
add_library(lfs_app STATIC
    ../Littlefs/port/lfs_app.c
)

# 公共测试代码库
add_library(test_common STATIC
    common/test_helpers.c
)

# 公共链接库
set(COMMON_LIBS unity lfs_bd lfs_core test_common)

# ==========================================
# 测试可执行文件（每个测试文件独立编译）
# ==========================================

# 核心库测试
add_executable(test_lfs_format test_lfs_core/test_lfs_format.c)
target_link_libraries(test_lfs_format ${COMMON_LIBS})

add_executable(test_lfs_file test_lfs_core/test_lfs_file.c)
target_link_libraries(test_lfs_file ${COMMON_LIBS})

add_executable(test_lfs_dir test_lfs_core/test_lfs_dir.c)
target_link_libraries(test_lfs_dir ${COMMON_LIBS})

add_executable(test_lfs_fs test_lfs_core/test_lfs_fs.c)
target_link_libraries(test_lfs_fs ${COMMON_LIBS})

# 应用层测试
add_executable(test_lfs_app_init test_lfs_app/test_lfs_app_init.c)
target_link_libraries(test_lfs_app_init lfs_app lfs_stubs ${COMMON_LIBS})

add_executable(test_lfs_app_file test_lfs_app/test_lfs_app_file.c)
target_link_libraries(test_lfs_app_file lfs_app lfs_stubs ${COMMON_LIBS})

add_executable(test_lfs_app_dir test_lfs_app/test_lfs_app_dir.c)
target_link_libraries(test_lfs_app_dir lfs_app lfs_stubs ${COMMON_LIBS})

add_executable(test_lfs_app_info test_lfs_app/test_lfs_app_info.c)
target_link_libraries(test_lfs_app_info lfs_app lfs_stubs ${COMMON_LIBS})

add_executable(test_lfs_app_dual_storage test_lfs_app/test_lfs_app_dual_storage.c)
target_link_libraries(test_lfs_app_dual_storage lfs_app lfs_stubs ${COMMON_LIBS})

# 块设备测试
add_executable(test_rambd test_block_device/test_rambd.c)
target_link_libraries(test_rambd ${COMMON_LIBS})

add_executable(test_emubd test_block_device/test_emubd.c)
target_link_libraries(test_emubd ${COMMON_LIBS})

# 端口层测试
add_executable(test_spi_flash_port test_port/test_spi_flash_port.c)
target_link_libraries(test_spi_flash_port lfs_app lfs_stubs ${COMMON_LIBS})

add_executable(test_int_flash_port test_port/test_int_flash_port.c)
target_link_libraries(test_int_flash_port lfs_app lfs_stubs ${COMMON_LIBS})

# 异常注入测试
add_executable(test_device_fault test_fault_injection/test_device_fault.c)
target_link_libraries(test_device_fault ${COMMON_LIBS})

add_executable(test_fs_corruption test_fault_injection/test_fs_corruption.c)
target_link_libraries(test_fs_corruption ${COMMON_LIBS})

add_executable(test_power_loss test_fault_injection/test_power_loss.c)
target_link_libraries(test_power_loss ${COMMON_LIBS})

add_executable(test_wear_leveling test_fault_injection/test_wear_leveling.c)
target_link_libraries(test_wear_leveling ${COMMON_LIBS})

# ==========================================
# CTest 集成
# ==========================================

enable_testing()
add_test(NAME test_lfs_format COMMAND test_lfs_format)
add_test(NAME test_lfs_file COMMAND test_lfs_file)
add_test(NAME test_lfs_dir COMMAND test_lfs_dir)
add_test(NAME test_lfs_fs COMMAND test_lfs_fs)
add_test(NAME test_lfs_app_init COMMAND test_lfs_app_init)
add_test(NAME test_lfs_app_file COMMAND test_lfs_app_file)
add_test(NAME test_lfs_app_dir COMMAND test_lfs_app_dir)
add_test(NAME test_lfs_app_info COMMAND test_lfs_app_info)
add_test(NAME test_lfs_app_dual_storage COMMAND test_lfs_app_dual_storage)
add_test(NAME test_rambd COMMAND test_rambd)
add_test(NAME test_emubd COMMAND test_emubd)
add_test(NAME test_spi_flash_port COMMAND test_spi_flash_port)
add_test(NAME test_int_flash_port COMMAND test_int_flash_port)
add_test(NAME test_device_fault COMMAND test_device_fault)
add_test(NAME test_fs_corruption COMMAND test_fs_corruption)
add_test(NAME test_power_loss COMMAND test_power_loss)
add_test(NAME test_wear_leveling COMMAND test_wear_leveling)

# ==========================================
# 自定义目标
# ==========================================

# 运行所有测试
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    COMMENT "Running all tests..."
)

# 生成覆盖率报告（包含源文件和测试代码）
add_custom_target(coverage
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage_report
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR} gcovr --root ${PROJECT_SOURCE_DIR}/.. --exclude ".*Unity.*" --html-details coverage_report/coverage.html --print-summary
    COMMENT "Generating code coverage report (source + test)..."
)
