/**
 * @file test_power_loss.c
 * @brief Power loss simulation tests
 */

#include "unity.h"
#include "lfs.h"
#include "lfs_emubd.h"
#include <string.h>
#include <stdlib.h>

#define TEST_BLOCK_SIZE     1024
#define TEST_BLOCK_COUNT    64

static lfs_t lfs;
static struct lfs_config config;
static lfs_emubd_t bd;
static struct lfs_emubd_config bd_config;

void setUp(void) {
    memset(&bd_config, 0, sizeof(bd_config));
    bd_config.read_size = 16;
    bd_config.prog_size = 16;
    bd_config.erase_size = TEST_BLOCK_SIZE;
    bd_config.erase_count = TEST_BLOCK_COUNT;
    bd_config.erase_value = -1;

    memset(&config, 0, sizeof(config));
    config.context = &bd;
    config.read = lfs_emubd_read;
    config.prog = lfs_emubd_prog;
    config.erase = lfs_emubd_erase;
    config.sync = lfs_emubd_sync;
    config.read_size = 16;
    config.prog_size = 16;
    config.block_size = TEST_BLOCK_SIZE;
    config.block_count = TEST_BLOCK_COUNT;
    config.cache_size = 256;
    config.lookahead_size = 16;
    config.block_cycles = 500;

    int err = lfs_emubd_create(&config, &bd_config);
    TEST_ASSERT_EQUAL(0, err);
}

void tearDown(void) {
    lfs_emubd_destroy(&config);
}

void test_power_loss_powercycles_tracking(void) {
    /* Initial power cycles should be 0 */
    int32_t initial_cycles = lfs_emubd_powercycles(&config);
    TEST_ASSERT_EQUAL_INT32(0, initial_cycles);

    /* Set power cycle count */
    int err = lfs_emubd_setpowercycles(&config, 100);
    TEST_ASSERT_EQUAL(0, err);

    /* Verify power cycles were set */
    int32_t new_cycles = lfs_emubd_powercycles(&config);
    TEST_ASSERT_EQUAL_INT32(100, new_cycles);
}

void test_data_integrity_after_operations(void) {
    /* Format and mount */
    int err = lfs_format(&lfs, &config);
    TEST_ASSERT_EQUAL(0, err);

    err = lfs_mount(&lfs, &config);
    TEST_ASSERT_EQUAL(0, err);

    /* Create a file */
    lfs_file_t file;
    err = lfs_file_open(&lfs, &file, "/test.txt", LFS_O_WRONLY | LFS_O_CREAT);
    TEST_ASSERT_EQUAL(0, err);

    /* Write some data */
    const char *data = "test data before power loss";
    err = lfs_file_write(&lfs, &file, data, strlen(data));
    TEST_ASSERT_EQUAL(strlen(data), err);

    err = lfs_file_close(&lfs, &file);
    TEST_ASSERT_EQUAL(0, err);

    err = lfs_unmount(&lfs);
    TEST_ASSERT_EQUAL(0, err);

    /* Remount */
    err = lfs_mount(&lfs, &config);
    TEST_ASSERT_EQUAL(0, err);

    /* Verify file still exists and data is intact */
    err = lfs_file_open(&lfs, &file, "/test.txt", LFS_O_RDONLY);
    TEST_ASSERT_EQUAL(0, err);

    char read_buf[64];
    lfs_ssize_t bytes_read = lfs_file_read(&lfs, &file, read_buf, sizeof(read_buf));
    TEST_ASSERT_EQUAL(strlen(data), bytes_read);
    TEST_ASSERT_EQUAL_MEMORY(data, read_buf, strlen(data));

    err = lfs_file_close(&lfs, &file);
    TEST_ASSERT_EQUAL(0, err);

    err = lfs_unmount(&lfs);
    TEST_ASSERT_EQUAL(0, err);
}

void test_multiple_files_integrity(void) {
    /* Format and mount */
    int err = lfs_format(&lfs, &config);
    TEST_ASSERT_EQUAL(0, err);

    err = lfs_mount(&lfs, &config);
    TEST_ASSERT_EQUAL(0, err);

    /* Create multiple files */
    for (int i = 0; i < 5; i++) {
        char filename[32];
        snprintf(filename, sizeof(filename), "/file%d.txt", i);

        lfs_file_t file;
        err = lfs_file_open(&lfs, &file, filename, LFS_O_WRONLY | LFS_O_CREAT);
        TEST_ASSERT_EQUAL(0, err);

        char data[64];
        snprintf(data, sizeof(data), "data for file %d", i);
        err = lfs_file_write(&lfs, &file, data, strlen(data));
        TEST_ASSERT_EQUAL(strlen(data), err);

        err = lfs_file_close(&lfs, &file);
        TEST_ASSERT_EQUAL(0, err);
    }

    err = lfs_unmount(&lfs);
    TEST_ASSERT_EQUAL(0, err);

    /* Remount */
    err = lfs_mount(&lfs, &config);
    TEST_ASSERT_EQUAL(0, err);

    /* Verify all files are intact */
    for (int i = 0; i < 5; i++) {
        char filename[32];
        snprintf(filename, sizeof(filename), "/file%d.txt", i);

        lfs_file_t file;
        err = lfs_file_open(&lfs, &file, filename, LFS_O_RDONLY);
        TEST_ASSERT_EQUAL(0, err);

        char expected[64];
        snprintf(expected, sizeof(expected), "data for file %d", i);

        char read_buf[64];
        lfs_ssize_t bytes_read = lfs_file_read(&lfs, &file, read_buf, sizeof(read_buf));
        TEST_ASSERT_EQUAL(strlen(expected), bytes_read);
        TEST_ASSERT_EQUAL_MEMORY(expected, read_buf, strlen(expected));

        err = lfs_file_close(&lfs, &file);
        TEST_ASSERT_EQUAL(0, err);
    }

    err = lfs_unmount(&lfs);
    TEST_ASSERT_EQUAL(0, err);
}

