#include <arch/i386/acpi.h>
#include <memory/vmm.h>
#include <memory/kheap.h>
#include <system/ports.h>
#include <system/kpanic.h>
#include <system/kmessage.h>
#include <util/string.h>

static acpi_poweroff_information_t acpi_poweroff_info;

static acpi_rsdp_t* acpi_rsdp = NULL;
static acpi_rsdt_t* acpi_rsdt = NULL;
static acpi_fadt_t* acpi_fadt = NULL;
static acpi_dsdt_t* acpi_dsdt = NULL;
static acpi_rsdt_t** acpi_sdts = NULL;
static uint32_t acpi_sdts_count = 0;

static acpi_rsdp_t* acpi_find_rsdp();
static void acpi_init_poweroff();
static void acpi_log_bytes(const char* prefix, const uint8_t* bytes, size_t count);

int32_t acpi_init(void) {
    kmessage(KMESSAGE_LEVEL_INFO, "acpi: Initializing ACPI...");

    acpi_rsdp = acpi_find_rsdp();

    if(acpi_rsdp == NULL) {
        kmessage(KMESSAGE_LEVEL_WARN, "acpi: RSDP not found, ACPI is not supported");
        return -1;
    }

    void* acpi_rsdt_base = vmm_map_memory(NULL, sizeof(acpi_rsdt_t), (void*) acpi_rsdp->rsdt_address, true, false);
    acpi_rsdt = (acpi_rsdt_t*) ((uintptr_t) acpi_rsdt_base + VMM_ALIGN_OFFSET(acpi_rsdp->rsdt_address));

    if(memcmp(acpi_rsdt->sdt.signature, ACPI_RSDT_SIGNATURE, 4) != 0) {
        kmessage(KMESSAGE_LEVEL_WARN, "acpi: RSDT signature is invalid, ACPI is not supported");
        return -1;
    }

    char* kernel_message = kmalloc(64);

    if(!kernel_message) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    strfmt(kernel_message, "acpi: ACPI supported with Version %d", acpi_rsdp->revision);

    kmessage(KMESSAGE_LEVEL_INFO, kernel_message);

    acpi_sdts_count = (acpi_rsdt->sdt.length - sizeof(acpi_sdt_t)) / sizeof(uint32_t);
    acpi_sdts = kcalloc(acpi_sdts_count, sizeof(acpi_rsdt_t*));

    for(uint8_t index = 0; index < acpi_sdts_count; index++) {
        void* acpi_sdt_tmp_base = vmm_map_memory(NULL, sizeof(acpi_rsdt_t), (void*) acpi_rsdt->sdt_entries[index], true, false);
        acpi_sdt_t* current_sdt = (acpi_sdt_t*) ((uintptr_t) acpi_sdt_tmp_base + VMM_ALIGN_OFFSET(acpi_rsdt->sdt_entries[index]));

        uint32_t current_sdt_length = current_sdt->length;

        vmm_unmap_memory(acpi_sdt_tmp_base, sizeof(acpi_rsdt_t));

        void* acpi_sdt_base = vmm_map_memory(NULL, current_sdt_length, (void*) acpi_rsdt->sdt_entries[index], true, false);
        acpi_sdts[index] = (acpi_rsdt_t*) ((uintptr_t) acpi_sdt_base + VMM_ALIGN_OFFSET(acpi_rsdt->sdt_entries[index]));

        if(memcmp(acpi_sdts[index]->sdt.signature, ACPI_FADT_SIGNATURE, strlen(ACPI_FADT_SIGNATURE)) == 0) {
            acpi_fadt = (acpi_fadt_t*) acpi_sdts[index];

            void* acpi_dsdt_base = vmm_map_memory(NULL, sizeof(acpi_dsdt_t), (void*) acpi_fadt->dsdt, true, false);
            acpi_dsdt = (acpi_dsdt_t*) ((uintptr_t) acpi_dsdt_base + VMM_ALIGN_OFFSET(acpi_fadt->dsdt));
        }
    }

    kmessage(KMESSAGE_LEVEL_INFO, "acpi: Initializing poweroff...");

    acpi_init_poweroff();

    return 0;
}

