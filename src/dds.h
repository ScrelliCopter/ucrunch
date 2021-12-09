#ifndef DDS_H
#define DDS_H

#include "util.h"
#include <stdint.h>

typedef enum
{
	PXFM_INVALID = 0u,

	PXFM_R8G8B8,
	PXFM_L8,
	PXFM_A8,
	PXFM_A8L8,
	PXFM_A8R8G8B8,

	PXFM_BC1_DXT1,
	PXFM_BC2_DXT2,
	PXFM_BC2_DXT3,
	PXFM_BC3_DXT4,
	PXFM_BC3_DXT5,
	PXFM_BC4_DXT5A,
	PXFM_BC5_3DC,

	NUM_PXFM

} PixelFormat;

typedef struct
{
	uint32_t size;
	uint32_t flags;
	uint32_t fourcc;
	uint32_t bits;
	uint32_t rmask;
	uint32_t gmask;
	uint32_t bmask;
	uint32_t amask;

} DDSPixelFormat;

typedef struct
{
	uint32_t size;
	uint32_t flags;
	uint32_t height;
	uint32_t width;
	uint32_t pitch;
	uint32_t depth;
	uint32_t mipnum;
	uint32_t reserved1[11];
	DDSPixelFormat pixfmt;
	uint32_t caps1;
	uint32_t caps2;
	uint32_t caps3;
	uint32_t caps4;
	uint32_t reserved2;

} DDSHeader;

#endif//DDS_H
