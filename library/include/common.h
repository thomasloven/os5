#pragma once

#ifndef __ASSEMBLER__

#define min(x,y) \
	({typeof(x) x_ = (x); \
	typeof(y) y_ = (y); \
	(x_ < y_)?x_:y_; })

#define max(x,y) \
	({typeof(x) x_ = (x); \
	typeof(y) y_ = (y); \
	(x_ > y_)?x_:y_; })

#define swap(x,y) \
	({ typeof(x) temp_ = (x); \
	x = y; \
	y = temp_; })

#endif
