/*!
 * @file        shell.c
 *
 * @brief       LittleFS Interactive Shell Implementation
 *
 * @version     V1.0.0
 *
 * @date        2026-08-16
 *
 * @attention
 *
 *  Copyright (C) 2020-2022 Geehy Semiconductor
 *
 *  You may not use this file except in compliance with the
 *  GEEHY COPYRIGHT NOTICE (GEEHY SOFTWARE PACKAGE LICENSE).
 *
 *  The program is only for reference, which is distributed in the hope
 *  that it will be useful and instructional for customers to develop
 *  their software. Unless required by applicable law or agreed to in
 *  writing, the program is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the GEEHY SOFTWARE PACKAGE LICENSE for the governing permissions
 *  and limitations under the License.
 */

/* Includes */
#include "shell.h"
#include "main.h"
#include <string.h>
#include <stdio.h>
#include "lfs_flash_int_port.h"
#include "lfs_spi_flash_port.h"

/** @addtogroup LFS_Shell
  @{
*/

/** @defgroup Shell_Variables
  @{
  */

static LFS_Storage_t g_storage;
static char g_cwd[SHELL_PATH_MAX_LEN] = "/";
static char g_cmd_buffer[SHELL_CMD_MAX_LEN];
static int g_cmd_pos = 0;
static volatile int g_cmd_ready = 0;  /* Flag: command line is ready to process */
static char g_last_char = 0;          /* Last received character */

/**@} end of group Shell_Variables */

/** @defgroup Shell_Path_Utils
  @{
  */

/**
 * @brief  Convert relative path to absolute path based on current working directory
 * @param  path: input path (relative or absolute)
 * @param  abs_path: output buffer for absolute path
 * @param  max_len: size of output buffer
 */
static void Shell_PathToAbsolute(const char *path, char *abs_path, int max_len)
{
    if (path[0] == '/')
    {
        /* Already absolute */
        strncpy(abs_path, path, max_len - 1);
        abs_path[max_len - 1] = '\0';
    }
    else
    {
        /* Relative path - prepend CWD */
        if (strcmp(g_cwd, "/") == 0)
        {
            snprintf(abs_path, max_len, "/%s", path);
        }
        else
        {
            snprintf(abs_path, max_len, "%s/%s", g_cwd, path);
        }
    }
}

/**
 * @brief  Normalize path (remove trailing slashes, handle . and ..)
 * @param  path: path to normalize (modified in place)
 */
static void Shell_NormalizePath(char *path)
{
    int len = strlen(path);

    /* Remove trailing slash (except for root) */
    while (len > 1 && path[len - 1] == '/')
    {
        path[--len] = '\0';
    }

    /* Handle empty path */
    if (len == 0)
    {
        path[0] = '/';
        path[1] = '\0';
    }
}

/**@} end of group Shell_Path_Utils */

/** @defgroup Shell_Commands
  @{
  */

static int cmd_help(int argc, char *argv[]);
static int cmd_ls(int argc, char *argv[]);
static int cmd_cd(int argc, char *argv[]);
static int cmd_pwd(int argc, char *argv[]);
static int cmd_mkdir(int argc, char *argv[]);
static int cmd_rmdir(int argc, char *argv[]);
static int cmd_touch(int argc, char *argv[]);
static int cmd_cat(int argc, char *argv[]);
static int cmd_echo(int argc, char *argv[]);
static int cmd_rm(int argc, char *argv[]);
static int cmd_mv(int argc, char *argv[]);
static int cmd_cp(int argc, char *argv[]);
static int cmd_stat(int argc, char *argv[]);
static int cmd_df(int argc, char *argv[]);
static int cmd_format(int argc, char *argv[]);

/**
 * @brief  Command table entry
 */
typedef struct
{
    const char *name;
    int (*handler)(int argc, char *argv[]);
    const char *help;
} shell_cmd_t;

