/**
 * @file test_lfs_app_file.c
 * @brief LittleFS application layer file operation tests
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
    /* Unmount first (closes all open files), then cleanup */
    (void)LFS_App_Unmount(LFS_STORAGE_INTERNAL);
    LFS_App_ResetState();
    LFS_Int_CleanupStub();
}

void test_app_file_open_close(void) {
    int err = LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/test.txt", LFS_MODE_WRITE);
    TEST_ASSERT_EQUAL(0, err);

    err = LFS_App_CloseFile(LFS_STORAGE_INTERNAL);
    TEST_ASSERT_EQUAL(0, err);
}

void test_app_file_read_write(void) {
    const char *write_data = "test data";
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/test.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(strlen(write_data), LFS_App_WriteFile(LFS_STORAGE_INTERNAL, write_data, strlen(write_data)));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));

    char buffer[32];
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/test.txt", LFS_MODE_READ));
    TEST_ASSERT_EQUAL(strlen(write_data), LFS_App_ReadFile(LFS_STORAGE_INTERNAL, buffer, strlen(write_data)));
    buffer[strlen(write_data)] = '\0';
    TEST_ASSERT_EQUAL_STRING(write_data, buffer);
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));
}

void test_app_file_seek_set(void) {
    const char *data = "0123456789";
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/seek.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(strlen(data), LFS_App_WriteFile(LFS_STORAGE_INTERNAL, data, strlen(data)));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));

    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/seek.txt", LFS_MODE_READ));
    TEST_ASSERT_EQUAL(5, LFS_App_SeekFile(LFS_STORAGE_INTERNAL, 5, LFS_SEEK_SET));
    TEST_ASSERT_EQUAL(5, LFS_App_TellFile(LFS_STORAGE_INTERNAL));

    char buffer[5];
    TEST_ASSERT_EQUAL(5, LFS_App_ReadFile(LFS_STORAGE_INTERNAL, buffer, 5));
    TEST_ASSERT_EQUAL_MEMORY("56789", buffer, 5);
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));
}

void test_app_file_seek_cur(void) {
    const char *data = "0123456789";
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/seek_cur.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(strlen(data), LFS_App_WriteFile(LFS_STORAGE_INTERNAL, data, strlen(data)));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));

    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/seek_cur.txt", LFS_MODE_READ));
    TEST_ASSERT_EQUAL(3, LFS_App_SeekFile(LFS_STORAGE_INTERNAL, 3, LFS_SEEK_CUR));
    TEST_ASSERT_EQUAL(3, LFS_App_TellFile(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));
}

void test_app_file_seek_end(void) {
    const char *data = "0123456789";
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/seek_end.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(strlen(data), LFS_App_WriteFile(LFS_STORAGE_INTERNAL, data, strlen(data)));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));

    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/seek_end.txt", LFS_MODE_READ));
    TEST_ASSERT_EQUAL(10, LFS_App_SeekFile(LFS_STORAGE_INTERNAL, 0, LFS_SEEK_END));
    TEST_ASSERT_EQUAL(8, LFS_App_SeekFile(LFS_STORAGE_INTERNAL, -2, LFS_SEEK_END));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));
}

void test_app_file_tell(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/tell.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(0, LFS_App_TellFile(LFS_STORAGE_INTERNAL));

    const char *data = "test";
    TEST_ASSERT_EQUAL(strlen(data), LFS_App_WriteFile(LFS_STORAGE_INTERNAL, data, strlen(data)));
    TEST_ASSERT_EQUAL(4, LFS_App_TellFile(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));
}

void test_app_file_truncate_shrink(void) {
    const char *data = "0123456789";
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/truncate.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(strlen(data), LFS_App_WriteFile(LFS_STORAGE_INTERNAL, data, strlen(data)));

    TEST_ASSERT_EQUAL(0, LFS_App_TruncateFile(LFS_STORAGE_INTERNAL, 5));
    TEST_ASSERT_EQUAL(5, LFS_App_GetCurrentFileSize(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));
}

void test_app_file_truncate_grow(void) {
    const char *data = "test";
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/truncate_grow.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(strlen(data), LFS_App_WriteFile(LFS_STORAGE_INTERNAL, data, strlen(data)));

    TEST_ASSERT_EQUAL(0, LFS_App_TruncateFile(LFS_STORAGE_INTERNAL, 10));
    TEST_ASSERT_EQUAL(10, LFS_App_GetCurrentFileSize(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));
}

