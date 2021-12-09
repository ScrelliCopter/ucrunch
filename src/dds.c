#include "dds.h"

const uint32_t DDS_MAGIC  = FOURCC("DDS ");
const uint32_t DDS_PFSIZE = sizeof(DDSPixelFormat);

// header flags
const uint32_t DDS_CAPS   =      0x1u;
const uint32_t DDS_HEIGHT =      0x2u;
const uint32_t DDS_WIDTH  =      0x4u;
const uint32_t DDS_PITCH  =      0x8u;
const uint32_t DDS_PIXFMT =   0x1000u;
const uint32_t DDS_MIPNUM =  0x20000u;
const uint32_t DDS_LINSZ  =  0x80000u;
const uint32_t DDS_DEPTH  = 0x800000u;

// pixelformat flags
const uint32_t DDS_ALPHAPIXELS =     0x1u;
const uint32_t DDS_ALPHA       =     0x2u;
const uint32_t DDS_FOURCC      =     0x4u;
const uint32_t DDS_RGB         =    0x40u;
const uint32_t DDS_RGBA        = DDS_RGB | DDS_ALPHAPIXELS;
//const uint32_t DDS_YUV         =   0x200u;
const uint32_t DDS_LUMINANCE   = 0x20000u;
const uint32_t DDS_LUMINANCEA  = DDS_LUMINANCE | DDS_ALPHAPIXELS;

static const DDSPixelFormat ddsPixFmts[NUM_PXFM] =
{
	[PXFM_R8G8B8]    = { DDS_PFSIZE, DDS_RGB,        0, 24,   0xFF0000,   0x00FF00,   0x0000FF,          0 },
	[PXFM_L8]        = { DDS_PFSIZE, DDS_LUMINANCE,  0,  8,       0xFF,          0,          0,          0 },
	[PXFM_A8]        = { DDS_PFSIZE, DDS_ALPHA,      0,  8,          0,          0,          0,       0xFF },
	[PXFM_A8L8]      = { DDS_PFSIZE, DDS_LUMINANCEA, 0, 16,     0x00FF,          0,          0,     0xFF00 },
	[PXFM_A8R8G8B8]  = { DDS_PFSIZE, DDS_RGBA,       0, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000 },

	[PXFM_BC1_DXT1]  = { DDS_PFSIZE, DDS_FOURCC, FOURCC("DXT1"), 0, 0, 0, 0, 0 },
	[PXFM_BC2_DXT2]  = { DDS_PFSIZE, DDS_FOURCC, FOURCC("DXT2"), 0, 0, 0, 0, 0 },
	[PXFM_BC2_DXT3]  = { DDS_PFSIZE, DDS_FOURCC, FOURCC("DXT3"), 0, 0, 0, 0, 0 },
	[PXFM_BC3_DXT4]  = { DDS_PFSIZE, DDS_FOURCC, FOURCC("DXT4"), 0, 0, 0, 0, 0 },
	[PXFM_BC3_DXT5]  = { DDS_PFSIZE, DDS_FOURCC, FOURCC("DXT5"), 0, 0, 0, 0, 0 },
	[PXFM_BC4_DXT5A] = { DDS_PFSIZE, DDS_FOURCC, FOURCC("ATI1"), 0, 0, 0, 0, 0 },
	[PXFM_BC5_3DC]   = { DDS_PFSIZE, DDS_FOURCC, FOURCC("ATI2"), 0, 0, 0, 0, 0 }
};
