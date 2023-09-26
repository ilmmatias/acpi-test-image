/* Taken from LAI's test boot image, available at: https://github.com/managarm/lai_tools */

#include <bit.h>
#include <limine.h>

volatile struct limine_hhdm_request pmm_hhdm_req = {.id = LIMINE_HHDM_REQUEST};
static volatile struct limine_memmap_request memmap_req = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0};

static void *bitmap;
static size_t last_used_index = 0;
static uintptr_t highest_addr = 0;

void pmm_init(void) {
    struct limine_memmap_entry **memmap = memmap_req.response->entries;
    size_t memmap_entries = memmap_req.response->entry_count;

    // First, calculate how big the bitmap needs to be.
    for (size_t i = 0; i < memmap_entries; i++) {
        if (memmap[i]->type != LIMINE_MEMMAP_USABLE &&
            memmap[i]->type != LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE)
            continue;

        uintptr_t top = memmap[i]->base + memmap[i]->length;

        if (top > highest_addr)
            highest_addr = top;
    }

    size_t bitmap_size = (((highest_addr >> 12) >> 3) + 4095) & ~0xFFF;

    // Second, find a location with enough free pages to host the bitmap.
    for (size_t i = 0; i < memmap_entries; i++) {
        if (memmap[i]->type != LIMINE_MEMMAP_USABLE)
            continue;

        if (memmap[i]->length >= bitmap_size) {
            bitmap = (void *)(memmap[i]->base + pmm_hhdm_req.response->offset);

            // Initialise entire bitmap to 1 (non-free)
            memset(bitmap, 0xff, bitmap_size);

            memmap[i]->length -= bitmap_size;
            memmap[i]->base += bitmap_size;

            break;
        }
    }

    // Third, populate free bitmap entries according to memory map.
    for (size_t i = 0; i < memmap_entries; i++) {
        if (memmap[i]->type != LIMINE_MEMMAP_USABLE)
            continue;

        for (uintptr_t j = 0; j < memmap[i]->length; j += 4096)
            bitmap_unset(bitmap, (memmap[i]->base + j) >> 12);
    }
}

static void *inner_alloc(size_t count, size_t limit) {
    size_t p = 0;

    while (last_used_index < limit) {
        if (!bitmap_test(bitmap, last_used_index++)) {
            if (++p == count) {
                size_t page = last_used_index - count;
                for (size_t i = page; i < last_used_index; i++) {
                    bitmap_set(bitmap, i);
                }
                return (void *)(page << 12);
            }
        } else {
            p = 0;
        }
    }

    return NULL;
}

void *pmm_alloc(size_t count) {
    size_t l = last_used_index;
    void *ret = inner_alloc(count, highest_addr >> 12);
    if (ret == NULL) {
        last_used_index = 0;
        ret = inner_alloc(count, l);
    }

    return ret;
}

void *pmm_allocz(size_t count) {
    char *ret = (char *)pmm_alloc(count);

    if (ret == NULL)
        return NULL;

    uint64_t *ptr = (uint64_t *)(ret + pmm_hhdm_req.response->offset);

    for (size_t i = 0; i < count * (4096 / sizeof(uint64_t)); i++)
        ptr[i] = 0;

    return ret;
}

void pmm_free(void *ptr, size_t count) {
    size_t page = (size_t)ptr >> 12;
    for (size_t i = page; i < page + count; i++)
        bitmap_unset(bitmap, i);
}
