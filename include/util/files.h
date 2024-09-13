#include "base.h"

#ifdef _WIN32
#define ETR_CHAR '\r'
#define DIR_SEP  "\\"
#else
#define ETR_CHAR '\n'
#define DIR_SEP  "/"
#endif

// Gets and creates config directory with name
// @throws EIO
// @throws ENFILE
// @throws EEXIST
errno_t get_cfg_path(_Out_writes_(US_MAX_PATH) char* buffer, _In_z_ const char* name);
errno_t create_directory(_In_z_ const char* path);
errno_t delete_directory(_Inout_z_ char* path);
void delete_self(void);
// @returns count
int get_directory_contents(_Inout_z_ char* path, _Out_writes_(max) char contents[][16], int max, bool is_dir);