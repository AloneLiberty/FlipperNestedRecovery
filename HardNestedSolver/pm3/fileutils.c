//-----------------------------------------------------------------------------
// Copyright (C) Proxmark3 contributors. See AUTHORS.md for details.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// See LICENSE.txt for the text of the license.
//-----------------------------------------------------------------------------

// this define is needed for scandir/alphasort to work
#define _GNU_SOURCE
#include "fileutils.h"

//#include <dirent.h>
#include <ctype.h>

#include "pm3_cmd.h"
#include "commonutil.h"
#include "util.h"
//#include <unistd.h>

#ifdef _WIN32
//#include "scandir.h"
#include <sys/stat.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
typedef int (*filter_func_t)(const struct dirent *);

struct dirent {
    char d_name[1024];
};

int alphasort(const struct dirent **a, const struct dirent **b) {
    return stricmp((*a)->d_name, (*b)->d_name);
}

int scandir(const char *path, struct dirent ***namelist, filter_func_t filter, int (*compar)(const struct dirent **, const struct dirent **)) {
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
    char search_path[1024];
    int count = 0;
    
    strcpy(search_path, path);
    strcat(search_path, "\\*");
    
    hFind = FindFirstFile(search_path, &FindFileData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        return -1;
    }
    
    do {
        if (filter != NULL && filter(&FindFileData) == 0) {
            continue;
        }
        
        count++;
    } while (FindNextFile(hFind, &FindFileData) != 0);
    
    FindClose(hFind);
    
    *namelist = (struct dirent **)malloc(count * sizeof(struct dirent *));
    
    if (*namelist == NULL) {
        return -1;
    }
    
    count = 0;
    
    hFind = FindFirstFile(search_path, &FindFileData);
    
    do {
        if (filter != NULL && filter(&FindFileData) == 0) {
            continue;
        }
        
        struct dirent *entry = (struct dirent *)malloc(sizeof(struct dirent));
        
        if (entry == NULL) {
            return -1;
        }
        
        strcpy(entry->d_name, FindFileData.cFileName);
        
        (*namelist)[count++] = entry;
    } while (FindNextFile(hFind, &FindFileData) != 0);
    
    FindClose(hFind);
    
    if (compar != NULL) {
        qsort(*namelist, count, sizeof(struct dirent *), compar);
    }
    
    return count;
}
#include <direct.h>
#else
#include <dirent.h>
#endif

#ifdef __APPLE__
#include <mach-o/dyld.h> /* for _NSGetExecutablePath */
#elif defined(__linux__)
#include <linux/limits.h> /* for PATH_MAX */
#elif defined(_WIN32)
#include <windows.h> /* for GetModuleFileName */
#endif

char *getExecutablePath(char *buffer, size_t size) {
    if (buffer == NULL) {
        return NULL;
    }

#ifdef __APPLE__
    if (_NSGetExecutablePath(buffer, &size) == 0) {
        return buffer;
    }
#elif defined(__linux__)
    ssize_t count = readlink("/proc/self/exe", buffer, size - 1);
    if (count != -1) {
        buffer[count] = '\0';
        return buffer;
    }
#elif defined(_WIN32)
    DWORD length = GetModuleFileName(NULL, buffer, size);
    if (length > 0 && length < size) {
        return buffer;
    }
#endif

    return NULL;
}

#define PATH_MAX_LENGTH 200


#ifndef getcwd
#ifdef _MSC_VER
#include <direct.h>
#define getcwd(buffer, size) GetCurrentDirectoryA(size, buffer)
#endif
#ifdef __APPLE__
#include <unistd.h>
#endif
#endif

#define GetCurrentDir getcwd
/**
 * @brief detects if file is of a supported filetype based on extension
 * @param filename
 * @return o
 */
DumpFileType_t getfiletype(const char *filename) {
    // assume unknown file is BINARY
    DumpFileType_t o = BIN;
    if (filename == NULL) {
        return o;
    }

    size_t len = strlen(filename);
    if (len > 4) {
        //  check if valid file extension and attempt to load data
        char s[FILE_PATH_SIZE];
        memset(s, 0, sizeof(s));
        memcpy(s, filename, len);
        str_lower(s);

        if (str_endswith(s, "bin")) {
            o = BIN;
        } else if (str_endswith(s, "eml")) {
            o = EML;
        } else if (str_endswith(s, "json")) {
            o = JSON;
        } else if (str_endswith(s, "dic")) {
            o = DICTIONARY;
        } else {
            // mfd, trc, trace is binary
            o = BIN;
            // log is text
            // .pm3 is text values of signal data
        }
    }
    return o;
}

/**
 * @brief checks if a file exists
 * @param filename
 * @return
 */
int fileExists(const char *filename) {

#ifdef _WIN32
    struct _stat st;
    int result = _stat(filename, &st);
#else
    struct stat st;
    int result = stat(filename, &st);
#endif
    return result == 0;
}

/**
 * @brief checks if path is directory.
 * @param filename
 * @return
 */
static bool is_directory(const char *filename) {
#ifdef _WIN32
    struct _stat st;
    if (_stat(filename, &st) == -1)
        return false;
#else
    struct stat st;
//    stat(filename, &st);
    if (lstat(filename, &st) == -1)
        return false;
#endif
}

