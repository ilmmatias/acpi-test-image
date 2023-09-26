#include <acpi.h>
#include <pmm.h>

// The following will be our kernel's entry point.
// If renaming _start() to something else, make sure to change the
// linker script accordingly.
void _start(void) {
    pmm_init();
    AcpiInitialize();
    for (;;)
        __asm__("hlt");
}
