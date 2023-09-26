/* SPDX-FileCopyrightText: (C) 2023 ilmmatias
 * SPDX-License-Identifier: BSD-3-Clause */

#ifndef CRT_IMPL_H
#define CRT_IMPL_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#ifndef __CRT_STDIO_H

int __vprintf(
    const char *format,
    va_list vlist,
    void *context,
    void (*put_buf)(const void *buffer, int size, void *context));

#endif /* __CRT_STDIO_H */

#endif /* CRT_IMPL_H */