bool setDefaultPath(savePaths_t pathIndex, const char *path) {

    if (pathIndex < spItemCount) {

        if ((path == NULL) && (g_session.defaultPaths[pathIndex] != NULL)) {
            free(g_session.defaultPaths[pathIndex]);
            g_session.defaultPaths[pathIndex] = NULL;
        }

        if (path == NULL) {
            return false;
        }

        size_t len = strlen(path);

        g_session.defaultPaths[pathIndex] = (char *)realloc(g_session.defaultPaths[pathIndex], len + 1);
        strcpy(g_session.defaultPaths[pathIndex], path);
        return true;
    }
    return false;
}

static char *filenamemcopy(const char *preferredName, const char *suffix) {
    if (preferredName == NULL) return NULL;
    if (suffix == NULL) return NULL;

    char *fileName = (char *) calloc(strlen(preferredName) + strlen(suffix) + 1, sizeof(uint8_t));
    if (fileName == NULL) {
        return NULL;
    }

    strcpy(fileName, preferredName);
    if (str_endswith(fileName, suffix)) {
        return fileName;
    }

    strcat(fileName, suffix);
    return fileName;
}

static size_t path_size(savePaths_t a) {
    if (a == spItemCount) {
        return 0;
    }
    return strlen(g_session.defaultPaths[a]);
}

char *newfilenamemcopyEx(const char *preferredName, const char *suffix, savePaths_t e_save_path) {
    if (preferredName == NULL || suffix == NULL) {
        return NULL;
    }

    uint16_t p_namelen = strlen(preferredName);
    if (str_endswith(preferredName, suffix)) {
        p_namelen -= strlen(suffix);
    }

    int save_path_len = path_size(e_save_path);

    // 1: null terminator
    // 16: room for filenum to ensure new filename
    // save_path_len + strlen(PATHSEP):  the user preference save paths
    const size_t len = p_namelen + strlen(suffix) + 1 + 16 + save_path_len + strlen(PATHSEP);

    char *fileName = (char *) calloc(len, sizeof(uint8_t));
    if (fileName == NULL) {
        return NULL;
    }

    char *pfn = fileName;

    // user preference save paths
    if (save_path_len) {
        snprintf(pfn, save_path_len + strlen(PATHSEP) + 1, "%s%s", g_session.defaultPaths[e_save_path], PATHSEP);
        pfn += save_path_len + strlen(PATHSEP);
    }

    int num = 1;

    // modify filename
    snprintf(pfn, len, "%.*s%s", p_namelen, preferredName, suffix);

    // check complete path/filename if exists
    while (fileExists(fileName)) {
        // modify filename
        snprintf(pfn, len, "%.*s-%03d%s", p_namelen, preferredName, num, suffix);
        num++;
    }

    return fileName;
}

char *newfilenamemcopy(const char *preferredName, const char *suffix) {
    return newfilenamemcopyEx(preferredName, suffix, spDefault);
}

// --------- SAVE FILES
int saveFile(const char *preferredName, const char *suffix, const void *data, size_t datalen) {

    if (data == NULL || datalen == 0) {
        return PM3_EINVARG;
    }

    char *fileName = newfilenamemcopy(preferredName, suffix);
    if (fileName == NULL) {
        return PM3_EMALLOC;
    }

    // We should have a valid filename now, e.g. dumpdata-3.bin

    // Opening file for writing in binary mode
    FILE *f = fopen(fileName, "wb");
    if (!f) {
        PrintAndLogEx(WARNING, "file not found or locked `" _YELLOW_("%s") "`", fileName);
        free(fileName);
        return PM3_EFILE;
    }
    fwrite(data, 1, datalen, f);
    fflush(f);
    fclose(f);
    PrintAndLogEx(SUCCESS, "saved " _YELLOW_("%zu") " bytes to binary file " _YELLOW_("%s"), datalen, fileName);
    free(fileName);
    return PM3_SUCCESS;
}

// dump file
int saveFileEML(const char *preferredName, uint8_t *data, size_t datalen, size_t blocksize) {

    if (data == NULL || datalen == 0) {
        return PM3_EINVARG;
    }

    char *fileName = newfilenamemcopyEx(preferredName, ".eml", spDump);
    if (fileName == NULL) {
        return PM3_EMALLOC;
    }

    int retval = PM3_SUCCESS;
    int blocks = datalen / blocksize;
    uint16_t currblock = 1;

    // We should have a valid filename now, e.g. dumpdata-3.bin

    // Opening file for writing in text mode
    FILE *f = fopen(fileName, "w+");
    if (!f) {
        PrintAndLogEx(WARNING, "file not found or locked `" _YELLOW_("%s") "`", fileName);
        retval = PM3_EFILE;
        goto out;
    }

    for (size_t i = 0; i < datalen; i++) {
        fprintf(f, "%02X", data[i]);

        // no extra line in the end
        if ((i + 1) % blocksize == 0 && currblock != blocks) {
            fprintf(f, "\n");
            currblock++;
        }
    }
    // left overs
    if (datalen % blocksize != 0) {
        int index = blocks * blocksize;
        for (size_t j = 0; j < datalen % blocksize; j++) {
            fprintf(f, "%02X", data[index + j]);
        }
    }
    fflush(f);
    fclose(f);
    PrintAndLogEx(SUCCESS, "saved " _YELLOW_("%" PRId32) " blocks to text file " _YELLOW_("%s"), blocks, fileName);

    out:
    free(fileName);
    return retval;
}

