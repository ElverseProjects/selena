
![Selena introduction image](.github/selena_intro.png)

# Selena

Selena is a tiny profile-based dispatch runtime for C.

The core idea is deliberately small:

- C owns typed function slots.
- Selena stores precompiled implementations for those slots.
- A category/profile selects which implementations should be active.
- `selena_activate()` binds selected implementations into the slots.
- After activation, the hot path is just a normal function pointer call.

Selena v0 is not a scripting engine, a JIT compiler, a plugin framework, or a universal function-call ABI.
It is only a small runtime for binding precompiled C function implementations by category/profile.

## Current prototype

This prototype supports:

- index-based runtime API;
- switch points;
- implementation registration;
- typed function slots owned by user code;
- categories/profiles;
- profile activation through `selena_activate()`;
- temporary custom overrides through `selena_activate_with_overrides()`;
- no string-based runtime dispatch;
- no parser;
- no dynamic type system.

## Build

Install Meson and Ninja, then run:

```bash
meson setup build
meson compile -C build
./build/selena_hello
```

You can also compile the example directly:

```bash
cc -std=c99 -Wall -Wextra -Iinclude \
  src/selena-runtime.c \
  examples/hello.c \
  -o selena_hello
./selena_hello
```

## Minimal usage

```c
#include <stdio.h>
#include "selena.h"

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

static void (*my_out)(const char*) = NULL;

static void my_out_stdout(const char* text) {
    fputs(text, stdout);
}

static void my_out_stderr(const char* text) {
    fputs(text, stderr);
}

int main(void) {
    selena_init();

    selena_register_impl(POINT_MY_OUT, MY_OUT_STDOUT, (void*)my_out_stdout);
    selena_register_impl(POINT_MY_OUT, MY_OUT_STDERR, (void*)my_out_stderr);

    selena_bind(POINT_MY_OUT, (void**)&my_out);

    selena_category_set(CATEGORY_BASE, POINT_MY_OUT, MY_OUT_STDOUT);
    selena_category_set(CATEGORY_DEBUG, POINT_MY_OUT, MY_OUT_STDERR);

    selena_activate(CATEGORY_BASE);
    my_out("Hello from base category\n");

    selena_activate(CATEGORY_DEBUG);
    my_out("Hello from debug category\n");

    return 0;
}
```

## Design stance

Selena is profile dispatch, not type dispatch.

It does not inspect function arguments and it does not choose an implementation per call.
It binds first, then the application runs through selected typed slots.

The main safety rule is simple:

> One switch point has one fixed C signature. All implementations registered for that point must be ABI-compatible with that signature.
