/**
 * @file test_helpers.c
 * @brief Common test helper functions implementation
 */

#include "test_helpers.h"
#include "unity.h"

/* Custom block device operations that properly simulate flash behavior */
static int test_bd_read(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, void *buffer, lfs_size_t size) {
    test_context_t *ctx = (test_context_t *)c->context;
    if (!ctx->buffer || block >= ctx->block_count) return LFS_ERR_CORRUPT;
    memcpy(buffer, &ctx->buffer[block * ctx->block_size + off], size);
    return 0;
}

static int test_bd_prog(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size) {
    test_context_t *ctx = (test_context_t *)c->context;
    if (!ctx->buffer || block >= ctx->block_count) return LFS_ERR_CORRUPT;
    memcpy(&ctx->buffer[block * ctx->block_size + off], buffer, size);
    return 0;
}

static int test_bd_erase(const struct lfs_config *c, lfs_block_t block) {
    test_context_t *ctx = (test_context_t *)c->context;
    if (!ctx->buffer || block >= ctx->block_count) return LFS_ERR_CORRUPT;
    /* Flash erase sets to 0xFF */
    memset(&ctx->buffer[block * ctx->block_size], 0xFF, ctx->block_size);
    return 0;
}

static int test_bd_sync(const struct lfs_config *c) {
    (void)c;
    return 0;
}

int test_context_init(test_context_t *ctx, uint32_t block_size, uint32_t block_count) {
    if (!ctx) return -1;

    memset(ctx, 0, sizeof(test_context_t));

    /* Store block parameters */
    ctx->block_size = block_size;
    ctx->block_count = block_count;

    /* Allocate buffer */
    ctx->buffer_size = block_size * block_count;
    ctx->buffer = (uint8_t *)malloc(ctx->buffer_size);
    if (!ctx->buffer) {
        return -1;
    }
    memset(ctx->buffer, 0xFF, ctx->buffer_size);

    /* Configure lfs_config */
    memset(&ctx->config, 0, sizeof(ctx->config));
    ctx->config.context = ctx;
    ctx->config.read = test_bd_read;
    ctx->config.prog = test_bd_prog;
    ctx->config.erase = test_bd_erase;
    ctx->config.sync = test_bd_sync;
    ctx->config.read_size = 16;
    ctx->config.prog_size = 16;
    ctx->config.block_size = block_size;
    ctx->config.block_count = block_count;
    ctx->config.cache_size = TEST_CACHE_SIZE;
    ctx->config.lookahead_size = TEST_LOOKAHEAD_SIZE;
    ctx->config.block_cycles = 500;  /* Wear leveling: 100-1000, or -1 to disable */

    return 0;
}

void test_context_cleanup(test_context_t *ctx) {
    if (!ctx) return;

    if (ctx->buffer) {
        free(ctx->buffer);
        ctx->buffer = NULL;
    }
}

int test_format_and_mount(test_context_t *ctx) {
    if (!ctx) return -1;

    int err = lfs_format(&ctx->lfs, &ctx->config);
    if (err != 0) return err;

    err = lfs_mount(&ctx->lfs, &ctx->config);
    return err;
}

int test_unmount_and_cleanup(test_context_t *ctx) {
    if (!ctx) return -1;

    return lfs_unmount(&ctx->lfs);
}

int test_write_file(lfs_t *lfs, const char *path, const void *data, uint32_t size) {
    lfs_file_t file;
    int err = lfs_file_open(lfs, &file, path, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
    if (err != 0) return err;

    lfs_ssize_t written = lfs_file_write(lfs, &file, data, size);
    lfs_file_close(lfs, &file);

    if (written < 0) return (int)written;
    if ((uint32_t)written != size) return -1;

    return 0;
}

int test_read_file(lfs_t *lfs, const char *path, void *data, uint32_t size) {
    lfs_file_t file;
    int err = lfs_file_open(lfs, &file, path, LFS_O_RDONLY);
    if (err != 0) return err;

    lfs_ssize_t read_bytes = lfs_file_read(lfs, &file, data, size);
    lfs_file_close(lfs, &file);

    if (read_bytes < 0) return (int)read_bytes;
    if ((uint32_t)read_bytes != size) return -1;

    return 0;
}

int test_file_exists(lfs_t *lfs, const char *path) {
    struct lfs_info info;
    int err = lfs_stat(lfs, path, &info);
    if (err != 0) return 0;
    return (info.type == LFS_TYPE_REG) ? 1 : 0;
}

int test_dir_exists(lfs_t *lfs, const char *path) {
    struct lfs_info info;
    int err = lfs_stat(lfs, path, &info);
    if (err != 0) return 0;
    return (info.type == LFS_TYPE_DIR) ? 1 : 0;
}

int test_count_dir_entries(lfs_t *lfs, const char *path) {
    lfs_dir_t dir;
    int err = lfs_dir_open(lfs, &dir, path);
    if (err != 0) return err;

    struct lfs_info info;
    int count = 0;
    while (lfs_dir_read(lfs, &dir, &info) > 0) {
        count++;
    }
    lfs_dir_close(lfs, &dir);

    return count;
}

void test_generate_pattern(uint8_t *buffer, uint32_t size, uint8_t seed) {
    uint8_t val = seed;
    for (uint32_t i = 0; i < size; i++) {
        buffer[i] = val;
        val = (val * 7 + 13) & 0xFF;
    }
}

int test_verify_pattern(const uint8_t *buffer, uint32_t size, uint8_t seed) {
    uint8_t val = seed;
    for (uint32_t i = 0; i < size; i++) {
        if (buffer[i] != val) {
            return -1;
        }
        val = (val * 7 + 13) & 0xFF;
    }
    return 0;
}
