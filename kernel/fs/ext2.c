#include <ext2.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ata.h>
#include <k_debug.h>


vfs_driver_t ext2_driver;
typedef struct
{
  ext2_data_t *fs;
  uint32_t inode_num;
  ext2_inode_t inode;
  uint32_t inode_dirty;
} ext2_data;

#define ext2data(node) ((ext2_data *)((node)->data))

int ext2_readblocks(ext2_data_t *fs, void *buffer, size_t start, size_t len)
{
  if(!fs)
    return 0;
  if(!buffer)
    return 0;

  size_t db_start = start*ext2_blocksize(fs)/ATA_SECTOR_SIZE;
  size_t db_len = len*ext2_blocksize(fs)/ATA_SECTOR_SIZE;

  return partition_readblocks(fs->p, buffer, db_start, db_len);
}

int ext2_writeblocks(ext2_data_t *fs, void *buffer, size_t start, size_t len)
{
  if(!fs)
    return 0;
  if(!buffer)
    return 0;

  size_t db_start = start*ext2_blocksize(fs)/ATA_SECTOR_SIZE;
  size_t db_len = len*ext2_blocksize(fs)/ATA_SECTOR_SIZE;

  return partition_writeblocks(fs->p, buffer, db_start, db_len);
}

int ext2_read_groupblocks(ext2_data_t *fs, int group, void *buffer, uint32_t start, uint32_t len)
{
  if(!fs)
    return 0;
  if(!buffer)
    return 0;
  if(group > (int)ext2_numgroups(fs))
    return 0;
  if(start > fs->superblock->blocks_per_group)
    return 0;
  if(start+len > fs->superblock->blocks_per_group)
    return 0;

  size_t offset = group*fs->superblock->blocks_per_group;
  return ext2_readblocks(fs, buffer, start+offset, len);
}

int ext2_write_groupblocks(ext2_data_t *fs, int group, void *buffer, uint32_t start, uint32_t len)
{
  if(!fs)
    return 0;
  if(!buffer)
    return 0;
  if(group > (int)ext2_numgroups(fs))
    return 0;
  if(start > fs->superblock->blocks_per_group)
    return 0;
  if(start+len > fs->superblock->blocks_per_group)
    return 0;

  size_t offset = group*fs->superblock->blocks_per_group;
  return ext2_writeblocks(fs, buffer, start+offset, len);
}

int ext2_read_inode(ext2_data_t *fs, ext2_inode_t *buffer, int num)
{
  if(!fs)
    return 0;
  if(!buffer)
    return 0;
  if(num > (int)fs->superblock->num_inodes)
    return 0;

  int group = (num-1) / fs->superblock->inodes_per_group;
  int offset =(num-1) % fs->superblock->inodes_per_group;

  int inoblock = (offset*sizeof(ext2_inode_t))/ext2_blocksize(fs);
  size_t inooffset = (offset*sizeof(ext2_inode_t))%ext2_blocksize(fs);
  inoblock += fs->groups[group].inode_table;

  char *buff = malloc(2*ext2_blocksize(fs));
  if(!ext2_readblocks(fs, buff, inoblock, 2))
    return 0;
  memcpy(buffer, (void *)((size_t)buff + inooffset), sizeof(ext2_inode_t));

  free(buff);

  return 1;
}

int ext2_write_inode(ext2_data_t *fs, ext2_inode_t *buffer, int num)
{
  if(!fs)
    return 0;
  if(!buffer)
    return 0;
  if(num > (int)fs->superblock->num_inodes)
    return 0;

  int group = (num-1) / fs->superblock->inodes_per_group;
  int offset = (num-1) % fs->superblock->inodes_per_group;

  int inoblock = (offset*sizeof(ext2_inode_t))/ext2_blocksize(fs);
  size_t inooffset = (offset*sizeof(ext2_inode_t))%ext2_blocksize(fs);
  inoblock += fs->groups[group].inode_table;

  char *buff = malloc(2*ext2_blocksize(fs));
  if(!ext2_readblocks(fs, buff, inoblock, 2))
    return 0;
  memcpy((void *)((size_t)buff + inooffset), buffer, sizeof(ext2_inode_t));
  ext2_writeblocks(fs, buff, inoblock, 2);

  free(buff);

  return 1;
}

