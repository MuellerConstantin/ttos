#ifndef _KERNEL_SYS_ACPI_H
#define _KERNEL_SYS_ACPI_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#define ACPI_RSDP_SIGNATURE "RSD PTR "
#define ACPI_RSDT_SIGNATURE "RSDT"
#define ACPI_FADT_SIGNATURE "FACP"
#define ACPI_DSDT_SIGNATURE "DSDT"

#define ACPI_SCI_ENABLE  (1)

// Poweroff definitions

#define ACPI_SLP_EN_CODE                            (1 << 13)
#define ACPI_AML_PACKAGE_OP_CODE                    (0x12)
#define ACPI_AML_NAME_OP_CODE                       (0x8)
#define ACPI_AML_BYTE_PREFIX_CODE                   (0xA)
#define ACPI_AMI_PACKAGE_LENGTH_ENCODING_BITS_MASK  (0xC0)
#define ACPI_AMI_PACKAGE_LENGTH_ENCODING_BITS_SHIFT (6)
#define ACPI_AML_MIN_PACKAGE_LENGTH                 (1)
#define ACPI_AML_NUM_ELEMENTS_LENGTH                (1)
#define ACPI_AML_S5_PACKET_LENGTH_OFFSET            (5)
#define ACPI_AML_SLP_TYPA_SHIFT                     (10)
#define ACPI_AML_SLP_TYPB_SHIFT                     (10)

typedef struct acpi_poweroff_information acpi_poweroff_information_t;

struct acpi_poweroff_information {
    uint8_t slp_type_a;
    uint8_t slp_type_b;
    uint32_t pm1a_cnt;
    uint32_t pm1b_cnt;
};

typedef struct acpi_rsdp acpi_rsdp_t;
typedef struct acpi_sdt acpi_sdt_t;
typedef struct acpi_rsdt acpi_rsdt_t;
typedef struct acpi_fadt_generic_address_structure acpi_fadt_generic_address_structure_t;
typedef struct acpi_fadt acpi_fadt_t;
typedef struct acpi_dsdt acpi_dsdt_t;

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

struct acpi_fadt {
    acpi_sdt_t sdt;
    uint32_t firmware_ctrl;
    uint32_t dsdt;
    uint8_t reserved;
    uint8_t preferred_pm_profile;
    uint16_t sci_int;
    uint32_t smi_cmd;
    uint8_t acpi_enable;
    uint8_t acpi_disable;
    uint8_t s4bios_req;
    uint8_t pstate_cnt;
    uint32_t pm1a_evt_blk;
    uint32_t pm1b_evt_blk;
    uint32_t pm1a_cnt_blk;
    uint32_t pm1b_cnt_blk;
    uint32_t pm2_cnt_blk;
    uint32_t pm_tmr_blk;
    uint32_t gpe0_blk;
    uint32_t gpe1_blk;
    uint8_t pm1_evt_len;
    uint8_t pm1_cnt_len;
    uint8_t pm2_cnt_len;
    uint8_t pm_tmr_len;
    uint8_t gpe0_blk_len;
    uint8_t gpe1_blk_len;
    uint8_t gpe1_base;
    uint8_t cst_cnt;
    uint16_t p_lvl2_lat;
    uint16_t p_lvl3_lat;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t duty_offset;
    uint8_t duty_width;
    uint8_t day_alarm;
    uint8_t mon_alarm;
    uint8_t century;
    uint16_t iapc_boot_arch;
    uint8_t reserved2;
    uint32_t flags;
    uint8_t reset_reg[12];
    uint8_t reset_value;
    uint8_t reserved3[3];
    uint64_t x_firmware_ctrl;
    uint64_t x_dsdt;
    uint8_t x_pm1a_evt_blk[12];
    uint8_t x_pm1b_evt_blk[12];
    uint8_t x_pm1a_cnt_blk[12];
    uint8_t x_pm1b_cnt_blk[12];
    uint8_t x_pm2_cnt_blk[12];
    uint8_t x_pm_tmr_blk[12];
    uint8_t x_gpe0_blk[12];
    uint8_t x_gpe1_blk[12];
} __attribute__((packed));

struct acpi_dsdt {
    acpi_sdt_t sdt;
    uint8_t aml_definitions[];
} __attribute__((packed));

/**
 * Initializes the ACPI subsystem.
 * 
 * @return 0 on success, -1 on failure.
 */
int32_t acpi_init(void);

/**
 * Powers off the system using ACPI.
 * 
 * @return 0 on success, -1 on failure.
 */
int32_t acpi_poweroff();

#endif // _KERNEL_SYS_ACPI_H
