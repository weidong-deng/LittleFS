/**
 * @file lfs_flash_int_port_stub.c
 * @brief Stub implementation of internal flash port for host testing
 */

#include "lfs.h"
#include <string.h>
#include <stdlib.h>

/* Simulated internal flash: 64KB (matching actual hardware) */
#define STUB_INTERNAL_FLASH_SIZE    (64 * 1024)
#define STUB_BLOCK_SIZE             1024
#define STUB_BLOCK_COUNT            (STUB_INTERNAL_FLASH_SIZE / STUB_BLOCK_SIZE)

static uint8_t *stub_buffer = NULL;
static int stub_initialized = 0;
static int stub_registered = 0;  /* Registration flag */
static struct lfs_config stub_config;
static int stub_context = 0;  /* Context for block device operations */

/* Custom block device operations */
static int stub_read(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, void *buffer, lfs_size_t size) {
    (void)c;
    if (!stub_buffer || block >= STUB_BLOCK_COUNT) return LFS_ERR_CORRUPT;
    memcpy(buffer, &stub_buffer[block * STUB_BLOCK_SIZE + off], size);
    return 0;
}

static int stub_prog(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size) {
    (void)c;
    if (!stub_buffer || block >= STUB_BLOCK_COUNT) return LFS_ERR_CORRUPT;
    memcpy(&stub_buffer[block * STUB_BLOCK_SIZE + off], buffer, size);
    return 0;
}

static int stub_erase(const struct lfs_config *c, lfs_block_t block) {
    (void)c;
    if (!stub_buffer || block >= STUB_BLOCK_COUNT) return LFS_ERR_CORRUPT;
    /* Flash erase sets to 0xFF */
    memset(&stub_buffer[block * STUB_BLOCK_SIZE], 0xFF, STUB_BLOCK_SIZE);
    return 0;
}

static int stub_sync(const struct lfs_config *c) {
    (void)c;
    return 0;
}

const struct lfs_config* LFS_Int_GetConfig(void) {
    if (!stub_initialized) {
        stub_buffer = (uint8_t *)malloc(STUB_INTERNAL_FLASH_SIZE);
        if (!stub_buffer) return NULL;
        memset(stub_buffer, 0xFF, STUB_INTERNAL_FLASH_SIZE);

        memset(&stub_config, 0, sizeof(stub_config));
        stub_config.context = &stub_context;
        stub_config.read = stub_read;
        stub_config.prog = stub_prog;
        stub_config.erase = stub_erase;
        stub_config.sync = stub_sync;
        stub_config.read_size = 16;
        stub_config.prog_size = 16;
        stub_config.block_size = STUB_BLOCK_SIZE;
        stub_config.block_count = STUB_BLOCK_COUNT;
        stub_config.cache_size = 256;
        stub_config.lookahead_size = 16;
        stub_config.block_cycles = 500;

        stub_initialized = 1;
    }
    stub_registered = 1;  /* Mark as registered when config is retrieved */
    return &stub_config;
}

void LFS_Int_CleanupStub(void) {
    if (stub_initialized) {
        free(stub_buffer);
        stub_buffer = NULL;
        stub_initialized = 0;
    }
    stub_registered = 0;
}
