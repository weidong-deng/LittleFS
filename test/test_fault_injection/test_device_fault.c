/**
 * @file test_device_fault.c
 * @brief Device wear and power cycle tests
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
    bd_config.erase_cycles = 10000;  /* Enable wear tracking */

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

void test_device_wear_tracking(void) {
    /* Initial wear should be 0 */
    int32_t initial_wear = lfs_emubd_wear(&config, 0);
    TEST_ASSERT_EQUAL_INT32(0, initial_wear);

    /* Set wear level */
    int err = lfs_emubd_setwear(&config, 0, 100);
    TEST_ASSERT_EQUAL(0, err);

    /* Verify wear was set */
    int32_t new_wear = lfs_emubd_wear(&config, 0);
    TEST_ASSERT_EQUAL_INT32(100, new_wear);
}

void test_device_power_cycles(void) {
    /* Initial power cycles should be 0 */
    int32_t initial_cycles = lfs_emubd_powercycles(&config);
    TEST_ASSERT_EQUAL_INT32(0, initial_cycles);

    /* Set power cycle count */
    int err = lfs_emubd_setpowercycles(&config, 50);
    TEST_ASSERT_EQUAL(0, err);

    /* Verify power cycles were set */
    int32_t new_cycles = lfs_emubd_powercycles(&config);
    TEST_ASSERT_EQUAL_INT32(50, new_cycles);
}

void test_device_multiple_blocks_wear(void) {
    /* Test wear tracking on multiple blocks */
    for (lfs_block_t block = 0; block < 5; block++) {
        int32_t wear = lfs_emubd_wear(&config, block);
        TEST_ASSERT_EQUAL_INT32(0, wear);

        int err = lfs_emubd_setwear(&config, block, block * 10);
        TEST_ASSERT_EQUAL(0, err);

        wear = lfs_emubd_wear(&config, block);
        TEST_ASSERT_EQUAL_INT32(block * 10, wear);
    }
}

void test_device_wear_after_operations(void) {
    /* Format and perform some operations */
    int err = lfs_format(&lfs, &config);
    TEST_ASSERT_EQUAL(0, err);

    err = lfs_mount(&lfs, &config);
    TEST_ASSERT_EQUAL(0, err);

    /* Create and write a file */
    lfs_file_t file;
    err = lfs_file_open(&lfs, &file, "/test.txt", LFS_O_WRONLY | LFS_O_CREAT);
    TEST_ASSERT_EQUAL(0, err);

    uint8_t buffer[64];
    memset(buffer, 0xAA, sizeof(buffer));
    lfs_ssize_t written = lfs_file_write(&lfs, &file, buffer, sizeof(buffer));
    TEST_ASSERT_EQUAL(sizeof(buffer), written);

    err = lfs_file_close(&lfs, &file);
    TEST_ASSERT_EQUAL(0, err);

    /* Check that wear has increased on at least one block */
    int wear_found = 0;
    for (lfs_block_t block = 0; block < TEST_BLOCK_COUNT; block++) {
        int32_t wear = lfs_emubd_wear(&config, block);
        if (wear > 0) {
            wear_found = 1;
            break;
        }
    }
    TEST_ASSERT_TRUE(wear_found);

    err = lfs_unmount(&lfs);
    TEST_ASSERT_EQUAL(0, err);
}

void test_device_power_cycle_simulation(void) {
    /* Format and mount */
    int err = lfs_format(&lfs, &config);
    TEST_ASSERT_EQUAL(0, err);

    err = lfs_mount(&lfs, &config);
    TEST_ASSERT_EQUAL(0, err);

    /* Write some data */
    lfs_file_t file;
    err = lfs_file_open(&lfs, &file, "/data.txt", LFS_O_WRONLY | LFS_O_CREAT);
    TEST_ASSERT_EQUAL(0, err);

    uint8_t buffer[128];
    memset(buffer, 0x55, sizeof(buffer));
    lfs_ssize_t written = lfs_file_write(&lfs, &file, buffer, sizeof(buffer));
    TEST_ASSERT_EQUAL(sizeof(buffer), written);

    err = lfs_file_close(&lfs, &file);
    TEST_ASSERT_EQUAL(0, err);

    err = lfs_unmount(&lfs);
    TEST_ASSERT_EQUAL(0, err);

    /* Simulate power cycles */
    err = lfs_emubd_setpowercycles(&config, 5);
    TEST_ASSERT_EQUAL(0, err);

    int32_t cycles = lfs_emubd_powercycles(&config);
    TEST_ASSERT_EQUAL_INT32(5, cycles);

    /* Remount should still work */
    err = lfs_mount(&lfs, &config);
    TEST_ASSERT_EQUAL(0, err);

    /* Verify data is still readable */
    err = lfs_file_open(&lfs, &file, "/data.txt", LFS_O_RDONLY);
    TEST_ASSERT_EQUAL(0, err);

    uint8_t read_buffer[128];
    lfs_ssize_t read_bytes = lfs_file_read(&lfs, &file, read_buffer, sizeof(read_buffer));
    TEST_ASSERT_EQUAL(sizeof(read_buffer), read_bytes);
    TEST_ASSERT_EQUAL_MEMORY(buffer, read_buffer, sizeof(buffer));

    err = lfs_file_close(&lfs, &file);
    TEST_ASSERT_EQUAL(0, err);

    err = lfs_unmount(&lfs);
    TEST_ASSERT_EQUAL(0, err);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_device_wear_tracking);
    RUN_TEST(test_device_power_cycles);
    RUN_TEST(test_device_multiple_blocks_wear);
    RUN_TEST(test_device_wear_after_operations);
    RUN_TEST(test_device_power_cycle_simulation);
    return UNITY_END();
}


