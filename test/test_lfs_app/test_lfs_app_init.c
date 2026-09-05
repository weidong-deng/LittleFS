/**
 * @file test_lfs_app_init.c
 * @brief LittleFS application layer initialization tests
 */

#include "unity.h"
#include "lfs_app.h"
#include "stubs.h"
#include <string.h>

void setUp(void) {
    LFS_Int_CleanupStub();
    LFS_SPIFlash_CleanupStub();
}

void tearDown(void) {
    /* Ignore unmount errors - filesystem may not be mounted */
    (void)LFS_App_Unmount(LFS_STORAGE_INTERNAL);
    (void)LFS_App_Unmount(LFS_STORAGE_EXTERNAL);
    LFS_Int_CleanupStub();
    LFS_SPIFlash_CleanupStub();
}

void test_app_init_internal(void) {
    int err = LFS_App_Init(LFS_STORAGE_INTERNAL);
    TEST_ASSERT_EQUAL(0, err);
    /* Init only sets flag, need format and mount to use */
    TEST_ASSERT_EQUAL(0, LFS_App_Format(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Mount(LFS_STORAGE_INTERNAL));
}

void test_app_init_external(void) {
    LFS_SPIFlash_RegisterStub();
    int err = LFS_App_Init(LFS_STORAGE_EXTERNAL);
    TEST_ASSERT_EQUAL(0, err);
    /* Init only sets flag, need format and mount to use */
    TEST_ASSERT_EQUAL(0, LFS_App_Format(LFS_STORAGE_EXTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Mount(LFS_STORAGE_EXTERNAL));
}

void test_app_init_external_no_driver(void) {
    /* Without registering driver, Init should still succeed (just sets flag) */
    /* But Format should fail because GetConfig returns NULL */
    int err = LFS_App_Init(LFS_STORAGE_EXTERNAL);
    TEST_ASSERT_EQUAL(0, err);
    
    /* Format should fail without registered driver */
    err = LFS_App_Format(LFS_STORAGE_EXTERNAL);
    TEST_ASSERT_NOT_EQUAL(0, err);
}

void test_app_format(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_Init(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Format(LFS_STORAGE_INTERNAL));
}

void test_app_mount_unmount(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_Init(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Format(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Mount(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Unmount(LFS_STORAGE_INTERNAL));
}

void test_app_init_twice(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_Init(LFS_STORAGE_INTERNAL));
    /* Second init returns OK (already initialized) */
    int err = LFS_App_Init(LFS_STORAGE_INTERNAL);
    TEST_ASSERT_EQUAL(0, err);
}

void test_app_uninit_operations(void) {
    /* Without Init, operations should still work because Mount auto-formats */
    /* But if we want to test uninit state, we need to check that Init wasn't called */
    /* Actually, LFS_App_Mount will succeed even without Init */
    int err = LFS_App_Mount(LFS_STORAGE_INTERNAL);
    TEST_ASSERT_EQUAL(0, err);
    
    /* After mount, file operations should work */
    err = LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/test.txt", LFS_MODE_WRITE);
    TEST_ASSERT_EQUAL(0, err);
    LFS_App_CloseFile(LFS_STORAGE_INTERNAL);
}

void test_app_mount_twice(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_Init(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Format(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Mount(LFS_STORAGE_INTERNAL));

    /* Second mount should fail because already mounted */
    int err = LFS_App_Mount(LFS_STORAGE_INTERNAL);
    TEST_ASSERT_NOT_EQUAL(0, err);
}

void test_app_unmount_twice(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_Init(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Format(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Mount(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Unmount(LFS_STORAGE_INTERNAL));

    int err = LFS_App_Unmount(LFS_STORAGE_INTERNAL);
    TEST_ASSERT_NOT_EQUAL(0, err);
}

void test_app_format_mount_cycle(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_Init(LFS_STORAGE_INTERNAL));

    for (int i = 0; i < 3; i++) {
        TEST_ASSERT_EQUAL(0, LFS_App_Format(LFS_STORAGE_INTERNAL));
        TEST_ASSERT_EQUAL(0, LFS_App_Mount(LFS_STORAGE_INTERNAL));
        TEST_ASSERT_EQUAL(0, LFS_App_Unmount(LFS_STORAGE_INTERNAL));
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_app_init_internal);
    RUN_TEST(test_app_init_external);
    RUN_TEST(test_app_init_external_no_driver);
    RUN_TEST(test_app_format);
    RUN_TEST(test_app_mount_unmount);
    RUN_TEST(test_app_init_twice);
    RUN_TEST(test_app_uninit_operations);
    RUN_TEST(test_app_mount_twice);
    RUN_TEST(test_app_unmount_twice);
    RUN_TEST(test_app_format_mount_cycle);
    return UNITY_END();
}