void ext2_free_block(ext2_data_t *fs, uint32_t block)
{
  if(!fs)
    return;
  if(!block)
    return;
  unsigned int group = block / fs->superblock->blocks_per_group;

  uint8_t *block_bitmap = malloc(ext2_blocksize(fs));
  if(!ext2_readblocks(fs, block_bitmap, fs->groups[group].block_bitmap, 1))
    return;
  unsigned int i = block % fs->superblock->blocks_per_group;
  i--;
  block_bitmap[i/0x8] &= ~(1<<(i&0x7));
  if(!ext2_writeblocks(fs, block_bitmap, fs->groups[group].block_bitmap, 1))
    return;
  free(block_bitmap);
  fs->groups[group].unallocated_blocks ++;
  fs->groups_dirty = 1;
}

uint32_t ext2_alloc_block(ext2_data_t *fs, unsigned int group)
{
  if(!fs)
    return 0;
  if(group > ext2_numgroups(fs))
    return 0;
  uint32_t retval = 0;

  // Check if preferred group is ok, or find another one
  if(!fs->groups[group].unallocated_blocks)
    for(group = 0; group < ext2_numgroups(fs); group++)
      if(fs->groups[group].unallocated_blocks)
        break;
  if(group == ext2_numgroups(fs))
    return 0;

  // Load block bitmap
  uint8_t *block_bitmap = malloc(ext2_blocksize(fs));
  if(!ext2_readblocks(fs, block_bitmap, fs->groups[group].block_bitmap, 1))
    goto error;

  // Allocate a block
  uint32_t i = 4 + fs->superblock->inodes_per_group*sizeof(ext2_inode_t)/ext2_blocksize(fs) + 1;
  while(block_bitmap[i/0x8]&(0x1<<(i&0x7)) && i < fs->superblock->blocks_per_group)
    i++;
  if(i == fs->superblock->blocks_per_group)
    goto error;
  block_bitmap[i/0x8] |= 0x1 << (i&0x7);
  fs->groups[group].unallocated_blocks--;
  fs->groups_dirty = 1;
  fs->superblock->num_free_blocks--;
  fs->superblock_dirty = 1;
  i++;
  i += fs->superblock->blocks_per_group*group;

  // Write block bitmap back
  if(!ext2_writeblocks(fs, block_bitmap, fs->groups[group].block_bitmap, 1))
    goto error;

  retval = i;

error:
  if(block_bitmap)
    free(block_bitmap);
  return retval;
}

uint32_t ext2_count_indirect(ext2_data_t *fs, size_t size)
{
  uint32_t num_blocks = size / ext2_blocksize(fs);
  uint32_t blocks_per_indirect = ext2_blocksize(fs)/sizeof(uint32_t);
  uint32_t block = 12;
  uint32_t ret = 0;

  // Indirect
  if(block < num_blocks)
  {
    ret++;
    block += blocks_per_indirect;
  }

  // Doubly indirect
  if(block < num_blocks)
  {
    ret++;
    uint32_t i = 0;
    while(i < blocks_per_indirect && block < num_blocks)
    {
      ret++;
      block += blocks_per_indirect;
      i++;
    }
  }

  // Triply indirect
  if(block < num_blocks)
  {
    ret++;
    uint32_t i = 0;
    while(i < blocks_per_indirect && block < num_blocks)
    {
      ret++;
      uint32_t j = 0;
      while(j < blocks_per_indirect && block < num_blocks)
      {
        ret ++;
        block += blocks_per_indirect;
        j++;
      }
      i++;
    }
  }

  return ret;
}