// wave file of trace,

// Signal trace file, PM3
int saveFilePM3(const char *preferredName, int *data, size_t datalen) {

    if (data == NULL || datalen == 0) {
        return PM3_EINVARG;
    }

    char *fileName = newfilenamemcopyEx(preferredName, ".pm3", spTrace);
    if (fileName == NULL) {
        return PM3_EMALLOC;
    }

    int retval = PM3_SUCCESS;

    FILE *f = fopen(fileName, "w");
    if (!f) {
        PrintAndLogEx(WARNING, "file not found or locked `" _YELLOW_("%s") "`", fileName);
        retval = PM3_EFILE;
        goto out;
    }

    for (uint32_t i = 0; i < datalen; i++) {
        fprintf(f, "%d\n", data[i]);
    }

    fflush(f);
    fclose(f);
    PrintAndLogEx(SUCCESS, "saved " _YELLOW_("%zu") " bytes to PM3 file " _YELLOW_("'%s'"), datalen, fileName);

    out:
    free(fileName);
    return retval;
}

// key file dump
int createMfcKeyDump(const char *preferredName, uint8_t sectorsCnt, sector_t *e_sector) {

    if (e_sector == NULL) return PM3_EINVARG;

    char *fileName = newfilenamemcopyEx(preferredName, ".bin", spDump);
    if (fileName == NULL) return PM3_EMALLOC;

    FILE *f = fopen(fileName, "wb");
    if (f == NULL) {
        PrintAndLogEx(WARNING, "Could not create file " _YELLOW_("%s"), fileName);
        free(fileName);
        return PM3_EFILE;
    }
    PrintAndLogEx(SUCCESS, "Generating binary key file");

    uint8_t empty[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t tmp[6] = {0, 0, 0, 0, 0, 0};

    for (int i = 0; i < sectorsCnt; i++) {
        if (e_sector[i].foundKey[0])
            num_to_bytes(e_sector[i].Key[0], sizeof(tmp), tmp);
        else
            memcpy(tmp, empty, sizeof(tmp));
        fwrite(tmp, 1, sizeof(tmp), f);
    }

    for (int i = 0; i < sectorsCnt; i++) {
        if (e_sector[i].foundKey[0])
            num_to_bytes(e_sector[i].Key[1], sizeof(tmp), tmp);
        else
            memcpy(tmp, empty, sizeof(tmp));
        fwrite(tmp, 1, sizeof(tmp), f);
    }

    fflush(f);
    fclose(f);
    PrintAndLogEx(SUCCESS, "Found keys have been dumped to " _YELLOW_("%s"), fileName);
    PrintAndLogEx(INFO, "FYI! --> " _YELLOW_("0xFFFFFFFFFFFF") " <-- has been inserted for unknown keys where " _YELLOW_("res") " is " _YELLOW_("0"));
    free(fileName);
    return PM3_SUCCESS;
}

// --------- LOAD FILES
int loadFile_safe(const char *preferredName, const char *suffix, void **pdata, size_t *datalen) {
    return loadFile_safeEx(preferredName, suffix, pdata, datalen, true);
}
int loadFile_safeEx(const char *preferredName, const char *suffix, void **pdata, size_t *datalen, bool verbose) {

    char *path;
    int res = searchFile(&path, RESOURCES_SUBDIR, preferredName, suffix, false);
    if (res != PM3_SUCCESS) {
        return PM3_EFILE;
    }

    FILE *f = fopen(path, "rb");
    if (!f) {
        PrintAndLogEx(WARNING, "file not found or locked `" _YELLOW_("%s") "`", path);
        free(path);
        return PM3_EFILE;
    }
    free(path);

    // get filesize in order to malloc memory
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize <= 0) {
        PrintAndLogEx(FAILED, "error, when getting filesize");
        fclose(f);
        return PM3_EFILE;
    }

    *pdata = calloc(fsize, sizeof(uint8_t));
    if (!*pdata) {
        PrintAndLogEx(FAILED, "error, cannot allocate memory");
        fclose(f);
        return PM3_EMALLOC;
    }

    size_t bytes_read = fread(*pdata, 1, fsize, f);

    fclose(f);

    if (bytes_read != fsize) {
        PrintAndLogEx(FAILED, "error, bytes read mismatch file size");
        free(*pdata);
        return PM3_EFILE;
    }

    *datalen = bytes_read;

    if (verbose) {
        PrintAndLogEx(SUCCESS, "loaded " _YELLOW_("%zu") " bytes from binary file " _YELLOW_("%s"), bytes_read, preferredName);
    }
    return PM3_SUCCESS;
}

