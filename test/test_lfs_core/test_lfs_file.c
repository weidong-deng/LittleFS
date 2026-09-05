/**
 * @file test_lfs_file.c
 * @brief LittleFS file operation tests
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

void test_file_create_and_write(void) {
    lfs_file_t file;
    int err = lfs_file_open(&ctx.lfs, &file, "/test.txt", LFS_O_WRONLY | LFS_O_CREAT);
    TEST_ASSERT_EQUAL(0, err);

    const char *data = "Hello, LittleFS!";
    lfs_ssize_t written = lfs_file_write(&ctx.lfs, &file, data, strlen(data));
    TEST_ASSERT_EQUAL(strlen(data), written);

    TEST_ASSERT_EQUAL(0, lfs_file_close(&ctx.lfs, &file));
}

void test_file_read_after_write(void) {
    const char *test_data = "test data 123";
    TEST_ASSERT_EQUAL(0, test_write_file(&ctx.lfs, "/test.txt", test_data, strlen(test_data)));

    char buffer[32];
    TEST_ASSERT_EQUAL(0, test_read_file(&ctx.lfs, "/test.txt", buffer, strlen(test_data)));
    buffer[strlen(test_data)] = '\0';
    TEST_ASSERT_EQUAL_STRING(test_data, buffer);
}

void test_file_open_nonexistent(void) {
    lfs_file_t file;
    int err = lfs_file_open(&ctx.lfs, &file, "/nonexistent.txt", LFS_O_RDONLY);
    TEST_ASSERT_NOT_EQUAL(0, err);
}

void test_file_open_flags(void) {
    lfs_file_t file;

    /* Test LFS_O_CREAT */
    int err = lfs_file_open(&ctx.lfs, &file, "/create.txt", LFS_O_WRONLY | LFS_O_CREAT);
    TEST_ASSERT_EQUAL(0, err);
    lfs_file_close(&ctx.lfs, &file);

    /* Test LFS_O_TRUNC */
    err = lfs_file_open(&ctx.lfs, &file, "/create.txt", LFS_O_WRONLY | LFS_O_TRUNC);
    TEST_ASSERT_EQUAL(0, err);
    lfs_file_close(&ctx.lfs, &file);

    /* Test LFS_O_RDONLY */
    err = lfs_file_open(&ctx.lfs, &file, "/create.txt", LFS_O_RDONLY);
    TEST_ASSERT_EQUAL(0, err);
    lfs_file_close(&ctx.lfs, &file);
}

void test_file_write_read_large(void) {
    uint32_t size = TEST_BLOCK_SIZE * 2;
    uint8_t *write_buf = (uint8_t *)malloc(size);
    uint8_t *read_buf = (uint8_t *)malloc(size);
    TEST_ASSERT_NOT_NULL(write_buf);
    TEST_ASSERT_NOT_NULL(read_buf);

    test_generate_pattern(write_buf, size, 0x42);
    TEST_ASSERT_EQUAL(0, test_write_file(&ctx.lfs, "/large.bin", write_buf, size));
    TEST_ASSERT_EQUAL(0, test_read_file(&ctx.lfs, "/large.bin", read_buf, size));
    TEST_ASSERT_EQUAL_MEMORY(write_buf, read_buf, size);

    free(write_buf);
    free(read_buf);
}

void test_file_seek_set(void) {
    lfs_file_t file;
    TEST_ASSERT_EQUAL(0, lfs_file_open(&ctx.lfs, &file, "/seek.txt", LFS_O_RDWR | LFS_O_CREAT));

    const char *data = "0123456789";
    lfs_file_write(&ctx.lfs, &file, data, strlen(data));

    lfs_soff_t seek_result = lfs_file_seek(&ctx.lfs, &file, 5, LFS_SEEK_SET);
    TEST_ASSERT_EQUAL(5, seek_result);
    TEST_ASSERT_EQUAL(5, lfs_file_tell(&ctx.lfs, &file));

    char buffer[5];
    lfs_file_read(&ctx.lfs, &file, buffer, 5);
    TEST_ASSERT_EQUAL_MEMORY("56789", buffer, 5);

    lfs_file_close(&ctx.lfs, &file);
}

void test_file_seek_cur(void) {
    lfs_file_t file;
    TEST_ASSERT_EQUAL(0, lfs_file_open(&ctx.lfs, &file, "/seek_cur.txt", LFS_O_RDWR | LFS_O_CREAT));

    const char *data = "0123456789";
    lfs_file_write(&ctx.lfs, &file, data, strlen(data));

    lfs_file_seek(&ctx.lfs, &file, 0, LFS_SEEK_SET);
    lfs_file_seek(&ctx.lfs, &file, 3, LFS_SEEK_CUR);
    TEST_ASSERT_EQUAL(3, lfs_file_tell(&ctx.lfs, &file));

    lfs_file_close(&ctx.lfs, &file);
}