size_t ext2_get_indirect(ext2_data_t *fs, uint32_t block, int level, uint32_t *block_list, size_t bl_index, size_t length, uint32_t *indirects)
{
  if(!fs)
    return 0;
  if(level > 3)
    return 0;

  if(level == 0)
  {
    block_list[bl_index] = block;
    return 1;
  } else {
    size_t i = 0;
    size_t read = 0;
    uint32_t *blocks = malloc(ext2_blocksize(fs));
    if(!ext2_readblocks(fs, blocks, block, 1))
      return 0;
    if(indirects)
    {
      indirects[indirects[0]] = block;
      indirects[0]++;
    }
    while(i < ext2_blocksize(fs)/sizeof(uint32_t) && bl_index < length)
    {
      size_t read2 = ext2_get_indirect(fs, blocks[i], level-1, block_list, bl_index, length, indirects);
      if(read2 == 0)
        return 0;
      bl_index += read2;
      read += read2;
      i++;
    }
    free(blocks);
    return read;
  }
}

  size_t ext2_set_indirect(ext2_data_t *fs, uint32_t *block, int level, uint32_t *block_list, size_t bl_index, int group, uint32_t *indirects)
{
  if(!fs)
    return 0;
  if(level > 3)
    return 0;

  if(level == 0)
  {
    *block = block_list[bl_index];
    return 1;
  } else {
    uint32_t *blocks = malloc(ext2_blocksize(fs));
    size_t i = 0;
    size_t total_set_count = 0;
    if(indirects)
    {
      *block = indirects[indirects[0]];
      indirects[0]++;
    } else {
      *block = ext2_alloc_block(fs, group);
    }
    while(i < ext2_blocksize(fs)/sizeof(uint32_t) && block_list[bl_index])
    {
      size_t set_count = ext2_set_indirect(fs, &blocks[i], level-1, block_list, bl_index, group, indirects);
      if(set_count == 0)
        return 0;
      bl_index += set_count;
      total_set_count += set_count;
      i++;
    }
    if(!ext2_writeblocks(fs, blocks, *block, 1))
      return 0;
    free(blocks);
    return total_set_count;
  }
}

uint32_t *ext2_get_blocks(ext2_data_t *fs, ext2_inode_t *node, uint32_t *indirects)
{
  if(!fs)
    return 0;
  if(!node)
    return 0;

  int num_blocks = node->size_low/ext2_blocksize(fs) + (node->size_low%ext2_blocksize(fs) != 0);

  uint32_t *block_list = calloc(num_blocks + 1, sizeof(uint32_t));

  if(indirects)
    indirects[0] = 1;
  int i;
  for(i = 0; i < num_blocks && i < 12; i++)
    block_list[i] = node->direct[i];
  if(i < num_blocks)
    i += ext2_get_indirect(fs, node->indirect, 1, block_list, i, num_blocks, indirects);
  if(i < num_blocks)
    i += ext2_get_indirect(fs, node->dindirect, 2, block_list, i, num_blocks, indirects);
  if(i < num_blocks)
    i += ext2_get_indirect(fs, node->tindirect, 3, block_list, i, num_blocks, indirects);

  block_list[i] = 0;
  return block_list;
}

uint32_t ext2_set_blocks(ext2_data_t *fs, ext2_inode_t *node, uint32_t *blocks, int group, uint32_t *indirects)
{
  if(!fs)
    return 0;
  if(!node)
    return 0;
  int i = 0;
  for(i = 0; blocks[i] && i < 12; i++)
  {
    node->direct[i] = blocks[i];
  }

  if(indirects)
    indirects[0] = 1;
  if(blocks[i])
    i += ext2_set_indirect(fs, &node->indirect, 1, blocks, i, group, indirects);
  if(blocks[i])
    i += ext2_set_indirect(fs, &node->dindirect, 2, blocks, i, group, indirects);
  if(blocks[i])
    i += ext2_set_indirect(fs, &node->tindirect, 3, blocks, i, group, indirects);

  if(blocks[i])
    return 0;
  return i;
}

uint32_t *ext2_make_blocks(ext2_data_t *fs, ext2_inode_t *node, int group)
{
  if(!fs)
    return 0;
  if(!node)
    return 0;
  size_t blocks_needed = node->size_low / ext2_blocksize(fs);
  if(node->size_low % ext2_blocksize(fs)) blocks_needed++;

  uint32_t *block_list = calloc(blocks_needed + 1, sizeof(uint32_t));
  unsigned int i;
  for(i = 0; i < blocks_needed && i < 12; i++)
  {
    node->direct[i] = ext2_alloc_block(fs, group);
    block_list[i] = node->direct[i];
    if(!node->direct[i])
      goto error;
  }

error:
  if(block_list)
    free(block_list);
  return 0;
}

