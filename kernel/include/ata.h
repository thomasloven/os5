#pragma once
#include <stdint.h>

#define ATA_1 0x1F0
#define ATA_2 0x170

#define ATA_PORT_DATA 0x000 // R/W Data
#define ATA_PORT_ERROR 0x001 // R Error
#define ATA_PORT_FEATURE 0x001 // W Set features options
#define ATA_PORT_SECCNT 0x002 // R/W Sector count
#define ATA_PORT_LBA0 0x003 // W LBA bits 0-7
#define ATA_PORT_LBA1 0x004 // W LBA bits 15-8
#define ATA_PORT_LBA2 0x005 // W LBA bits 23-16
#define ATA_PORT_DRVHEAD 0x006 // W Drive and Head/LBA bits 27-24
#define ATA_PORT_STATUS 0x007 // R Status
#define ATA_PORT_CMD 0x007 // W Command
#define ATA_PORT_ALTST 0x206 // R Alternate status
#define ATA_PORT_DEVCTRL 0x206 // W Device control
#define ATA_PORT_DRVADDR 0x207 // R Drive address

#define ATA_DRVHEAD 0xA0
#define ATA_DRVHEAD_LBA 0x40
#define ATA_DRVHEAD_MASTER 0x00
#define ATA_DRVHEAD_SLAVE 0x10

#define ATA_ERROR_BBK 0x80 // Bad block
#define ATA_ERROR_UNC 0x40 // Uncorrectable data error
#define ATA_ERROR_MC 0x20 // Reserved for removable media
#define ATA_ERROR_IDNF 0x10 // ID not found
#define ATA_ERROR_MCR 0x08 // Reserved for removable media
#define ATA_ERROR_ABRT 0x04 // Command aborted
#define ATA_ERROR_TKONF 0x02 // Recalibration failed
#define ATA_ERROR_AMNF 0x01 // Address mark not found

#define ATA_STATUS_BSY 0x80 // Drive bussy
#define ATA_STATUS_DRDY 0x40 // Drive ready
#define ATA_STATUS_DWF 0x20 // Drive Write Fault
#define ATA_STATUS_DSC 0x10 // Drive Seek Complete
#define ATA_STATUS_DRQ 0x08 // Data request
#define ATA_STATUS_CORR 0x04 // Corrected data
#define ATA_STATUS_IDX 0x02 // Index mark detected
#define ATA_STATUS_ERR 0x01 // Error

#define ATA_DEVCTRL_BASE 0x08
#define ATA_DEVCTRL_SRST 0x04 // Software reset
#define ATA_DEVCTRL_NIEN 0x02 // Disable IRQ

#define ATA_CMD_ID 0xEC
#define ATA_CMD_READ 0x20
#define ATA_CMD_WRITE 0x30
#define ATA_CMD_EDD 0x90 // Execute Drive Diagnostic
#define ATA_CMD_FT 0x50 // Format Track
#define ATA_CMF_IDP 0x91 // Initialize Drive Parameters
#define ATA_CMD_RSWR 0x20 // Read Sectors With Retry
#define ATA_CMD_RS 0x21 // Read Sectors
#define ATA_CMD_RLWR 0x22 // Read Long With Retry
#define ATA_CMD_RL 0x23 // Read Long
#define ATA_CMD_RVSWR 0x40 // Read Verify Sectors With Retry
#define ATA_CMD_RVS 0x41 // Read Verify Sectors
#define ATA_CMD_RECAL 0x10 // Recalibrate
#define ATA_CMD_SEEK 0x70 // Seek
#define ATA_CMD_WSWR 0x30
#define ATA_CMD_WS 0x31
#define ATA_CMD_WLWR 0x32
#define ATA_CMD_WL 0x33

struct ata_id
{
  uint16_t configuration; // 0
  uint16_t res0[3]; // 1
  uint16_t res1[3]; // 4
  uint16_t res2[3]; // 7
  char serial_number[20]; // 10
  uint16_t res3[3]; // 20
  char firmware[8]; // 23
  char model[40]; // 27
  uint8_t res4; // 47
  uint8_t max_transfer_sectors;
  uint16_t res5; // 48
  uint16_t capabilities[2]; // 49
  uint16_t res6[2]; // 51
  uint16_t word88valid; // 53
  uint16_t res7[5]; // 54
  uint8_t multiple_sectors; // 59
  uint8_t current_transfer_sectors;
  uint32_t total_sectors; // 60
  uint16_t res8; // 62
  uint16_t dma_mode; // 63
  uint8_t res9; // 64
  uint8_t pio_modes;
  uint16_t dma_time_min; // 65
  uint16_t dma_time_rec; // 66
  uint16_t pio_time0; // 67
  uint16_t pio_time1; // 68
  uint16_t res10[2]; // 69
  uint16_t id_packet[4]; // 71
  uint16_t queue_depth; // 75
  uint16_t res11[4]; // 76
  uint16_t version_major; // 80
  uint16_t version_minor; // 81
  uint16_t supported_commands[6]; // 82
  uint16_t dma_mode2; // 88
  uint16_t erase_time; // 89
  uint16_t enhanced_erase_time; //90
  uint16_t apm_value; //91
  uint16_t password_revision; // 92
  uint16_t reset_result; // 93
  uint16_t aam_value; // 94
  uint16_t stream_request_min; // 95
  uint16_t dma_stream_time; // 96
  uint16_t latency; // 97
  uint32_t granularity; // 98
  uint64_t lba_max; // 100
  uint16_t pio_stream_time; // 104
  uint16_t res12; // 105
  uint16_t physical_logical_size; // 106
  uint16_t new_stuff[5]; // 107
  uint16_t res13[5]; // 112
  uint32_t words_per_sector; // 117
  uint16_t res14[8]; // 119
  uint16_t removable; // 127
  uint16_t security; //128
  uint16_t res15[31]; // 129
  uint16_t power_mode; // 160
  uint16_t res16[15]; // 161
  uint16_t serial_no[30]; // 176
  uint16_t res17[49]; // 206
  uint16_t integrity; // 255
} __attribute__((packed));

struct ata_drive
{
  int bus;
  int masterslave;
  struct ata_id id;
};

void init_ata();
struct ata_drive *ata_init_drive(int primary, int master);