void test_file_seek_end(void) {
    lfs_file_t file;
    TEST_ASSERT_EQUAL(0, lfs_file_open(&ctx.lfs, &file, "/seek_end.txt", LFS_O_RDWR | LFS_O_CREAT));

    const char *data = "0123456789";
    lfs_file_write(&ctx.lfs, &file, data, strlen(data));

    TEST_ASSERT_EQUAL(10, lfs_file_seek(&ctx.lfs, &file, 0, LFS_SEEK_END));
    TEST_ASSERT_EQUAL(8, lfs_file_seek(&ctx.lfs, &file, -2, LFS_SEEK_END));

    lfs_file_close(&ctx.lfs, &file);
}

void test_file_tell(void) {
    lfs_file_t file;
    TEST_ASSERT_EQUAL(0, lfs_file_open(&ctx.lfs, &file, "/tell.txt", LFS_O_RDWR | LFS_O_CREAT));

    TEST_ASSERT_EQUAL(0, lfs_file_tell(&ctx.lfs, &file));

    const char *data = "test";
    lfs_file_write(&ctx.lfs, &file, data, strlen(data));
    TEST_ASSERT_EQUAL(4, lfs_file_tell(&ctx.lfs, &file));

    lfs_file_close(&ctx.lfs, &file);
}

void test_file_truncate_shrink(void) {
    lfs_file_t file;
    TEST_ASSERT_EQUAL(0, lfs_file_open(&ctx.lfs, &file, "/truncate.txt", LFS_O_RDWR | LFS_O_CREAT));

    const char *data = "0123456789";
    lfs_file_write(&ctx.lfs, &file, data, strlen(data));

    TEST_ASSERT_EQUAL(0, lfs_file_truncate(&ctx.lfs, &file, 5));
    TEST_ASSERT_EQUAL(5, lfs_file_size(&ctx.lfs, &file));

    lfs_file_close(&ctx.lfs, &file);
}

void test_file_truncate_grow(void) {
    lfs_file_t file;
    TEST_ASSERT_EQUAL(0, lfs_file_open(&ctx.lfs, &file, "/truncate_grow.txt", LFS_O_RDWR | LFS_O_CREAT));

    const char *data = "test";
    lfs_file_write(&ctx.lfs, &file, data, strlen(data));

    TEST_ASSERT_EQUAL(0, lfs_file_truncate(&ctx.lfs, &file, 10));
    TEST_ASSERT_EQUAL(10, lfs_file_size(&ctx.lfs, &file));

    lfs_file_close(&ctx.lfs, &file);
}

void test_file_sync(void) {
    lfs_file_t file;
    TEST_ASSERT_EQUAL(0, lfs_file_open(&ctx.lfs, &file, "/sync.txt", LFS_O_WRONLY | LFS_O_CREAT));

    const char *data = "sync test";
    lfs_file_write(&ctx.lfs, &file, data, strlen(data));

    TEST_ASSERT_EQUAL(0, lfs_file_sync(&ctx.lfs, &file));
    lfs_file_close(&ctx.lfs, &file);
}

void test_file_rewind(void) {
    lfs_file_t file;
    TEST_ASSERT_EQUAL(0, lfs_file_open(&ctx.lfs, &file, "/rewind.txt", LFS_O_RDWR | LFS_O_CREAT));

    const char *data = "test";
    lfs_file_write(&ctx.lfs, &file, data, strlen(data));

    TEST_ASSERT_EQUAL(4, lfs_file_tell(&ctx.lfs, &file));
    lfs_file_rewind(&ctx.lfs, &file);
    TEST_ASSERT_EQUAL(0, lfs_file_tell(&ctx.lfs, &file));

    lfs_file_close(&ctx.lfs, &file);
}

void test_file_size(void) {
    lfs_file_t file;
    TEST_ASSERT_EQUAL(0, lfs_file_open(&ctx.lfs, &file, "/size.txt", LFS_O_WRONLY | LFS_O_CREAT));

    const char *data = "test data";
    lfs_file_write(&ctx.lfs, &file, data, strlen(data));

    TEST_ASSERT_EQUAL(9, lfs_file_size(&ctx.lfs, &file));
    lfs_file_close(&ctx.lfs, &file);
}

void test_file_close(void) {
    lfs_file_t file;
    TEST_ASSERT_EQUAL(0, lfs_file_open(&ctx.lfs, &file, "/close.txt", LFS_O_WRONLY | LFS_O_CREAT));
    TEST_ASSERT_EQUAL(0, lfs_file_close(&ctx.lfs, &file));
}

void test_file_remove(void) {
    TEST_ASSERT_EQUAL(0, test_write_file(&ctx.lfs, "/remove.txt", "test", 4));
    TEST_ASSERT_EQUAL(1, test_file_exists(&ctx.lfs, "/remove.txt"));

    TEST_ASSERT_EQUAL(0, lfs_remove(&ctx.lfs, "/remove.txt"));
    TEST_ASSERT_EQUAL(0, test_file_exists(&ctx.lfs, "/remove.txt"));
}