int loadFileEML_safe(const char *preferredName, void **pdata, size_t *datalen) {
    char *path;
    int res = searchFile(&path, RESOURCES_SUBDIR, preferredName, "", false);
    if (res != PM3_SUCCESS) {
        return PM3_EFILE;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        PrintAndLogEx(WARNING, "file not found or locked `" _YELLOW_("%s") "`", path);
        free(path);
        return PM3_EFILE;
    }
    free(path);

    // get filesize in order to malloc memory
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize <= 0) {
        PrintAndLogEx(FAILED, "error, when getting filesize");
        fclose(f);
        return PM3_EFILE;
    }

    *pdata = calloc(fsize, sizeof(uint8_t));
    if (!*pdata) {
        PrintAndLogEx(FAILED, "error, cannot allocate memory");
        fclose(f);
        return PM3_EMALLOC;
    }

    // 128 + 2 newline chars + 1 null terminator
    char line[131];
    memset(line, 0, sizeof(line));
    uint8_t buf[64] = {0x00};
    size_t counter = 0;
    int retval = PM3_SUCCESS, hexlen = 0;

    uint8_t *tmp = (uint8_t *)*pdata;

    while (!feof(f)) {

        memset(line, 0, sizeof(line));

        if (fgets(line, sizeof(line), f) == NULL) {
            if (feof(f))
                break;

            fclose(f);
            PrintAndLogEx(FAILED, "File reading error.");
            return PM3_EFILE;
        }

        if (line[0] == '#')
            continue;

        strcleanrn(line, sizeof(line));

        res = param_gethex_to_eol(line, 0, buf, sizeof(buf), &hexlen);
        if (res == 0) {
            memcpy(tmp + counter, buf, hexlen);
            counter += hexlen;
        } else {
            retval = PM3_ESOFT;
        }
    }
    fclose(f);
    PrintAndLogEx(SUCCESS, "loaded " _YELLOW_("%zu") " bytes from text file " _YELLOW_("%s"), counter, preferredName);


    uint8_t *newdump = realloc(*pdata, counter);
    if (newdump == NULL) {
        free(*pdata);
        return PM3_EMALLOC;
    } else {
        *pdata = newdump;
    }

    if (datalen)
        *datalen = counter;

    return retval;
}

int loadFileMCT_safe(const char *preferredName, void **pdata, size_t *datalen) {
    char *path;
    int res = searchFile(&path, RESOURCES_SUBDIR, preferredName, "", false);
    if (res != PM3_SUCCESS) {
        return PM3_EFILE;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        PrintAndLogEx(WARNING, "file not found or locked `" _YELLOW_("%s") "`", path);
        free(path);
        return PM3_EFILE;
    }
    free(path);

    // get filesize in order to malloc memory
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize <= 0) {
        PrintAndLogEx(FAILED, "error, when getting filesize");
        fclose(f);
        return PM3_EFILE;
    }

    *pdata = calloc(fsize, sizeof(uint8_t));
    if (!*pdata) {
        PrintAndLogEx(FAILED, "error, cannot allocate memory");
        fclose(f);
        return PM3_EMALLOC;
    }

    // 128 + 2 newline chars + 1 null terminator
    char line[131];
    memset(line, 0, sizeof(line));
    uint8_t buf[64] = {0x00};
    size_t counter = 0;
    int retval = PM3_SUCCESS, hexlen = 0;

    uint8_t *tmp = (uint8_t *)*pdata;

    while (!feof(f)) {

        memset(line, 0, sizeof(line));

        if (fgets(line, sizeof(line), f) == NULL) {
            if (feof(f))
                break;

            fclose(f);
            PrintAndLogEx(FAILED, "File reading error.");
            return PM3_EFILE;
        }

        // skip lines like "+Sector:"
        if (line[0] == '+')
            continue;

        strcleanrn(line, sizeof(line));

        res = param_gethex_to_eol(line, 0, buf, sizeof(buf), &hexlen);
        if (res == 0) {
            memcpy(tmp + counter, buf, hexlen);
            counter += hexlen;
        } else {
            retval = PM3_ESOFT;
        }
    }
    fclose(f);
    PrintAndLogEx(SUCCESS, "loaded " _YELLOW_("%zu") " bytes from MCT file " _YELLOW_("%s"), counter, preferredName);


    uint8_t *newdump = realloc(*pdata, counter);
    if (newdump == NULL) {
        free(*pdata);
        return PM3_EMALLOC;
    } else {
        *pdata = newdump;
    }

    if (datalen)
        *datalen = counter;

    return retval;
}

// iceman:  todo - move all unsafe functions like this from client source.
int loadFileDICTIONARY(const char *preferredName, void *data, size_t *datalen, uint8_t keylen, uint32_t *keycnt) {
    // t5577 == 4 bytes
    // mifare == 6 bytes
    // mf plus == 16 bytes
    // mf desfire == 3des3k 24 bytes
    // iclass == 8 bytes
    // default to 6 bytes.
    if (keylen != 4 && keylen != 6 && keylen != 8 && keylen != 16 && keylen != 24) {
        keylen = 6;
    }

    return loadFileDICTIONARYEx(preferredName, data, 0, datalen, keylen, keycnt, 0, NULL, true);
}

