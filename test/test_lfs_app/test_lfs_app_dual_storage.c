/**
 * @file test_lfs_app_dual_storage.c
 * @brief LittleFS application layer dual storage operation tests
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
    LFS_App_Unmount(LFS_STORAGE_INTERNAL);
    LFS_App_Unmount(LFS_STORAGE_EXTERNAL);
    LFS_Int_CleanupStub();
    LFS_SPIFlash_CleanupStub();
}

void test_dual_storage_both_init(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_Init(LFS_STORAGE_INTERNAL));
    LFS_SPIFlash_RegisterStub();
    TEST_ASSERT_EQUAL(0, LFS_App_Init(LFS_STORAGE_EXTERNAL));
}

void test_dual_storage_cross_rw(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_Init(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Format(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Mount(LFS_STORAGE_INTERNAL));

    LFS_SPIFlash_RegisterStub();
    TEST_ASSERT_EQUAL(0, LFS_App_Init(LFS_STORAGE_EXTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Format(LFS_STORAGE_EXTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Mount(LFS_STORAGE_EXTERNAL));

    /* Write to internal storage */
    const char *internal_data = "internal data";
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/internal.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(strlen(internal_data), LFS_App_WriteFile(LFS_STORAGE_INTERNAL, internal_data, strlen(internal_data)));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));

    /* Write to external storage */
    const char *external_data = "external data";
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_EXTERNAL, "/external.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(strlen(external_data), LFS_App_WriteFile(LFS_STORAGE_EXTERNAL, external_data, strlen(external_data)));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_EXTERNAL));

    /* Read from internal storage */
    char buffer[32];
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/internal.txt", LFS_MODE_READ));
    TEST_ASSERT_EQUAL(strlen(internal_data), LFS_App_ReadFile(LFS_STORAGE_INTERNAL, buffer, strlen(internal_data)));
    buffer[strlen(internal_data)] = '\0';
    TEST_ASSERT_EQUAL_STRING(internal_data, buffer);
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));

    /* Read from external storage */
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_EXTERNAL, "/external.txt", LFS_MODE_READ));
    TEST_ASSERT_EQUAL(strlen(external_data), LFS_App_ReadFile(LFS_STORAGE_EXTERNAL, buffer, strlen(external_data)));
    buffer[strlen(external_data)] = '\0';
    TEST_ASSERT_EQUAL_STRING(external_data, buffer);
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_EXTERNAL));
}

void test_dual_storage_independent(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_Init(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Format(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Mount(LFS_STORAGE_INTERNAL));

    LFS_SPIFlash_RegisterStub();
    TEST_ASSERT_EQUAL(0, LFS_App_Init(LFS_STORAGE_EXTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Format(LFS_STORAGE_EXTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Mount(LFS_STORAGE_EXTERNAL));

    /* Write to both storages */
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/test.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(8, LFS_App_WriteFile(LFS_STORAGE_INTERNAL, "internal", 8));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));

    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_EXTERNAL, "/test.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(8, LFS_App_WriteFile(LFS_STORAGE_EXTERNAL, "external", 8));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_EXTERNAL));

    /* Unmount external storage */
    TEST_ASSERT_EQUAL(0, LFS_App_Unmount(LFS_STORAGE_EXTERNAL));

    /* Internal storage should still be accessible */
    char buffer[16];
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/test.txt", LFS_MODE_READ));
    TEST_ASSERT_EQUAL(8, LFS_App_ReadFile(LFS_STORAGE_INTERNAL, buffer, 8));
    buffer[8] = '\0';
    TEST_ASSERT_EQUAL_STRING("internal", buffer);
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));
}

void test_dual_storage_same_filename(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_Init(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Format(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Mount(LFS_STORAGE_INTERNAL));

    LFS_SPIFlash_RegisterStub();
    TEST_ASSERT_EQUAL(0, LFS_App_Init(LFS_STORAGE_EXTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Format(LFS_STORAGE_EXTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Mount(LFS_STORAGE_EXTERNAL));

    /* Write same filename to both storages with different content */
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/same.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(16, LFS_App_WriteFile(LFS_STORAGE_INTERNAL, "internal content", 16));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));

    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_EXTERNAL, "/same.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(16, LFS_App_WriteFile(LFS_STORAGE_EXTERNAL, "external content", 16));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_EXTERNAL));

    /* Verify they are independent */
    char buffer[32];
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/same.txt", LFS_MODE_READ));
    TEST_ASSERT_EQUAL(16, LFS_App_ReadFile(LFS_STORAGE_INTERNAL, buffer, 16));
    buffer[16] = '\0';
    TEST_ASSERT_EQUAL_STRING("internal content", buffer);
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));

    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_EXTERNAL, "/same.txt", LFS_MODE_READ));
    TEST_ASSERT_EQUAL(16, LFS_App_ReadFile(LFS_STORAGE_EXTERNAL, buffer, 16));
    buffer[16] = '\0';
    TEST_ASSERT_EQUAL_STRING("external content", buffer);
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_EXTERNAL));
}

