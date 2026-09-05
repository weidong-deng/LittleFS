/*!
 * @file        lfs_app.c

 * @brief       LittleFS Application Layer Implementation
 *
 * @version     V1.0.0
 *
 * @date        2026-08-07
 *
 * @attention
 *
 *  LittleFS application layer for easy file system operations.
 *  External flash uses generic SPI flash port - any chip can be
 *  plugged in by registering a driver that implements lfs_spi_flash_drv_t.
 */

/* Includes */
#include "lfs_app.h"
#include "lfs_flash_int_port.h"
#include "lfs_spi_flash_port.h"
#include <string.h>

/** @addtogroup LFS_Application
  @{
*/

/** @addtogroup LFS_APP_Variables
  @{
  */

/* LittleFS instances */
static lfs_t lfs_internal;
static lfs_t lfs_external;

/* File handles */
static lfs_file_t file_internal;
static lfs_file_t file_external;

/* Initialization flags */
static uint8_t internal_initialized = 0;
static uint8_t external_initialized = 0;

/* Mount state tracking */
static uint8_t internal_mounted = 0;
static uint8_t external_mounted = 0;

/* File open state tracking */
static uint8_t internal_file_open = 0;
static uint8_t external_file_open = 0;

/**@} end of group LFS_APP_Variables */

/** @addtogroup LFS_APP_Functions
  @{
*/

/*!
 * @brief       Reset all state (for testing)
 *
 * This function resets all internal state variables. Used by test teardown
 * to ensure clean state between tests.
 */
void LFS_App_ResetState(void)
{
    memset(&lfs_internal, 0, sizeof(lfs_internal));
    memset(&lfs_external, 0, sizeof(lfs_external));
    memset(&file_internal, 0, sizeof(file_internal));
    memset(&file_external, 0, sizeof(file_external));
    internal_initialized = 0;
    external_initialized = 0;
    internal_mounted = 0;
    external_mounted = 0;
    internal_file_open = 0;
    external_file_open = 0;
}

/*!
 * @brief       Initialize LittleFS for selected storage
 *
 * For LFS_STORAGE_EXTERNAL, the caller must have already registered
 * a flash driver via LFS_SPIFlash_Register() before calling this.
 *
 * @param       storage     Storage device (internal or external)
 *
 * @retval      LFS_ERR_OK on success, negative error code on failure
 */
int LFS_App_Init(LFS_Storage_t storage)
{
    if (storage == LFS_STORAGE_INTERNAL)
    {
        if (internal_initialized)
        {
            return LFS_ERR_OK;
        }
        internal_initialized = 1;
        return LFS_ERR_OK;
    }
    else if (storage == LFS_STORAGE_EXTERNAL)
    {
        if (external_initialized)
        {
            return LFS_ERR_OK;
        }

        /* Verify that a flash driver has been registered */
        if (LFS_SPIFlash_GetConfig() == NULL)
        {
            return LFS_ERR_IO;
        }

        /* Call the driver's init function via the port layer */
        /* The driver init is done during registration, we just mark ready */
        external_initialized = 1;
        return LFS_ERR_OK;
    }

    return LFS_ERR_INVAL;
}

/*!
 * @brief       Format LittleFS file system
 *
 * @param       storage     Storage device (internal or external)
 *
 * @retval      LFS_ERR_OK on success, negative error code on failure
 */
int LFS_App_Format(LFS_Storage_t storage)
{
    if (storage == LFS_STORAGE_INTERNAL)
    {
        return lfs_format(&lfs_internal, LFS_Int_GetConfig());
    }
    else if (storage == LFS_STORAGE_EXTERNAL)
    {
        const struct lfs_config *cfg = LFS_SPIFlash_GetConfig();
        if (cfg == NULL)
        {
            return LFS_ERR_IO;
        }
        return lfs_format(&lfs_external, cfg);
    }

    return LFS_ERR_INVAL;
}

/*!
 * @brief       Mount LittleFS file system
 *
 * If mount fails (e.g. first boot), automatically formats and retries.
 *
 * @param       storage     Storage device (internal or external)
 *
 * @retval      LFS_ERR_OK on success, negative error code on failure
 */
