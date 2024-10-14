#include <sys/acpi.h>
#include <memory/vmm.h>

static acpi_rsdp_t* acpi_rsdp = NULL;
static acpi_rsdt_t* acpi_rsdt = NULL;

static acpi_rsdp_t* acpi_find_rsdp();

int32_t acpi_init(void) {
    acpi_rsdp = acpi_find_rsdp();

    if(acpi_rsdp == NULL) {
        return -1;
    }

    void* acpi_rsdt_base = vmm_map_memory(NULL, sizeof(acpi_rsdt_t), (void*) acpi_rsdp->rsdt_address, true, false);
    acpi_rsdt = (acpi_rsdt_t*) ((uintptr_t) acpi_rsdt_base + VMM_OFFSET(acpi_rsdp->rsdt_address));

    if(memcmp(acpi_rsdt->sdt.signature, ACPI_RSDT_SIGNATURE, 4) != 0) {
        return -1;
    }

    return 0;
}

static acpi_rsdp_t* acpi_find_rsdp() {
    acpi_rsdp_t* rsdp = NULL;

    // EBDA real mode address is stored at 0x40E
    uint32_t ebda_location_address = 0x40E;
    // Transform the real mode address to a linear address
    ebda_location_address <<= 4;
    // Transform linear address to a virtual address
    ebda_location_address += VMM_LOWER_MEMORY_BASE;

    // Read the EBDA real mode address
    uint32_t ebda_address = *((uint16_t*) ebda_location_address);
    // Transform the real mode address to a linear address
    ebda_address <<= 4;
    // Transform linear address to a virtual address
    ebda_address += VMM_LOWER_MEMORY_BASE;

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
        for(uint32_t current_address = 0xE0000 + VMM_LOWER_MEMORY_BASE; current_address < 0xFFFFF + VMM_LOWER_MEMORY_BASE; current_address += 16) {
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
