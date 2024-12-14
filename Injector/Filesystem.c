#include "Filesystem.h"
#include "Windows.h"

bool check_file_exists(const char* file_path)
{
	FILE* f = fopen(file_path, "r");
	if (!f)
	{
		return false;
	}
	fclose(f);
	return true;
}

char* read_file(const char* file_path, size_t* buf_len) 
{
    FILE* file = fopen(file_path, "rb");
    if (!file) {
        return NULL;  
    }
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(file_size + 1); 
    if (!buffer) 
    {
        fclose(file);
        return NULL;  
    }

    size_t read_size = fread(buffer, 1, file_size, file);
    fclose(file);

    if (read_size != file_size)
    {
        free(buffer);
        return NULL;
    }

	*buf_len = file_size;

    buffer[file_size] = '\0'; 
    return buffer;
}

int is_absolute_path(const char* path) 
{
    if (!path || strlen(path) == 0) 
    {
        return 0;
    }

    return ((strlen(path) > 2 && path[1] == ':' && (path[2] == '\\' || path[2] == '/')) ||
        (strlen(path) > 1 && path[0] == '\\' && path[1] == '\\'));
}

char* make_absolute_path(const char* path)
{
    if (!path)
    {
        return NULL;
    }

    char* absolute_path = (char*)malloc(MAX_PATH);
    if (!absolute_path)
    {
        return NULL;
    }

    if (is_absolute_path(path))
    {
        strncpy(absolute_path, path, MAX_PATH - 1);
        absolute_path[MAX_PATH - 1] = '\0';
        return absolute_path;
    }

    DWORD length = GetFullPathNameA(path, MAX_PATH, absolute_path, NULL);
    if (length == 0 || length > MAX_PATH)
    {
        free(absolute_path);
        return NULL;
    }

    return absolute_path;
}

void get_file_name(const char* file_path, char* file_name, size_t buffer_size)
{
    if (!file_path || !file_name || buffer_size == 0)
    {
        return;
    }

    const char* lastSeparator = strrchr(file_path, '\\');
    if (!lastSeparator) 
    {
        lastSeparator = strrchr(file_path, '/');
    }

    const char* extractedName = lastSeparator ? lastSeparator + 1 : file_path;

    strncpy(file_name, extractedName, buffer_size - 1);
    file_name[buffer_size - 1] = '\0';
}