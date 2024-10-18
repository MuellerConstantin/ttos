#include <system/kmessage.h>
#include <system/kpanic.h>
#include <memory/kheap.h>

static linked_list_t* kmessage_messages = NULL;

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

    kmessage_message->level = KMESSAGE_LEVEL_INFO;
    kmessage_message->message = message;

    linked_list_node_t* node = linked_list_create_node(kmessage_message);

    if(node == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, KPANIC_KHEAP_OUT_OF_MEMORY_CODE, NULL);
    }

    linked_list_append(kmessage_messages, node);
}
