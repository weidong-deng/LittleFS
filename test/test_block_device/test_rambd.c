/**
 * @file test_rambd.c
 * @brief RAM block device tests using actual lfs_rambd API
 */

#include "unity.h"
#include "lfs.h"
#include "lfs_rambd.h"

#define RAMBD_BLOCK_SIZE    1024
#define RAMBD_BLOCK_COUNT   16

static uint8_t *rambd_buffer = NULL;
static struct lfs_config cfg;
static struct lfs_rambd_config bdcfg;
static struct lfs_rambd bd;

void setUp(void) {
    rambd_buffer = (uint8_t *)malloc(RAMBD_BLOCK_SIZE * RAMBD_BLOCK_COUNT);
    TEST_ASSERT_NOT_NULL(rambd_buffer);
    memset(rambd_buffer, 0xFF, RAMBD_BLOCK_SIZE * RAMBD_BLOCK_COUNT);

    memset(&bdcfg, 0, sizeof(bdcfg));
    bdcfg.read_size = 1;
    bdcfg.prog_size = 1;
    bdcfg.erase_size = RAMBD_BLOCK_SIZE;
    bdcfg.erase_count = RAMBD_BLOCK_COUNT;
    bdcfg.buffer = rambd_buffer;

    memset(&cfg, 0, sizeof(cfg));
    cfg.context = &bd;
    cfg.read = lfs_rambd_read;
    cfg.prog = lfs_rambd_prog;
    cfg.erase = lfs_rambd_erase;
    cfg.sync = lfs_rambd_sync;
    cfg.read_size = 1;
    cfg.prog_size = 1;
    cfg.block_size = RAMBD_BLOCK_SIZE;
    cfg.block_count = RAMBD_BLOCK_COUNT;
    cfg.cache_size = 256;
    cfg.lookahead_size = 16;
    cfg.block_cycles = 500;

    TEST_ASSERT_EQUAL(0, lfs_rambd_create(&cfg, &bdcfg));
}

void tearDown(void) {
    TEST_ASSERT_EQUAL(0, lfs_rambd_destroy(&cfg));
    free(rambd_buffer);
    rambd_buffer = NULL;
}

void test_rambd_create_destroy(void) {
    /* Already created in setUp, just verify buffer is allocated */
    TEST_ASSERT_NOT_NULL(bd.buffer);
    TEST_ASSERT_EQUAL_PTR(rambd_buffer, bd.buffer);
}

void test_rambd_create_with_malloc(void) {
    /* Destroy the current one first */
    TEST_ASSERT_EQUAL(0, lfs_rambd_destroy(&cfg));

    /* Create without providing buffer - should malloc */
    struct lfs_rambd_config cfg_no_buf;
    memset(&cfg_no_buf, 0, sizeof(cfg_no_buf));
    cfg_no_buf.read_size = 16;
    cfg_no_buf.prog_size = 16;
    cfg_no_buf.erase_size = RAMBD_BLOCK_SIZE;
    cfg_no_buf.erase_count = RAMBD_BLOCK_COUNT;
    cfg_no_buf.buffer = NULL;

    TEST_ASSERT_EQUAL(0, lfs_rambd_create(&cfg, &cfg_no_buf));
    TEST_ASSERT_NOT_NULL(bd.buffer);
    TEST_ASSERT_NOT_EQUAL_PTR(rambd_buffer, bd.buffer);

    TEST_ASSERT_EQUAL(0, lfs_rambd_destroy(&cfg));

    /* Re-create with the static buffer for tearDown */
    bdcfg.buffer = rambd_buffer;
    TEST_ASSERT_EQUAL(0, lfs_rambd_create(&cfg, &bdcfg));
}

