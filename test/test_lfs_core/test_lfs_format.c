/**
 * @file test_lfs_format.c
 * @brief LittleFS format operation tests
 */

#include "unity.h"
#include "lfs.h"
#include "test_helpers.h"

static test_context_t ctx;

void setUp(void) {
    TEST_ASSERT_EQUAL(0, test_context_init(&ctx, TEST_BLOCK_SIZE, TEST_BLOCK_COUNT));
}

void tearDown(void) {
    test_context_cleanup(&ctx);
}

void test_format_empty_device(void) {
    int err = lfs_format(&ctx.lfs, &ctx.config);
    TEST_ASSERT_EQUAL(0, err);
}

void test_format_then_mount(void) {
    TEST_ASSERT_EQUAL(0, lfs_format(&ctx.lfs, &ctx.config));
    TEST_ASSERT_EQUAL(0, lfs_mount(&ctx.lfs, &ctx.config));
    TEST_ASSERT_EQUAL(0, lfs_unmount(&ctx.lfs));
}

void test_format_twice(void) {
    TEST_ASSERT_EQUAL(0, lfs_format(&ctx.lfs, &ctx.config));
    TEST_ASSERT_EQUAL(0, lfs_mount(&ctx.lfs, &ctx.config));
    TEST_ASSERT_EQUAL(0, lfs_unmount(&ctx.lfs));

    TEST_ASSERT_EQUAL(0, lfs_format(&ctx.lfs, &ctx.config));
    TEST_ASSERT_EQUAL(0, lfs_mount(&ctx.lfs, &ctx.config));
    TEST_ASSERT_EQUAL(0, lfs_unmount(&ctx.lfs));
}

void test_format_preserves_data_after_remount(void) {
    TEST_ASSERT_EQUAL(0, test_format_and_mount(&ctx));

    const char *test_data = "test data";
    TEST_ASSERT_EQUAL(0, test_write_file(&ctx.lfs, "/test.txt", test_data, strlen(test_data)));

    TEST_ASSERT_EQUAL(0, test_unmount_and_cleanup(&ctx));
    TEST_ASSERT_EQUAL(0, lfs_mount(&ctx.lfs, &ctx.config));

    char buffer[32];
    TEST_ASSERT_EQUAL(0, test_read_file(&ctx.lfs, "/test.txt", buffer, strlen(test_data)));
    buffer[strlen(test_data)] = '\0';
    TEST_ASSERT_EQUAL_STRING(test_data, buffer);

    TEST_ASSERT_EQUAL(0, lfs_unmount(&ctx.lfs));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_format_empty_device);
    RUN_TEST(test_format_then_mount);
    RUN_TEST(test_format_twice);
    RUN_TEST(test_format_preserves_data_after_remount);
    return UNITY_END();
}


