#pragma once

#ifndef __ASSEMBLER__

#define min(x,y) \
  ({__typeof__(x) x_ = (x); \
  __typeof__(y) y_ = (y); \
  (x_ < y_)?x_:y_; })

#define max(x,y) \
  ({__typeof__(x) x_ = (x); \
  __typeof__(y) y_ = (y); \
  (x_ > y_)?x_:y_; })

#define swap(x,y) \
  ({ __typeof__(x) temp_ = (x); \
  x = y; \
  y = temp_; })

#endif
