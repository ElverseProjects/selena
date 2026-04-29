/*
 * selena.h
 *
 *  Created on: 19 мар. 2026 г.
 *      Author: Matvey
 */

#ifndef SELENA_H_
#define SELENA_H_

#include <stddef.h>

extern sln_err_t selena_init(const char* config_path);
extern sln_err_t selena_register_impl(const char* label, const char* func_name)

#endif /* SELENA_H_ */