int loadFileDICTIONARYEx(const char *preferredName, void *data, size_t maxdatalen, size_t *datalen, uint8_t keylen, uint32_t *keycnt,
                         size_t startFilePosition, size_t *endFilePosition, bool verbose) {

    if (data == NULL) return PM3_EINVARG;

    if (endFilePosition)
        *endFilePosition = 0;

    char *path;
    if (searchFile(&path, DICTIONARIES_SUBDIR, preferredName, ".dic", false) != PM3_SUCCESS)
        return PM3_EFILE;

    // double up since its chars
    keylen <<= 1;

    char line[255];
    uint32_t vkeycnt = 0;
    size_t counter = 0;
    int retval = PM3_SUCCESS;

    FILE *f = fopen(path, "r");
    if (!f) {
        PrintAndLogEx(WARNING, "file not found or locked `" _YELLOW_("%s") "`", path);
        retval = PM3_EFILE;
        goto out;
    }

    if (startFilePosition) {
        if (fseek(f, startFilePosition, SEEK_SET) < 0) {
            fclose(f);
            retval = PM3_EFILE;
            goto out;
        }
    }

    uint8_t *udata = (uint8_t *)data;

    // read file
    while (!feof(f)) {
        long filepos = ftell(f);

        if (!fgets(line, sizeof(line), f)) {
            if (endFilePosition)
                *endFilePosition = 0;
            break;
        }

        // add null terminator
        line[keylen] = 0;

        // smaller keys than expected is skipped
        if (strlen(line) < keylen)
            continue;

        // The line start with # is comment, skip
        if (line[0] == '#')
            continue;

        if (!CheckStringIsHEXValue(line))
            continue;

        // cant store more data
        if (maxdatalen && (counter + (keylen >> 1) > maxdatalen)) {
            retval = 1;
            if (endFilePosition)
                *endFilePosition = filepos;
            break;
        }

        if (hex_to_bytes(line, udata + counter, keylen >> 1) != (keylen >> 1))
            continue;

        vkeycnt++;
        memset(line, 0, sizeof(line));
        counter += (keylen >> 1);
    }
    fclose(f);
    if (verbose)
        PrintAndLogEx(SUCCESS, "loaded " _GREEN_("%2d") " keys from dictionary file " _YELLOW_("%s"), vkeycnt, path);

    if (datalen)
        *datalen = counter;
    if (keycnt)
        *keycnt = vkeycnt;
    out:
    free(path);
    return retval;
}

int loadFileDICTIONARY_safe(const char *preferredName, void **pdata, uint8_t keylen, uint32_t *keycnt) {

    int retval = PM3_SUCCESS;

    char *path;
    if (searchFile(&path, DICTIONARIES_SUBDIR, preferredName, ".dic", false) != PM3_SUCCESS)
        return PM3_EFILE;

    // t5577 == 4bytes
    // mifare == 6 bytes
    // mf plus == 16 bytes
    // mf desfire == 3des3k 24 bytes
    // iclass == 8 bytes
    // default to 6 bytes.
    if (keylen != 4 && keylen != 6 && keylen != 8 && keylen != 16 && keylen != 24) {
        keylen = 6;
    }

    size_t mem_size;
    size_t block_size = 10 * keylen;

    // double up since its chars
    keylen <<= 1;

    char line[255];

    // allocate some space for the dictionary
    *pdata = calloc(block_size, sizeof(uint8_t));
    if (*pdata == NULL) {
        free(path);
        return PM3_EFILE;
    }
    mem_size = block_size;

    FILE *f = fopen(path, "r");
    if (!f) {
        PrintAndLogEx(WARNING, "file not found or locked `" _YELLOW_("%s") "`", path);
        retval = PM3_EFILE;
        goto out;
    }

    // read file
    while (fgets(line, sizeof(line), f)) {

        // check if we have enough space (if not allocate more)
        if ((*keycnt * (keylen >> 1)) >= mem_size) {

            mem_size += block_size;
            *pdata = realloc(*pdata, mem_size);

            if (*pdata == NULL) {
                retval = PM3_EFILE;
                fclose(f);
                goto out;
            } else {
                memset((uint8_t *)*pdata + (mem_size - block_size), 0, block_size);
            }
        }

        // add null terminator
        line[keylen] = 0;

        // smaller keys than expected is skipped
        if (strlen(line) < keylen)
            continue;

        // The line start with # is comment, skip
        if (line[0] == '#')
            continue;

        if (!CheckStringIsHEXValue(line))
            continue;

        uint64_t key = strtoull(line, NULL, 16);

        num_to_bytes(key, keylen >> 1, (uint8_t *)*pdata + (*keycnt * (keylen >> 1)));

        (*keycnt)++;

        memset(line, 0, sizeof(line));
    }
    fclose(f);
    PrintAndLogEx(SUCCESS, "loaded " _GREEN_("%2d") " keys from dictionary file " _YELLOW_("%s"), *keycnt, path);

    out:
    free(path);
    return retval;
}

int loadFileBinaryKey(const char *preferredName, const char *suffix, void **keya, void **keyb, size_t *alen, size_t *blen) {

    char *path;
    int res = searchFile(&path, RESOURCES_SUBDIR, preferredName, suffix, false);
    if (res != PM3_SUCCESS) {
        return PM3_EFILE;
    }

    FILE *f = fopen(path, "rb");
    if (!f) {
        PrintAndLogEx(WARNING, "file not found or locked `" _YELLOW_("%s") "`", path);
        free(path);
        return PM3_EFILE;
    }
    free(path);

    // get filesize in order to malloc memory
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize <= 0) {
        PrintAndLogEx(FAILED, "error, when getting filesize");
        fclose(f);
        return PM3_EFILE;
    }

    // Half is KEY A,  half is KEY B
    fsize /= 2;

    *keya = calloc(fsize, sizeof(uint8_t));
    if (*keya == NULL) {
        PrintAndLogEx(FAILED, "error, cannot allocate memory");
        fclose(f);
        return PM3_EMALLOC;
    }

    *alen = fread(*keya, 1, fsize, f);

    *keyb = calloc(fsize, sizeof(uint8_t));
    if (*keyb == NULL) {
        PrintAndLogEx(FAILED, "error, cannot allocate memory");
        fclose(f);
        return PM3_EMALLOC;
    }

    *blen = fread(*keyb, 1, fsize, f);
    fclose(f);
    return PM3_SUCCESS;
}