size_t ext2_read_data(ext2_data_t *fs, ext2_inode_t *node, void *buffer, uint32_t length)
{
  if(!fs)
    return 0;
  if(!node)
    return 0;
  if(!buffer)
    return 0;

  if(length > node->size_low)
    length = node->size_low;
  uint32_t *block_list = ext2_get_blocks(fs, node, 0);

  int i = 0;
  size_t readcount = 0;
  while(block_list[i])
  {
    size_t readlength = length;
    if(readlength > (size_t)ext2_blocksize(fs))
      readlength = ext2_blocksize(fs);
    ext2_readblocks(fs, buffer, block_list[i], 1);

    length -= readlength;
    readcount += readlength;
    buffer = (void *)((size_t)buffer + readlength);
    i++;
  }
  free(block_list);
  return readcount;
}

INODE ext2_mkinode(ext2_data_t *fs, uint32_t num, char *name)
{
  vfs_node_t *node = calloc(1, sizeof(vfs_node_t));
  strcpy(node->name, name);
  node->d = &ext2_driver;

  node->data = calloc(1, sizeof(ext2_data));
  ext2data(node)->fs = fs;
  ext2data(node)->inode_num = num;
  if(!ext2_read_inode(fs, &ext2data(node)->inode, num))
  {
    free(node->data);
    free(node);
    return 0;
  }

  node->type = \
    FS_FILE*((ext2data(node)->inode.type & 0xF000) == EXT2_REGULAR) + \
    FS_DIRECTORY*((ext2data(node)->inode.type & 0xF000) == EXT2_DIR) + \
    FS_CHARDEV*((ext2data(node)->inode.type & 0xF000) == EXT2_CHDEV) + \
    FS_BLOCKDEV*((ext2data(node)->inode.type & 0xF000) == EXT2_BDEV) + \
    FS_PIPE*((ext2data(node)->inode.type & 0xF000) == EXT2_FIFO) + \
    FS_SYMLINK*((ext2data(node)->inode.type & 0xF000) == EXT2_SYMLINK);

  node->length = ext2data(node)->inode.size_low;

  return node;

}


int32_t ext2_open(INODE node, uint32_t flags)
{
  // Check permissions and stuff
  // Also set flags for appending and stuff
  (void)node;
  (void)flags;
  return 0;
}

int32_t ext2_close(INODE node)
{
  // If cached, write to disk
  (void)node;
  return 0;
}

uint32_t ext2_read(INODE ino, void *buffer, uint32_t length, uint32_t offset)
{
  if(!ino)
    return 0;
  if(ext2data(ino)->inode_num < 2)
    return 0;
  if(!buffer)
    return 0;

  uint32_t *block_list = 0;
  void *buff = 0;
  ext2_inode_t *inode = malloc(sizeof(ext2_inode_t));
  ext2_data_t *fs = ext2data(ino)->fs;
  if(!ext2_read_inode(fs, inode, ext2data(ino)->inode_num))
    goto error;
  if(offset > inode->size_low)
    goto error;
  if(offset + length > inode->size_low)
    length = inode->size_low - offset;


  uint32_t start_block = offset/ext2_blocksize(fs);
  size_t block_offset = offset%ext2_blocksize(fs);
  int num_blocks = length/ext2_blocksize(fs);
  if((length+block_offset)%ext2_blocksize(fs))
    num_blocks++;

  
  block_list = ext2_get_blocks(fs, inode, 0);

  
  void *b = buff = malloc(num_blocks*ext2_blocksize(fs));
  uint32_t i = start_block;
  while(i < start_block + num_blocks && block_list[i])
  {
    ext2_readblocks(fs, b, block_list[i], 1);
    b = (void *)((size_t)b + ext2_blocksize(fs));
    i++;
  }
  if(i < start_block + num_blocks)
    goto error;

  memcpy(buffer, (void *)((size_t)buff + block_offset), length);

  free(buff);
  free(block_list);
  free(inode);

  return length;

error:
  if(inode)
    free(inode);
  if(block_list)
    free(block_list);
  if(buff)
    free(buff);
  return 0;
}