void test_app_file_rewind(void) {
    const char *data = "test";
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/rewind.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(strlen(data), LFS_App_WriteFile(LFS_STORAGE_INTERNAL, data, strlen(data)));

    TEST_ASSERT_EQUAL(4, LFS_App_TellFile(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_RewindFile(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_TellFile(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));
}

void test_app_file_sync(void) {
    const char *data = "sync test";
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/sync.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(strlen(data), LFS_App_WriteFile(LFS_STORAGE_INTERNAL, data, strlen(data)));

    TEST_ASSERT_EQUAL(0, LFS_App_SyncFile(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));
}

void test_app_file_get_current_size(void) {
    const char *data = "test data";
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/size.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(strlen(data), LFS_App_WriteFile(LFS_STORAGE_INTERNAL, data, strlen(data)));

    TEST_ASSERT_EQUAL(9, LFS_App_GetCurrentFileSize(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));
}

void test_app_file_remove(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/remove.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(4, LFS_App_WriteFile(LFS_STORAGE_INTERNAL, "test", 4));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));

    TEST_ASSERT_EQUAL(1, LFS_App_FileExists(LFS_STORAGE_INTERNAL, "/remove.txt"));
    TEST_ASSERT_EQUAL(0, LFS_App_RemoveFile(LFS_STORAGE_INTERNAL, "/remove.txt"));
    TEST_ASSERT_EQUAL(0, LFS_App_FileExists(LFS_STORAGE_INTERNAL, "/remove.txt"));
}

void test_app_file_rename(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/old.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(4, LFS_App_WriteFile(LFS_STORAGE_INTERNAL, "test", 4));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));

    TEST_ASSERT_EQUAL(1, LFS_App_FileExists(LFS_STORAGE_INTERNAL, "/old.txt"));
    TEST_ASSERT_EQUAL(0, LFS_App_Rename(LFS_STORAGE_INTERNAL, "/old.txt", "/new.txt"));
    TEST_ASSERT_EQUAL(0, LFS_App_FileExists(LFS_STORAGE_INTERNAL, "/old.txt"));  /* old file should not exist */
    TEST_ASSERT_EQUAL(1, LFS_App_FileExists(LFS_STORAGE_INTERNAL, "/new.txt"));
}

void test_app_file_rename_cross_dir(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_MakeDir(LFS_STORAGE_INTERNAL, "/dir1"));
    TEST_ASSERT_EQUAL(0, LFS_App_MakeDir(LFS_STORAGE_INTERNAL, "/dir2"));

    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/dir1/file.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(4, LFS_App_WriteFile(LFS_STORAGE_INTERNAL, "test", 4));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));

    TEST_ASSERT_EQUAL(0, LFS_App_Rename(LFS_STORAGE_INTERNAL, "/dir1/file.txt", "/dir2/file.txt"));
    TEST_ASSERT_EQUAL(1, LFS_App_FileExists(LFS_STORAGE_INTERNAL, "/dir2/file.txt"));
}

void test_app_file_open_unopened_close(void) {
    int err = LFS_App_CloseFile(LFS_STORAGE_INTERNAL);
    TEST_ASSERT_NOT_EQUAL(0, err);
}

void test_app_file_append(void) {
    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/append.txt", LFS_MODE_WRITE));
    TEST_ASSERT_EQUAL(5, LFS_App_WriteFile(LFS_STORAGE_INTERNAL, "test1", 5));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));

    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/append.txt", LFS_MODE_APPEND));
    TEST_ASSERT_EQUAL(5, LFS_App_WriteFile(LFS_STORAGE_INTERNAL, "test2", 5));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));

    TEST_ASSERT_EQUAL(0, LFS_App_OpenFile(LFS_STORAGE_INTERNAL, "/append.txt", LFS_MODE_READ));
    TEST_ASSERT_EQUAL(10, LFS_App_GetCurrentFileSize(LFS_STORAGE_INTERNAL));
    TEST_ASSERT_EQUAL(0, LFS_App_CloseFile(LFS_STORAGE_INTERNAL));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_app_file_open_close);
    RUN_TEST(test_app_file_read_write);
    RUN_TEST(test_app_file_seek_set);
    RUN_TEST(test_app_file_seek_cur);
    RUN_TEST(test_app_file_seek_end);
    RUN_TEST(test_app_file_tell);
    RUN_TEST(test_app_file_truncate_shrink);
    RUN_TEST(test_app_file_truncate_grow);
    RUN_TEST(test_app_file_rewind);
    RUN_TEST(test_app_file_sync);
    RUN_TEST(test_app_file_get_current_size);
    RUN_TEST(test_app_file_remove);
    RUN_TEST(test_app_file_rename);
    RUN_TEST(test_app_file_rename_cross_dir);
    RUN_TEST(test_app_file_open_unopened_close);
    RUN_TEST(test_app_file_append);
    return UNITY_END();
}


