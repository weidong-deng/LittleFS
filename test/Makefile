# LittleFS Test Makefile
#
# 每个测试文件独立编译为可执行文件。

# 编译器
CC = gcc
CFLAGS = -Wall -Wextra -g -O0 --coverage -DLFS_ASSERT_TEST_FAIL=1
LDFLAGS = --coverage

# 包含目录
CFLAGS += -IUnity/src
CFLAGS += -I../Littlefs
CFLAGS += -I../Littlefs/port
CFLAGS += -I../Littlefs/bd
CFLAGS += -Icommon
CFLAGS += -Istubs

# 库源文件
UNITY_SRC = Unity/src/unity.c
LFS_CORE_SRC = ../Littlefs/lfs.c ../Littlefs/lfs_util.c
LFS_BD_SRC = ../Littlefs/bd/lfs_rambd.c ../Littlefs/bd/lfs_emubd.c
STUB_SRC = stubs/lfs_flash_int_port_stub.c stubs/lfs_spi_flash_port_stub.c
APP_SRC = ../Littlefs/port/lfs_app.c
COMMON_SRC = common/test_helpers.c

# 构建目录
BUILD_DIR = build

.PHONY: all clean test coverage help

all: $(BUILD_DIR)/test_lfs_format \
     $(BUILD_DIR)/test_lfs_file \
     $(BUILD_DIR)/test_lfs_dir \
     $(BUILD_DIR)/test_lfs_fs \
     $(BUILD_DIR)/test_lfs_app_init \
     $(BUILD_DIR)/test_lfs_app_file \
     $(BUILD_DIR)/test_lfs_app_dir \
     $(BUILD_DIR)/test_lfs_app_info \
     $(BUILD_DIR)/test_lfs_app_dual_storage \
     $(BUILD_DIR)/test_rambd \
     $(BUILD_DIR)/test_emubd \
     $(BUILD_DIR)/test_spi_flash_port \
     $(BUILD_DIR)/test_int_flash_port \
     $(BUILD_DIR)/test_device_fault \
     $(BUILD_DIR)/test_fs_corruption \
     $(BUILD_DIR)/test_power_loss \
     $(BUILD_DIR)/test_wear_leveling

