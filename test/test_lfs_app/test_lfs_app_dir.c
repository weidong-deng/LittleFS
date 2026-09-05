/**
 * @file test_lfs_app_dir.c
 * @brief LittleFS application layer directory operation tests
 */

#include "unity.h"
#include "lfs_app.h"
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

void test_app_mkdir(void) {
    int err = LFS_App_MakeDir(LFS_STORAGE_INTERNAL, "/testdir");
    TEST_ASSERT_EQUAL(0, err);
}

void test_app_mkdir_nested(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_MakeDir(LFS_STORAGE_INTERNAL, "/level1"));
    TEST_ASSERT_EQUAL(0, LFS_App_MakeDir(LFS_STORAGE_INTERNAL, "/level1/level2"));
    TEST_ASSERT_EQUAL(0, LFS_App_MakeDir(LFS_STORAGE_INTERNAL, "/level1/level2/level3"));
}

void test_app_rmdir(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_MakeDir(LFS_STORAGE_INTERNAL, "/removedir"));
    TEST_ASSERT_EQUAL(0, LFS_App_RemoveDir(LFS_STORAGE_INTERNAL, "/removedir"));
}

void test_app_rmdir_nonempty(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_MakeDir(LFS_STORAGE_INTERNAL, "/nonemptydir"));
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/nonemptydir/file.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(4, LFS_App_WriteFile(LFS_STORAGE_INTERNAL, "test", 4));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));

    int err = LFS_App_RemoveDir(LFS_STORAGE_INTERNAL, "/nonemptydir");
    TEST_ASSERT_NOT_EQUAL(0, err);
}

void test_app_listdir(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_MakeDir(LFS_STORAGE_INTERNAL, "/listdir"));
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/listdir/file1.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(5, LFS_App_WriteFile(LFS_STORAGE_INTERNAL, "test1", 5));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));

    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/listdir/file2.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(5, LFS_App_WriteFile(LFS_STORAGE_INTERNAL, "test2", 5));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));

    /* ListDir should succeed without error */
    TEST_ASSERT_EQUAL(0, LFS_App_ListDir(LFS_STORAGE_INTERNAL, "/listdir"));
}

void test_app_listdir_empty(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_MakeDir(LFS_STORAGE_INTERNAL, "/emptydir"));
    TEST_ASSERT_EQUAL(0, LFS_App_ListDir(LFS_STORAGE_INTERNAL, "/emptydir"));
}

void test_app_listdir_nested(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_MakeDir(LFS_STORAGE_INTERNAL, "/nested"));
    TEST_ASSERT_EQUAL(0, LFS_App_MakeDir(LFS_STORAGE_INTERNAL, "/nested/sub1"));
    TEST_ASSERT_EQUAL(0, LFS_App_MakeDir(LFS_STORAGE_INTERNAL, "/nested/sub2"));

    TEST_ASSERT_EQUAL(0, LFS_App_ListDir(LFS_STORAGE_INTERNAL, "/nested"));
}

void test_app_listdir_with_files(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_MakeDir(LFS_STORAGE_INTERNAL, "/mixed"));
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/mixed/file1.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(5, LFS_App_WriteFile(LFS_STORAGE_INTERNAL, "test1", 5));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));

    TEST_ASSERT_EQUAL(0, LFS_App_MakeDir(LFS_STORAGE_INTERNAL, "/mixed/subdir"));
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/mixed/file2.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(5, LFS_App_WriteFile(LFS_STORAGE_INTERNAL, "test2", 5));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));

    TEST_ASSERT_EQUAL(0, LFS_App_ListDir(LFS_STORAGE_INTERNAL, "/mixed"));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_app_mkdir);
    RUN_TEST(test_app_mkdir_nested);
    RUN_TEST(test_app_rmdir);
    RUN_TEST(test_app_rmdir_nonempty);
    RUN_TEST(test_app_listdir);
    RUN_TEST(test_app_listdir_empty);
    RUN_TEST(test_app_listdir_nested);
    RUN_TEST(test_app_listdir_with_files);
    return UNITY_END();
}


