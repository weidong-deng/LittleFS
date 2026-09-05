/**
 * @file test_wear_leveling.c
 * @brief Wear leveling tests
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
    config.block_cycles = 500;  /* Enable wear leveling */

    int err = lfs_emubd_create(&config, &bd_config);
    TEST_ASSERT_EQUAL(0, err);
}

void tearDown(void) {
    lfs_emubd_destroy(&config);
}

void test_wear_distribution(void) {
    /* Format and mount */
    int err = lfs_format(&lfs, &config);
    TEST_ASSERT_EQUAL(0, err);

    err = lfs_mount(&lfs, &config);
    TEST_ASSERT_EQUAL(0, err);

    /* Perform many write operations to trigger wear leveling */
    for (int i = 0; i < 100; i++) {
        lfs_file_t file;
        err = lfs_file_open(&lfs, &file, "/test.txt", LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
        TEST_ASSERT_EQUAL(0, err);

        char data[128];
        snprintf(data, sizeof(data), "write operation %d", i);
        err = lfs_file_write(&lfs, &file, data, strlen(data));
        TEST_ASSERT_EQUAL(strlen(data), err);

        err = lfs_file_close(&lfs, &file);
        TEST_ASSERT_EQUAL(0, err);
    }

    err = lfs_unmount(&lfs);
    TEST_ASSERT_EQUAL(0, err);

    /* Check wear counts across blocks */
    int32_t min_wear = INT32_MAX;
    int32_t max_wear = 0;
    int32_t total_wear = 0;

    for (lfs_block_t block = 0; block < TEST_BLOCK_COUNT; block++) {
        int32_t wear = lfs_emubd_wear(&config, block);
        if (wear < min_wear) min_wear = wear;
        if (wear > max_wear) max_wear = wear;
        total_wear += wear;
    }

    /* Verify wear leveling is working */
    /* The difference between max and min should not be too large */
    int32_t wear_spread = max_wear - min_wear;
    TEST_ASSERT_LESS_THAN(50, wear_spread);

    /* Total wear should be greater than 0 (some blocks were erased) */
    TEST_ASSERT_GREATER_THAN(0, total_wear);
}

void test_wear_no_single_block_hot(void) {
    /* Format and mount */
    int err = lfs_format(&lfs, &config);
    TEST_ASSERT_EQUAL(0, err);

    err = lfs_mount(&lfs, &config);
    TEST_ASSERT_EQUAL(0, err);

    /* Repeatedly write to the same file */
    for (int i = 0; i < 200; i++) {
        lfs_file_t file;
        err = lfs_file_open(&lfs, &file, "/hot_file.txt", LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
        TEST_ASSERT_EQUAL(0, err);

        char data[256];
        memset(data, 'A' + (i % 26), sizeof(data));
        err = lfs_file_write(&lfs, &file, data, sizeof(data));
        TEST_ASSERT_EQUAL(sizeof(data), err);

        err = lfs_file_close(&lfs, &file);
        TEST_ASSERT_EQUAL(0, err);
    }

    err = lfs_unmount(&lfs);
    TEST_ASSERT_EQUAL(0, err);

    /* Find the most worn block */
    int32_t max_wear = 0;
    lfs_block_t max_block = 0;

    for (lfs_block_t block = 0; block < TEST_BLOCK_COUNT; block++) {
        int32_t wear = lfs_emubd_wear(&config, block);
        if (wear > max_wear) {
            max_wear = wear;
            max_block = block;
        }
    }

    /* No single block should be excessively worn */
    /* With wear leveling, the max should be less than 2x the average */
    int32_t total_wear = 0;
    for (lfs_block_t block = 0; block < TEST_BLOCK_COUNT; block++) {
        total_wear += lfs_emubd_wear(&config, block);
    }
    int32_t avg_wear = total_wear / TEST_BLOCK_COUNT;

    TEST_ASSERT_LESS_THAN(avg_wear * 2, max_wear);
}

void test_wear_stats_after_cycles(void) {
    /* Format and mount */
    int err = lfs_format(&lfs, &config);
    TEST_ASSERT_EQUAL(0, err);

    err = lfs_mount(&lfs, &config);
    TEST_ASSERT_EQUAL(0, err);

    /* Perform multiple cycles of file operations */
    for (int cycle = 0; cycle < 50; cycle++) {
        /* Create multiple files */
        for (int i = 0; i < 10; i++) {
            char filename[32];
            snprintf(filename, sizeof(filename), "/file%d.txt", i);

            lfs_file_t file;
            err = lfs_file_open(&lfs, &file, filename, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
            TEST_ASSERT_EQUAL(0, err);

            char data[64];
            snprintf(data, sizeof(data), "cycle %d file %d", cycle, i);
            err = lfs_file_write(&lfs, &file, data, strlen(data));
            TEST_ASSERT_EQUAL(strlen(data), err);

            err = lfs_file_close(&lfs, &file);
            TEST_ASSERT_EQUAL(0, err);
        }

        /* Remove some files */
        for (int i = 0; i < 5; i++) {
            char filename[32];
            snprintf(filename, sizeof(filename), "/file%d.txt", i);
            lfs_remove(&lfs, filename);
        }
    }

    err = lfs_unmount(&lfs);
    TEST_ASSERT_EQUAL(0, err);

    /* Collect wear statistics */
    int32_t wear_counts[TEST_BLOCK_COUNT];
    int32_t min_wear = INT32_MAX;
    int32_t max_wear = 0;
    int32_t total_wear = 0;

    for (lfs_block_t block = 0; block < TEST_BLOCK_COUNT; block++) {
        wear_counts[block] = lfs_emubd_wear(&config, block);
        if (wear_counts[block] < min_wear) min_wear = wear_counts[block];
        if (wear_counts[block] > max_wear) max_wear = wear_counts[block];
        total_wear += wear_counts[block];
    }

    int32_t avg_wear = total_wear / TEST_BLOCK_COUNT;

    /* Print statistics for debugging */
    printf("Wear leveling statistics:\n");
    printf("  Min wear: %d\n", min_wear);
    printf("  Max wear: %d\n", max_wear);
    printf("  Avg wear: %d\n", avg_wear);
    printf("  Wear spread: %d\n", max_wear - min_wear);

    /* Verify wear leveling effectiveness */
    int32_t wear_spread = max_wear - min_wear;
    TEST_ASSERT_LESS_THAN(100, wear_spread);
}

void test_wear_leveling_with_directories(void) {
    /* Format and mount */
    int err = lfs_format(&lfs, &config);
    TEST_ASSERT_EQUAL(0, err);

    err = lfs_mount(&lfs, &config);
    TEST_ASSERT_EQUAL(0, err);

    /* Create and modify files in multiple directories */
    for (int cycle = 0; cycle < 30; cycle++) {
        for (int dir = 0; dir < 5; dir++) {
            char dirname[32];
            snprintf(dirname, sizeof(dirname), "/dir%d", dir);

            lfs_mkdir(&lfs, dirname);

            for (int i = 0; i < 3; i++) {
                char filename[64];
                snprintf(filename, sizeof(filename), "%s/file%d.txt", dirname, i);

                lfs_file_t file;
                err = lfs_file_open(&lfs, &file, filename, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
                if (err == 0) {
                    char data[64];
                    snprintf(data, sizeof(data), "cycle %d dir %d file %d", cycle, dir, i);
                    lfs_file_write(&lfs, &file, data, strlen(data));
                    lfs_file_close(&lfs, &file);
                }
            }
        }
    }

    err = lfs_unmount(&lfs);
    TEST_ASSERT_EQUAL(0, err);

    /* Check wear distribution */
    int32_t min_wear = INT32_MAX;
    int32_t max_wear = 0;

    for (lfs_block_t block = 0; block < TEST_BLOCK_COUNT; block++) {
        int32_t wear = lfs_emubd_wear(&config, block);
        if (wear < min_wear) min_wear = wear;
        if (wear > max_wear) max_wear = wear;
    }

    /* Verify wear is distributed */
    int32_t wear_spread = max_wear - min_wear;
    TEST_ASSERT_LESS_THAN(80, wear_spread);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_wear_distribution);
    RUN_TEST(test_wear_no_single_block_hot);
    RUN_TEST(test_wear_stats_after_cycles);
    RUN_TEST(test_wear_leveling_with_directories);
    return UNITY_END();
}