static char *my_executable_path = NULL;
static char *my_executable_directory = NULL;

const char *get_my_executable_path(void) {
    return my_executable_path;
}

char *getExecutableDirectory(const char *path, int *length) {
    if (path == NULL || length == NULL) {
        return NULL;
    }

    const char *last_sep = NULL;
    const char *p = path;

    while (*p != '\0') {
        if (*p == '\\' || *p == '/') {
            last_sep = p;
        }
        p++;
    }

    if (last_sep == NULL) {
        *length = 0;
    } else {
        *length = last_sep - path + 1;
    }

    return (char *)last_sep;
}

static void set_my_executable_path(void) {
    char path[1024];
    if (getExecutablePath(path, sizeof(path)) == NULL) {
        return;
    }

    int path_length = strlen(path);
    my_executable_path = (char *)calloc(path_length + 1, sizeof(char));
    int dirname_length = 0;
    if (my_executable_path != NULL) {
        strncpy(my_executable_path, path, path_length + 1);
        if (getExecutableDirectory(my_executable_path, &dirname_length) != NULL) {
            my_executable_directory = (char *)calloc(dirname_length + 1, sizeof(char));
            if (my_executable_directory != NULL) {
                strncpy(my_executable_directory, my_executable_path, dirname_length);
                my_executable_directory[dirname_length] = '\0';
            }
        }
    }
}

const char *get_my_executable_directory(void) {
    set_my_executable_path();
    return my_executable_directory;
}

static const char *my_user_directory = NULL;
// static char _cwd_Buffer [FILENAME_MAX] = {0};

static void set_my_user_directory(void) {
    /*    my_user_directory = getenv("HOME");

        // if not found, default to current directory
        if (my_user_directory == NULL) {
            my_user_directory = GetCurrentDir(_cwd_Buffer, sizeof(_cwd_Buffer));
            // change all slashes to / (windows should not care...
            for (int i = 0; i < strlen(_cwd_Buffer); i++)
                if (_cwd_Buffer[i] == '\\') _cwd_Buffer[i] = '/';
            //      my_user_directory = ".";
        }
    */
    my_user_directory = getenv("HOME");

    // if not found, default to current directory
    if (my_user_directory == NULL) {

        uint16_t pathLen = FILENAME_MAX; // should be a good starting point
        char *cwd_buffer = (char *)calloc(pathLen, sizeof(uint8_t));
        if (cwd_buffer == NULL) {
            PrintAndLogEx(WARNING, "failed to allocate memory");
            return;
        }

        getcwd(cwd_buffer, pathLen);

        for (int i = 0; i < strlen(cwd_buffer); i++) {
            if (cwd_buffer[i] == '\\') {
                cwd_buffer[i] = '/';
            }
        }

        my_user_directory = cwd_buffer;
    }
}

const char *get_my_user_directory(void) {
    set_my_user_directory();
    return my_user_directory;
}



static int filelist(const char *path, const char *ext, uint8_t last, bool tentative, uint8_t indent, uint16_t strip) {
    struct dirent **namelist;
    int n;

    n = scandir(path, &namelist, NULL, alphasort);
    if (n == -1) {

        if (tentative == false) {

            for (uint8_t j = 0; j < indent; j++) {
                PrintAndLogEx(NORMAL, "%s   " NOLF, ((last >> j) & 1) ? " " : "│");
            }
            PrintAndLogEx(NORMAL, "%s── "_GREEN_("%s"), last ? "└" : "├", &path[strip]);
        }
        return PM3_EFILE;
    }

    for (uint8_t j = 0; j < indent; j++) {
        PrintAndLogEx(NORMAL, "%s   " NOLF, ((last >> j) & 1) ? " " : "│");
    }

    PrintAndLogEx(NORMAL, "%s── "_GREEN_("%s"), last ? "└" : "├", &path[strip]);

    for (int i = 0; i < n; i++) {

        char tmp_fullpath[1024] = {0};
        strncat(tmp_fullpath, path, sizeof(tmp_fullpath) - 1);
        tmp_fullpath[1023] = 0x00;
        strncat(tmp_fullpath, namelist[i]->d_name, strlen(tmp_fullpath) - 1);

        if (is_directory(tmp_fullpath)) {

            char newpath[1024];
            if (strcmp(namelist[i]->d_name, ".") == 0 || strcmp(namelist[i]->d_name, "..") == 0)
                continue;

            snprintf(newpath, sizeof(newpath), "%s", path);
            strncat(newpath, namelist[i]->d_name, sizeof(newpath) - strlen(newpath) - 1);
            strncat(newpath, "/", sizeof(newpath) - strlen(newpath) - 1);

            filelist(newpath, ext, last + ((i == n - 1) << (indent + 1)), tentative, indent + 1, strlen(path));
        } else {

            if ((ext == NULL) || ((str_endswith(namelist[i]->d_name, ext)))) {

                for (uint8_t j = 0; j < indent + 1; j++) {
                    PrintAndLogEx(NORMAL, "%s   " NOLF, ((last >> j) & 1) ? " " : "│");
                }
                PrintAndLogEx(NORMAL, "%s── %-21s", i == n - 1 ? "└" : "├", namelist[i]->d_name);
            }
        }
        free(namelist[i]);
    }
    free(namelist);
    return PM3_SUCCESS;
}

