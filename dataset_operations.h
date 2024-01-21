#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <cstdio>

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
void download_file(const char *url, const char *file_path);
void extract_tar_file(const char *file_path);

#endif // FILE_OPERATIONS_H