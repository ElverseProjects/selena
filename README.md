
![Selena introduction image](.github/selena_intro.png)

# Selena

Selena is a tiny profile-based dispatch runtime for C.

The core idea is deliberately small:

- C owns typed function slots.
- Selena stores precompiled implementations for those slots.
- A profile/category selects which implementations should be active.
- `selena_activate()` binds the selected implementations into the slots.
- After activation, the hot path is just a normal function pointer call.

Selena should **not** become a scripting engine, a JIT compiler, a plugin framework, or a universal runtime. It should only provide a small and predictable mechanism for binding precompiled function implementations according to an active execution profile.

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
- no dynamic type system;
- no universal function call ABI.

## Core concept

A Selena switch point is a logical function slot with one fixed C signature.

For example:

```c
void (*my_out)(const char*) = NULL;
````

The application may provide several compatible implementations:

```c
void my_out_stdout(const char* text);
void my_out_stderr(const char* text);
```

Selena does not call these functions directly.
It only decides which implementation should be written into the slot.

```text
before activation

my_out ─────► NULL


after activating base profile

my_out ─────► my_out_stdout


after activating debug profile

my_out ─────► my_out_stderr
```

The important rule is simple:

> One switch point has one signature.
> All implementations of that switch point must be ABI-compatible with that signature.

Selena does not try to erase function types or provide a universal `call()` mechanism.

## Runtime model

Selena v0 uses indexes, not strings.

```text
process

┌─────────────────────────────────────────────┐
│ C application                               │
│                                             │
│ typed slots                                 │
│                                             │
│  my_out ─────────────┐                      │
│                      │                      │
│ implementations      │                      │
│  stdout_stream       │                      │
│  stderr_stream       │                      │
│                                             │
│ Selena runtime                              │
│                                             │
│  point table                                │
│  impl table                                 │
│  category table                             │
│                                             │
│  category base:                             |
│    my_out -> stdout_stream                  │
│                                             │
│  category debug:                            │
│    my_out -> stderr_stream                  │
└─────────────────────────────────────────────┘
```

Activation is just binding:

```c
selena_activate(CATEGORY_DEBUG);
```

After that, user code calls the slot normally:

```c
my_out("Hello from Selena\n");
```

No profile lookup or condition evaluation happens inside the hot path.

## Terminology

Selena v0 has four core entities.

### Point

A logical switch point.

Example:

```c
POINT_MY_OUT
```

A point represents a place where one of several implementations may be bound.

### Implementation

A concrete function implementation for a point.

Example:

```c
MY_OUT_STDOUT
MY_OUT_STDERR
```

All implementations of the same point must share the same C signature.

### Slot

A typed function pointer owned by the application.

Example:

```c
void (*my_out)(const char*) = NULL;
```

Selena receives the address of the slot and writes the selected implementation into it.

### Category

A profile: a set of point-to-implementation bindings.

Example:

```c
CATEGORY_BASE
CATEGORY_DEBUG
```

A category answers:

> for this execution profile, which implementation should each point use?

## Core C API

```c
void selena_reset(void);
void selena_init(void);

int selena_register_impl(SelenaPointId point,
                         SelenaImplId impl,
                         void* fn);

int selena_bind(SelenaPointId point,
                void** slot_addr);

int selena_category_set(SelenaCategoryId category,
                        SelenaPointId point,
                        SelenaImplId impl);

int selena_category_clear(SelenaCategoryId category);

int selena_activate(SelenaCategoryId category);

int selena_activate_with_overrides(SelenaCategoryId category,
                                   const SelenaBinding* overrides,
                                   size_t override_count);

int selena_apply_bindings(const SelenaBinding* bindings,
                          size_t binding_count);

SelenaCategoryId selena_active_category(void);

int selena_has_impl(SelenaPointId point,
                    SelenaImplId impl);

void* selena_get_impl(SelenaPointId point,
                      SelenaImplId impl);
```

The runtime API is intentionally index-based.

Human-readable names may exist later in tooling, debug metadata, or a `.sl` compiler, but they are not required for the core runtime.

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

    selena_category_set(CATEGORY_BASE,  POINT_MY_OUT, MY_OUT_STDOUT);
    selena_category_set(CATEGORY_DEBUG, POINT_MY_OUT, MY_OUT_STDERR);

    selena_activate(CATEGORY_BASE);
    my_out("Hello from base profile\n");

    selena_activate(CATEGORY_DEBUG);
    my_out("Hello from debug profile\n");

    return 0;
}
```

## Overrides

A category can be activated with temporary overrides.

This is useful when most of a profile should stay the same, but a few points need to be replaced for a specific mode.

```c
SelenaBinding overrides[] = {
    { POINT_MY_OUT, MY_OUT_STDERR }
};

selena_activate_with_overrides(CATEGORY_BASE, overrides, 1);

my_out("Hello from base profile with override\n");
```

This does not create a new permanent category.
It simply activates a base category and then applies a small patch over it.

## Why profiles instead of direct switches?

A single manual function switch is easy to write in C.

Selena is not trying to replace that.

Selena becomes useful when a program has many switch points that should be changed together as one execution profile.

For example:

```text
profile base
  io.read       -> read_base
  io.prefetch   -> prefetch_basic
  render.mesh   -> render_base

profile external_hdd_safe
  io.read       -> read_safe
  io.prefetch   -> prefetch_off
  render.mesh   -> render_base

profile debug
  io.read       -> read_checked
  io.prefetch   -> prefetch_basic
  render.mesh   -> render_debug
```

The value is not the individual function pointer assignment.

The value is the ability to describe and activate a coherent set of implementation choices.

## Design stance

Selena is profile dispatch, not type dispatch.

In Julia, dispatch selects a method based on argument types.

Selena selects implementations based on an active execution profile.

```text
Julia:
  arguments -> types -> method

Selena:
  active profile -> implementation bindings -> typed slots
```

Selena does not inspect function arguments.
It does not decide per call.
It binds first, then the application runs through the selected slots.

## What Selena is not

Selena is not:

* a JIT compiler;
* a scripting engine;
* a dynamic language runtime;
* a plugin loader;
* a reflection system;
* a universal ABI for arbitrary function calls.

Selena v0 is only:

> a small runtime for binding precompiled C function implementations into typed slots by profile/category.

## Future direction

The current runtime is intentionally small.

Possible future layers:

1. macro helpers for declaring typed switch points;
2. generated enum IDs from a Selena manifest;
3. text `.sl` profile files;
4. an offline Selena compiler that turns `.sl` into C tables;
5. debug metadata for point/profile/implementation names;
6. category inheritance or fallback profiles;
7. optional environment-based profile selection.

The runtime should stay simple even if tooling becomes more expressive.

A future `.sl` file may look like:

```text
profile base {
    use my_out = stdout_stream;
}

profile debug : base {
    use my_out = stderr_stream;
}
```

But the runtime should still receive compact tables and indexes.

## Safety rules

These rules keep Selena sane:

1. All implementations of one point must have the same C signature.
2. The application owns typed slots; Selena only writes function pointers into them.
3. Do not call a slot before activating a category that initializes it.
4. Do not register incompatible functions under the same point.
5. Avoid changing active bindings while another thread may be calling the same slot, unless synchronization is provided by the application.
6. Strings may be useful for tooling, but runtime identity should remain index-based.

## Build

For a tiny C prototype:

```bash
cc -std=c99 -Wall -Wextra -I. \
  selena-runtime.c \
  examples/hello.c \
  -o hello
```

Then run:

```bash
./hello
```