int searchAndList(const char *pm3dir, const char *ext) {
    // display in same order as searched by searchFile
    // try pm3 dirs in current workdir (dev mode)
    if (get_my_executable_directory() != NULL) {
        char script_directory_path[1024];
        strcpy(script_directory_path, get_my_executable_directory());
        strcat(script_directory_path, pm3dir);
        filelist(script_directory_path, ext, false, true, 0, 0);
    }
    // try pm3 dirs in user .proxmark3 (user mode)
    const char *user_path = get_my_user_directory();
    if (user_path != NULL) {
        char script_directory_path[1024];
        strcpy(script_directory_path, user_path);
        strcat(script_directory_path, PM3_USER_DIRECTORY);
        strcat(script_directory_path, pm3dir);
        filelist(script_directory_path, ext, false, false, 0, 0);
    }
    // try pm3 dirs in pm3 installation dir (install mode)
    const char *exec_path = get_my_executable_directory();
    if (exec_path != NULL) {
        char script_directory_path[1024];
        strcpy(script_directory_path, exec_path);
        strcat(script_directory_path, PM3_SHARE_RELPATH);
        strcat(script_directory_path, pm3dir);
        filelist(script_directory_path, ext, true, false, 0, 0);
    }
    return PM3_SUCCESS;
}

static int searchFinalFile(char **foundpath, const char *pm3dir, const char *searchname, bool silent) {

    if ((foundpath == NULL) || (pm3dir == NULL) || (searchname == NULL)) {
        return PM3_ESOFT;
    }

    // explicit absolute (/) or relative path (./) => try only to match it directly
    char *filename = calloc(strlen(searchname) + 1, sizeof(char));
    if (filename == NULL) {
        return PM3_EMALLOC;
    }

    strcpy(filename, searchname);
    if ((g_debugMode == 2) && (!silent)) {
        PrintAndLogEx(INFO, "pm3dir...... %s", pm3dir);
        PrintAndLogEx(INFO, "Searching... %s", filename);
    }

    // try implicit relative path
    PrintAndLogEx(DEBUG, "Searching implicit relative paths");
    if (fileExists(filename)) {
        *foundpath = filename;
        if ((g_debugMode == 2) && (!silent)) {
            PrintAndLogEx(INFO, "Found %s", *foundpath);
        }
        return PM3_SUCCESS;
    }

    if (((strlen(filename) > 1) && (filename[0] == '/')) ||
        ((strlen(filename) > 2) && (filename[0] == '.') && (filename[1] == '/'))) {
        goto out;
    }

    // try the session paths
    PrintAndLogEx(DEBUG, "Searching preferences paths");
    for (int i = 0; i < spItemCount; i++) {

        size_t sn = strlen(g_session.defaultPaths[i]) + strlen(filename) + strlen(PATHSEP) + 1;
        char *default_path = calloc(sn, sizeof(char));
        if (default_path == NULL) {
            goto out;
        }

        snprintf(default_path, sn, "%s%s%s", g_session.defaultPaths[i], PATHSEP, filename);

        if ((g_debugMode == 2) && (!silent)) {
            PrintAndLogEx(INFO, "Searching %s", default_path);
        }

        if (fileExists(default_path)) {
            free(filename);
            *foundpath = default_path;
            if ((g_debugMode == 2) && (!silent)) {
                PrintAndLogEx(INFO, "Found %s", *foundpath);
            }
            return PM3_SUCCESS;
        } else {
            free(default_path);
        }
    }

    // try pm3 dirs in user .proxmark3 (user mode)
    PrintAndLogEx(DEBUG, "Searching user .proxmark3 paths");
    const char *user_path = get_my_user_directory();
    if (user_path != NULL) {
        char *path = calloc(strlen(user_path) + strlen(PM3_USER_DIRECTORY) + strlen(pm3dir) + strlen(filename) + 1, sizeof(char));
        if (path == NULL) {
            goto out;
        }

        strcpy(path, user_path);
        strcat(path, PM3_USER_DIRECTORY);
        strcat(path, pm3dir);
        strcat(path, filename);

        if ((g_debugMode == 2) && (!silent)) {
            PrintAndLogEx(INFO, "Searching %s", path);
        }

        if (fileExists(path)) {
            free(filename);
            *foundpath = path;
            if ((g_debugMode == 2) && (!silent)) {
                PrintAndLogEx(INFO, "Found %s", *foundpath);
            }
            return PM3_SUCCESS;
        } else {
            free(path);
        }
    }

    // try pm3 dirs in current client workdir (dev mode)
    PrintAndLogEx(DEBUG, "Searching current workdir paths");
    const char *exec_path = get_my_executable_directory();
    if ((exec_path != NULL) &&
        ((strcmp(DICTIONARIES_SUBDIR, pm3dir) == 0) ||
         (strcmp(LUA_LIBRARIES_SUBDIR, pm3dir) == 0) ||
         (strcmp(LUA_SCRIPTS_SUBDIR, pm3dir) == 0) ||
         (strcmp(CMD_SCRIPTS_SUBDIR, pm3dir) == 0) ||
         (strcmp(PYTHON_SCRIPTS_SUBDIR, pm3dir) == 0) ||
         (strcmp(RESOURCES_SUBDIR, pm3dir) == 0))) {
        char *path = calloc(strlen(exec_path) + strlen(pm3dir) + strlen(filename) + 1, sizeof(char));
        if (path == NULL) {
            goto out;
        }

        strcpy(path, exec_path);
        strcat(path, pm3dir);
        strcat(path, filename);

        if ((g_debugMode == 2) && (!silent)) {
            PrintAndLogEx(INFO, "Searching %s", path);
        }

        if (fileExists(path)) {
            free(filename);
            *foundpath = path;
            if ((g_debugMode == 2) && (!silent)) {
                PrintAndLogEx(INFO, "Found %s", *foundpath);
            }
            return PM3_SUCCESS;
        } else {
            free(path);
        }
    }

    // try pm3 dirs in current repo workdir (dev mode)
    PrintAndLogEx(DEBUG, "Searching PM3 dirs in current workdir");
    if ((exec_path != NULL) &&
        ((strcmp(TRACES_SUBDIR, pm3dir) == 0) ||
         (strcmp(FIRMWARES_SUBDIR, pm3dir) == 0) ||
         (strcmp(BOOTROM_SUBDIR, pm3dir) == 0) ||
         (strcmp(FULLIMAGE_SUBDIR, pm3dir) == 0))) {
        char *path = calloc(strlen(exec_path) + strlen(ABOVE) + strlen(pm3dir) + strlen(filename) + 1, sizeof(char));
        if (path == NULL) {
            goto out;
        }

        strcpy(path, exec_path);
        strcat(path, ABOVE);
        strcat(path, pm3dir);
        strcat(path, filename);

        if ((g_debugMode == 2) && (!silent)) {
            PrintAndLogEx(INFO, "Searching %s", path);
        }

        if (fileExists(path)) {
            free(filename);
            *foundpath = path;
            if ((g_debugMode == 2) && (!silent)) {
                PrintAndLogEx(INFO, "Found %s", *foundpath);
            }
            return PM3_SUCCESS;
        } else {
            free(path);
        }
    }

    // try pm3 dirs in pm3 installation dir (install mode)
    PrintAndLogEx(DEBUG, "Searching PM3 installation dir paths");
    if (exec_path != NULL) {
        char *path = calloc(strlen(exec_path) + strlen(PM3_SHARE_RELPATH) + strlen(pm3dir) + strlen(filename) + 1, sizeof(char));
        if (path == NULL) {
            goto out;
        }

        strcpy(path, exec_path);
        strcat(path, PM3_SHARE_RELPATH);
        strcat(path, pm3dir);
        strcat(path, filename);

        if ((g_debugMode == 2) && (!silent)) {
            PrintAndLogEx(INFO, "Searching %s", path);
        }

        if (fileExists(path)) {
            free(filename);
            *foundpath = path;
            if ((g_debugMode == 2) && (!silent)) {
                PrintAndLogEx(INFO, "Found %s", *foundpath);
            }
            return PM3_SUCCESS;
        } else {
            free(path);
        }
    }
    out:
    free(filename);
    return PM3_EFILE;
}