# 核心库测试
$(BUILD_DIR)/test_lfs_format: $(UNITY_SRC) $(LFS_CORE_SRC) $(LFS_BD_SRC) $(COMMON_SRC) test_lfs_core/test_lfs_format.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/test_lfs_file: $(UNITY_SRC) $(LFS_CORE_SRC) $(LFS_BD_SRC) $(COMMON_SRC) test_lfs_core/test_lfs_file.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/test_lfs_dir: $(UNITY_SRC) $(LFS_CORE_SRC) $(LFS_BD_SRC) $(COMMON_SRC) test_lfs_core/test_lfs_dir.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/test_lfs_fs: $(UNITY_SRC) $(LFS_CORE_SRC) $(LFS_BD_SRC) $(COMMON_SRC) test_lfs_core/test_lfs_fs.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# 应用层测试（需要 stub）
$(BUILD_DIR)/test_lfs_app_init: $(UNITY_SRC) $(LFS_CORE_SRC) $(LFS_BD_SRC) $(COMMON_SRC) $(APP_SRC) $(STUB_SRC) test_lfs_app/test_lfs_app_init.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/test_lfs_app_file: $(UNITY_SRC) $(LFS_CORE_SRC) $(LFS_BD_SRC) $(COMMON_SRC) $(APP_SRC) $(STUB_SRC) test_lfs_app/test_lfs_app_file.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/test_lfs_app_dir: $(UNITY_SRC) $(LFS_CORE_SRC) $(LFS_BD_SRC) $(COMMON_SRC) $(APP_SRC) $(STUB_SRC) test_lfs_app/test_lfs_app_dir.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/test_lfs_app_info: $(UNITY_SRC) $(LFS_CORE_SRC) $(LFS_BD_SRC) $(COMMON_SRC) $(APP_SRC) $(STUB_SRC) test_lfs_app/test_lfs_app_info.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/test_lfs_app_dual_storage: $(UNITY_SRC) $(LFS_CORE_SRC) $(LFS_BD_SRC) $(COMMON_SRC) $(APP_SRC) $(STUB_SRC) test_lfs_app/test_lfs_app_dual_storage.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# 块设备测试
$(BUILD_DIR)/test_rambd: $(UNITY_SRC) $(LFS_CORE_SRC) $(LFS_BD_SRC) $(COMMON_SRC) test_block_device/test_rambd.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/test_emubd: $(UNITY_SRC) $(LFS_CORE_SRC) $(LFS_BD_SRC) $(COMMON_SRC) test_block_device/test_emubd.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# 端口层测试（需要 stub）
$(BUILD_DIR)/test_spi_flash_port: $(UNITY_SRC) $(LFS_CORE_SRC) $(LFS_BD_SRC) $(COMMON_SRC) $(APP_SRC) $(STUB_SRC) test_port/test_spi_flash_port.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/test_int_flash_port: $(UNITY_SRC) $(LFS_CORE_SRC) $(LFS_BD_SRC) $(COMMON_SRC) $(APP_SRC) $(STUB_SRC) test_port/test_int_flash_port.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# 异常注入测试
$(BUILD_DIR)/test_device_fault: $(UNITY_SRC) $(LFS_CORE_SRC) $(LFS_BD_SRC) $(COMMON_SRC) test_fault_injection/test_device_fault.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/test_fs_corruption: $(UNITY_SRC) $(LFS_CORE_SRC) $(LFS_BD_SRC) $(COMMON_SRC) test_fault_injection/test_fs_corruption.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/test_power_loss: $(UNITY_SRC) $(LFS_CORE_SRC) $(LFS_BD_SRC) $(COMMON_SRC) test_fault_injection/test_power_loss.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/test_wear_leveling: $(UNITY_SRC) $(LFS_CORE_SRC) $(LFS_BD_SRC) $(COMMON_SRC) test_fault_injection/test_wear_leveling.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# 运行所有测试
test: all
	@echo "========== Running Core Tests =========="
	@$(BUILD_DIR)/test_lfs_format
	@$(BUILD_DIR)/test_lfs_file
	@$(BUILD_DIR)/test_lfs_dir
	@$(BUILD_DIR)/test_lfs_fs
	@echo ""
	@echo "========== Running App Tests =========="
	@$(BUILD_DIR)/test_lfs_app_init
	@$(BUILD_DIR)/test_lfs_app_file
	@$(BUILD_DIR)/test_lfs_app_dir
	@$(BUILD_DIR)/test_lfs_app_info
	@$(BUILD_DIR)/test_lfs_app_dual_storage
	@echo ""
	@echo "========== Running Block Device Tests =========="
	@$(BUILD_DIR)/test_rambd
	@$(BUILD_DIR)/test_emubd
	@echo ""
	@echo "========== Running Port Tests =========="
	@$(BUILD_DIR)/test_spi_flash_port
	@$(BUILD_DIR)/test_int_flash_port
	@echo ""
	@echo "========== Running Fault Injection Tests =========="
	@$(BUILD_DIR)/test_device_fault
	@$(BUILD_DIR)/test_fs_corruption
	@$(BUILD_DIR)/test_power_loss
	@$(BUILD_DIR)/test_wear_leveling
	@echo ""
	@echo "========== All Tests Complete =========="

# 生成覆盖率报告
coverage: all
	@mkdir -p $(BUILD_DIR)/coverage
	@gcovr --root .. --exclude ".*Unity.*" --html --html-details -o $(BUILD_DIR)/coverage/coverage_report.html .
	@echo "Coverage report generated: $(BUILD_DIR)/coverage/coverage_report.html"

# 清理
clean:
	rm -rf $(BUILD_DIR)

# 帮助
help:
	@echo "Available targets:"
	@echo "  all       - Build all test executables"
	@echo "  test      - Run all tests"
	@echo "  coverage  - Generate code coverage report"
	@echo "  clean     - Clean build files"
	@echo "  help      - Show this help message"