int LFS_App_Mount(LFS_Storage_t storage)
{
    int err;

    if (storage == LFS_STORAGE_INTERNAL)
    {
        if (internal_mounted) return LFS_ERR_INVAL;
        err = lfs_mount(&lfs_internal, LFS_Int_GetConfig());
        if (err != LFS_ERR_OK)
        {
            err = lfs_format(&lfs_internal, LFS_Int_GetConfig());
            if (err != LFS_ERR_OK) return err;
            err = lfs_mount(&lfs_internal, LFS_Int_GetConfig());
        }
        if (err == LFS_ERR_OK) internal_mounted = 1;
        return err;
    }
    else if (storage == LFS_STORAGE_EXTERNAL)
    {
        if (external_mounted) return LFS_ERR_INVAL;
        const struct lfs_config *cfg = LFS_SPIFlash_GetConfig();
        if (cfg == NULL) return LFS_ERR_IO;

        err = lfs_mount(&lfs_external, cfg);
        if (err != LFS_ERR_OK)
        {
            err = lfs_format(&lfs_external, cfg);
            if (err != LFS_ERR_OK) return err;
            err = lfs_mount(&lfs_external, cfg);
        }
        if (err == LFS_ERR_OK) external_mounted = 1;
        return err;
    }

    return LFS_ERR_INVAL;
}

/*!
 * @brief       Unmount LittleFS file system
 *
 * @param       storage     Storage device (internal or external)
 *
 * @retval      LFS_ERR_OK on success, negative error code on failure
 */
int LFS_App_Unmount(LFS_Storage_t storage)
{
    if (storage == LFS_STORAGE_INTERNAL)
    {
        if (!internal_mounted) return LFS_ERR_INVAL;
        int err = lfs_unmount(&lfs_internal);
        if (err == LFS_ERR_OK) internal_mounted = 0;
        return err;
    }
    else if (storage == LFS_STORAGE_EXTERNAL)
    {
        if (!external_mounted) return LFS_ERR_INVAL;
        int err = lfs_unmount(&lfs_external);
        if (err == LFS_ERR_OK) external_mounted = 0;
        return err;
    }

    return LFS_ERR_INVAL;
}

/*!
 * @brief       Open a file
 *
 * @param       storage     Storage device (internal or external)
 * @param       path        File path
 * @param       mode        File open mode (read, write, append)
 *
 * @retval      LFS_ERR_OK on success, negative error code on failure
 */
int LFS_App_OpenFile(LFS_Storage_t storage, const char *path, LFS_FileMode_t mode)
{
    int flags;
    lfs_t *lfs;
    lfs_file_t *file;
    uint8_t *file_open;

    switch (mode)
    {
        case LFS_MODE_READ:
            flags = LFS_O_RDONLY;
            break;
        case LFS_MODE_WRITE:
            flags = LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC;
            break;
        case LFS_MODE_APPEND:
            flags = LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND;
            break;
        default:
            return LFS_ERR_INVAL;
    }

    if (storage == LFS_STORAGE_INTERNAL)
    {
        lfs = &lfs_internal;
        file = &file_internal;
        file_open = &internal_file_open;
    }
    else if (storage == LFS_STORAGE_EXTERNAL)
    {
        lfs = &lfs_external;
        file = &file_external;
        file_open = &external_file_open;
    }
    else
    {
        return LFS_ERR_INVAL;
    }

    int err = lfs_file_open(lfs, file, path, flags);
    if (err == LFS_ERR_OK)
    {
        *file_open = 1;
    }
    return err;
}

/*!
 * @brief       Close a file
 *
 * @param       storage     Storage device (internal or external)
 *
 * @retval      LFS_ERR_OK on success, negative error code on failure
 */
int LFS_App_CloseFile(LFS_Storage_t storage)
{
    if (storage == LFS_STORAGE_INTERNAL)
    {
        if (!internal_file_open) return LFS_ERR_BADF;
        int err = lfs_file_close(&lfs_internal, &file_internal);
        if (err == LFS_ERR_OK)
        {
            internal_file_open = 0;
        }
        return err;
    }
    else if (storage == LFS_STORAGE_EXTERNAL)
    {
        if (!external_file_open) return LFS_ERR_BADF;
        int err = lfs_file_close(&lfs_external, &file_external);
        if (err == LFS_ERR_OK)
        {
            external_file_open = 0;
        }
        return err;
    }

    return LFS_ERR_INVAL;
}

