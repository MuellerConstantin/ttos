#ifndef _KERNEL_SYS_ACPI_H
#define _KERNEL_SYS_ACPI_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define ACPI_RSDP_SIGNATURE "RSD PTR "
#define ACPI_RSDT_SIGNATURE "RSDT"

typedef struct acpi_rsdp acpi_rsdp_t;
typedef struct acpi_sdt acpi_sdt_t;
typedef struct acpi_rsdt acpi_rsdt_t;

struct acpi_rsdp {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
} __attribute__((packed));

struct acpi_sdt {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed));

struct acpi_rsdt {
    acpi_sdt_t sdt;
    uint32_t sdt_entries[];
} __attribute__((packed));

/**
 * Initializes the ACPI subsystem.
 * 
 * @return 0 on success, -1 on failure.
 */
int32_t acpi_init(void);

#endif // _KERNEL_SYS_ACPI_H