int searchFile(char **foundpath, const char *pm3dir, const char *searchname, const char *suffix, bool silent) {

    if (foundpath == NULL)
        return PM3_EINVARG;

    if (searchname == NULL || strlen(searchname) == 0)
        return PM3_EINVARG;

    if (is_directory(searchname))
        return PM3_EINVARG;

    char *filename = filenamemcopy(searchname, suffix);
    if (filename == NULL)
        return PM3_EMALLOC;

    if (strlen(filename) == 0) {
        free(filename);
        return PM3_EFILE;
    }

    int res = searchFinalFile(foundpath, pm3dir, filename, silent);
    if (res != PM3_SUCCESS) {
        if ((res == PM3_EFILE) && (!silent)) {
            PrintAndLogEx(FAILED, "Error - can't find `" _YELLOW_("%s") "`", filename);
        }
    }
    free(filename);
    return res;
}

int pm3_load_dump(const char *fn, void **pdump, size_t *dumplen, size_t maxdumplen) {

    int res = 0;
    DumpFileType_t dftype = getfiletype(fn);
    switch (dftype) {
        case BIN: {
            res = loadFile_safe(fn, ".bin", pdump, dumplen);
            break;
        }
        case EML: {
            res = loadFileEML_safe(fn, pdump, dumplen);
            break;
        }
        case JSON: {
            *pdump = calloc(maxdumplen, sizeof(uint8_t));
            if (*pdump == NULL) {
                PrintAndLogEx(WARNING, "Fail, cannot allocate memory");
                return PM3_EMALLOC;
            }
            break;
        }
        case DICTIONARY: {
            PrintAndLogEx(ERR, "Error: Only BIN/EML/JSON formats allowed");
            return PM3_EINVARG;
        }
    }

    if (res != PM3_SUCCESS) {
        PrintAndLogEx(WARNING, "file not found or locked `" _YELLOW_("%s") "`", fn);
        return PM3_EFILE;
    }

    return res;
}

int pm3_save_dump(const char *fn, uint8_t *d, size_t n, JSONFileType jsft, size_t blocksize) {

    if (d == NULL || n == 0) {
        PrintAndLogEx(INFO, "No data to save, skipping...");
        return PM3_EINVARG;
    }

    saveFile(fn, ".bin", d, n);
    saveFileEML(fn, d, n, blocksize);
    return PM3_SUCCESS;
}