int32_t acpi_poweroff() {
    if(acpi_poweroff_info.pm1a_cnt == 0) {
        return -1;
    }

    outw(acpi_poweroff_info.pm1a_cnt, acpi_poweroff_info.slp_type_a | ACPI_SLP_EN_CODE);

    if(acpi_poweroff_info.pm1b_cnt != 0) {
        outw(acpi_poweroff_info.pm1b_cnt, acpi_poweroff_info.slp_type_b | ACPI_SLP_EN_CODE);
    }
}

static acpi_rsdp_t* acpi_find_rsdp() {
    acpi_rsdp_t* rsdp = NULL;

    // EBDA real mode address is stored at 0x40E
    uint32_t ebda_location_address = 0x40E;
    // Transform the real mode address to a linear address
    ebda_location_address <<= 4;
    // Transform linear address to a virtual address
    ebda_location_address += VMM_REAL_MODE_MEMORY_BASE;

    // Read the EBDA real mode address
    uint32_t ebda_address = *((uint16_t*) ebda_location_address);
    // Transform the real mode address to a linear address
    ebda_address <<= 4;
    // Transform linear address to a virtual address
    ebda_address += VMM_REAL_MODE_MEMORY_BASE;

    // Search for the RSDP in the EBDA
    for(uint32_t current_address = ebda_address; current_address < ebda_address + 0x1000; current_address += 16) {
        acpi_rsdp_t* current_rsdp = (acpi_rsdp_t*) current_address;

        if(memcmp(current_rsdp->signature, ACPI_RSDP_SIGNATURE, 8) == 0) {
            rsdp = current_rsdp;
            break;
        }
    }

    if(rsdp == NULL) {
        // Search for the RSDP in the BIOS ROM
        for(uint32_t current_address = 0xE0000 + VMM_REAL_MODE_MEMORY_BASE; current_address < 0xFFFFF + VMM_REAL_MODE_MEMORY_BASE; current_address += 16) {
            acpi_rsdp_t* current_rsdp = (acpi_rsdp_t*) current_address;

            if(memcmp(current_rsdp->signature, ACPI_RSDP_SIGNATURE, 8) == 0) {
                rsdp = current_rsdp;
                break;
            }
        }
    }

    if(rsdp != NULL) {
        // Check RSDP revision

        if(rsdp->revision != 0) {
            return NULL;
        }

        // Check RSDP checksum

        uint8_t checksum = 0;

        for(size_t index = 0; index < sizeof(acpi_rsdp_t); index++) {
            checksum += ((uint8_t*) rsdp)[index];
        }

        if(checksum != 0) {
            return NULL;
        }

        return rsdp;
    }

    return NULL;
}