void test_dual_storage_format_one(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_Init(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Format(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Mount(LFS_STORAGE_INTERNAL));

    LFS_SPIFlash_RegisterStub();
    TEST_ASSERT_EQUAL(0, LFS_App_Init(LFS_STORAGE_EXTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Format(LFS_STORAGE_EXTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Mount(LFS_STORAGE_EXTERNAL));

    /* Write to both storages */
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/test.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(8, LFS_App_WriteFile(LFS_STORAGE_INTERNAL, "internal", 8));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));

    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_EXTERNAL, "/test.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(8, LFS_App_WriteFile(LFS_STORAGE_EXTERNAL, "external", 8));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_EXTERNAL));

    /* Unmount and format internal storage */
    TEST_ASSERT_EQUAL(0, LFS_App_Unmount(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Format(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Mount(LFS_STORAGE_INTERNAL));

    /* External storage should still have its data */
    char buffer[16];
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_EXTERNAL, "/test.txt", LFS_MODE_READ));
    TEST_ASSERT_EQUAL(8, LFS_App_ReadFile(LFS_STORAGE_EXTERNAL, buffer, 8));
    buffer[8] = '\0';
    TEST_ASSERT_EQUAL_STRING("external", buffer);
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_EXTERNAL));
}

void test_external_storage_file_operations(void) {
    LFS_SPIFlash_RegisterStub();
    TEST_ASSERT_EQUAL(0, LFS_App_Init(LFS_STORAGE_EXTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Format(LFS_STORAGE_EXTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Mount(LFS_STORAGE_EXTERNAL));

    /* Test file operations on external storage */
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_EXTERNAL, "/ext_file.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(12, LFS_App_WriteFile(LFS_STORAGE_EXTERNAL, "external data", 12));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_EXTERNAL));

    /* Test seek and tell */
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_EXTERNAL, "/ext_file.txt", LFS_MODE_READ));
    TEST_ASSERT_EQUAL(5, LFS_App_SeekFile(LFS_STORAGE_EXTERNAL, 5, LFS_SEEK_SET));
    TEST_ASSERT_EQUAL(5, LFS_App_TellFile(LFS_STORAGE_EXTERNAL));

    char buffer[16];
    TEST_ASSERT_EQUAL(7, LFS_App_ReadFile(LFS_STORAGE_EXTERNAL, buffer, 7));
    buffer[7] = '\0';
    TEST_ASSERT_EQUAL_STRING("nal dat", buffer);
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_EXTERNAL));

    /* Test file size */
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_EXTERNAL, "/ext_file.txt", LFS_MODE_READ));
    TEST_ASSERT_EQUAL(12, LFS_App_GetCurrentFileSize(LFS_STORAGE_EXTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_EXTERNAL));

    /* Test sync */
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_EXTERNAL, "/ext_file.txt", LFS_MODE_READ));
    TEST_ASSERT_EQUAL(0, LFS_App_SyncFile(LFS_STORAGE_EXTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_EXTERNAL));

    /* Test rewind */
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_EXTERNAL, "/ext_file.txt", LFS_MODE_READ));
    TEST_ASSERT_EQUAL(5, LFS_App_SeekFile(LFS_STORAGE_EXTERNAL, 5, LFS_SEEK_SET));
    TEST_ASSERT_EQUAL(0, LFS_App_RewindFile(LFS_STORAGE_EXTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_TellFile(LFS_STORAGE_EXTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_EXTERNAL));

    /* Test truncate */
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_EXTERNAL, "/ext_file.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(0, LFS_App_TruncateFile(LFS_STORAGE_EXTERNAL, 6));
    TEST_ASSERT_EQUAL(6, LFS_App_GetCurrentFileSize(LFS_STORAGE_EXTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_EXTERNAL));

    /* Test remove */
    TEST_ASSERT_EQUAL(0, LFS_App_RemoveFile(LFS_STORAGE_EXTERNAL, "/ext_file.txt"));
    TEST_ASSERT_EQUAL(0, LFS_App_FileExists(LFS_STORAGE_EXTERNAL, "/ext_file.txt"));

    /* Test rename */
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_EXTERNAL, "/rename_src.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(4, LFS_App_WriteFile(LFS_STORAGE_EXTERNAL, "test", 4));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_EXTERNAL));

    TEST_ASSERT_EQUAL(0, LFS_App_Rename(LFS_STORAGE_EXTERNAL, "/rename_src.txt", "/rename_dst.txt"));
    TEST_ASSERT_EQUAL(1, LFS_App_FileExists(LFS_STORAGE_EXTERNAL, "/rename_dst.txt"));
}

