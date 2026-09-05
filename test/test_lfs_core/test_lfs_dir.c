/**
 * @file test_lfs_dir.c
 * @brief LittleFS directory operation tests
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

void test_dir_create(void) {
    int err = lfs_mkdir(&ctx.lfs, "/testdir");
    TEST_ASSERT_EQUAL(0, err);
    TEST_ASSERT_EQUAL(1, test_dir_exists(&ctx.lfs, "/testdir"));
}

void test_dir_open_close(void) {
    TEST_ASSERT_EQUAL(0, lfs_mkdir(&ctx.lfs, "/opendir"));

    lfs_dir_t dir;
    int err = lfs_dir_open(&ctx.lfs, &dir, "/opendir");
    TEST_ASSERT_EQUAL(0, err);

    err = lfs_dir_close(&ctx.lfs, &dir);
    TEST_ASSERT_EQUAL(0, err);
}

void test_dir_read_entries(void) {
    TEST_ASSERT_EQUAL(0, lfs_mkdir(&ctx.lfs, "/readir"));
    TEST_ASSERT_EQUAL(0, test_write_file(&ctx.lfs, "/readir/file1.txt", "test1", 5));
    TEST_ASSERT_EQUAL(0, test_write_file(&ctx.lfs, "/readir/file2.txt", "test2", 5));

    lfs_dir_t dir;
    TEST_ASSERT_EQUAL(0, lfs_dir_open(&ctx.lfs, &dir, "/readir"));

    struct lfs_info info;
    int count = 0;
    while (lfs_dir_read(&ctx.lfs, &dir, &info) > 0) {
        count++;
    }

    TEST_ASSERT_EQUAL(0, lfs_dir_close(&ctx.lfs, &dir));
    TEST_ASSERT_GREATER_THAN(2, count);
}

void test_dir_read_empty(void) {
    TEST_ASSERT_EQUAL(0, lfs_mkdir(&ctx.lfs, "/emptydir"));

    lfs_dir_t dir;
    TEST_ASSERT_EQUAL(0, lfs_dir_open(&ctx.lfs, &dir, "/emptydir"));

    struct lfs_info info;
    int count = 0;
    while (lfs_dir_read(&ctx.lfs, &dir, &info) > 0) {
        count++;
    }

    TEST_ASSERT_EQUAL(0, lfs_dir_close(&ctx.lfs, &dir));
    TEST_ASSERT_EQUAL(2, count);
}

void test_dir_remove_empty(void) {
    TEST_ASSERT_EQUAL(0, lfs_mkdir(&ctx.lfs, "/removedir"));
    TEST_ASSERT_EQUAL(1, test_dir_exists(&ctx.lfs, "/removedir"));

    int err = lfs_remove(&ctx.lfs, "/removedir");
    TEST_ASSERT_EQUAL(0, err);
    TEST_ASSERT_EQUAL(0, test_dir_exists(&ctx.lfs, "/removedir"));
}

void test_dir_remove_nonempty(void) {
    TEST_ASSERT_EQUAL(0, lfs_mkdir(&ctx.lfs, "/nonemptydir"));
    TEST_ASSERT_EQUAL(0, test_write_file(&ctx.lfs, "/nonemptydir/file.txt", "test", 4));

    int err = lfs_remove(&ctx.lfs, "/nonemptydir");
    TEST_ASSERT_NOT_EQUAL(0, err);
}

void test_dir_nested(void) {
    TEST_ASSERT_EQUAL(0, lfs_mkdir(&ctx.lfs, "/level1"));
    TEST_ASSERT_EQUAL(0, lfs_mkdir(&ctx.lfs, "/level1/level2"));
    TEST_ASSERT_EQUAL(0, lfs_mkdir(&ctx.lfs, "/level1/level2/level3"));

    TEST_ASSERT_EQUAL(1, test_dir_exists(&ctx.lfs, "/level1"));
    TEST_ASSERT_EQUAL(1, test_dir_exists(&ctx.lfs, "/level1/level2"));
    TEST_ASSERT_EQUAL(1, test_dir_exists(&ctx.lfs, "/level1/level2/level3"));
}

void test_dir_remove_recursive(void) {
    TEST_ASSERT_EQUAL(0, lfs_mkdir(&ctx.lfs, "/recursive"));
    TEST_ASSERT_EQUAL(0, lfs_mkdir(&ctx.lfs, "/recursive/sub1"));
    TEST_ASSERT_EQUAL(0, lfs_mkdir(&ctx.lfs, "/recursive/sub2"));
    TEST_ASSERT_EQUAL(0, test_write_file(&ctx.lfs, "/recursive/sub1/file1.txt", "test1", 5));
    TEST_ASSERT_EQUAL(0, test_write_file(&ctx.lfs, "/recursive/sub2/file2.txt", "test2", 5));

    TEST_ASSERT_EQUAL(0, lfs_remove(&ctx.lfs, "/recursive/sub1/file1.txt"));
    TEST_ASSERT_EQUAL(0, lfs_remove(&ctx.lfs, "/recursive/sub2/file2.txt"));
    TEST_ASSERT_EQUAL(0, lfs_remove(&ctx.lfs, "/recursive/sub1"));
    TEST_ASSERT_EQUAL(0, lfs_remove(&ctx.lfs, "/recursive/sub2"));
    TEST_ASSERT_EQUAL(0, lfs_remove(&ctx.lfs, "/recursive"));

    TEST_ASSERT_EQUAL(0, test_dir_exists(&ctx.lfs, "/recursive"));
}

void test_dir_file_in_dir(void) {
    TEST_ASSERT_EQUAL(0, lfs_mkdir(&ctx.lfs, "/filedir"));
    TEST_ASSERT_EQUAL(0, test_write_file(&ctx.lfs, "/filedir/test.txt", "content", 7));

    TEST_ASSERT_EQUAL(1, test_file_exists(&ctx.lfs, "/filedir/test.txt"));

    char buffer[16];
    TEST_ASSERT_EQUAL(0, test_read_file(&ctx.lfs, "/filedir/test.txt", buffer, 7));
    buffer[7] = '\0';
    TEST_ASSERT_EQUAL_STRING("content", buffer);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_dir_create);
    RUN_TEST(test_dir_open_close);
    RUN_TEST(test_dir_read_entries);
    RUN_TEST(test_dir_read_empty);
    RUN_TEST(test_dir_remove_empty);
    RUN_TEST(test_dir_remove_nonempty);
    RUN_TEST(test_dir_nested);
    RUN_TEST(test_dir_remove_recursive);
    RUN_TEST(test_dir_file_in_dir);
    return UNITY_END();
}


