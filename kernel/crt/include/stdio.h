/* SPDX-FileCopyrightText: (C) 2023 ilmmatias
 * SPDX-License-Identifier: BSD-3-Clause */

#ifndef STDIO_H
#define STDIO_H

#define __CRT_STDIO_H
#include <crt_impl.h>
#include <stdarg.h>
#include <stddef.h>
#undef __CRT_STDIO_H

int snprintf(char *buffer, size_t bufsz, const char *format, ...);
int vsnprintf(char *buffer, size_t bufsz, const char *format, va_list vlist);

#endif /* STDIO_H */