void test_file_rename(void) {
    TEST_ASSERT_EQUAL(0, test_write_file(&ctx.lfs, "/old.txt", "test", 4));
    TEST_ASSERT_EQUAL(1, test_file_exists(&ctx.lfs, "/old.txt"));

    TEST_ASSERT_EQUAL(0, lfs_rename(&ctx.lfs, "/old.txt", "/new.txt"));
    TEST_ASSERT_EQUAL(0, test_file_exists(&ctx.lfs, "/old.txt"));
    TEST_ASSERT_EQUAL(1, test_file_exists(&ctx.lfs, "/new.txt"));
}

void test_file_rename_cross_dir(void) {
    TEST_ASSERT_EQUAL(0, lfs_mkdir(&ctx.lfs, "/dir1"));
    TEST_ASSERT_EQUAL(0, lfs_mkdir(&ctx.lfs, "/dir2"));

    TEST_ASSERT_EQUAL(0, test_write_file(&ctx.lfs, "/dir1/file.txt", "test", 4));
    TEST_ASSERT_EQUAL(0, lfs_rename(&ctx.lfs, "/dir1/file.txt", "/dir2/file.txt"));

    TEST_ASSERT_EQUAL(1, test_file_exists(&ctx.lfs, "/dir2/file.txt"));
}

void test_file_multiple_open(void) {
    lfs_file_t file1, file2;

    TEST_ASSERT_EQUAL(0, lfs_file_open(&ctx.lfs, &file1, "/file1.txt", LFS_O_WRONLY | LFS_O_CREAT));
    TEST_ASSERT_EQUAL(0, lfs_file_open(&ctx.lfs, &file2, "/file2.txt", LFS_O_WRONLY | LFS_O_CREAT));

    lfs_file_write(&ctx.lfs, &file1, "test1", 5);
    lfs_file_write(&ctx.lfs, &file2, "test2", 5);

    lfs_file_close(&ctx.lfs, &file1);
    lfs_file_close(&ctx.lfs, &file2);
}

void test_file_append_mode(void) {
    lfs_file_t file;

    TEST_ASSERT_EQUAL(0, lfs_file_open(&ctx.lfs, &file, "/append.txt", LFS_O_WRONLY | LFS_O_CREAT));
    lfs_file_write(&ctx.lfs, &file, "test1", 5);
    lfs_file_close(&ctx.lfs, &file);

    TEST_ASSERT_EQUAL(0, lfs_file_open(&ctx.lfs, &file, "/append.txt", LFS_O_WRONLY | LFS_O_APPEND));
    lfs_file_write(&ctx.lfs, &file, "test2", 5);
    TEST_ASSERT_EQUAL(10, lfs_file_size(&ctx.lfs, &file));
    lfs_file_close(&ctx.lfs, &file);
}

void test_file_cross_block(void) {
    uint32_t size = TEST_BLOCK_SIZE * 3;
    uint8_t *write_buf = (uint8_t *)malloc(size);
    uint8_t *read_buf = (uint8_t *)malloc(size);
    TEST_ASSERT_NOT_NULL(write_buf);
    TEST_ASSERT_NOT_NULL(read_buf);

    test_generate_pattern(write_buf, size, 0xAB);
    TEST_ASSERT_EQUAL(0, test_write_file(&ctx.lfs, "/cross_block.bin", write_buf, size));
    TEST_ASSERT_EQUAL(0, test_read_file(&ctx.lfs, "/cross_block.bin", read_buf, size));
    TEST_ASSERT_EQUAL_MEMORY(write_buf, read_buf, size);

    free(write_buf);
    free(read_buf);
}

void test_file_name_max_length(void) {
    char long_name[LFS_NAME_MAX + 1];
    memset(long_name, 'a', LFS_NAME_MAX);
    long_name[LFS_NAME_MAX] = '\0';

    lfs_file_t file;
    int err = lfs_file_open(&ctx.lfs, &file, long_name, LFS_O_WRONLY | LFS_O_CREAT);
    TEST_ASSERT_EQUAL(0, err);
    lfs_file_close(&ctx.lfs, &file);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_file_create_and_write);
    RUN_TEST(test_file_read_after_write);
    RUN_TEST(test_file_open_nonexistent);
    RUN_TEST(test_file_open_flags);
    RUN_TEST(test_file_write_read_large);
    RUN_TEST(test_file_seek_set);
    RUN_TEST(test_file_seek_cur);
    RUN_TEST(test_file_seek_end);
    RUN_TEST(test_file_tell);
    RUN_TEST(test_file_truncate_shrink);
    RUN_TEST(test_file_truncate_grow);
    RUN_TEST(test_file_sync);
    RUN_TEST(test_file_rewind);
    RUN_TEST(test_file_size);
    RUN_TEST(test_file_close);
    RUN_TEST(test_file_remove);
    RUN_TEST(test_file_rename);
    RUN_TEST(test_file_rename_cross_dir);
    RUN_TEST(test_file_multiple_open);
    RUN_TEST(test_file_append_mode);
    RUN_TEST(test_file_cross_block);
    RUN_TEST(test_file_name_max_length);
    return UNITY_END();
}