void test_external_storage_directory_operations(void) {
    LFS_SPIFlash_RegisterStub();
    TEST_ASSERT_EQUAL(0, LFS_App_Init(LFS_STORAGE_EXTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Format(LFS_STORAGE_EXTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Mount(LFS_STORAGE_EXTERNAL));

    /* Test mkdir */
    TEST_ASSERT_EQUAL(0, LFS_App_MakeDir(LFS_STORAGE_EXTERNAL, "/ext_dir"));

    /* Test listdir */
    TEST_ASSERT_EQUAL(0, LFS_App_ListDir(LFS_STORAGE_EXTERNAL, "/"));

    /* Test remove dir */
    TEST_ASSERT_EQUAL(0, LFS_App_RemoveDir(LFS_STORAGE_EXTERNAL, "/ext_dir"));
}

void test_external_storage_info_operations(void) {
    LFS_SPIFlash_RegisterStub();
    TEST_ASSERT_EQUAL(0, LFS_App_Init(LFS_STORAGE_EXTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Format(LFS_STORAGE_EXTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Mount(LFS_STORAGE_EXTERNAL));

    /* Test file exists */
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_EXTERNAL, "/info_test.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(8, LFS_App_WriteFile(LFS_STORAGE_EXTERNAL, "testinfo", 8));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_EXTERNAL));

    TEST_ASSERT_EQUAL(1, LFS_App_FileExists(LFS_STORAGE_EXTERNAL, "/info_test.txt"));

    /* Test get file size */
    uint32_t file_size = 0;
    TEST_ASSERT_EQUAL(0, LFS_App_GetFileSize(LFS_STORAGE_EXTERNAL, "/info_test.txt", &file_size));
    TEST_ASSERT_EQUAL(8, file_size);

    /* Test get fs info */
    struct lfs_fsinfo fs_info;
    TEST_ASSERT_EQUAL(0, LFS_App_GetFSInfo(LFS_STORAGE_EXTERNAL, &fs_info));
    TEST_ASSERT_EQUAL(4096, fs_info.block_size);

    /* Test get fs size */
    int32_t fs_size = LFS_App_GetFSSize(LFS_STORAGE_EXTERNAL);
    TEST_ASSERT_GREATER_THAN(0, fs_size);

    /* Test gc */
    TEST_ASSERT_EQUAL(0, LFS_App_FSGC(LFS_STORAGE_EXTERNAL));
}

void test_external_storage_invalid_operations(void) {
    LFS_SPIFlash_RegisterStub();
    TEST_ASSERT_EQUAL(0, LFS_App_Init(LFS_STORAGE_EXTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Format(LFS_STORAGE_EXTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_Mount(LFS_STORAGE_EXTERNAL));

    /* Test close on unopened file */
    TEST_ASSERT_EQUAL(LFS_ERR_BADF, LFS_App_CloseFile(LFS_STORAGE_EXTERNAL));

    /* Test file operations on non-existent file */
    TEST_ASSERT_EQUAL(0, LFS_App_FileExists(LFS_STORAGE_EXTERNAL, "/no_such_file.txt"));
    uint32_t sz = 0;
    TEST_ASSERT_NOT_EQUAL(0, LFS_App_GetFileSize(LFS_STORAGE_EXTERNAL, "/no_such_file.txt", &sz));

    /* Test remove non-existent file */
    TEST_ASSERT_NOT_EQUAL(0, LFS_App_RemoveFile(LFS_STORAGE_EXTERNAL, "/no_such_file.txt"));

    /* Test rename non-existent file */
    TEST_ASSERT_NOT_EQUAL(0, LFS_App_Rename(LFS_STORAGE_EXTERNAL, "/no_src.txt", "/no_dst.txt"));

    /* Test remove non-existent dir */
    TEST_ASSERT_NOT_EQUAL(0, LFS_App_RemoveDir(LFS_STORAGE_EXTERNAL, "/no_such_dir"));

    /* Test listdir non-existent dir */
    TEST_ASSERT_NOT_EQUAL(0, LFS_App_ListDir(LFS_STORAGE_EXTERNAL, "/no_such_dir"));

    /* Test double mount */
    TEST_ASSERT_NOT_EQUAL(0, LFS_App_Mount(LFS_STORAGE_EXTERNAL));

    /* Test double unmount */
    TEST_ASSERT_EQUAL(0, LFS_App_Unmount(LFS_STORAGE_EXTERNAL));
    TEST_ASSERT_NOT_EQUAL(0, LFS_App_Unmount(LFS_STORAGE_EXTERNAL));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_dual_storage_both_init);
    RUN_TEST(test_dual_storage_cross_rw);
    RUN_TEST(test_dual_storage_independent);
    RUN_TEST(test_dual_storage_same_filename);
    RUN_TEST(test_dual_storage_format_one);
    RUN_TEST(test_external_storage_file_operations);
    RUN_TEST(test_external_storage_directory_operations);
    RUN_TEST(test_external_storage_info_operations);
    RUN_TEST(test_external_storage_invalid_operations);
    return UNITY_END();
}


