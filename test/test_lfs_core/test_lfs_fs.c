/**
 * @file test_lfs_fs.c
 * @brief LittleFS filesystem information tests
 */

#include "unity.h"
#include "lfs.h"
#include "test_helpers.h"

static test_context_t ctx;

void setUp(void) {
    TEST_ASSERT_EQUAL(0, test_context_init(&ctx, TEST_BLOCK_SIZE, TEST_BLOCK_COUNT));
    TEST_ASSERT_EQUAL(0, test_format_and_mount(&ctx));
}

void tearDown(void) {
    test_unmount_and_cleanup(&ctx);
    test_context_cleanup(&ctx);
}

void test_fs_stat(void) {
    struct lfs_fsinfo info;
    int err = lfs_fs_stat(&ctx.lfs, &info);
    TEST_ASSERT_EQUAL(0, err);

    TEST_ASSERT_EQUAL(TEST_BLOCK_SIZE, info.block_size);
    TEST_ASSERT_EQUAL(TEST_BLOCK_COUNT, info.block_count);
}

void test_fs_size(void) {
    lfs_ssize_t size_before = lfs_fs_size(&ctx.lfs);
    TEST_ASSERT_GREATER_THAN(0, size_before);

    TEST_ASSERT_EQUAL(0, test_write_file(&ctx.lfs, "/size_test.txt", "test data", 9));

    lfs_ssize_t size_after = lfs_fs_size(&ctx.lfs);
    TEST_ASSERT_GREATER_OR_EQUAL(size_before, size_after);
}

void test_fs_gc(void) {
    TEST_ASSERT_EQUAL(0, test_write_file(&ctx.lfs, "/gc_test.txt", "gc test", 7));
    TEST_ASSERT_EQUAL(0, lfs_remove(&ctx.lfs, "/gc_test.txt"));

    int err = lfs_fs_gc(&ctx.lfs);
    TEST_ASSERT_EQUAL(0, err);
}

void test_fs_stat_after_operations(void) {
    struct lfs_fsinfo info1;
    TEST_ASSERT_EQUAL(0, lfs_fs_stat(&ctx.lfs, &info1));

    TEST_ASSERT_EQUAL(0, test_write_file(&ctx.lfs, "/stat_test.txt", "test", 4));
    TEST_ASSERT_EQUAL(0, lfs_mkdir(&ctx.lfs, "/stat_dir"));

    struct lfs_fsinfo info2;
    TEST_ASSERT_EQUAL(0, lfs_fs_stat(&ctx.lfs, &info2));

    TEST_ASSERT_EQUAL(info1.block_size, info2.block_size);
    TEST_ASSERT_EQUAL(info1.block_count, info2.block_count);
}

static int traverse_visit_count;

static int traverse_cb(void *data, lfs_block_t block) {
    (void)data;
    (void)block;
    traverse_visit_count++;
    return 0;
}

static int traverse_cb_stop(void *data, lfs_block_t block) {
    (void)data;
    (void)block;
    traverse_visit_count++;
    return 1; /* stop after first block */
}

void test_fs_traverse(void) {
    /* Create some files and directories */
    TEST_ASSERT_EQUAL(0, test_write_file(&ctx.lfs, "/traverse1.txt", "data1", 5));
    TEST_ASSERT_EQUAL(0, test_write_file(&ctx.lfs, "/traverse2.txt", "data2", 5));
    TEST_ASSERT_EQUAL(0, lfs_mkdir(&ctx.lfs, "/traverse_dir"));
    TEST_ASSERT_EQUAL(0, test_write_file(&ctx.lfs, "/traverse_dir/file.txt", "data3", 5));

    /* Traverse all blocks - callback counts visited blocks */
    traverse_visit_count = 0;

    int err = lfs_fs_traverse(&ctx.lfs, traverse_cb, NULL);
    TEST_ASSERT_EQUAL(0, err);
    TEST_ASSERT_GREATER_THAN(0, traverse_visit_count);
}

void test_fs_traverse_callback_stop(void) {
    TEST_ASSERT_EQUAL(0, test_write_file(&ctx.lfs, "/stop_traverse.txt", "data", 4));

    traverse_visit_count = 0;

    /* Callback returns non-zero to stop traversal early */
    int err = lfs_fs_traverse(&ctx.lfs, traverse_cb_stop, NULL);
    TEST_ASSERT_EQUAL(1, err);
    TEST_ASSERT_EQUAL(1, traverse_visit_count);
}

void test_fs_grow(void) {
    /* Get current block count from fsinfo */
    struct lfs_fsinfo info;
    TEST_ASSERT_EQUAL(0, lfs_fs_stat(&ctx.lfs, &info));
    lfs_size_t block_count = info.block_count;

    /* Grow by 2 blocks */
    int err = lfs_fs_grow(&ctx.lfs, block_count + 2);
    TEST_ASSERT_EQUAL(0, err);

    /* Verify block count increased */
    struct lfs_fsinfo info2;
    TEST_ASSERT_EQUAL(0, lfs_fs_stat(&ctx.lfs, &info2));
    TEST_ASSERT_EQUAL(block_count + 2, info2.block_count);
}

