#ifndef _KERNEL_DRIVERS_PCI_TYPES_H
#define _KERNEL_DRIVERS_PCI_TYPES_H

#define PCI_TYPE_UNDEFINED 0x00
#define PCI_TYPE_MASS_STORAGE_CONTROLLER 0x01
#define PCI_TYPE_NETWORK_CONTROLLER 0x02
#define PCI_TYPE_DISPLAY_CONTROLLER 0x03
#define PCI_TYPE_MULTIMEDIA_CONTROLLER 0x04
#define PCI_TYPE_MEMORY_CONTROLLER 0x05
#define PCI_TYPE_BRIDGE_DEVICE 0x06
#define PCI_TYPE_SIMPLE_COMMUNICATIONS_CONTROLLER 0x07
#define PCI_TYPE_BASE_SYSTEM_PERIPHERAL 0x08
#define PCI_TYPE_INPUT_DEVICE_CONTROLLER 0x09
#define PCI_TYPE_DOCKING_STATION 0x0A
#define PCI_TYPE_PROCESSOR 0x0B
#define PCI_TYPE_SERIAL_BUS_CONTROLLER 0x0C
#define PCI_TYPE_WIRELESS_CONTROLLER 0x0D
#define PCI_TYPE_INTELLIGENT_IO_CONTROLLER 0x0E
#define PCI_TYPE_SATELLITE_COMMUNICATIONS_CONTROLLER 0x0F
#define PCI_TYPE_ENCRYPTION_CONTROLLER 0x10
#define PCI_TYPE_SIGNAL_PROCESSING_CONTROLLER 0x11
#define PCI_TYPE_PROCESSING_ACCELERATOR 0x12
#define PCI_TYPE_NON_ESSENTIAL_INSTRUMENTATION 0x13
#define PCI_TYPE_COPROCESSOR 0x40
#define PCI_TYPE_UNASSIGNED 0xFF

// Subtypes for PCI_TYPE_MASS_STORAGE_CONTROLLER

#define PCI_SUBTYPE_SCSI_BUS_CONTROLLER 0x00
#define PCI_SUBTYPE_IDE_CONTROLLER 0x01
#define PCI_SUBTYPE_FLOPPY_DISK_CONTROLLER 0x02
#define PCI_SUBTYPE_IPI_BUS_CONTROLLER 0x03
#define PCI_SUBTYPE_RAID_CONTROLLER 0x04
#define PCI_SUBTYPE_ATA_CONTROLLER 0x05
#define PCI_SUBTYPE_SATA_CONTROLLER 0x06
#define PCI_SUBTYPE_SAS_CONTROLLER 0x07
#define PCI_SUBTYPE_NVM_CONTROLLER 0x08
#define PCI_SUBTYPE_OTHER_MASS_STORAGE_CONTROLLER 0x80

// Subtypes for PCI_TYPE_NETWORK_CONTROLLER

#define PCI_SUBTYPE_ETHERNET_CONTROLLER 0x00
#define PCI_SUBTYPE_TOKEN_RING_CONTROLLER 0x01
#define PCI_SUBTYPE_FDDI_CONTROLLER 0x02
#define PCI_SUBTYPE_ATM_CONTROLLER 0x03
#define PCI_SUBTYPE_ISDN_CONTROLLER 0x04
#define PCI_SUBTYPE_WORLDFIP_CONTROLLER 0x05
#define PCI_SUBTYPE_PICMG_CONTROLLER 0x06
#define PCI_SUBTYPE_INFINIBAND_CONTROLLER 0x07
#define PCI_SUBTYPE_FABRIC_CONTROLLER 0x08
#define PCI_SUBTYPE_OTHER_NETWORK_CONTROLLER 0x80

// Subtypes for PCI_TYPE_DISPLAY_CONTROLLER

#define PCI_SUBTYPE_VGA_CONTROLLER 0x00
#define PCI_SUBTYPE_XGA_CONTROLLER 0x01
#define PCI_SUBTYPE_3D_CONTROLLER 0x02
#define PCI_SUBTYPE_OTHER_DISPLAY_CONTROLLER 0x80

// Subtypes for PCI_TYPE_MULTIMEDIA_CONTROLLER

#define PCI_SUBTYPE_VIDEO_CONTROLLER 0x00
#define PCI_SUBTYPE_AUDIO_CONTROLLER 0x01
#define PCI_SUBTYPE_TELEPHONY_CONTROLLER 0x02
#define PCI_SUBTYPE_AUDIO_DEVICE 0x03
#define PCI_SUBTYPE_OTHER_MULTIMEDIA_CONTROLLER 0x80

// Subtypes for PCI_TYPE_MEMORY_CONTROLLER

#define PCI_SUBTYPE_RAM_CONTROLLER 0x00
#define PCI_SUBTYPE_FLASH_CONTROLLER 0x01
#define PCI_SUBTYPE_OTHER_MEMORY_CONTROLLER 0x80

// Subtypes for PCI_TYPE_BRIDGE_DEVICE

#define PCI_SUBTYPE_HOST_BRIDGE 0x00
#define PCI_SUBTYPE_ISA_BRIDGE 0x01
#define PCI_SUBTYPE_EISA_BRIDGE 0x02
#define PCI_SUBTYPE_MCA_BRIDGE 0x03
#define PCI_SUBTYPE_PCI_TO_PCI_BRIDGE 0x04
#define PCI_SUBTYPE_PCMCIA_BRIDGE 0x05
#define PCI_SUBTYPE_NUBUS_BRIDGE 0x06
#define PCI_SUBTYPE_CARDBUS_BRIDGE 0x07
#define PCI_SUBTYPE_RACEWAY_BRIDGE 0x08
#define PCI_SUBTYPE_OTHER_BRIDGE_DEVICE 0x80

