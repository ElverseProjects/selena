#ifndef SELENA_H
#define SELENA_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SELENA_MAX_POINTS
#define SELENA_MAX_POINTS 64
#endif

#ifndef SELENA_MAX_IMPLS_PER_POINT
#define SELENA_MAX_IMPLS_PER_POINT 16
#endif

#ifndef SELENA_MAX_CATEGORIES
#define SELENA_MAX_CATEGORIES 32
#endif

typedef uint16_t SelenaPointId;
typedef uint16_t SelenaImplId;
typedef uint16_t SelenaCategoryId;

#define SELENA_NONE ((uint16_t)0xFFFFu)

typedef enum SelenaResult {
    SELENA_OK = 0,
    SELENA_ERR_INVALID_ARG = -1,
    SELENA_ERR_OUT_OF_RANGE = -2,
    SELENA_ERR_NOT_FOUND = -3,
    SELENA_ERR_ALREADY_BOUND = -4,
    SELENA_ERR_SLOT_NOT_BOUND = -5,
    SELENA_ERR_IMPL_NOT_REGISTERED = -6
} SelenaResult;

typedef struct SelenaBinding {
    SelenaPointId point;
    SelenaImplId impl;
} SelenaBinding;

void selena_reset(void);

static inline void selena_init(void) {
    selena_reset();
}

int selena_register_impl(SelenaPointId point, SelenaImplId impl, void *fn);
int selena_bind(SelenaPointId point, void **slot_addr);

int selena_category_set(SelenaCategoryId category, SelenaPointId point, SelenaImplId impl);
int selena_category_clear(SelenaCategoryId category);

int selena_activate(SelenaCategoryId category);
int selena_activate_with_overrides(
    SelenaCategoryId category,
    const SelenaBinding *overrides,
    size_t override_count
);

int selena_apply_bindings(const SelenaBinding *bindings, size_t binding_count);

SelenaCategoryId selena_active_category(void);

int selena_has_impl(SelenaPointId point, SelenaImplId impl);
void *selena_get_impl(SelenaPointId point, SelenaImplId impl);

#ifdef __cplusplus
}
#endif

#endif
