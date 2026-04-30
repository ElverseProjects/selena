#include <stdio.h>
#include <stdlib.h>

#include <selena.h>

enum {
    POINT_MY_OUT = 0
};

enum {
    MY_OUT_STDOUT = 0,
    MY_OUT_STDERR = 1
};

enum {
    CATEGORY_BASE = 0,
    CATEGORY_DEBUG = 1
};

static void (*my_out)(const char *) = NULL;

static void my_out_stdout(const char *text) {
    fputs(text, stdout);
}

static void my_out_stderr(const char *text) {
    fputs(text, stderr);
}

static void check_selena(int result, const char *label) {
    if (result != SELENA_OK) {
        fprintf(stderr, "%s failed with Selena error %d\n", label, result);
        exit(1);
    }
}

int main(void) {
    SelenaBinding debug_override[] = {
        { POINT_MY_OUT, MY_OUT_STDERR }
    };

    selena_init();

    check_selena(
        selena_register_impl(POINT_MY_OUT, MY_OUT_STDOUT, (void *)my_out_stdout),
        "register stdout implementation"
    );

    check_selena(
        selena_register_impl(POINT_MY_OUT, MY_OUT_STDERR, (void *)my_out_stderr),
        "register stderr implementation"
    );

    check_selena(
        selena_bind(POINT_MY_OUT, (void **)&my_out),
        "bind my_out slot"
    );

    check_selena(
        selena_category_set(CATEGORY_BASE, POINT_MY_OUT, MY_OUT_STDOUT),
        "set base category"
    );

    check_selena(
        selena_category_set(CATEGORY_DEBUG, POINT_MY_OUT, MY_OUT_STDERR),
        "set debug category"
    );

    check_selena(selena_activate(CATEGORY_BASE), "activate base category");
    my_out("Hello from base category\n");

    check_selena(selena_activate(CATEGORY_DEBUG), "activate debug category");
    my_out("Hello from debug category\n");

    check_selena(
        selena_activate_with_overrides(CATEGORY_BASE, debug_override, 1),
        "activate base category with override"
    );
    my_out("Hello from base category with override\n");

    return 0;
}