uint32_t ext2_write(INODE ino, void *buffer, uint32_t length, uint32_t offset)
{
  if(!ino)
    return 0;
  if(ext2data(ino)->inode_num < 2)
    return 0;
  if(!buffer)
    return 0;

  uint32_t *block_list = 0;
  void *buff = 0;
  ext2_inode_t *inode = malloc(sizeof(ext2_inode_t));
  ext2_data_t *fs = ext2data(ino)->fs;
  if(!ext2_read_inode(fs, inode, ext2data(ino)->inode_num))
    goto error;
  if(offset > inode->size_low)
    goto error;
  if(offset + length > inode->size_low)
    length = inode->size_low - offset;

  uint32_t start_block = offset/ext2_blocksize(fs);
  size_t block_offset = offset%ext2_blocksize(fs);
  int num_blocks = length/ext2_blocksize(fs);
  if((length+block_offset)%ext2_blocksize(fs))
    num_blocks++;

  block_list = ext2_get_blocks(fs, inode, 0);

  void *b = buff = malloc(num_blocks*ext2_blocksize(fs));

  // Copy first part of first block to buffer
  // and fill the rest with input buffer data
  void *b2 = malloc(ext2_blocksize(fs));
  ext2_readblocks(fs, b2, block_list[start_block], 1);
  memcpy(buff, b2, block_offset);
  memcpy((void *)((size_t)buff + block_offset), buffer, length);

  uint32_t i = start_block;
  // Write first block from temp buffer
  ext2_writeblocks(fs, buff, block_list[i], 1);
  i++;
  b = (void *)((size_t)b + ext2_blocksize(fs));
  // Write rest from ordinary buffer
  while(i < start_block + num_blocks && block_list[i])
  {
    ext2_writeblocks(fs, b, block_list[i], 1);
    b = (void *)((size_t)b + ext2_blocksize(fs));
    i++;
  }
  if(i < start_block + num_blocks)
      goto error;

  free(buff);
  free(b2);
  free(block_list);
  free(inode);

  return length;

error:
  if(inode)
    free(inode);
  if(block_list)
    free(block_list);
  if(buff)
    free(buff);
  return 0;
}

void ext2_flush(INODE ino)
{
  free(ino->data);
}

int32_t ext2_stat(INODE node, struct stat *st)
{
  memset(st, 0, sizeof(struct stat));
  ext2_inode_t *ino = &ext2data(node)->inode;

  st->st_dev = 0;
  st->st_ino = ext2data(node)->inode_num;
  st->st_nlink = ino->link_count;
  st->st_uid = ino->uid;
  st->st_gid = ino->gid;
  st->st_size = ino->size_low;
  st->st_atime = ino->atime;
  st->st_mtime = ino->mtime;
  st->st_ctime = ino->ctime;
  return 0;
}

int32_t ext2_isatty(INODE node)
{
  (void)node;
  return 0;
}