/*!
 * @brief       Read from a file
 *
 * @param       storage     Storage device (internal or external)
 * @param       buffer      Buffer to store read data
 * @param       size        Number of bytes to read
 *
 * @retval      Number of bytes read on success, negative error code on failure
 */
int LFS_App_ReadFile(LFS_Storage_t storage, void *buffer, uint32_t size)
{
    if (storage == LFS_STORAGE_INTERNAL)
    {
        return lfs_file_read(&lfs_internal, &file_internal, buffer, size);
    }
    else if (storage == LFS_STORAGE_EXTERNAL)
    {
        return lfs_file_read(&lfs_external, &file_external, buffer, size);
    }

    return LFS_ERR_INVAL;
}

/*!
 * @brief       Write to a file
 *
 * @param       storage     Storage device (internal or external)
 * @param       buffer      Data buffer to write
 * @param       size        Number of bytes to write
 *
 * @retval      Number of bytes written on success, negative error code on failure
 */
int LFS_App_WriteFile(LFS_Storage_t storage, const void *buffer, uint32_t size)
{
    if (storage == LFS_STORAGE_INTERNAL)
    {
        return lfs_file_write(&lfs_internal, &file_internal, buffer, size);
    }
    else if (storage == LFS_STORAGE_EXTERNAL)
    {
        return lfs_file_write(&lfs_external, &file_external, buffer, size);
    }

    return LFS_ERR_INVAL;
}

/*!
 * @brief       Seek to a position in a file
 *
 * @param       storage     Storage device (internal or external)
 * @param       offset      Offset to seek to
 * @param       whence      Seek origin (LFS_SEEK_SET, LFS_SEEK_CUR, LFS_SEEK_END)  
 *
 * @retval      New file position on success, negative error code on failure
 */
int LFS_App_SeekFile(LFS_Storage_t storage, int32_t offset, int whence)
{
    if (storage == LFS_STORAGE_INTERNAL)
    {
        return lfs_file_seek(&lfs_internal, &file_internal, offset, whence);
    }
    else if (storage == LFS_STORAGE_EXTERNAL)
    {
        return lfs_file_seek(&lfs_external, &file_external, offset, whence);
    }

    return LFS_ERR_INVAL;
}

/*!
 * @brief       Get current file position
 *
 * @param       storage     Storage device (internal or external)
 *
 * @retval      Current file position on success, negative error code on failure
 */
int32_t LFS_App_TellFile(LFS_Storage_t storage)
{
    if (storage == LFS_STORAGE_INTERNAL)
    {
        return lfs_file_tell(&lfs_internal, &file_internal);
    }
    else if (storage == LFS_STORAGE_EXTERNAL)
    {
        return lfs_file_tell(&lfs_external, &file_external);
    }

    return LFS_ERR_INVAL;
}

/*!
 * @brief       Remove a file
 *
 * @param       storage     Storage device (internal or external)
 * @param       path        File path to remove
 *
 * @retval      LFS_ERR_OK on success, negative error code on failure
 */
int LFS_App_RemoveFile(LFS_Storage_t storage, const char *path)
{
    if (storage == LFS_STORAGE_INTERNAL)
    {
        return lfs_remove(&lfs_internal, path);
    }
    else if (storage == LFS_STORAGE_EXTERNAL)
    {
        return lfs_remove(&lfs_external, path);
    }

    return LFS_ERR_INVAL;
}

/*!
 * @brief       Create a directory
 *
 * @param       storage     Storage device (internal or external)
 * @param       path        Directory path to create
 *
 * @retval      LFS_ERR_OK on success, negative error code on failure
 */
int LFS_App_MakeDir(LFS_Storage_t storage, const char *path)
{
    if (storage == LFS_STORAGE_INTERNAL)
    {
        return lfs_mkdir(&lfs_internal, path);
    }
    else if (storage == LFS_STORAGE_EXTERNAL)
    {
        return lfs_mkdir(&lfs_external, path);
    }

    return LFS_ERR_INVAL;
}

