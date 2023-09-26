/* SPDX-FileCopyrightText: (C) 2023 ilmmatias
 * SPDX-License-Identifier: BSD-3-Clause */

#ifndef STDLIB_H
#define STDLIB_H

#include <limits.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

long strtol(const char *str, char **str_end, int base);
unsigned long long strtoull(const char *str, char **str_end, int base);

void *malloc(size_t size);
void *calloc(size_t num, size_t size);
void free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* STDLIB_H */