int32_t ext2_link(INODE ino, INODE dir, const char *name)
{
  if(!ino)
    return 1;
  if(!dir)
    return 1;
  if(!name)
    return 1;

  ext2_data_t *fs = ext2data(dir)->fs;

  ext2_inode_t *dino = &ext2data(dir)->inode;
  ext2_inode_t *iino = &ext2data(ino)->inode;
  iino->link_count++;
  ext2_write_inode(fs, iino, ext2data(ino)->inode_num);

  ext2_dirinfo_t *di = malloc(dino->size_low + ext2_blocksize(fs));
  ext2_read(dir, di, dino->size_low, 0);
  ext2_dirinfo_t *next = di, *current = di;
  // Find last direntry
  while((size_t)next < ((size_t)di + dino->size_low))
  {
    current = next;
    next = (void *)((size_t)current + current->record_length);
  }
  // New direntry is right after the last one
  current->record_length = ((size_t)current->name + current->name_length + 1 - (size_t)current);
  if(current->record_length < 12) current->record_length = 12;
  current->record_length += (current->record_length%4)?4-current->record_length%4:0;
  next = (void *)((size_t)current + current->record_length);

  next->inode = ext2data(ino)->inode_num;
  strcpy(next->name, name);
  next->name_length = strlen(name);
  next->file_type = \
    (((iino->type & EXT2_FIFO) == EXT2_FIFO)?EXT2_DIR_FIFO:0) + \
    (((iino->type & EXT2_CHDEV) == EXT2_CHDEV)?EXT2_DIR_CHDEV:0) + \
    (((iino->type & EXT2_DIR) == EXT2_DIR)?EXT2_DIR_DIR:0) + \
    (((iino->type & EXT2_BDEV) == EXT2_BDEV)?EXT2_DIR_BDEV:0) + \
    (((iino->type & EXT2_REGULAR) == EXT2_REGULAR)?EXT2_DIR_REGULAR:0) + \
    (((iino->type & EXT2_SYMLINK) == EXT2_SYMLINK)?EXT2_DIR_SYMLINK:0) + \
    (((iino->type & EXT2_SOCKET) == EXT2_SOCKET)?EXT2_DIR_SOCKET:0);
  next->record_length = (size_t)next->name + next->name_length + 1 - (size_t)next;
  if(next->record_length < 12)
    next->record_length = 12;

  // Lengthen last entry
  if(((uint32_t)next + next->record_length - (uint32_t)di) > dino->size_low)
  {
    // Increase size of directory
    uint32_t *blocks = ext2_get_blocks(fs, dino, 0);
    uint32_t *blocks2 = calloc(dino->size_low/ext2_blocksize(fs) + 1, sizeof(uint32_t));
    uint32_t i = 0;
    for(i = 0; i < dino->size_low/ext2_blocksize(fs); i++)
    {
      blocks2[i] = blocks[i];
    }
    blocks2[i] = ext2_alloc_block(fs, ext2data(ino)->inode_num/fs->superblock->inodes_per_group);
    ext2_set_blocks(fs, dino, blocks2, ext2data(ino)->inode_num/fs->superblock->inodes_per_group, 0);
    free(blocks);
    free(blocks2);
    dino->size_low += ext2_blocksize(fs);
    ext2_write_inode(fs, dino, ext2data(dir)->inode_num);
  }
  next->record_length = dino->size_low - ((uint32_t)next - (uint32_t)di);

  // Write index back
  ext2_write(dir, di, dino->size_low, 0);

  free(di);
  
  return 0;
}

int32_t ext2_unlink(INODE dir, const char * name)
{
  if(!dir)
    return 1;
  ext2_data_t *fs = ext2data(dir)->fs;

  // Remove from directory listing
  ext2_inode_t *dir_ino = malloc(sizeof(ext2_inode_t));
  if(!ext2_read_inode(fs, dir_ino, ext2data(dir)->inode_num))
    return 1;

  ext2_dirinfo_t *buffer = malloc(dir_ino->size_low);
  if(!ext2_read_data(fs, dir_ino, buffer, dir_ino->size_low))
    return 1;

  ext2_dirinfo_t *p = buffer, *di = buffer;
  while((size_t)di < ((size_t)buffer + dir_ino->size_low))
  {
    p = di;
    di = (ext2_dirinfo_t *)((size_t)di + di->record_length);
    if(!strcmp(name, di->name))
      break;
  }
  if(strcmp(name, di->name))
    return 1;

  uint32_t child = di->inode;

  p->record_length += di->record_length;
  ext2_write(dir, buffer, dir_ino->size_low, 0);

  free(buffer);
  free(dir_ino);

  // Decrease link count
  ext2_inode_t *child_ino = malloc(sizeof(ext2_inode_t));
  if(!ext2_read_inode(fs, child_ino, child))
    return 1;
  
  child_ino->link_count --;
  if(child_ino->link_count < 1)
  {
    // Delete inode

    // Mark as deleted
    /* child_ino->dtime = time(0); */
    
    // Free blocks and inode if link count is zero
    unsigned int indirect_num = ext2_count_indirect(fs, child_ino->size_low);
    uint32_t *iblocks = calloc(indirect_num+1, sizeof(uint32_t));
    uint32_t *blocks = ext2_get_blocks(fs, child_ino, iblocks);

    unsigned int i=0;
    while(blocks[i])
    {
      ext2_free_block(fs, blocks[i]);
      i++;
    }
    i = 1;
    while(i <= indirect_num)
    {
      ext2_free_block(fs, iblocks[i]);
      i++;
    }
    free(iblocks);
    free(blocks);

    unsigned int group = child / fs->superblock->inodes_per_group;
    i = child % fs->superblock->inodes_per_group;
    i--;
    uint8_t *inode_bitmap = malloc(ext2_blocksize(fs));
    if(!ext2_readblocks(fs, inode_bitmap, fs->groups[group].inode_bitmap, 1))
      return 1;
    inode_bitmap[i/0x8] &= ~(1<<(i&0x7));
    if(!ext2_writeblocks(fs, inode_bitmap, fs->groups[group].inode_bitmap, 1))
      return 1;
    free(inode_bitmap);
    fs->groups[group].unallocated_inodes ++;
    fs->groups_dirty = 1;
  }
  if(!ext2_write_inode(fs, child_ino, child))
    return 1;
  free(child_ino);

  return 0;
}