static void acpi_init_poweroff() {
    if(acpi_dsdt == NULL || acpi_fadt == NULL) {
        kmessage(KMESSAGE_LEVEL_WARN, "acpi: DSDT or FADT not found, poweroff is not supported");
        return;
    }

    char* kernel_message_dsdt = kmalloc(64);

    if(!kernel_message_dsdt) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    strfmt(kernel_message_dsdt, "acpi: DSDT at %x, length %d", acpi_fadt->dsdt, acpi_dsdt->sdt.length);

    kmessage(KMESSAGE_LEVEL_INFO, kernel_message_dsdt);

    /*
     * Whether the firmware still owns the power management registers: SCI_EN
     * in PM1a_CNT is set once ACPI mode is on, and SMI_CMD/ACPI_ENABLE are
     * how it is asked for.
     */
    char* kernel_message_mode = kmalloc(64);

    if(!kernel_message_mode) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    strfmt(kernel_message_mode, "acpi: SMI_CMD: %x ACPI_ENABLE: %x SCI_EN: %d", acpi_fadt->smi_cmd, acpi_fadt->acpi_enable,
           acpi_fadt->pm1a_cnt_blk != 0 ? (inw(acpi_fadt->pm1a_cnt_blk) & ACPI_SCI_ENABLE) : 0);

    kmessage(KMESSAGE_LEVEL_INFO, kernel_message_mode);

    uint8_t* aml_pointer = acpi_dsdt->aml_definitions;
    size_t aml_length = acpi_dsdt->sdt.length - sizeof(acpi_dsdt_t);
    size_t aml_offset = 0;

    while(aml_offset < aml_length && memcmp(aml_pointer, "_S5_", 4) != 0) {
        aml_pointer++;
        aml_offset++;
    }

    if(aml_offset >= aml_length) {
        kmessage(KMESSAGE_LEVEL_WARN, "acpi: _S5_ not found in DSDT, poweroff is not supported");
        return;
    }

    char* kernel_message_s5 = kmalloc(64);

    if(!kernel_message_s5) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    // The two bytes before the name and the package behind it are what the parser judges by.
    strfmt(kernel_message_s5, "acpi: _S5_ at DSDT offset %d:", aml_offset + sizeof(acpi_dsdt_t));
    acpi_log_bytes(kernel_message_s5, aml_pointer - 2, 16);

    // Validate the AML pointer
    if ((*(aml_pointer - 1) == ACPI_AML_NAME_OP_CODE || ( *(aml_pointer - 2) == ACPI_AML_NAME_OP_CODE && *(aml_pointer - 1) == '\\')) && *(aml_pointer + 4) == ACPI_AML_PACKAGE_OP_CODE) {
        acpi_poweroff_info.pm1a_cnt = acpi_fadt->pm1a_cnt_blk;
        acpi_poweroff_info.pm1b_cnt = acpi_fadt->pm1b_cnt_blk;

        // Skip NameOp, Name and PackageOp
        aml_pointer += ACPI_AML_S5_PACKET_LENGTH_OFFSET;
        // Skip Package length
		aml_pointer += ((*aml_pointer & ACPI_AMI_PACKAGE_LENGTH_ENCODING_BITS_MASK) >> ACPI_AMI_PACKAGE_LENGTH_ENCODING_BITS_SHIFT) + ACPI_AML_MIN_PACKAGE_LENGTH + ACPI_AML_NUM_ELEMENTS_LENGTH;

        if(*aml_pointer == ACPI_AML_BYTE_PREFIX_CODE) {
            aml_pointer++;
        }

        acpi_poweroff_info.slp_type_a = (*aml_pointer) << ACPI_AML_SLP_TYPA_SHIFT;

        aml_pointer++;

        if(*aml_pointer == ACPI_AML_BYTE_PREFIX_CODE) {
            aml_pointer++;
        }

        acpi_poweroff_info.slp_type_b = (*aml_pointer) << ACPI_AML_SLP_TYPB_SHIFT;
    }

    kmessage(KMESSAGE_LEVEL_INFO, "acpi: Poweroff initialized");

    char* kernel_message_pm1 = kmalloc(64);
    char* kernel_message_pm2 = kmalloc(64);

    if(!kernel_message_pm1) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    if(!kernel_message_pm2) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    strfmt(kernel_message_pm1, "acpi: PM1A_CNT: %x SLP_TYP_A: %x", acpi_poweroff_info.pm1a_cnt, acpi_poweroff_info.slp_type_a);

    kmessage(KMESSAGE_LEVEL_INFO, kernel_message_pm1);

    strfmt(kernel_message_pm2, "acpi: PM1B_CNT: %x SLP_TYP_B: %x", acpi_poweroff_info.pm1b_cnt, acpi_poweroff_info.slp_type_b);

    kmessage(KMESSAGE_LEVEL_INFO, kernel_message_pm2);
}

/** Logs a run of bytes as hex behind a prefix, one entry, for tables that are only readable as raw AML. */
static void acpi_log_bytes(const char* prefix, const uint8_t* bytes, size_t count) {
    static const char digits[] = "0123456789abcdef";
    char* kernel_message = kmalloc(strlen(prefix) + count * 3 + 1);

    if(!kernel_message) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    strcpy(kernel_message, prefix);

    char* cursor = kernel_message + strlen(prefix);

    for(size_t index = 0; index < count; index++) {
        *cursor++ = ' ';
        *cursor++ = digits[bytes[index] >> 4];
        *cursor++ = digits[bytes[index] & 0x0F];
    }

    *cursor = '\0';

    kmessage(KMESSAGE_LEVEL_INFO, kernel_message);
}
