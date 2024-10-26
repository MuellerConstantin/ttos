#include <arch/i386/acpi.h>
#include <memory/vmm.h>
#include <memory/kheap.h>
#include <system/ports.h>
#include <system/kpanic.h>
#include <system/kmessage.h>

static acpi_poweroff_information_t acpi_poweroff_info;

static acpi_rsdp_t* acpi_rsdp = NULL;
static acpi_rsdt_t* acpi_rsdt = NULL;
static acpi_fadt_t* acpi_fadt = NULL;
static acpi_dsdt_t* acpi_dsdt = NULL;
static acpi_rsdt_t** acpi_sdts = NULL;
static uint32_t acpi_sdts_count = 0;

static acpi_rsdp_t* acpi_find_rsdp();
static void acpi_init_poweroff();

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

    sprintf(kernel_message, "acpi: ACPI supported with Version %d", acpi_rsdp->revision);

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

    uint8_t* aml_pointer = acpi_dsdt->aml_definitions;

    for(size_t index = 0; index < acpi_dsdt->sdt.length - sizeof(acpi_dsdt_t); index++) {
        if(memcmp(aml_pointer, "_S5_", 4) == 0) {
            break;
        }

        aml_pointer++;
    }

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

    sprintf(kernel_message_pm1, "acpi: PM1A_CNT: %x SLP_TYP_A: %x", acpi_poweroff_info.pm1a_cnt, acpi_poweroff_info.slp_type_a);

    kmessage(KMESSAGE_LEVEL_INFO, kernel_message_pm1);

    sprintf(kernel_message_pm2, "acpi: PM1B_CNT: %x SLP_TYP_B: %x", acpi_poweroff_info.pm1b_cnt, acpi_poweroff_info.slp_type_b);

    kmessage(KMESSAGE_LEVEL_INFO, kernel_message_pm2);
}
