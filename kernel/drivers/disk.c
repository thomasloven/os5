#include <ata.h>
#include <stdint.h>
#include <stdlib.h>
#include <k_debug.h>

partition_t *init_partition(int primary, int master, uint32_t partition)
{
  partition_t *p = calloc(1, sizeof(partition_t));
  p->drive = ata_init_drive(primary, master);

  // Read MBR table
  void *buf = calloc(1, ATA_SECTOR_SIZE);
  ata_read_block(p->drive, 0, buf);
  MBR_entry_t *mbr = (MBR_entry_t *)&((uint8_t *)buf)[MBR_OFFSET];

  p->partition = partition;
  p->offset = mbr[partition].start_LBA;
  p->length = mbr[partition].num_sectors;

  debug("Offset: %x, Length %x\n", p->offset, p->length);
  return p;
}

uint32_t partition_readblocks(partition_t *p, void *buffer, uint32_t start, uint32_t len)
{
  (void)p;
  (void)buffer;
  (void)start;
  (void)len;
  int readbytes = 0;
  uint32_t lba = start + p->offset;
  while(len)
  {
    if(ata_read_block(p->drive, lba, buffer))
      return 0;
    readbytes += ATA_SECTOR_SIZE;
    lba++;
    len--;
    buffer = (void *)&((uint8_t *)buffer)[ATA_SECTOR_SIZE];
  }
  return readbytes;
}

uint32_t partition_writeblocks(partition_t *p, void *buffer, uint32_t start, uint32_t len)
{
  (void)p;
  (void)buffer;
  (void)start;
  (void)len;
  int readbytes = 0;
  uint32_t lba = start + p->offset;
  while(len)
  {
    if(ata_write_block(p->drive, lba, buffer))
      return 0;
    readbytes += ATA_SECTOR_SIZE;
    lba++;
    len--;
    buffer = (void *)&((uint8_t *)buffer)[ATA_SECTOR_SIZE];
  }
  return readbytes;
}

