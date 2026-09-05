/**
 * @file test_lfs_app_info.c
 * @brief LittleFS application layer information query tests
 */

#include "unity.h"
#include "lfs_app.h"
#include "lfs.h"
#include "stubs.h"
#include <string.h>

void setUp(void) {
    LFS_Int_CleanupStub();
    TEST_ASSERT_EQUAL(0, LFS_App_Init(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Format(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Mount(LFS_STORAGE_INTERNAL));
}

void tearDown(void) {
    LFS_App_Unmount(LFS_STORAGE_INTERNAL);
    LFS_Int_CleanupStub();
}

void test_app_file_exists_true(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/exists.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(4, LFS_App_WriteFile(LFS_STORAGE_INTERNAL, "test", 4));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));

    TEST_ASSERT_EQUAL(1, LFS_App_FileExists(LFS_STORAGE_INTERNAL, "/exists.txt"));
}

void test_app_file_exists_false(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_FileExists(LFS_STORAGE_INTERNAL, "/nonexistent.txt"));
}

void test_app_get_file_size(void) {
    const char *data = "test data";
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/size.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(strlen(data), LFS_App_WriteFile(LFS_STORAGE_INTERNAL, data, strlen(data)));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));

    uint32_t size;
    TEST_ASSERT_EQUAL(0, LFS_App_GetFileSize(LFS_STORAGE_INTERNAL, "/size.txt", &size));
    TEST_ASSERT_EQUAL(strlen(data), size);
}

void test_app_get_file_size_nonexist(void) {
    uint32_t size;
    int err = LFS_App_GetFileSize(LFS_STORAGE_INTERNAL, "/nonexistent.txt", &size);
    TEST_ASSERT_NOT_EQUAL(0, err);
}

void test_app_get_fs_info(void) {
    struct lfs_fsinfo info;
    int err = LFS_App_GetFSInfo(LFS_STORAGE_INTERNAL, &info);
    TEST_ASSERT_EQUAL(0, err);

    TEST_ASSERT_GREATER_THAN(0, info.block_size);
    TEST_ASSERT_GREATER_THAN(0, info.block_count);
}

void test_app_get_fs_size(void) {
    lfs_ssize_t size = LFS_App_GetFSSize(LFS_STORAGE_INTERNAL);
    TEST_ASSERT_GREATER_THAN(0, size);
}

void test_app_fs_gc(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/gc.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(4, LFS_App_WriteFile(LFS_STORAGE_INTERNAL, "test", 4));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));

    TEST_ASSERT_EQUAL(0, LFS_App_RemoveFile(LFS_STORAGE_INTERNAL, "/gc.txt"));
    TEST_ASSERT_EQUAL(0, LFS_App_FSGC(LFS_STORAGE_INTERNAL));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_app_file_exists_true);
    RUN_TEST(test_app_file_exists_false);
    RUN_TEST(test_app_get_file_size);
    RUN_TEST(test_app_get_file_size_nonexist);
    RUN_TEST(test_app_get_fs_info);
    RUN_TEST(test_app_get_fs_size);
    RUN_TEST(test_app_fs_gc);
    return UNITY_END();
}


