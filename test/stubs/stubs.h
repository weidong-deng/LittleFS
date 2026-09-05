/**
 * @file stubs.h
 * @brief Stub function declarations for host testing
 */

#ifndef STUBS_H
#define STUBS_H

#include "lfs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Internal flash stub functions */
const struct lfs_config* LFS_Int_GetConfig(void);
void LFS_Int_CleanupStub(void);

/* SPI flash stub functions */
const struct lfs_config* LFS_SPIFlash_GetConfig(void);
void LFS_SPIFlash_RegisterStub(void);
void LFS_SPIFlash_UnregisterStub(void);
void LFS_SPIFlash_CleanupStub(void);

#ifdef __cplusplus
}
#endif

#endif /* STUBS_H */
