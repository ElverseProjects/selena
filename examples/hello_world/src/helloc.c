
#include <hello.h>


SELENA_SWITCHABLE(int, io_read, (const char* path, void* out, size_t size), (path, out, size));
SELENA_DEFINE_SLOT(io_read);

