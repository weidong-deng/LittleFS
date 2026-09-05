/**
 * @file test_spi_flash_port.c
 * @brief SPI Flash port layer tests
 */

#include "unity.h"
#include "lfs.h"
#include "stubs.h"
#include <string.h>
#include <stdlib.h>

/* Mock SPI Flash configuration */
#define MOCK_FLASH_SIZE     (256 * 1024)  /* 256KB */
#define MOCK_BLOCK_SIZE     4096
#define MOCK_BLOCK_COUNT    (MOCK_FLASH_SIZE / MOCK_BLOCK_SIZE)

static uint8_t *mock_flash_buffer = NULL;
static struct lfs_config mock_config;
static int mock_initialized = 0;

/* Mock SPI Flash read function */
static int mock_spi_flash_read(const struct lfs_config *c, lfs_block_t block,
                               lfs_off_t off, void *buffer, lfs_size_t size) {
    (void)c;
    if (block >= MOCK_BLOCK_COUNT) return LFS_ERR_IO;
    if (off + size > MOCK_BLOCK_SIZE) return LFS_ERR_IO;

    uint32_t addr = block * MOCK_BLOCK_SIZE + off;
    memcpy(buffer, mock_flash_buffer + addr, size);
    return 0;
}

/* Mock SPI Flash prog function */
static int mock_spi_flash_prog(const struct lfs_config *c, lfs_block_t block,
                               lfs_off_t off, const void *buffer, lfs_size_t size) {
    (void)c;
    if (block >= MOCK_BLOCK_COUNT) return LFS_ERR_IO;
    if (off + size > MOCK_BLOCK_SIZE) return LFS_ERR_IO;

    uint32_t addr = block * MOCK_BLOCK_SIZE + off;
    memcpy(mock_flash_buffer + addr, buffer, size);
    return 0;
}

/* Mock SPI Flash erase function */
static int mock_spi_flash_erase(const struct lfs_config *c, lfs_block_t block) {
    (void)c;
    if (block >= MOCK_BLOCK_COUNT) return LFS_ERR_IO;

    uint32_t addr = block * MOCK_BLOCK_SIZE;
    memset(mock_flash_buffer + addr, 0xFF, MOCK_BLOCK_SIZE);
    return 0;
}

/* Mock SPI Flash sync function */
static int mock_spi_flash_sync(const struct lfs_config *c) {
    (void)c;
    return 0;
}

void setUp(void) {
    /* Reset stub state before each test */
    LFS_SPIFlash_CleanupStub();

    mock_flash_buffer = (uint8_t *)malloc(MOCK_FLASH_SIZE);
    TEST_ASSERT_NOT_NULL(mock_flash_buffer);
    memset(mock_flash_buffer, 0xFF, MOCK_FLASH_SIZE);

    memset(&mock_config, 0, sizeof(mock_config));
    mock_config.read = mock_spi_flash_read;
    mock_config.prog = mock_spi_flash_prog;
    mock_config.erase = mock_spi_flash_erase;
    mock_config.sync = mock_spi_flash_sync;
    mock_config.read_size = 256;
    mock_config.prog_size = 256;
    mock_config.block_size = MOCK_BLOCK_SIZE;
    mock_config.block_count = MOCK_BLOCK_COUNT;
    mock_config.cache_size = 256;
    mock_config.lookahead_size = 16;
    mock_config.block_cycles = 500;

    mock_initialized = 1;
}

void tearDown(void) {
    if (mock_flash_buffer) {
        free(mock_flash_buffer);
        mock_flash_buffer = NULL;
    }
    mock_initialized = 0;
    /* Clean up stub after each test */
    LFS_SPIFlash_CleanupStub();
}

void test_spi_flash_register_driver(void) {
    /* Register stub */
    LFS_SPIFlash_RegisterStub();

    /* Verify config is available */
    const struct lfs_config *cfg = LFS_SPIFlash_GetConfig();
    TEST_ASSERT_NOT_NULL(cfg);
}

void test_spi_flash_get_config_null(void) {
    /* Before registration, should return NULL */
    const struct lfs_config *cfg = LFS_SPIFlash_GetConfig();
    TEST_ASSERT_NULL(cfg);
}

void test_spi_flash_get_config_valid(void) {
    /* Register stub */
    LFS_SPIFlash_RegisterStub();

    /* Get config should return valid pointer */
    const struct lfs_config *cfg = LFS_SPIFlash_GetConfig();
    TEST_ASSERT_NOT_NULL(cfg);

    /* Verify config parameters */
    TEST_ASSERT_EQUAL(4096, cfg->block_size);
    TEST_ASSERT_GREATER_THAN(0, cfg->block_count);
    TEST_ASSERT_EQUAL(256, cfg->read_size);
    TEST_ASSERT_EQUAL(256, cfg->prog_size);
}

void test_spi_flash_read_write(void) {
    LFS_SPIFlash_RegisterStub();
    const struct lfs_config *cfg = LFS_SPIFlash_GetConfig();
    TEST_ASSERT_NOT_NULL(cfg);

    uint8_t write_buf[256];
    uint8_t read_buf[256];

    /* Initialize write buffer with test pattern */
    for (int i = 0; i < 256; i++) {
        write_buf[i] = (uint8_t)(i & 0xFF);
    }

    /* Write data */
    int err = cfg->prog(cfg, 0, 0, write_buf, 256);
    TEST_ASSERT_EQUAL(0, err);

    /* Read data back */
    err = cfg->read(cfg, 0, 0, read_buf, 256);
    TEST_ASSERT_EQUAL(0, err);

    /* Verify data matches */
    TEST_ASSERT_EQUAL_MEMORY(write_buf, read_buf, 256);
}

void test_spi_flash_erase(void) {
    LFS_SPIFlash_RegisterStub();
    const struct lfs_config *cfg = LFS_SPIFlash_GetConfig();
    TEST_ASSERT_NOT_NULL(cfg);

    /* Write some data first */
    uint8_t write_buf[256];
    memset(write_buf, 0xAA, 256);
    int err = cfg->prog(cfg, 0, 0, write_buf, 256);
    TEST_ASSERT_EQUAL(0, err);

    /* Erase block */
    err = cfg->erase(cfg, 0);
    TEST_ASSERT_EQUAL(0, err);

    /* Read back - should be all 0xFF */
    uint8_t read_buf[256];
    err = cfg->read(cfg, 0, 0, read_buf, 256);
    TEST_ASSERT_EQUAL(0, err);

    for (int i = 0; i < 256; i++) {
        TEST_ASSERT_EQUAL_HEX8(0xFF, read_buf[i]);
    }
}

void test_spi_flash_unregister(void) {
    /* Register stub */
    LFS_SPIFlash_RegisterStub();
    TEST_ASSERT_NOT_NULL(LFS_SPIFlash_GetConfig());

    /* Unregister stub */
    LFS_SPIFlash_UnregisterStub();

    /* Should return NULL after unregistration */
    const struct lfs_config *cfg = LFS_SPIFlash_GetConfig();
    TEST_ASSERT_NULL(cfg);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_spi_flash_register_driver);
    RUN_TEST(test_spi_flash_get_config_null);
    RUN_TEST(test_spi_flash_get_config_valid);
    RUN_TEST(test_spi_flash_read_write);
    RUN_TEST(test_spi_flash_erase);
    RUN_TEST(test_spi_flash_unregister);
    return UNITY_END();
}