void test_format_and_remount(void) {
    /* Format */
    int err = lfs_format(&lfs, &config);
    TEST_ASSERT_EQUAL(0, err);

    /* Mount */
    err = lfs_mount(&lfs, &config);
    TEST_ASSERT_EQUAL(0, err);

    /* Create a file */
    lfs_file_t file;
    err = lfs_file_open(&lfs, &file, "/data.txt", LFS_O_WRONLY | LFS_O_CREAT);
    TEST_ASSERT_EQUAL(0, err);

    const char *data = "persistent data";
    err = lfs_file_write(&lfs, &file, data, strlen(data));
    TEST_ASSERT_EQUAL(strlen(data), err);

    err = lfs_file_close(&lfs, &file);
    TEST_ASSERT_EQUAL(0, err);

    err = lfs_unmount(&lfs);
    TEST_ASSERT_EQUAL(0, err);

    /* Remount and verify */
    err = lfs_mount(&lfs, &config);
    TEST_ASSERT_EQUAL(0, err);

    err = lfs_file_open(&lfs, &file, "/data.txt", LFS_O_RDONLY);
    TEST_ASSERT_EQUAL(0, err);

    char read_buf[64];
    lfs_ssize_t bytes_read = lfs_file_read(&lfs, &file, read_buf, sizeof(read_buf));
    TEST_ASSERT_EQUAL(strlen(data), bytes_read);
    TEST_ASSERT_EQUAL_MEMORY(data, read_buf, strlen(data));

    err = lfs_file_close(&lfs, &file);
    TEST_ASSERT_EQUAL(0, err);

    err = lfs_unmount(&lfs);
    TEST_ASSERT_EQUAL(0, err);
}

void test_multiple_mount_unmount_cycles(void) {
    /* Format */
    int err = lfs_format(&lfs, &config);
    TEST_ASSERT_EQUAL(0, err);

    /* Multiple mount/unmount cycles */
    for (int cycle = 0; cycle < 10; cycle++) {
        err = lfs_mount(&lfs, &config);
        TEST_ASSERT_EQUAL(0, err);

        /* Create or update a file */
        lfs_file_t file;
        err = lfs_file_open(&lfs, &file, "/cycle.txt", LFS_O_WRONLY | LFS_O_CREAT);
        TEST_ASSERT_EQUAL(0, err);

        char data[64];
        snprintf(data, sizeof(data), "cycle %d", cycle);
        err = lfs_file_write(&lfs, &file, data, strlen(data));
        TEST_ASSERT_EQUAL(strlen(data), err);

        err = lfs_file_close(&lfs, &file);
        TEST_ASSERT_EQUAL(0, err);

        err = lfs_unmount(&lfs);
        TEST_ASSERT_EQUAL(0, err);
    }

    /* Final mount and verify */
    err = lfs_mount(&lfs, &config);
    TEST_ASSERT_EQUAL(0, err);

    lfs_file_t file;
    err = lfs_file_open(&lfs, &file, "/cycle.txt", LFS_O_RDONLY);
    TEST_ASSERT_EQUAL(0, err);

    char read_buf[64];
    lfs_ssize_t bytes_read = lfs_file_read(&lfs, &file, read_buf, sizeof(read_buf));
    TEST_ASSERT_EQUAL(strlen("cycle 9"), bytes_read);
    TEST_ASSERT_EQUAL_MEMORY("cycle 9", read_buf, strlen("cycle 9"));

    err = lfs_file_close(&lfs, &file);
    TEST_ASSERT_EQUAL(0, err);

    err = lfs_unmount(&lfs);
    TEST_ASSERT_EQUAL(0, err);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_power_loss_powercycles_tracking);
    RUN_TEST(test_data_integrity_after_operations);
    RUN_TEST(test_multiple_files_integrity);
    RUN_TEST(test_format_and_remount);
    RUN_TEST(test_multiple_mount_unmount_cycles);
    return UNITY_END();
}