void test_fs_grow_same_size(void) {
    struct lfs_fsinfo info;
    TEST_ASSERT_EQUAL(0, lfs_fs_stat(&ctx.lfs, &info));

    /* Grow to same size should be no-op */
    int err = lfs_fs_grow(&ctx.lfs, info.block_count);
    TEST_ASSERT_EQUAL(0, err);
}

void test_stat_file(void) {
    TEST_ASSERT_EQUAL(0, test_write_file(&ctx.lfs, "/stat_file.txt", "hello stat", 10));

    struct lfs_info info;
    int err = lfs_stat(&ctx.lfs, "/stat_file.txt", &info);
    TEST_ASSERT_EQUAL(0, err);
    TEST_ASSERT_EQUAL(LFS_TYPE_REG, info.type);
    TEST_ASSERT_EQUAL(10, info.size);
    TEST_ASSERT_EQUAL_STRING("stat_file.txt", info.name);
}

void test_stat_directory(void) {
    TEST_ASSERT_EQUAL(0, lfs_mkdir(&ctx.lfs, "/stat_dir"));

    struct lfs_info info;
    int err = lfs_stat(&ctx.lfs, "/stat_dir", &info);
    TEST_ASSERT_EQUAL(0, err);
    TEST_ASSERT_EQUAL(LFS_TYPE_DIR, info.type);
    TEST_ASSERT_EQUAL_STRING("stat_dir", info.name);
}

void test_stat_nonexistent(void) {
    struct lfs_info info;
    int err = lfs_stat(&ctx.lfs, "/nonexistent", &info);
    TEST_ASSERT_NOT_EQUAL(0, err);
}

void test_getattr_setattr(void) {
    /* Create a file first */
    lfs_file_t file;
    TEST_ASSERT_EQUAL(0, lfs_file_open(&ctx.lfs, &file, "/attr_test.txt",
                                       LFS_O_WRONLY | LFS_O_CREAT));

    /* Set an attribute */
    uint8_t attr_value[] = {0x01, 0x02, 0x03};
    int err = lfs_setattr(&ctx.lfs, "/attr_test.txt", 0x80, attr_value, sizeof(attr_value));
    TEST_ASSERT_EQUAL(0, err);

    /* Get the attribute back */
    uint8_t read_value[4];
    lfs_ssize_t attr_size = lfs_getattr(&ctx.lfs, "/attr_test.txt", 0x80, read_value, sizeof(read_value));
    TEST_ASSERT_EQUAL(3, attr_size);
    TEST_ASSERT_EQUAL_MEMORY(attr_value, read_value, 3);

    lfs_file_close(&ctx.lfs, &file);
}

void test_removeattr(void) {
    lfs_file_t file;
    TEST_ASSERT_EQUAL(0, lfs_file_open(&ctx.lfs, &file, "/rmattr.txt",
                                       LFS_O_WRONLY | LFS_O_CREAT));

    uint8_t attr_value[] = {0xAA, 0xBB};
    TEST_ASSERT_EQUAL(0, lfs_setattr(&ctx.lfs, "/rmattr.txt", 0x81, attr_value, sizeof(attr_value)));

    /* Remove the attribute */
    int err = lfs_removeattr(&ctx.lfs, "/rmattr.txt", 0x81);
    TEST_ASSERT_EQUAL(0, err);

    /* Verify it's gone */
    uint8_t read_value[4];
    lfs_ssize_t attr_size = lfs_getattr(&ctx.lfs, "/rmattr.txt", 0x81, read_value, sizeof(read_value));
    TEST_ASSERT_EQUAL(LFS_ERR_NOATTR, attr_size);

    lfs_file_close(&ctx.lfs, &file);
}

void test_getattr_nonexistent(void) {
    uint8_t buf[4];
    lfs_ssize_t err = lfs_getattr(&ctx.lfs, "/no_file.txt", 0x80, buf, sizeof(buf));
    TEST_ASSERT_NOT_EQUAL(0, err);
}

void test_setattr_invalid_type(void) {
    /* Attribute type 0x7F is reserved (below 0x80 user range) */
    uint8_t val = 0x01;
    int err = lfs_setattr(&ctx.lfs, "/attr.txt", 0x7F, &val, 1);
    TEST_ASSERT_NOT_EQUAL(0, err);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_fs_stat);
    RUN_TEST(test_fs_size);
    RUN_TEST(test_fs_gc);
    RUN_TEST(test_fs_stat_after_operations);
    RUN_TEST(test_fs_traverse);
    RUN_TEST(test_fs_traverse_callback_stop);
    RUN_TEST(test_fs_grow);
    RUN_TEST(test_fs_grow_same_size);
    RUN_TEST(test_stat_file);
    RUN_TEST(test_stat_directory);
    RUN_TEST(test_stat_nonexistent);
    RUN_TEST(test_getattr_setattr);
    RUN_TEST(test_removeattr);
    RUN_TEST(test_getattr_nonexistent);
    RUN_TEST(test_setattr_invalid_type);
    return UNITY_END();
}