static const shell_cmd_t g_commands[] =
{
    {"help",   cmd_help,   "Show available commands"},
    {"ls",     cmd_ls,     "List directory contents [path]"},
    {"cd",     cmd_cd,     "Change directory <path>"},
    {"pwd",    cmd_pwd,    "Print working directory"},
    {"mkdir",  cmd_mkdir,  "Create directory <path>"},
    {"rmdir",  cmd_rmdir,  "Remove directory <path>"},
    {"touch",  cmd_touch,  "Create empty file <file>"},
    {"cat",    cmd_cat,    "Display file content <file>"},
    {"echo",   cmd_echo,   "Write text: echo <text> > <file> (overwrite) or >> <file> (append)"},
    {"rm",     cmd_rm,     "Remove file <file>"},
    {"mv",     cmd_mv,     "Move/rename file <old> <new>"},
    {"cp",     cmd_cp,     "Copy file <src> <dst>"},
    {"stat",   cmd_stat,   "Show file information <file>"},
    {"df",     cmd_df,     "Show filesystem information"},
    {"format", cmd_format, "Format filesystem (WARNING: erases all data)"},
    {NULL, NULL, NULL}
};

/**@} end of group Shell_Commands */

/** @defgroup Shell_Cmd_Impl
  @{
  */

static int cmd_help(int argc, char *argv[])
{
    printf("Available commands:\n");

    for (int i = 0; g_commands[i].name != NULL; i++)
    {
        printf("  %-8s - %s\n", g_commands[i].name, g_commands[i].help);
    }

    return 0;
}

static int cmd_ls(int argc, char *argv[])
{
    char path[SHELL_PATH_MAX_LEN];

    if (argc > 1)
    {
        Shell_PathToAbsolute(argv[1], path, sizeof(path));
    }
    else
    {
        strncpy(path, g_cwd, sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';
    }

    Shell_NormalizePath(path);

    int err = LFS_App_ListDir(g_storage, path);

    if (err != LFS_ERR_OK)
    {
        printf("Error: cannot list '%s' (err=%d)\n", path, err);
        return err;
    }

    return 0;
}

static int cmd_cd(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: cd <path>\n");
        return -1;
    }

    char new_path[SHELL_PATH_MAX_LEN];

    if (strcmp(argv[1], "..") == 0)
    {
        /* Go to parent directory */
        strncpy(new_path, g_cwd, sizeof(new_path) - 1);
        new_path[sizeof(new_path) - 1] = '\0';

        /* Find last slash */
        char *last_slash = strrchr(new_path, '/');

        if (last_slash != NULL && last_slash != new_path)
        {
            *last_slash = '\0';
        }
        else
        {
            strcpy(new_path, "/");
        }
    }
    else if (strcmp(argv[1], ".") == 0)
    {
        /* Stay in current directory */
        return 0;
    }
    else
    {
        Shell_PathToAbsolute(argv[1], new_path, sizeof(new_path));
    }

    Shell_NormalizePath(new_path);

    /* Verify directory exists */
    if (!LFS_App_FileExists(g_storage, new_path))
    {
        printf("Error: directory '%s' does not exist\n", new_path);
        return -1;
    }

    strncpy(g_cwd, new_path, sizeof(g_cwd) - 1);
    g_cwd[sizeof(g_cwd) - 1] = '\0';

    return 0;
}

static int cmd_pwd(int argc, char *argv[])
{
    printf("%s\n", g_cwd);
    return 0;
}

static int cmd_mkdir(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: mkdir <path>\n");
        return -1;
    }

    char path[SHELL_PATH_MAX_LEN];
    Shell_PathToAbsolute(argv[1], path, sizeof(path));
    Shell_NormalizePath(path);

    int err = LFS_App_MakeDir(g_storage, path);

    if (err != LFS_ERR_OK)
    {
        printf("Error: cannot create directory '%s' (err=%d)\n", path, err);
        return err;
    }

    printf("Created directory: %s\n", path);
    return 0;
}

static int cmd_rmdir(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: rmdir <path>\n");
        return -1;
    }

    char path[SHELL_PATH_MAX_LEN];
    Shell_PathToAbsolute(argv[1], path, sizeof(path));
    Shell_NormalizePath(path);

    int err = LFS_App_RemoveDir(g_storage, path);

    if (err != LFS_ERR_OK)
    {
        printf("Error: cannot remove directory '%s' (err=%d)\n", path, err);
        return err;
    }

    printf("Removed directory: %s\n", path);
    return 0;
}

