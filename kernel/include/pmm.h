#ifndef __PMM_H__
#define __PMM_H__

#include <limine.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern volatile struct limine_hhdm_request pmm_hhdm_req;

void *pmm_alloc(size_t);
void *pmm_allocz(size_t);
void pmm_free(void *, size_t);
void pmm_init(void);

#endif
