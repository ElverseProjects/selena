#include "selena.h"

#include <string.h>

static void *g_impl_table[SELENA_MAX_POINTS][SELENA_MAX_IMPLS_PER_POINT];
static void **g_slot_table[SELENA_MAX_POINTS];
static SelenaImplId g_category_table[SELENA_MAX_CATEGORIES][SELENA_MAX_POINTS];
static SelenaCategoryId g_active_category = SELENA_NONE;

static inline int selena_validate_point(SelenaPointId point) {
    return point < SELENA_MAX_POINTS;
}

static inline int selena_validate_impl(SelenaImplId impl) {
    return impl < SELENA_MAX_IMPLS_PER_POINT;
}

static inline int selena_validate_category(SelenaCategoryId category) {
    return category < SELENA_MAX_CATEGORIES;
}

static int selena_apply_one(SelenaPointId point, SelenaImplId impl) {
    void *fn;

    if (!selena_validate_point(point) || !selena_validate_impl(impl)) {
        return SELENA_ERR_OUT_OF_RANGE;
    }

    if (g_slot_table[point] == NULL) {
        return SELENA_ERR_SLOT_NOT_BOUND;
    }

    fn = g_impl_table[point][impl];
    if (fn == NULL) {
        return SELENA_ERR_IMPL_NOT_REGISTERED;
    }

    *g_slot_table[point] = fn;
    return SELENA_OK;
}

void selena_reset(void) {
    size_t category;
    size_t point;

    memset(g_impl_table, 0, sizeof(g_impl_table));
    memset(g_slot_table, 0, sizeof(g_slot_table));

    for (category = 0; category < SELENA_MAX_CATEGORIES; ++category) {
        for (point = 0; point < SELENA_MAX_POINTS; ++point) {
            g_category_table[category][point] = SELENA_NONE;
        }
    }

    g_active_category = SELENA_NONE;
}

int selena_register_impl(SelenaPointId point, SelenaImplId impl, void *fn) {
    if (!selena_validate_point(point) || !selena_validate_impl(impl) || fn == NULL) {
        return SELENA_ERR_INVALID_ARG;
    }

    g_impl_table[point][impl] = fn;
    return SELENA_OK;
}

int selena_bind(SelenaPointId point, void **slot_addr) {
    if (!selena_validate_point(point) || slot_addr == NULL) {
        return SELENA_ERR_INVALID_ARG;
    }

    if (g_slot_table[point] != NULL && g_slot_table[point] != slot_addr) {
        return SELENA_ERR_ALREADY_BOUND;
    }

    g_slot_table[point] = slot_addr;
    return SELENA_OK;
}

int selena_category_set(SelenaCategoryId category, SelenaPointId point, SelenaImplId impl) {
    if (!selena_validate_category(category) ||
        !selena_validate_point(point) ||
        !selena_validate_impl(impl)) {
        return SELENA_ERR_INVALID_ARG;
    }

    g_category_table[category][point] = impl;
    return SELENA_OK;
}

int selena_category_clear(SelenaCategoryId category) {
    size_t point;

    if (!selena_validate_category(category)) {
        return SELENA_ERR_INVALID_ARG;
    }

    for (point = 0; point < SELENA_MAX_POINTS; ++point) {
        g_category_table[category][point] = SELENA_NONE;
    }

    return SELENA_OK;
}

int selena_activate(SelenaCategoryId category) {
    int result;

    if (!selena_validate_category(category)) {
        return SELENA_ERR_INVALID_ARG;
    }

    for (size_t point = 0; point < SELENA_MAX_POINTS; ++point) {
        SelenaImplId impl = g_category_table[category][point];

        if (impl == SELENA_NONE) {
            continue;
        }

        if (g_slot_table[point] == NULL) {
            continue;
        }

        result = selena_apply_one((SelenaPointId)point, impl);
        if (result != SELENA_OK) {
            return result;
        }
    }

    g_active_category = category;
    return SELENA_OK;
}

int selena_apply_bindings(const SelenaBinding *bindings, size_t binding_count) {
    int result;

    if (bindings == NULL && binding_count != 0) {
        return SELENA_ERR_INVALID_ARG;
    }

    for (size_t i = 0; i < binding_count; ++i) {
        result = selena_apply_one(bindings[i].point, bindings[i].impl);
        if (result != SELENA_OK) {
            return result;
        }
    }

    return SELENA_OK;
}

int selena_activate_with_overrides(
    SelenaCategoryId category,
    const SelenaBinding *overrides,
    size_t override_count
) {
    int result = selena_activate(category);

    if (result != SELENA_OK) {
        return result;
    }

    return selena_apply_bindings(overrides, override_count);
}

SelenaCategoryId selena_active_category(void) {
    return g_active_category;
}

int selena_has_impl(SelenaPointId point, SelenaImplId impl) {
    if (!selena_validate_point(point) || !selena_validate_impl(impl)) {
        return 0;
    }

    return g_impl_table[point][impl] != NULL;
}

void *selena_get_impl(SelenaPointId point, SelenaImplId impl) {
    if (!selena_validate_point(point) || !selena_validate_impl(impl)) {
        return NULL;
    }

    return g_impl_table[point][impl];
}
