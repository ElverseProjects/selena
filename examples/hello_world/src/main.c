#include <stdio.h>
#include <selena.h>

#include <hello.h>

void (*my_out)(const char*) = NULL;

void my_out_stdout(const char* label) {
    fputs(label, stdout);
}
void my_out_stderr(const char* label) {
    fputs(label, stderr);
}

int main(int argc, char* argv[]) {

    selena_register_impl("my_out", "stdout_stream", (void*)&my_out_stdout);
    selena_register_impl("my_out", "stderr_stream", (void*)&my_out_stderr);
    selena_bind("my_out", (void*)&my_out);

    if (argc > 1)
        selena_set("my_out", "stdout_stream");
    else
        selena_set("my_out", "stderr_stream");

    selena_init(NULL);

    
    return 0;
}