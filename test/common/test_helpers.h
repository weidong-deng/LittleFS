/**
 * @file test_helpers.h
 * @brief Common test helper functions and macros
 */

#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include "lfs.h"
#include "lfs_rambd.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Test configuration */
#define TEST_BLOCK_SIZE     1024
#define TEST_BLOCK_COUNT    64
#define TEST_CACHE_SIZE     256
#define TEST_LOOKAHEAD_SIZE 16

/* Helper macros */
#define TEST_ASSERT_LFS_OK(err) \
    do { \
        int _err = (err); \
        if (_err < 0) { \
            TEST_FAIL_MESSAGE("LittleFS operation failed"); \
        } \
    } while (0)

#define TEST_ASSERT_LFS_ERR(err, expected) \
    TEST_ASSERT_EQUAL(expected, err)

/* Test context structure */
typedef struct {
    lfs_t lfs;
    struct lfs_config config;
    lfs_rambd_t rambd;
    struct lfs_rambd_config rambd_cfg;
    uint8_t *buffer;
    size_t buffer_size;
    uint32_t block_size;
    uint32_t block_count;
} test_context_t;

/* Helper functions */
int test_context_init(test_context_t *ctx, uint32_t block_size, uint32_t block_count);
void test_context_cleanup(test_context_t *ctx);
int test_format_and_mount(test_context_t *ctx);
int test_unmount_and_cleanup(test_context_t *ctx);

/* File operation helpers */
int test_write_file(lfs_t *lfs, const char *path, const void *data, uint32_t size);
int test_read_file(lfs_t *lfs, const char *path, void *data, uint32_t size);
int test_file_exists(lfs_t *lfs, const char *path);

/* Directory operation helpers */
int test_dir_exists(lfs_t *lfs, const char *path);
int test_count_dir_entries(lfs_t *lfs, const char *path);

/* Data generation helpers */
void test_generate_pattern(uint8_t *buffer, uint32_t size, uint8_t seed);
int test_verify_pattern(const uint8_t *buffer, uint32_t size, uint8_t seed);

#endif /* TEST_HELPERS_H */
