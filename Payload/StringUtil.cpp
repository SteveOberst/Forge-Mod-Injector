#include "StringUtil.h"

#include <cstring>
#include <algorithm>

bool strings_ends_with(const char* str, const char* suffix) 
{
    size_t str_len = std::strlen(str);
    size_t suffix_len = std::strlen(suffix);

    if (suffix_len > str_len) 
    {
        return false;
    }

    return std::memcmp(str + str_len - suffix_len, suffix, suffix_len) == 0;
}


std::string strings_convert_to_class_name(const char* class_path) 
{
    std::string path(class_path);

    path.erase(0, path.find_first_not_of("/"));
    path.erase(path.find_last_not_of("/") + 1);

    std::replace(path.begin(), path.end(), '/', '.');
    if (path.size() > 6 && path.compare(path.size() - 6, 6, ".class") == 0)
    {
        path = path.substr(0, path.size() - 6);
    }

    return path;
}

std::string strings_remove_class_suffix(const char* class_path)
{
    std::string path(class_path);
    const std::string suffix = ".class";
    if (path.size() >= suffix.size() && path.compare(path.size() - suffix.size(), suffix.size(), suffix) == 0)
    {
        path = path.substr(0, path.size() - suffix.size());
    }

    return path;
}

char* filepath_to_java_url(const char* filepath) {
    if (filepath == NULL) {
        return NULL;
    }

    size_t len = strlen(filepath);

    size_t url_len = len + 7 + 1;
    char* java_url = (char*)malloc(url_len);

    if (java_url == NULL) {
        return NULL;
    }

    strcpy(java_url, "file://");

    for (size_t i = 0, j = 7; i < len; i++, j++) {
        if (filepath[i] == '\\') {
            java_url[j] = '/';
        }
        else {
            java_url[j] = filepath[i];
        }
    }

    java_url[url_len - 1] = '\0';

    return java_url;
}