static int cmd_touch(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: touch <file>\n");
        return -1;
    }

    char path[SHELL_PATH_MAX_LEN];
    Shell_PathToAbsolute(argv[1], path, sizeof(path));
    Shell_NormalizePath(path);

    /* Check if file already exists */
    if (LFS_App_FileExists(g_storage, path))
    {
        printf("File already exists: %s\n", path);
        return 0;
    }

    /* Create empty file */
    int err = LFS_App_OpenFile(g_storage, path, LFS_MODE_WRITE);

    if (err != LFS_ERR_OK)
    {
        printf("Error: cannot create file '%s' (err=%d)\n", path, err);
        return err;
    }

    LFS_App_CloseFile(g_storage);
    printf("Created file: %s\n", path);
    return 0;
}

static int cmd_cat(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: cat <file>\n");
        return -1;
    }

    char path[SHELL_PATH_MAX_LEN];
    Shell_PathToAbsolute(argv[1], path, sizeof(path));
    Shell_NormalizePath(path);

    int err = LFS_App_OpenFile(g_storage, path, LFS_MODE_READ);

    if (err != LFS_ERR_OK)
    {
        printf("Error: cannot open file '%s' (err=%d)\n", path, err);
        return err;
    }

    char buffer[256];
    int bytes_read;

    while ((bytes_read = LFS_App_ReadFile(g_storage, buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
    }

    printf("\n");

    LFS_App_CloseFile(g_storage);
    return 0;
}

static int cmd_echo(int argc, char *argv[])
{
    /* echo <text> > <file>  (overwrite) */
    /* echo <text> >> <file> (append) */
    if (argc < 4)
    {
        printf("Usage: echo <text> > <file>\n");
        printf("       echo <text> >> <file> (append)\n");
        return -1;
    }

    /* Check for >> (append) or > (overwrite) */
    int append_mode = 0;
    int redirect_idx = -1;

    for (int i = 1; i < argc - 1; i++)
    {
        if (strcmp(argv[i], ">>") == 0)
        {
            append_mode = 1;
            redirect_idx = i;
            break;
        }
        else if (strcmp(argv[i], ">") == 0)
        {
            append_mode = 0;
            redirect_idx = i;
            break;
        }
    }

    if (redirect_idx < 0)
    {
        printf("Error: missing redirect operator (> or >>)\n");
        return -1;
    }

    /* Filename is after the redirect operator */
    if (redirect_idx + 1 >= argc)
    {
        printf("Error: missing filename\n");
        return -1;
    }

    /* Find the output file (last argument) */
    char path[SHELL_PATH_MAX_LEN];
    Shell_PathToAbsolute(argv[argc - 1], path, sizeof(path));
    Shell_NormalizePath(path);

    /* Build text from arguments (skip "echo", redirect operator, and filename) */
    char text[SHELL_CMD_MAX_LEN];
    text[0] = '\0';

    for (int i = 1; i < redirect_idx; i++)
    {
        if (i > 1)
        {
            strcat(text, " ");
        }

        strcat(text, argv[i]);
    }

    /* Open file in appropriate mode */
    LFS_FileMode_t mode;

    if (append_mode)
    {
        mode = LFS_MODE_APPEND;
    }
    else
    {
        mode = LFS_MODE_WRITE;
    }

    int err = LFS_App_OpenFile(g_storage, path, mode);

    if (err != LFS_ERR_OK)
    {
        printf("Error: cannot open file '%s' (err=%d)\n", path, err);
        return err;
    }

    LFS_App_WriteFile(g_storage, text, strlen(text));
    LFS_App_CloseFile(g_storage);

    printf("Written %d bytes to %s (%s)\n", strlen(text), path,
           append_mode ? "appended" : "overwritten");
    return 0;
}

static int cmd_rm(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: rm <file>\n");
        return -1;
    }

    char path[SHELL_PATH_MAX_LEN];
    Shell_PathToAbsolute(argv[1], path, sizeof(path));
    Shell_NormalizePath(path);

    int err = LFS_App_RemoveFile(g_storage, path);

    if (err != LFS_ERR_OK)
    {
        printf("Error: cannot remove file '%s' (err=%d)\n", path, err);
        return err;
    }

    printf("Removed: %s\n", path);
    return 0;
}

