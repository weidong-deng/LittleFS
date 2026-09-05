/**
 * @file lfs_spi_flash_port_stub.c
 * @brief Stub implementation of SPI flash port for host testing
 */

#include "lfs.h"
#include <string.h>
#include <stdlib.h>

/* Simulated external flash: 256KB (typical SPI flash size) */
#define STUB_EXTERNAL_FLASH_SIZE    (256 * 1024)
#define STUB_BLOCK_SIZE             4096
#define STUB_BLOCK_COUNT            (STUB_EXTERNAL_FLASH_SIZE / STUB_BLOCK_SIZE)

static uint8_t *stub_ext_buffer = NULL;
static int stub_ext_initialized = 0;
static int stub_ext_registered = 0;
static struct lfs_config stub_ext_config;
static int stub_ext_context = 0;  /* Context for block device operations */

/* Custom block device operations */
static int stub_ext_read(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, void *buffer, lfs_size_t size) {
    (void)c;
    if (!stub_ext_buffer || block >= STUB_BLOCK_COUNT) return LFS_ERR_CORRUPT;
    memcpy(buffer, &stub_ext_buffer[block * STUB_BLOCK_SIZE + off], size);
    return 0;
}

static int stub_ext_prog(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size) {
    (void)c;
    if (!stub_ext_buffer || block >= STUB_BLOCK_COUNT) return LFS_ERR_CORRUPT;
    memcpy(&stub_ext_buffer[block * STUB_BLOCK_SIZE + off], buffer, size);
    return 0;
}

static int stub_ext_erase(const struct lfs_config *c, lfs_block_t block) {
    (void)c;
    if (!stub_ext_buffer || block >= STUB_BLOCK_COUNT) return LFS_ERR_CORRUPT;
    /* Flash erase sets to 0xFF */
    memset(&stub_ext_buffer[block * STUB_BLOCK_SIZE], 0xFF, STUB_BLOCK_SIZE);
    return 0;
}

static int stub_ext_sync(const struct lfs_config *c) {
    (void)c;
    return 0;
}

const struct lfs_config* LFS_SPIFlash_GetConfig(void) {
    if (!stub_ext_registered) {
        return NULL;
    }

    if (!stub_ext_initialized) {
        stub_ext_buffer = (uint8_t *)malloc(STUB_EXTERNAL_FLASH_SIZE);
        if (!stub_ext_buffer) return NULL;
        memset(stub_ext_buffer, 0xFF, STUB_EXTERNAL_FLASH_SIZE);

        memset(&stub_ext_config, 0, sizeof(stub_ext_config));
        stub_ext_config.context = &stub_ext_context;
        stub_ext_config.read = stub_ext_read;
        stub_ext_config.prog = stub_ext_prog;
        stub_ext_config.erase = stub_ext_erase;
        stub_ext_config.sync = stub_ext_sync;
        stub_ext_config.read_size = 256;
        stub_ext_config.prog_size = 256;
        stub_ext_config.block_size = STUB_BLOCK_SIZE;
        stub_ext_config.block_count = STUB_BLOCK_COUNT;
        stub_ext_config.cache_size = 256;
        stub_ext_config.lookahead_size = 16;
        stub_ext_config.block_cycles = 500;

        stub_ext_initialized = 1;
    }

    return &stub_ext_config;
}

void LFS_SPIFlash_RegisterStub(void) {
    stub_ext_registered = 1;
}

void LFS_SPIFlash_UnregisterStub(void) {
    stub_ext_registered = 0;
}

void LFS_SPIFlash_CleanupStub(void) {
    if (stub_ext_initialized) {
        free(stub_ext_buffer);
        stub_ext_buffer = NULL;
        stub_ext_initialized = 0;
    }
    stub_ext_registered = 0;
}
