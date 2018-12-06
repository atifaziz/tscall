/* Copyright (C) 2018 Atif Aziz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#if defined(_WIN64) || defined(_WIN32) && !defined(WINDOWS)
#define WINDOWS
#endif

#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <process.h>
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#define DIM(x) (sizeof(x) / sizeof((x)[0]))

#define print_op_error(op) \
    fprintf(stderr, "Operation '%s' failed due to:\n%s\nat: %s:%d\n", (op), strerror(errno), __FILE__, __LINE__)

#define print_app_error(msg) \
    fprintf(stderr, "%s\nat: %s:%d\n", (msg), __FILE__, __LINE__)

#define printf_app_error(msg, ...) \
    fprintf(stderr, msg "\nat: %s:%d\n", __VA_ARGS__, __FILE__, __LINE__)

int copy_file(const char *source_path, const char *out);

int main(int argc, char **argv)
{
    if (argc == 1) {
        puts("Missing executable file specification.");
        return 1;
    }

    struct stat finfo;
    if (stat(argv[1], &finfo)) {
        print_op_error("stat");
        return 1;
    }

    struct tm *tm = gmtime(&finfo.st_mtime);
    if (!tm) {
        print_app_error("Invalid time!");
        return 1;
    }

    char fname[FILENAME_MAX] = { 0 };
    char ext[FILENAME_MAX] = { 0 };
    strncpy(fname, argv[1], DIM(fname) - 1);
    char *dot = strrchr(fname, '.');
    if (dot)
    {
        strncpy(ext, dot, DIM(ext) - 1);
        *dot = 0;
    }

    char tsname[FILENAME_MAX];
    if (snprintf(tsname, DIM(tsname), "%s-%04d%02d%02d%02d%02d%02d%s",
        fname,
        1900 + tm->tm_year, 1 + tm->tm_mon, tm->tm_mday,
        tm->tm_hour, tm->tm_min, tm->tm_sec,
        ext) >= DIM(tsname)) {
            print_app_error("Timestamp is too long!");
            return 1;
    }

    int missing = 0;
    struct stat tsfinfo;
    if (stat(tsname, &tsfinfo) && !(missing = errno == ENOENT)) {
        print_op_error("stat");
        return 1;
    }

    if (missing || finfo.st_size != tsfinfo.st_size) {
        if (!missing && (tsfinfo.st_mode & _S_IFMT) != _S_IFREG) {
            printf_app_error("Cannot refresh \"%s\" as it is not a file.", tsname);
            return 1;
        }
        if (!copy_file(argv[1], tsname)) {
            print_op_error("copy_file");
            return 1;
        }
        #ifdef WINDOWS
        DWORD attribs = GetFileAttributes(tsname);
        if (INVALID_FILE_ATTRIBUTES != attribs) {
            SetFileAttributes(tsname, attribs | FILE_ATTRIBUTE_HIDDEN);
        }
        #endif
    }

    argv[1] = tsname;
    intptr_t result = _spawnv(_P_WAIT, tsname, argv + 1);
    if (result == -1) {
        printf_app_error("Error launching: %s\nReason: %s", tsname, strerror(errno));
        return 1;
    }

    return result;
}

int copy_file(const char *src_path, const char *dest_path)
{
    int src_file = open(src_path,  O_RDONLY);
    if (-1 == src_file)
        return 0;

    int success = 0;

    struct stat src_stat;
    if (fstat(src_file, &src_stat))
        goto bail;

    int dest_file = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, src_stat.st_mode & 0777);
    if (-1 == dest_file)
        goto bail;

    int len;
    char *buffer[4096];
    while ((len = read(src_file, buffer, sizeof(buffer))) > 0) {
        if (-1 == write(dest_file, buffer, len))
            goto bail;
    }

    success = len ? 0 : 1; /* last read should be 0 */

bail:

    if (src_file != -1)
        close(src_file);
    if (dest_file != -1)
        close(dest_file);

    return success;
}
