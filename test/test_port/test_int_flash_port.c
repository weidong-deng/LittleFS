/**
 * @file test_int_flash_port.c
 * @brief Internal Flash port layer tests
 */

#include "unity.h"
#include "lfs.h"
#include "stubs.h"
#include <string.h>

void setUp(void) {
    /* Initialize internal flash stub */
    LFS_Int_CleanupStub();
}

void tearDown(void) {
    /* Cleanup */
    LFS_Int_CleanupStub();
}

void test_int_flash_get_config(void) {
    /* Get configuration */
    const struct lfs_config *cfg = LFS_Int_GetConfig();
    TEST_ASSERT_NOT_NULL(cfg);

    /* Verify basic parameters */
    TEST_ASSERT_GREATER_THAN(0, cfg->block_size);
    TEST_ASSERT_GREATER_THAN(0, cfg->block_count);
    TEST_ASSERT_GREATER_THAN(0, cfg->read_size);
    TEST_ASSERT_GREATER_THAN(0, cfg->prog_size);
}

void test_int_flash_config_valid(void) {
    const struct lfs_config *cfg = LFS_Int_GetConfig();
    TEST_ASSERT_NOT_NULL(cfg);

    /* Verify function pointers are set */
    TEST_ASSERT_NOT_NULL(cfg->read);
    TEST_ASSERT_NOT_NULL(cfg->prog);
    TEST_ASSERT_NOT_NULL(cfg->erase);
    TEST_ASSERT_NOT_NULL(cfg->sync);

    /* Verify block size is reasonable */
    TEST_ASSERT_GREATER_OR_EQUAL(256, cfg->block_size);
    TEST_ASSERT_LESS_OR_EQUAL(8192, cfg->block_size);

    /* Verify block count is reasonable */
    TEST_ASSERT_GREATER_THAN(0, cfg->block_count);
}

void test_int_flash_read_write(void) {
    const struct lfs_config *cfg = LFS_Int_GetConfig();
    TEST_ASSERT_NOT_NULL(cfg);

    /* Erase first block */
    int err = cfg->erase(cfg, 0);
    TEST_ASSERT_EQUAL(0, err);

    /* Write test data */
    uint8_t write_buf[64];
    for (int i = 0; i < 64; i++) {
        write_buf[i] = (uint8_t)(i & 0xFF);
    }

    err = cfg->prog(cfg, 0, 0, write_buf, 64);
    TEST_ASSERT_EQUAL(0, err);

    /* Read back and verify */
    uint8_t read_buf[64];
    err = cfg->read(cfg, 0, 0, read_buf, 64);
    TEST_ASSERT_EQUAL(0, err);

    TEST_ASSERT_EQUAL_MEMORY(write_buf, read_buf, 64);
}

void test_int_flash_erase(void) {
    const struct lfs_config *cfg = LFS_Int_GetConfig();
    TEST_ASSERT_NOT_NULL(cfg);

    /* Write some data first */
    uint8_t write_buf[64];
    memset(write_buf, 0xAA, 64);
    int err = cfg->prog(cfg, 0, 0, write_buf, 64);
    TEST_ASSERT_EQUAL(0, err);

    /* Erase block */
    err = cfg->erase(cfg, 0);
    TEST_ASSERT_EQUAL(0, err);

    /* Read back - should be all 0xFF */
    uint8_t read_buf[64];
    err = cfg->read(cfg, 0, 0, read_buf, 64);
    TEST_ASSERT_EQUAL(0, err);

    for (int i = 0; i < 64; i++) {
        TEST_ASSERT_EQUAL_HEX8(0xFF, read_buf[i]);
    }
}

void test_int_flash_multiple_blocks(void) {
    const struct lfs_config *cfg = LFS_Int_GetConfig();
    TEST_ASSERT_NOT_NULL(cfg);

    /* Test operations on multiple blocks */
    uint8_t write_buf[64];
    uint8_t read_buf[64];

    for (lfs_block_t block = 0; block < 3 && block < cfg->block_count; block++) {
        /* Erase block */
        int err = cfg->erase(cfg, block);
        TEST_ASSERT_EQUAL(0, err);

        /* Write pattern */
        for (int i = 0; i < 64; i++) {
            write_buf[i] = (uint8_t)((block + i) & 0xFF);
        }

        err = cfg->prog(cfg, block, 0, write_buf, 64);
        TEST_ASSERT_EQUAL(0, err);

        /* Read and verify */
        err = cfg->read(cfg, block, 0, read_buf, 64);
        TEST_ASSERT_EQUAL(0, err);

        TEST_ASSERT_EQUAL_MEMORY(write_buf, read_buf, 64);
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_int_flash_get_config);
    RUN_TEST(test_int_flash_config_valid);
    RUN_TEST(test_int_flash_read_write);
    RUN_TEST(test_int_flash_erase);
    RUN_TEST(test_int_flash_multiple_blocks);
    return UNITY_END();
}