int32_t ext2_mkdir(INODE ino, const char *name)
{
  (void)ino;
  (void)name;
  return 0;
}


dirent_t *ext2_readdir(INODE dir, uint32_t num)
{
  if(!dir)
    return 0;
  if(ext2data(dir)->inode_num < 2)
    return 0;

  ext2_inode_t *dir_ino = 0;
  void *data = 0;
  dirent_t *de = 0;
  

  dir_ino = malloc(sizeof(ext2_inode_t));
  ext2_data_t *fs = ext2data(dir)->fs;
  if(!ext2_read_inode(fs, dir_ino, ext2data(dir)->inode_num))
    goto end;

  data = malloc(dir_ino->size_low);
  if(!ext2_read_data(fs, dir_ino, data, dir_ino->size_low))
    goto end;

  ext2_dirinfo_t *di = data;
  while(num && (size_t)di < ((size_t)data + dir_ino->size_low))
  {
    di = (ext2_dirinfo_t *)((size_t)di + di->record_length);
    num--;
  }
  if((size_t)di >= ((size_t)data + dir_ino->size_low))
    goto end;

  de = malloc(sizeof(dirent_t));
  de->ino = ext2_mkinode(ext2data(dir)->fs, di->inode, di->name);
  strcpy(de->name, di->name);

end:
  if(data)
    free(data);
  if(dir_ino)
    free(dir_ino);

  return de;
}



vfs_driver_t ext2_driver =
{
  ext2_open,
  ext2_close,
  ext2_read,
  ext2_write,
  0, // link
  0, // unlink
  ext2_stat,
  ext2_isatty,
  0, // mkdir
  ext2_readdir,
  0, // finddir
  ext2_flush
};

INODE ext2_init(partition_t *p)
{
  ext2_data_t *fs = malloc(sizeof(ext2_data_t));
  fs->p = p;

  // Read superblock
  fs->superblock = malloc(EXT2_SUPERBLOCK_SIZE);
  partition_readblocks(p, fs->superblock, 2, 2);
  fs->superblock_dirty = 0;

  // Calculate number of groups
  fs->num_groups = ext2_numgroups(fs);

  // Calculate size of group descriptors
  size_t groups_size = fs->num_groups*sizeof(ext2_groupd_t);
  size_t groups_blocks = groups_size / ext2_blocksize(fs);
  if(groups_size % ext2_blocksize(fs))
    groups_blocks++;

  // Find location of group descriptor table
  fs->groups = malloc(groups_blocks*ext2_blocksize(fs));
  size_t groups_start = 1;
  if(ext2_blocksize(fs) == 1024)
    groups_start++;

  // Read group descriptor table
  ext2_readblocks(fs, fs->groups, groups_start, groups_blocks);
  fs->groups_dirty = 0;

  vfs_node_t *node = ext2_mkinode(fs, 2, "ext2");

  return node;
}
