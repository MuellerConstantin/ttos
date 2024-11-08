/**
 * @file kmessage.h
 * @brief Kernel message handling.
 * 
 * This file contains the definitions for the kernel message handling. The message handling is used to
 * write kernel log messages.
 */

#ifndef _KERNEL_SYSTEM_KMESSAGE_H
#define _KERNEL_SYSTEM_KMESSAGE_H

#include <util/linked_list.h>

#define KMESSAGE_LEVEL_PANIC "DEBUG"
#define KMESSAGE_LEVEL_INFO "INFO"
#define KMESSAGE_LEVEL_WARN "WARN"
#define KMESSAGE_LEVEL_ERROR "ERROR"

typedef struct kmessage_message kmessage_message_t;

struct kmessage_message {
    const char* level;
    const char* message;
};

/**
 * Initializes the kernel message handling. This function should be called before any other message
 * handling function.
 */
void kmessage_init();

/**
 * Writes a message to the kernel log.
 * 
 * @param level The level of the message.
 * @param message The message to write.
 */
void kmessage(const char* level, const char* message);

/**
 * Get the messages from the kernel log.
 * 
 * @return The messages from the kernel log.
 */
const linked_list_t* kmessage_get_messages();

#endif // _KERNEL_SYSTEM_KMESSAGE_H
