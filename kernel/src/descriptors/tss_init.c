#include <descriptors/tss.h>

static tss_segment_descriptor_t tss;

extern void tss_flush();

void tss_init(uint16_t ss0, uintptr_t esp0) {
    uintptr_t tss_base = (uintptr_t) &tss;
    uintptr_t tss_limit = sizeof(tss);

    gdt_init_descriptor(GDT_TASK_STATE_SELEKTOR, tss_base, tss_limit,
        GDT_ACCESS_PRESENT|GDT_ACCESS_RING3|GDT_ACCESS_EXECUTABLE|GDT_ACCESS_ACCESSED,
        0x00);
    
    memset(&tss, 0, sizeof(tss));

    tss.ss0 = ss0;
    tss.esp0 = esp0;

    /*
     * Initialize segment registers used when processor switches to kernel mode. We use basically our
     * kernel data segment (offset 0x10) and kernel code segment (offset 0x08). But we also need to
     * set the last two bits (Request Privilege Level) for ring 3, so we get 0x13 for data segment
     * and 0x0B for code segment.
     */

    tss.cs = 0x0B;
    tss.ss = 0x13;
    tss.ds = 0x13;
    tss.es = 0x13;
    tss.fs = 0x13;
    tss.gs = 0x13;

    tss_flush();
}

void tss_update_stack(uint16_t ss0, uintptr_t esp0) {
    tss.ss0 = ss0;
    tss.esp0 = esp0;
}