/*!
 * @brief       Remove a directory
 *
 * @param       storage     Storage device (internal or external)
 * @param       path        Directory path to remove
 *
 * @retval      LFS_ERR_OK on success, negative error code on failure
 */
int LFS_App_RemoveDir(LFS_Storage_t storage, const char *path)
{
    if (storage == LFS_STORAGE_INTERNAL)
    {
        return lfs_remove(&lfs_internal, path);
    }
    else if (storage == LFS_STORAGE_EXTERNAL)
    {
        return lfs_remove(&lfs_external, path);
    }

    return LFS_ERR_INVAL;
}

/*!
 * @brief       Check if a file exists
 *
 * @param       storage     Storage device (internal or external)
 * @param       path        File path to check
 *
 * @retval      1 if file exists, 0 if not, negative error code on failure
 */
int LFS_App_FileExists(LFS_Storage_t storage, const char *path)
{
    struct lfs_info info;
    int err;

    if (storage == LFS_STORAGE_INTERNAL)
    {
        err = lfs_stat(&lfs_internal, path, &info);
    }
    else if (storage == LFS_STORAGE_EXTERNAL)
    {
        err = lfs_stat(&lfs_external, path, &info);
    }
    else
    {
        return LFS_ERR_INVAL;
    }

    return (err == LFS_ERR_OK) ? 1 : 0;
}

/*!
 * @brief       Get file size
 *
 * @param       storage     Storage device (internal or external)
 * @param       path        File path
 * @param       size        Pointer to store file size
 *
 * @retval      LFS_ERR_OK on success, negative error code on failure
 */
int LFS_App_GetFileSize(LFS_Storage_t storage, const char *path, uint32_t *size)
{
    struct lfs_info info;
    int err;

    if (storage == LFS_STORAGE_INTERNAL)
    {
        err = lfs_stat(&lfs_internal, path, &info);
    }
    else if (storage == LFS_STORAGE_EXTERNAL)
    {
        err = lfs_stat(&lfs_external, path, &info);
    }
    else
    {
        return LFS_ERR_INVAL;
    }

    if (err == LFS_ERR_OK)
    {
        *size = info.size;
    }

    return err;
}

/*!
 * @brief       Get file system information
 *
 * @param       storage     Storage device (internal or external)
 * @param       info        Pointer to store file system info
 *
 * @retval      LFS_ERR_OK on success, negative error code on failure
 */
int LFS_App_GetFSInfo(LFS_Storage_t storage, struct lfs_fsinfo *info)
{
    if (storage == LFS_STORAGE_INTERNAL)
    {
        return lfs_fs_stat(&lfs_internal, info);
    }
    else if (storage == LFS_STORAGE_EXTERNAL)
    {
        return lfs_fs_stat(&lfs_external, info);
    }

    return LFS_ERR_INVAL;
}

/*!
 * @brief       Get file system size (number of allocated blocks)
 *
 * @param       storage     Storage device (internal or external)
 *
 * @retval      Number of allocated blocks on success, negative error code on failure
 */
int32_t LFS_App_GetFSSize(LFS_Storage_t storage)
{
    if (storage == LFS_STORAGE_INTERNAL)
    {
        return lfs_fs_size(&lfs_internal);
    }
    else if (storage == LFS_STORAGE_EXTERNAL)
    {
        return lfs_fs_size(&lfs_external);
    }

    return LFS_ERR_INVAL;
}

/*!
 * @brief       Sync file to storage
 *
 * @param       storage     Storage device (internal or external)
 *
 * @retval      LFS_ERR_OK on success, negative error code on failure
 */
int LFS_App_SyncFile(LFS_Storage_t storage)
{
    if (storage == LFS_STORAGE_INTERNAL)
    {
        return lfs_file_sync(&lfs_internal, &file_internal);
    }
    else if (storage == LFS_STORAGE_EXTERNAL)
    {
        return lfs_file_sync(&lfs_external, &file_external);
    }

    return LFS_ERR_INVAL;
}

/*!
 * @brief       Get current file size
 *
 * @param       storage     Storage device (internal or external)
 *
 * @retval      File size on success, negative error code on failure
 */