static int cmd_mv(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: mv <old> <new>\n");
        return -1;
    }

    char old_path[SHELL_PATH_MAX_LEN], new_path[SHELL_PATH_MAX_LEN];
    Shell_PathToAbsolute(argv[1], old_path, sizeof(old_path));
    Shell_PathToAbsolute(argv[2], new_path, sizeof(new_path));
    Shell_NormalizePath(old_path);
    Shell_NormalizePath(new_path);

    int err = LFS_App_Rename(g_storage, old_path, new_path);

    if (err != LFS_ERR_OK)
    {
        printf("Error: cannot rename '%s' to '%s' (err=%d)\n", old_path, new_path, err);
        return err;
    }

    printf("Renamed: %s -> %s\n", old_path, new_path);
    return 0;
}

static int cmd_cp(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: cp <src> <dst>\n");
        return -1;
    }

    char src_path[SHELL_PATH_MAX_LEN], dst_path[SHELL_PATH_MAX_LEN];
    Shell_PathToAbsolute(argv[1], src_path, sizeof(src_path));
    Shell_PathToAbsolute(argv[2], dst_path, sizeof(dst_path));
    Shell_NormalizePath(src_path);
    Shell_NormalizePath(dst_path);

    /* Get source file size */
    uint32_t src_size;
    int err = LFS_App_GetFileSize(g_storage, src_path, &src_size);

    if (err != LFS_ERR_OK)
    {
        printf("Error: cannot stat source file '%s' (err=%d)\n", src_path, err);
        return err;
    }

    /* Check if file is too large for buffer */
#define CP_BUFFER_SIZE 1024

    if (src_size > CP_BUFFER_SIZE)
    {
        printf("Error: file too large for copy (max %d bytes)\n", CP_BUFFER_SIZE);
        return -1;
    }

    /* Read source file into buffer */
    static uint8_t cp_buffer[CP_BUFFER_SIZE];

    err = LFS_App_OpenFile(g_storage, src_path, LFS_MODE_READ);

    if (err != LFS_ERR_OK)
    {
        printf("Error: cannot open source file '%s' (err=%d)\n", src_path, err);
        return err;
    }

    int bytes_read = LFS_App_ReadFile(g_storage, cp_buffer, src_size);
    LFS_App_CloseFile(g_storage);

    if (bytes_read < 0)
    {
        printf("Error: read failed (err=%d)\n", bytes_read);
        return bytes_read;
    }

    /* Write to destination file */
    err = LFS_App_OpenFile(g_storage, dst_path, LFS_MODE_WRITE);

    if (err != LFS_ERR_OK)
    {
        printf("Error: cannot create destination file '%s' (err=%d)\n", dst_path, err);
        return err;
    }

    int bytes_written = LFS_App_WriteFile(g_storage, cp_buffer, bytes_read);
    LFS_App_CloseFile(g_storage);

    if (bytes_written < 0)
    {
        printf("Error: write failed (err=%d)\n", bytes_written);
        return bytes_written;
    }

    printf("Copied %d bytes: %s -> %s\n", bytes_written, src_path, dst_path);
    return 0;
}

static int cmd_stat(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: stat <file>\n");
        return -1;
    }

    char path[SHELL_PATH_MAX_LEN];
    Shell_PathToAbsolute(argv[1], path, sizeof(path));
    Shell_NormalizePath(path);

    uint32_t size;
    int err = LFS_App_GetFileSize(g_storage, path, &size);

    if (err != LFS_ERR_OK)
    {
        printf("Error: cannot stat '%s' (err=%d)\n", path, err);
        return err;
    }

    printf("File: %s\n", path);
    printf("Size: %lu bytes\n", size);
    printf("Type: Regular File\n");

    return 0;
}

