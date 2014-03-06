#include <ata.h>
#include <k_debug.h>
#include <arch.h>
#include <stdlib.h>


int ata_io_wait_status(int bus)
{
  inb(bus + ATA_PORT_STATUS);
  inb(bus + ATA_PORT_STATUS);
  inb(bus + ATA_PORT_STATUS);
  return inb(bus + ATA_PORT_STATUS);
}

int ata_wait(int bus)
{
  ata_io_wait_status(bus);

  // Wait for device
  while(inb(bus + ATA_PORT_STATUS) & ATA_STATUS_BSY)
  {
    ;
  }

  // Check for errors
  int status = inb(bus + ATA_PORT_STATUS);
  if(status & (ATA_STATUS_DWF | ATA_STATUS_ERR))
    return 1;
  if(!(status & ATA_STATUS_DRQ))
    return 2;
  return 0;
}

void ata_print_registers(int bus)
{
  int ret = ata_io_wait_status(bus);
  debug("Status: %x\n", ret);
  ret = inb(bus + ATA_PORT_ERROR);
  debug("Error: %x\n", ret);
  ret = inb(bus + ATA_PORT_SECCNT);
  debug("Sector count: %x\n", ret);
  ret = inb(bus + ATA_PORT_LBA0);
  debug("LBA0: %x\n", ret);
  ret = inb(bus + ATA_PORT_LBA1);
  debug("LBA1: %x\n", ret);
  ret = inb(bus + ATA_PORT_LBA2);
  debug("LBA2: %x\n", ret);
  ret = inb(bus + ATA_PORT_DRVHEAD);
  debug("Drive/LBA3: %x\n", ret);
  ret = inb(bus + ATA_PORT_STATUS);
  debug("Status: %x\n", ret);
  ret = inb(bus + ATA_PORT_ALTST);
  debug("Alternate status: %x\n", ret);
  ret = inb(bus + ATA_PORT_DRVADDR);
  debug("Drive address: %x\n", ret);
}

void insm(unsigned short port, void *data, unsigned long size)
{
  // Read size words from port to data
  __asm__ volatile ("rep insw" : "+D" (data), "+c" (size) : "d" (port) : "memory");
}

void outsm(unsigned short port, void *data, unsigned long size)
{
  // Write size words from data to port
  __asm__ volatile ("rep outsw" : "+S" (data), "+c" (size) : "d" (port));
}

int ata_read_block(struct ata_drive *drv, uint32_t lba, void *buf)
{
  if(lba > drv->id.total_sectors)
  {
    return -1;
  }

  int retries = 5;
  while(retries)
  {
    // Select LBA
    outb(drv->bus + ATA_PORT_LBA0, (lba & 0xFF));
    outb(drv->bus + ATA_PORT_LBA1, (lba & 0xFF00) >> 8);
    outb(drv->bus + ATA_PORT_LBA2, (lba & 0xFF0000) >> 16);
    // Select drive
    outb(drv->bus + ATA_PORT_DRVHEAD, \
        ATA_DRVHEAD | ATA_DRVHEAD_LBA | drv->masterslave | \
        ((lba & 0x0F000000)>> 24));
    // Select number of sectors to read
    outb(drv->bus + ATA_PORT_SECCNT, 1);

    // Send read command
    outb(drv->bus + ATA_PORT_CMD, ATA_CMD_READ);

    // Retry five times
    if(ata_wait(drv->bus))
    {
      retries--;
      continue;
    }

    // Read data to buffer
    insm(drv->bus + ATA_PORT_DATA, buf, 256);

    return 0;
  }

  return -1;
}

int ata_write_block(struct ata_drive *drv, uint32_t lba, void *buf)
{
  if(lba > drv->id.total_sectors)
  {
    return -1;
  }

  // Select LBA
  outb(drv->bus + ATA_PORT_LBA0, (lba & 0xFF));
  outb(drv->bus + ATA_PORT_LBA1, (lba & 0xFF00) >> 8);
  outb(drv->bus + ATA_PORT_LBA2, (lba & 0xFF0000) >> 16);
  // Select drive
  outb(drv->bus + ATA_PORT_DRVHEAD, \
      ATA_DRVHEAD | ATA_DRVHEAD_LBA | drv->masterslave | \
      ((lba & 0x0F000000)>> 24));
  // Select number of sectors to read
  outb(drv->bus + ATA_PORT_SECCNT, 1);

  // Send write command
  outb(drv->bus + ATA_PORT_CMD, ATA_CMD_WRITE);

  ata_wait(drv->bus);

  // Send data
  outsm(drv->bus + ATA_PORT_DATA, buf, 256);
  return 0;
}

struct ata_drive *ata_init_drive(int primary, int master)
{
  struct ata_drive *drive = calloc(1, sizeof(struct ata_drive));
  drive->bus = primary?ATA_1:ATA_2;
  drive->masterslave = master?ATA_DRVHEAD_MASTER:ATA_DRVHEAD_SLAVE;

  // Check if the controller exists
  int v = inb(drive->bus + ATA_PORT_LBA0);
  outb(drive->bus + ATA_PORT_LBA0, (~v)&0xFF);
  int v2 = inb(drive->bus + ATA_PORT_LBA0);
  if(v2 != ((~v)&0xFF))
  {
    free(drive);
    debug("No controller");
    return 0;
  }

  // Check if the drive exists
  outb(drive->bus + ATA_PORT_DRVHEAD, ATA_DRVHEAD | drive->masterslave);
  if(!(ata_io_wait_status(drive->bus) & ATA_STATUS_DRDY))
  {
    free(drive);
    debug("No drive");
    return 0;
  }

  //Reset bus
  outb(drive->bus + ATA_PORT_DEVCTRL, ATA_DEVCTRL_BASE | ATA_DEVCTRL_SRST);
  ata_io_wait_status(drive->bus);
  outb(drive->bus + ATA_PORT_DEVCTRL, ATA_DEVCTRL_BASE );
  ata_io_wait_status(drive->bus);

  // Read device info
  outb(drive->bus + ATA_PORT_DRVHEAD, ATA_DRVHEAD | drive->masterslave);
  outb(drive->bus + ATA_PORT_CMD, ATA_CMD_ID);
  ata_io_wait_status(drive->bus);
  while(inb(drive->bus + ATA_PORT_STATUS) & ATA_STATUS_BSY);
  insm(drive->bus + ATA_PORT_DATA, &drive->id, 256);

  return drive;

}
