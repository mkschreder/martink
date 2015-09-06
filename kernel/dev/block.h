 
#pragma once 
struct block_device_ops; 
typedef struct block_device_ops **block_dev_t; 

typedef uint16_t blk_address_t; 

typedef enum {
	BLKDEV_BUSY = (1 << 0)
} blkdev_status_t; 

struct block_device_geometry {
	uint16_t page_size; 
	uint16_t pages; 
	uint16_t sectors; 
}; 

typedef unsigned int ioctl_req_t; 

struct block_device_ops {
	uint8_t (*open)(block_dev_t self); 
	int8_t 	(*close)(block_dev_t self); 
	ssize_t (*write)(block_dev_t self, const uint8_t *data, ssize_t data_size); 
	ssize_t (*read)(block_dev_t self, uint8_t *data, ssize_t data_size); 
	ssize_t (*seek)(block_dev_t self, ssize_t ofs, int whence);  
	int16_t (*ioctl)(block_dev_t self, ioctl_req_t req, ...); 
	//int8_t 	(*get_geometry)(block_dev_t self, struct block_device_geometry *geom); 
	//uint8_t (*get_status)(block_dev_t self, blkdev_status_t flags); 
}; 

#define blk_open(dev) (*dev)->open(dev)
#define blk_close(dev) (*dev)->close(dev)
#define blk_write(dev, data, size) (*dev)->write(dev, data, size)
#define blk_read(dev, data, size) (*dev)->read(dev, data, size)
#define blk_seek(dev, ofs, cur) (*dev)->seek(dev, ofs, cur)
#define blk_ioctl(dev, ...) (*dev)->ioctl(dev, __VA_ARGS__)
//#define blk_get_geometry(dev, geom) (*dev)->get_geometry(dev, geom)
//#define blk_get_status(dev, flags) (*dev)->get_status(dev, flags)