static int cmd_df(int argc, char *argv[])
{
    struct lfs_fsinfo info;
    int err = LFS_App_GetFSInfo(g_storage, &info);

    if (err != LFS_ERR_OK)
    {
        printf("Error: cannot get filesystem info (err=%d)\n", err);
        return err;
    }

    int32_t used_blocks = LFS_App_GetFSSize(g_storage);

    printf("Filesystem Information:\n");
    printf("  Block Size:  %lu bytes\n", info.block_size);
    printf("  Block Count: %lu\n", info.block_count);
    printf("  Used Blocks: %ld (%.1f%%)\n", used_blocks,
           (float)used_blocks / info.block_count * 100.0f);
    printf("  Free Blocks: %ld\n", info.block_count - used_blocks);
    printf("  Version:     %lu.%lu\n",
           (info.disk_version >> 16) & 0xFFFF, info.disk_version & 0xFFFF);

    return 0;
}

static int cmd_format(int argc, char *argv[])
{
    printf("WARNING: This will erase all data on the filesystem!\n");
    printf("Formatting...\n");

    /* Unmount first */
    LFS_App_Unmount(g_storage);

    int err = LFS_App_Format(g_storage);

    if (err != LFS_ERR_OK)
    {
        printf("Error: format failed (err=%d)\n", err);
        /* Try to remount */
        LFS_App_Mount(g_storage);
        return err;
    }

    /* Remount */
    err = LFS_App_Mount(g_storage);

    if (err != LFS_ERR_OK)
    {
        printf("Warning: format succeeded but remount failed (err=%d)\n", err);
        return err;
    }

    /* Reset CWD to root */
    strcpy(g_cwd, "/");

    printf("Format complete. Filesystem mounted.\n");
    return 0;
}

/**@} end of group Shell_Cmd_Impl */

/** @defgroup Shell_Core
  @{
  */

/**
 * @brief  Parse command line into argc/argv
 * @param  line: input command line
 * @param  argv: output argument array
 * @retval number of arguments
 */
static int Shell_ParseLine(const char *line, char *argv[])
{
    static char line_buf[SHELL_CMD_MAX_LEN];
    strncpy(line_buf, line, sizeof(line_buf) - 1);
    line_buf[sizeof(line_buf) - 1] = '\0';

    int argc = 0;
    char *p = line_buf;

    while (*p != '\0' && argc < SHELL_MAX_ARGS)
    {
        /* Skip whitespace */
        while (*p == ' ' || *p == '\t')
        {
            p++;
        }

        if (*p == '\0')
        {
            break;
        }

        /* Check for quoted string */
        if (*p == '"')
        {
            p++;
            argv[argc++] = p;

            while (*p != '\0' && *p != '"')
            {
                p++;
            }

            if (*p == '"')
            {
                *p++ = '\0';
            }
        }
        else
        {
            argv[argc++] = p;

            while (*p != '\0' && *p != ' ' && *p != '\t')
            {
                p++;
            }

            if (*p != '\0')
            {
                *p++ = '\0';
            }
        }
    }

    return argc;
}

