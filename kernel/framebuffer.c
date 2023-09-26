#include <flanterm/backends/fb.h>
#include <limine.h>
#include <pmm.h>

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0};

void *alloc_pages(size_t size) {
    if (size == 0)
        return (void *)0x8000000000000000;
    return pmm_allocz((size + 4095) >> 12) + pmm_hhdm_req.response->offset;
}

static void free_pages(void *p, size_t oldsize) {
    if (oldsize == 0)
        return;
    pmm_free(p - pmm_hhdm_req.response->offset, (oldsize + 4095) >> 12);
}

void putc(char ch) {
    if (framebuffer_request.response == NULL) {
        return;
    }

    if (framebuffer_request.response->framebuffer_count < 1) {
        return;
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

    static struct flanterm_context *term = NULL;
    if (term == NULL) {
        term = flanterm_fb_init(
            alloc_pages,
            free_pages,
            fb->address,
            fb->width,
            fb->height,
            fb->pitch,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            0,
            0,
            1,
            1,
            1,
            0);
    }

    flanterm_write(term, &ch, 1);
}

void puts(const char *string) {
    while (*string) {
        putc(*(string++));
    }
}
