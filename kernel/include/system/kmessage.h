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

/*
 * Colors the log is drawn in while it is mirrored to the screen. Only the cells
 * the text lands in are painted, so the background matches the cleared screen
 * rather than leaving a colored block behind every line.
 */
#define KMESSAGE_ECHO_FOREGROUND 0x0F
#define KMESSAGE_ECHO_BACKGROUND 0x00

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

/**
 * Starts writing the kernel log to the screen as it is written, beginning with
 * everything logged so far.
 *
 * Until userland comes up there is nothing to read the log with, so a kernel
 * that stops during boot leaves an empty screen and no way to tell how far it
 * got. Echoing the log turns that into the last line before the stall. Needs
 * the video driver, so it cannot be turned on before that is initialized.
 */
void kmessage_echo_enable();

/**
 * Stops writing the kernel log to the screen, for when the console takes the
 * screen over.
 */
void kmessage_echo_disable();

#endif // _KERNEL_SYSTEM_KMESSAGE_H
