#include "Filesystem.h"

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

char* read_file(const char* file_path, size_t* buf_len) {
    FILE* file = fopen(file_path, "rb");
    if (!file) {
        return NULL;  
    }
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(file_size + 1); 
    if (!buffer) {
        fclose(file);
        return NULL;  
    }

    size_t read_size = fread(buffer, 1, file_size, file);
    fclose(file);

    if (read_size != file_size) {
        free(buffer);
        return NULL;
    }

	*buf_len = file_size;

    buffer[file_size] = '\0'; 
    return buffer;
}