int Shell_Init(LFS_Storage_t storage)
{
    g_storage = storage;
    strcpy(g_cwd, "/");
    g_cmd_pos = 0;

    int err = LFS_App_Init(storage);

    if (err != LFS_ERR_OK)
    {
        printf("Error: failed to initialize filesystem (err=%d)\n", err);
        return err;
    }

    /* Check if flash is empty (first run) by reading first bytes */
    uint8_t check_buf[16];
    int is_empty = 1;

    if (storage == LFS_STORAGE_INTERNAL)
    {
        /* Internal flash: direct memory access */
        uint32_t addr = LFS_INT_FLASH_BASE_ADDR;

        for (int i = 0; i < sizeof(check_buf); i++)
        {
            check_buf[i] = *(volatile uint8_t *)(addr + i);
        }
    }
    else
    {
        /* External flash: use LittleFS read function */
        const struct lfs_config *cfg = LFS_SPIFlash_GetConfig();

        if (cfg != NULL && cfg->read != NULL)
        {
            /* Read first block to check if empty */
            err = cfg->read(cfg, 0, 0, check_buf, sizeof(check_buf));

            if (err != LFS_ERR_OK)
            {
                /* Read failed, assume empty */
                memset(check_buf, 0xFF, sizeof(check_buf));
            }
        }
        else
        {
            /* No config, assume empty */
            memset(check_buf, 0xFF, sizeof(check_buf));
        }
    }

    for (int i = 0; i < sizeof(check_buf); i++)
    {
        if (check_buf[i] != 0xFF)
        {
            is_empty = 0;
            break;
        }
    }

    if (is_empty)
    {
        /* Flash is empty, format first to avoid mount error */
        printf("Initializing filesystem...\n");
        err = LFS_App_Format(storage);

        if (err != LFS_ERR_OK)
        {
            printf("Error: failed to format filesystem (err=%d)\n", err);
            return err;
        }
    }

    /* Mount the filesystem */
    err = LFS_App_Mount(storage);

    if (err != LFS_ERR_OK)
    {
        printf("Error: failed to mount filesystem (err=%d)\n", err);
        return err;
    }

    if (is_empty)
    {
        printf("Filesystem created and mounted.\n");
    }

    return 0;
}

void Shell_ProcessLine(const char *line)
{
    char *argv[SHELL_MAX_ARGS];
    int argc;

    /* Skip empty lines */
    if (strlen(line) == 0)
    {
        return;
    }

    argc = Shell_ParseLine(line, argv);

    if (argc == 0)
    {
        return;
    }

    /* Find and execute command */
    for (int i = 0; g_commands[i].name != NULL; i++)
    {
        if (strcmp(argv[0], g_commands[i].name) == 0)
        {
            g_commands[i].handler(argc, argv);
            return;
        }
    }

    printf("Unknown command: %s\n", argv[0]);
    printf("Type 'help' for available commands\n");
}

/**
 * @brief  Feed one character to shell (call from USART RX interrupt)
 * @param  ch: received character
 * @note   This function only buffers the character and sets flag.
 *         Call Shell_Task() in main loop to process input.
 */
void Shell_PutChar(char ch)
{
    g_last_char = ch;
    g_cmd_ready = 1;
}

/**
 * @brief  Check if a command is ready to process
 * @retval 1 if command ready, 0 otherwise
 */
int Shell_IsReady(void)
{
    return g_cmd_ready;
}

/**
 * @brief  Get the last received character
 * @retval last character
 */
char Shell_GetChar(void)
{
    g_cmd_ready = 0;
    return g_last_char;
}

void Shell_PrintPrompt(void)
{
    printf("%s%s", SHELL_PROMPT, g_cwd);

    if (strcmp(g_cwd, "/") != 0)
    {
        printf("/");
    }

    printf(" $ ");
    fflush(stdout);
}

/**
 * @brief  Process one character (handle echo and buffering)
 * @param  ch: character to process
 */
static void Shell_ProcessChar(char ch)
{
    if (ch == '\r' || ch == '\n')
    {
        /* End of line - process command */
        printf("\n");
        g_cmd_buffer[g_cmd_pos] = '\0';
        Shell_ProcessLine(g_cmd_buffer);
        g_cmd_pos = 0;
        Shell_PrintPrompt();
    }
    else if (ch == '\b' || ch == 127)
    {
        /* Backspace */
        if (g_cmd_pos > 0)
        {
            g_cmd_pos--;
            printf("\b \b");
        }
    }
    else if (g_cmd_pos < SHELL_CMD_MAX_LEN - 1)
    {
        /* Regular character */
        g_cmd_buffer[g_cmd_pos++] = ch;
        printf("%c", ch);  /* Echo */
    }
}

/**
 * @brief  Shell task - call in main loop to process input
 * @note   This function handles character echo and command processing
 */
void Shell_Task(void)
{
    if (Shell_IsReady())
    {
        char ch = Shell_GetChar();
        Shell_ProcessChar(ch);
    }
}

/**@} end of group Shell_Core */

/**@} end of group LFS_Shell */
