/* SPDX-FileCopyrightText: (C) 2023 ilmmatias
 * SPDX-License-Identifier: BSD-3-Clause */

#include <acpip.h>
#include <crt_impl.h>
#include <framebuffer.h>
#include <pmm.h>
#include <stdarg.h>
#include <string.h>

static volatile struct limine_rsdp_request rsdp_req = {.id = LIMINE_RSDP_REQUEST, .revision = 0};

typedef struct {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t rev;
    uint32_t rsdt_addr;
    /* ver 2.0 only */
    uint32_t length;
    uint64_t xsdt_addr;
    uint8_t ext_checksum;
    uint8_t reserved[3];
} __attribute__((packed)) rsdp_t;

/*-------------------------------------------------------------------------------------------------
 * PURPOSE:
 *     This function calculates and validates the checksum for a system table.
 *
 * PARAMETERS:
 *     Table - Pointer to the start of the table.
 *     Length - Size of the table.
 *
 * RETURN VALUE:
 *     1 if the checksum is valid, 0 otherwise.
 *-----------------------------------------------------------------------------------------------*/
static int Checksum(const char *Table, uint32_t Length) {
    uint8_t Sum = 0;

    while (Length--) {
        Sum += *(Table++);
    }

    return !Sum;
}

/*-------------------------------------------------------------------------------------------------
 * PURPOSE:
 *     This functin searches for a specific table inside the RSDT/XSDT.
 *
 * PARAMETERS:
 *     Signature - Signature of the required entry.
 *
 * RETURN VALUE:
 *     Pointer to the header of the entry, or NULL on failure.
 *-----------------------------------------------------------------------------------------------*/
SdtHeader *AcpipFindTable(char Signature[4], int Index) {
    /* DSDT is contained inside the XDsdt (or the Dsdt) field of the FADT; Other than that, just
       do a linear search on the R/XSDT. */

    if (rsdp_req.response == NULL) {
        return NULL;
    }

    rsdp_t *rsdp = rsdp_req.response->address;
    int IsXsdt = rsdp->rev >= 2 && rsdp->xsdt_addr;

    SdtHeader *Header =
        (SdtHeader *)((IsXsdt ? rsdp->xsdt_addr : rsdp->rsdt_addr) + pmm_hhdm_req.response->offset);

    uint32_t *RsdtTables = (uint32_t *)(Header + 1);
    uint64_t *XsdtTables = (uint64_t *)(Header + 1);

    if (!memcmp(Signature, "DSDT", 4)) {
        FadtHeader *Fadt = (FadtHeader *)AcpipFindTable("FACP", 0);
        if (!Header) {
            return NULL;
        }

        Header = (SdtHeader *)((IsXsdt && Fadt->XDsdt ? Fadt->XDsdt : Fadt->Dsdt) +
                               pmm_hhdm_req.response->offset);
        if (!Checksum((char *)Header, Header->Length) || memcmp(Header->Signature, "DSDT", 4)) {
            AcpipShowErrorMessage(ACPI_REASON_CORRUPTED_TABLES, "invalid DSDT table\n");
        }

        return Header;
    }

    if (memcmp(Header->Signature, IsXsdt ? "XSDT" : "RSDT", 4) ||
        !Checksum((char *)Header, Header->Length)) {
        AcpipShowErrorMessage(ACPI_REASON_CORRUPTED_TABLES, "invalid R/XSDT table\n");
    }

    int Occourances = 0;
    for (uint32_t i = 0; i < (Header->Length - sizeof(SdtHeader)) / (IsXsdt ? 8 : 4); i++) {
        SdtHeader *Header =
            (SdtHeader *)((IsXsdt ? XsdtTables[i] : RsdtTables[i]) + pmm_hhdm_req.response->offset);

        if (!Checksum((char *)Header, Header->Length)) {
            continue;
        } else if (!memcmp(Header->Signature, Signature, 4) && Occourances++ == Index) {
            return Header;
        }
    }

    return NULL;
}

/*-------------------------------------------------------------------------------------------------
 * PURPOSE:
 *     Wrapper around putc for __vprint.
 *
 * PARAMETERS:
 *     Buffer - What we need to display.
 *     Size - Size of that data; The data is not required to be NULL terminated, so this need to be
 *            taken into account!
 *     Context - Always NULL for us.
 *
 * RETURN VALUE:
 *     None.
 *-----------------------------------------------------------------------------------------------*/
static void PutBuffer(const void *buffer, int size, void *context) {
    (void)context;
    for (int i = 0; i < size; i++) {
        putc(((const char *)buffer)[i]);
    }
}

/*-------------------------------------------------------------------------------------------------
 * PURPOSE:
 *     This function shows a debug message to the screen.
 *
 * PARAMETERS:
 *     Message - Format string; Works the same as printf().
 *     ... - Variadic arguments.
 *
 * RETURN VALUE:
 *     None.
 *-----------------------------------------------------------------------------------------------*/
void AcpipShowDebugMessage(const char *Format, ...) {
    va_list vlist;
    va_start(vlist, Format);

    puts("ACPI Debug: ");
    __vprintf(Format, vlist, NULL, PutBuffer);

    va_end(vlist);
}

/*-------------------------------------------------------------------------------------------------
 * PURPOSE:
 *     This function halts the system with the given reason, printing a debug message to the
 *     screen if possible.
 *
 * PARAMETERS:
 *     Reason - What went wrong.
 *     Message - Format string; Works the same as printf().
 *     ... - Variadic arguments.
 *
 * RETURN VALUE:
 *     Does not return.
 *-----------------------------------------------------------------------------------------------*/
[[noreturn]] void AcpipShowErrorMessage(int Reason, const char *Format, ...) {
    (void)Reason;
    va_list vlist;
    va_start(vlist, Format);

    puts("ACPI Error: ");
    __vprintf(Format, vlist, NULL, PutBuffer);

    va_end(vlist);

    for (;;)
        __asm__("hlt");
}