void test_rambd_read_write(void) {
    uint8_t write_buf[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                              0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
    uint8_t read_buf[16] = {0};

    int err = lfs_rambd_prog(&cfg, 0, 0, write_buf, 16);
    TEST_ASSERT_EQUAL(0, err);

    err = lfs_rambd_read(&cfg, 0, 0, read_buf, 16);
    TEST_ASSERT_EQUAL(0, err);

    TEST_ASSERT_EQUAL_MEMORY(write_buf, read_buf, 16);
}

void test_rambd_erase(void) {
    uint8_t write_buf[16] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint8_t read_buf[16] = {0};

    int err = lfs_rambd_prog(&cfg, 0, 0, write_buf, 16);
    TEST_ASSERT_EQUAL(0, err);

    /* rambd erase is a noop - data should remain unchanged */
    err = lfs_rambd_erase(&cfg, 0);
    TEST_ASSERT_EQUAL(0, err);

    err = lfs_rambd_read(&cfg, 0, 0, read_buf, 16);
    TEST_ASSERT_EQUAL(0, err);

    /* Data should still be present since erase is noop */
    TEST_ASSERT_EQUAL_HEX8(0xAA, read_buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0xBB, read_buf[1]);
}

void test_rambd_sync(void) {
    /* Sync is a noop for rambd */
    TEST_ASSERT_EQUAL(0, lfs_rambd_sync(&cfg));
}

void test_rambd_boundary_blocks(void) {
    uint8_t write_buf[16] = {0x11, 0x22, 0x33, 0x44};
    uint8_t read_buf[16] = {0};

    /* First block */
    TEST_ASSERT_EQUAL(0, lfs_rambd_prog(&cfg, 0, 0, write_buf, 16));
    TEST_ASSERT_EQUAL(0, lfs_rambd_read(&cfg, 0, 0, read_buf, 16));
    TEST_ASSERT_EQUAL_MEMORY(write_buf, read_buf, 16);

    /* Last block */
    TEST_ASSERT_EQUAL(0, lfs_rambd_prog(&cfg, RAMBD_BLOCK_COUNT - 1, 0, write_buf, 16));
    TEST_ASSERT_EQUAL(0, lfs_rambd_read(&cfg, RAMBD_BLOCK_COUNT - 1, 0, read_buf, 16));
    TEST_ASSERT_EQUAL_MEMORY(write_buf, read_buf, 16);
}

void test_rambd_partial_block(void) {
    uint8_t write_buf[8] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
    uint8_t read_buf[8] = {0};

    TEST_ASSERT_EQUAL(0, lfs_rambd_prog(&cfg, 0, 0, write_buf, 8));
    TEST_ASSERT_EQUAL(0, lfs_rambd_read(&cfg, 0, 0, read_buf, 8));
    TEST_ASSERT_EQUAL_MEMORY(write_buf, read_buf, 8);
}

void test_rambd_offset_write(void) {
    uint8_t write_buf[16] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22,
                              0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00};
    uint8_t read_buf[16] = {0};

    /* Write at offset 16 */
    TEST_ASSERT_EQUAL(0, lfs_rambd_prog(&cfg, 0, 16, write_buf, 16));
    TEST_ASSERT_EQUAL(0, lfs_rambd_read(&cfg, 0, 16, read_buf, 16));
    TEST_ASSERT_EQUAL_MEMORY(write_buf, read_buf, 16);
}

void test_rambd_full_block_write(void) {
    uint8_t *write_buf = (uint8_t *)malloc(RAMBD_BLOCK_SIZE);
    uint8_t *read_buf = (uint8_t *)malloc(RAMBD_BLOCK_SIZE);
    TEST_ASSERT_NOT_NULL(write_buf);
    TEST_ASSERT_NOT_NULL(read_buf);

    for (int i = 0; i < RAMBD_BLOCK_SIZE; i++) {
        write_buf[i] = (uint8_t)(i & 0xFF);
    }

    /* Write full block at offset 0 */
    TEST_ASSERT_EQUAL(0, lfs_rambd_prog(&cfg, 0, 0, write_buf, RAMBD_BLOCK_SIZE));
    TEST_ASSERT_EQUAL(0, lfs_rambd_read(&cfg, 0, 0, read_buf, RAMBD_BLOCK_SIZE));
    TEST_ASSERT_EQUAL_MEMORY(write_buf, read_buf, RAMBD_BLOCK_SIZE);

    free(write_buf);
    free(read_buf);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_rambd_create_destroy);
    RUN_TEST(test_rambd_create_with_malloc);
    RUN_TEST(test_rambd_read_write);
    RUN_TEST(test_rambd_erase);
    RUN_TEST(test_rambd_sync);
    RUN_TEST(test_rambd_boundary_blocks);
    RUN_TEST(test_rambd_partial_block);
    RUN_TEST(test_rambd_offset_write);
    RUN_TEST(test_rambd_full_block_write);
    return UNITY_END();
}