int32_t LFS_App_GetCurrentFileSize(LFS_Storage_t storage)
{
    if (storage == LFS_STORAGE_INTERNAL)
    {
        return lfs_file_size(&lfs_internal, &file_internal);
    }
    else if (storage == LFS_STORAGE_EXTERNAL)
    {
        return lfs_file_size(&lfs_external, &file_external);
    }

    return LFS_ERR_INVAL;
}

/*!
 * @brief       Rewind file position to beginning
 *
 * @param       storage     Storage device (internal or external)
 *
 * @retval      LFS_ERR_OK on success, negative error code on failure
 */
int LFS_App_RewindFile(LFS_Storage_t storage)
{
    if (storage == LFS_STORAGE_INTERNAL)
    {
        return lfs_file_rewind(&lfs_internal, &file_internal);
    }
    else if (storage == LFS_STORAGE_EXTERNAL)
    {
        return lfs_file_rewind(&lfs_external, &file_external);
    }

    return LFS_ERR_INVAL;
}

/*!
 * @brief       Truncate file to specified size
 *
 * @param       storage     Storage device (internal or external)
 * @param       size        New file size
 *
 * @retval      LFS_ERR_OK on success, negative error code on failure
 */
int LFS_App_TruncateFile(LFS_Storage_t storage, uint32_t size)
{
    if (storage == LFS_STORAGE_INTERNAL)
    {
        return lfs_file_truncate(&lfs_internal, &file_internal, size);
    }
    else if (storage == LFS_STORAGE_EXTERNAL)
    {
        return lfs_file_truncate(&lfs_external, &file_external, size);
    }

    return LFS_ERR_INVAL;
}

/*!
 * @brief       Rename a file or directory
 *
 * @param       storage     Storage device (internal or external)
 * @param       oldpath     Old path
 * @param       newpath     New path
 *
 * @retval      LFS_ERR_OK on success, negative error code on failure
 */
int LFS_App_Rename(LFS_Storage_t storage, const char *oldpath, const char *newpath)
{
    if (storage == LFS_STORAGE_INTERNAL)
    {
        return lfs_rename(&lfs_internal, oldpath, newpath);
    }
    else if (storage == LFS_STORAGE_EXTERNAL)
    {
        return lfs_rename(&lfs_external, oldpath, newpath);
    }

    return LFS_ERR_INVAL;
}

/*!
 * @brief       List directory contents (print to stdout)
 *
 * @param       storage     Storage device (internal or external)
 * @param       path        Directory path
 *
 * @retval      LFS_ERR_OK on success, negative error code on failure
 */
int LFS_App_ListDir(LFS_Storage_t storage, const char *path)
{
    lfs_dir_t dir;
    struct lfs_info info;
    int err;
    lfs_t *lfs;

    if (storage == LFS_STORAGE_INTERNAL)
    {
        lfs = &lfs_internal;
    }
    else if (storage == LFS_STORAGE_EXTERNAL)
    {
        lfs = &lfs_external;
    }
    else
    {
        return LFS_ERR_INVAL;
    }

    err = lfs_dir_open(lfs, &dir, path);
    if (err != LFS_ERR_OK)
    {
        return err;
    }

    while (lfs_dir_read(lfs, &dir, &info) > 0)
    {
        printf("  %s%s%s\n",
               (info.type == LFS_TYPE_DIR) ? "[DIR] " : "",
               info.name,
               (info.type == LFS_TYPE_REG) ? " (file)" : "");
    }

    return lfs_dir_close(lfs, &dir);
}

/*!
 * @brief       Perform garbage collection
 *
 * @param       storage     Storage device (internal or external)
 *
 * @retval      LFS_ERR_OK on success, negative error code on failure
 */
int LFS_App_FSGC(LFS_Storage_t storage)
{
    if (storage == LFS_STORAGE_INTERNAL)
    {
        return lfs_fs_gc(&lfs_internal);
    }
    else if (storage == LFS_STORAGE_EXTERNAL)
    {
        return lfs_fs_gc(&lfs_external);
    }

    return LFS_ERR_INVAL;
}

/**@} end of group LFS_APP_Functions */
/**@} end of group LFS_Application */