// Subtypes for PCI_TYPE_SIMPLE_COMM

#define PCI_SUBTYPE_SERIAL_CONTROLLER 0x00
#define PCI_SUBTYPE_PARALLEL_CONTROLLER 0x01
#define PCI_SUBTYPE_MULTIPORT_SERIAL_CONTROLLER 0x02
#define PCI_SUBTYPE_MODEM 0x03
#define PCI_SUBTYPE_GPIB_CONTROLLER 0x04
#define PCI_SUBTYPE_SMARTCARD_CONTROLLER 0x05
#define PCI_SUBTYPE_OTHER_SIMPLE_COMM_CONTROLLER 0x80

// Subtypes for PCI_TYPE_BASE_SYSTEM

#define PCI_SUBTYPE_PIC 0x00
#define PCI_SUBTYPE_DMA_CONTROLLER 0x01
#define PCI_SUBTYPE_TIMER 0x02
#define PCI_SUBTYPE_RTC_CONTROLLER 0x03
#define PCI_SUBTYPE_PCI_HOTPLUG_CONTROLLER 0x04
#define PCI_SUBTYPE_SD_HOST_CONTROLLER 0x05
#define PCI_SUBTYPE_OTHER_BASE_SYSTEM_PERIPHERAL 0x80

// Subtypes for PCI_TYPE_INPUT_DEVICE

#define PCI_SUBTYPE_KEYBOARD_CONTROLLER 0x00
#define PCI_SUBTYPE_DIGITIZER_PEN 0x01
#define PCI_SUBTYPE_MOUSE_CONTROLLER 0x02
#define PCI_SUBTYPE_SCANNER_CONTROLLER 0x03
#define PCI_SUBTYPE_GAMEPORT_CONTROLLER 0x04
#define PCI_SUBTYPE_OTHER_INPUT_DEVICE_CONTROLLER 0x80

// Subtypes for PCI_TYPE_DOCKING_STATION

#define PCI_SUBTYPE_GENERIC_DOCKING_STATION 0x00
#define PCI_SUBTYPE_OTHER_DOCKING_STATION 0x80

// Subtypes for PCI_TYPE_PROCESSOR

#define PCI_SUBTYPE_386 0x00
#define PCI_SUBTYPE_486 0x01
#define PCI_SUBTYPE_PENTIUM 0x02
#define PCI_SUBTYPE_ALPHA 0x10
#define PCI_SUBTYPE_POWERPC 0x20
#define PCI_SUBTYPE_MIPS 0x30
#define PCI_SUBTYPE_COPROCESSOR 0x40
#define PCI_SUBTYPE_OTHER_PROCESSOR 0x80

// Subtypes for PCI_TYPE_SERIAL_BUS

#define PCI_SUBTYPE_FIREWIRE_CONTROLLER 0x00
#define PCI_SUBTYPE_ACCESS_BUS_CONTROLLER 0x01
#define PCI_SUBTYPE_SSA_CONTROLLER 0x02
#define PCI_SUBTYPE_USB_CONTROLLER 0x03
#define PCI_SUBTYPE_FIBRE_CHANNEL_CONTROLLER 0x04
#define PCI_SUBTYPE_SMBUS_CONTROLLER 0x05
#define PCI_SUBTYPE_INFINIBAND_CONTROLLER 0x06
#define PCI_SUBTYPE_IPMI_CONTROLLER 0x07
#define PCI_SUBTYPE_SERCOS_CONTROLLER 0x08
#define PCI_SUBTYPE_CANBUS_CONTROLLER 0x09
#define PCI_SUBTYPE_OTHER_SERIAL_BUS_CONTROLLER 0x80

// Subtypes for PCI_TYPE_WIRELESS_CONTROLLER

#define PCI_SUBTYPE_IRDA_CONTROLLER 0x00
#define PCI_SUBTYPE_CONSUMER_IR_CONTROLLER 0x01
#define PCI_SUBTYPE_RF_CONTROLLER 0x10
#define PCI_SUBTYPE_BLUETOOTH_CONTROLLER 0x11
#define PCI_SUBTYPE_BROADBAND_CONTROLLER 0x12
#define PCI_SUBTYPE_ETHERNET_CONTROLLER 0x20
#define PCI_SUBTYPE_OTHER_WIRELESS_CONTROLLER 0x80

// Subtypes for PCI_TYPE_INTELLIGENT_IO

#define PCI_SUBTYPE_I2O_CONTROLLER 0x00

// Subtypes for PCI_TYPE_SATELLITE_COMMUNICATIONS

#define PCI_SUBTYPE_TV_CONTROLLER 0x01
#define PCI_SUBTYPE_AUDIO_CONTROLLER 0x02
#define PCI_SUBTYPE_VOICE_CONTROLLER 0x03
#define PCI_SUBTYPE_DATA_CONTROLLER 0x04

// Subtypes for PCI_TYPE_ENCRYPTION

#define PCI_SUBTYPE_NETWORK_CONTROLLER 0x00
#define PCI_SUBTYPE_ENTERTAINMENT_CONTROLLER 0x10
#define PCI_SUBTYPE_OTHER_ENCRYPTION_CONTROLLER 0x80

// Subtypes for PCI_TYPE_SIGNAL_PROCESSING

#define PCI_SUBTYPE_DPIO_CONTROLLER 0x00
#define PCI_SUBTYPE_OTHER_SIGNAL_PROCESSING_CONTROLLER 0x80

#endif // _KERNEL_DRIVERS_PCI_TYPES_H