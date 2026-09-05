/*!
 * @file        lfs_app.h
 *
 * @brief       LittleFS Application Layer Header
 *
 * @version     V1.0.0
 *
 * @date        2026-08-07
 *
 * @attention
 *
 *  LittleFS application layer for easy file system operations.
 *  Supports internal flash and any external SPI flash via driver registration.
 */

#ifndef __LFS_APP_H
#define __LFS_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes */
#include "lfs.h"
#include <stdint.h>

/** @addtogroup LFS_Application
  @{
*/

/** @defgroup LFS_APP_Config
  * @{
  */

/* Storage device selection */
typedef enum {
    LFS_STORAGE_INTERNAL = 0,  /* Internal flash */
    LFS_STORAGE_EXTERNAL = 1   /* External SPI flash (any chip via driver) */
} LFS_Storage_t;

/* File open modes */
typedef enum {
    LFS_MODE_READ = 0,
    LFS_MODE_WRITE,
    LFS_MODE_APPEND
} LFS_FileMode_t;

/**@} end of group LFS_APP_Config */

/** @defgroup LFS_APP_Exported_Functions
  * @{
  */

/* Initialization */
int LFS_App_Init(LFS_Storage_t storage);
int LFS_App_Format(LFS_Storage_t storage);

/* Mount/Unmount */
int LFS_App_Mount(LFS_Storage_t storage);
int LFS_App_Unmount(LFS_Storage_t storage);

void LFS_App_ResetState(void);

/* File Operations */
int LFS_App_OpenFile(LFS_Storage_t storage, const char *path, LFS_FileMode_t mode);
int LFS_App_CloseFile(LFS_Storage_t storage);
int LFS_App_ReadFile(LFS_Storage_t storage, void *buffer, uint32_t size);
int LFS_App_WriteFile(LFS_Storage_t storage, const void *buffer, uint32_t size);
int LFS_App_SeekFile(LFS_Storage_t storage, int32_t offset, int whence);
int32_t LFS_App_TellFile(LFS_Storage_t storage);
int LFS_App_SyncFile(LFS_Storage_t storage);
int32_t LFS_App_GetCurrentFileSize(LFS_Storage_t storage);
int LFS_App_RewindFile(LFS_Storage_t storage);
int LFS_App_TruncateFile(LFS_Storage_t storage, uint32_t size);
int LFS_App_RemoveFile(LFS_Storage_t storage, const char *path);
int LFS_App_Rename(LFS_Storage_t storage, const char *oldpath, const char *newpath);

/* Directory Operations */
int LFS_App_MakeDir(LFS_Storage_t storage, const char *path);
int LFS_App_RemoveDir(LFS_Storage_t storage, const char *path);
int LFS_App_ListDir(LFS_Storage_t storage, const char *path);

/* File Info */
int LFS_App_FileExists(LFS_Storage_t storage, const char *path);
int LFS_App_GetFileSize(LFS_Storage_t storage, const char *path, uint32_t *size);

/* File System Info */
int LFS_App_GetFSInfo(LFS_Storage_t storage, struct lfs_fsinfo *info);
int32_t LFS_App_GetFSSize(LFS_Storage_t storage);
int LFS_App_FSGC(LFS_Storage_t storage);

/**@} end of group LFS_APP_Exported_Functions */

/**@} end of group LFS_Application */

#ifdef __cplusplus
}
#endif

#endif /* __LFS_APP_H */
