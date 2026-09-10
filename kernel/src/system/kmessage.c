#include <system/kmessage.h>
#include <system/kpanic.h>
#include <memory/kheap.h>
#include <drivers/video/vga/tm.h>
#include <util/string.h>

static linked_list_t* kmessage_messages = NULL;

/** Whether the log is currently mirrored to the screen, and how far down it got. */
static bool kmessage_echo = false;
static size_t kmessage_echo_row = 0;

static void kmessage_echo_message(const kmessage_message_t* message);

void kmessage_init() {
    kmessage_messages = linked_list_create();

    if(kmessage_messages == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, KPANIC_KHEAP_OUT_OF_MEMORY_CODE, NULL);
    }
}

const linked_list_t* kmessage_get_messages() {
    return kmessage_messages;
}

void kmessage(const char* level, const char* message) {
    kmessage_message_t* kmessage_message = kmalloc(sizeof(kmessage_message_t));

    if(kmessage_message == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, KPANIC_KHEAP_OUT_OF_MEMORY_CODE, NULL);
    }

    kmessage_message->level = level;
    kmessage_message->message = message;

    linked_list_node_t* node = linked_list_create_node(kmessage_message);

    if(node == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, KPANIC_KHEAP_OUT_OF_MEMORY_CODE, NULL);
    }

    linked_list_append(kmessage_messages, node);

    if(kmessage_echo) {
        kmessage_echo_message(kmessage_message);
    }
}

void kmessage_echo_enable() {
    kmessage_echo = true;

    // The interesting part of a boot is usually over before the screen exists.
    linked_list_foreach(kmessage_messages, node) {
        kmessage_echo_message((kmessage_message_t*) node->data);
    }
}

void kmessage_echo_disable() {
    kmessage_echo = false;
}

/**
 * Puts one message on the screen, scrolling to make room for it. A message
 * wider than the screen wraps, and takes as many rows as it wraps into.
 */
static void kmessage_echo_message(const kmessage_message_t* message) {
    size_t columns = vga_tm_total_columns();
    size_t rows = vga_tm_total_rows();

    if(columns == 0 || rows == 0) {
        return;
    }

    size_t length = strlen(message->level) + strlen(message->message) + 3;
    size_t used_rows = (length + columns - 1) / columns;

    while(kmessage_echo_row + used_rows > rows && kmessage_echo_row > 0) {
        vga_tm_scroll(KMESSAGE_ECHO_FOREGROUND, KMESSAGE_ECHO_BACKGROUND);

        kmessage_echo_row--;
    }

    size_t offset = kmessage_echo_row * columns;

    vga_tm_strwrite(offset, "[", KMESSAGE_ECHO_FOREGROUND, KMESSAGE_ECHO_BACKGROUND);
    vga_tm_strwrite(offset + 1, message->level, KMESSAGE_ECHO_FOREGROUND, KMESSAGE_ECHO_BACKGROUND);
    vga_tm_strwrite(offset + 1 + strlen(message->level), "] ", KMESSAGE_ECHO_FOREGROUND, KMESSAGE_ECHO_BACKGROUND);
    vga_tm_strwrite(offset + 3 + strlen(message->level), message->message, KMESSAGE_ECHO_FOREGROUND, KMESSAGE_ECHO_BACKGROUND);

    kmessage_echo_row += used_rows;
}
