// File: crnlib.h - Advanced DXTn texture compression library.
// This software is in the public domain. Please see license.txt.
//
// This header file contains the public crnlib declarations for DXTn,
// clustered DXTn, and CRN compression/decompression.
//
// Note: This library does NOT need to be linked into your game executable if
// all you want to do is transcode .CRN files to raw DXTn bits at run-time.
// The crn_decomp.h header file library contains all the code necessary for
// decompression.
//
// Important: If compiling with gcc, be sure strict aliasing is disabled: -fno-strict-aliasing
#ifndef CRNLIB_H
#define CRNLIB_H

#include <stddef.h>

#ifdef _MSC_VER
#pragma warning (disable: 4127) //  conditional expression is constant
#endif

#define CRNLIB_VERSION 104

#define CRNLIB_SUPPORT_ATI_COMPRESS 0
#define CRNLIB_SUPPORT_SQUISH 0

typedef unsigned char   crn_uint8;
typedef unsigned short  crn_uint16;
typedef unsigned int    crn_uint32;
typedef signed char     crn_int8;
typedef signed short    crn_int16;
typedef signed int      crn_int32;
typedef unsigned int    crn_bool;

#define crn_true  1
#define crn_false 0

// crnlib can compress to these file types.
typedef enum
{
   // .CRN
   cCRNFileTypeCRN = 0,

   // .DDS using regular DXT or clustered DXT
   cCRNFileTypeDDS,

   cCRNFileTypeForceDWORD = 0xFFFFFFFF

} crn_file_type;

// Supported compressed pixel formats.
// Basically all the standard DX9 formats, with some swizzled DXT5 formats
// (most of them supported by ATI's Compressonator), along with some ATI/X360 GPU specific formats.
typedef enum
{
   cCRNFmtInvalid = -1,

   cCRNFmtDXT1 = 0,

   cCRNFmtFirstValid = cCRNFmtDXT1,

   // cCRNFmtDXT3 is not currently supported when writing to CRN - only DDS.
   cCRNFmtDXT3,

   cCRNFmtDXT5,

   // Various DXT5 derivatives
   cCRNFmtDXT5_CCxY,    // Luma-chroma
   cCRNFmtDXT5_xGxR,    // Swizzled 2-component
   cCRNFmtDXT5_xGBR,    // Swizzled 3-component
   cCRNFmtDXT5_AGBR,    // Swizzled 4-component

   // ATI 3DC and X360 DXN
   cCRNFmtDXN_XY,
   cCRNFmtDXN_YX,

   // DXT5 alpha blocks only
   cCRNFmtDXT5A,

   cCRNFmtETC1,

   cCRNFmtTotal,

   cCRNFmtForceDWORD = 0xFFFFFFFF

} crn_format;

// Various library/file format limits.
typedef enum
{
   // Max. mipmap level resolution on any axis.
   cCRNMaxLevelResolution     = 4096,

   cCRNMinPaletteSize         = 8,
   cCRNMaxPaletteSize         = 8192,

   cCRNMaxFaces               = 6,
   cCRNMaxLevels              = 16,

   cCRNMaxHelperThreads       = 16,

   cCRNMinQualityLevel        = 0,
   cCRNMaxQualityLevel        = 255

} crn_limits;

// CRN/DDS compression flags.
// See the flags member in the crn_comp_params struct, below.
typedef enum
{
   // Enables perceptual colorspace distance metrics if set.
   // Important: Be sure to disable this when compressing non-sRGB colorspace images, like normal maps!
   // Default: Set
   cCRNCompFlagPerceptual = 1,

   // Enables (up to) 8x8 macroblock usage if set. If disabled, only 4x4 blocks are allowed.
   // Compression ratio will be lower when disabled, but may cut down on blocky artifacts because the process used to determine
   // where large macroblocks can be used without artifacts isn't perfect.
   // Default: Set.
   cCRNCompFlagHierarchical = 2,

   // cCRNCompFlagQuick disables several output file optimizations - intended for things like quicker previews.
   // Default: Not set.
   cCRNCompFlagQuick = 4,

   // DXT1: OK to use DXT1 alpha blocks for better quality or DXT1A transparency.
   // DXT5: OK to use both DXT5 block types.
   // Currently only used when writing to .DDS files, as .CRN uses only a subset of the possible DXTn block types.
   // Default: Set.
   cCRNCompFlagUseBothBlockTypes = 8,

   // OK to use DXT1A transparent indices to encode black (assumes pixel shader ignores fetched alpha).
   // Currently only used when writing to .DDS files, .CRN never uses alpha blocks.
   // Default: Not set.
   cCRNCompFlagUseTransparentIndicesForBlack = 16,

   // Disables endpoint caching, for more deterministic output.
   // Currently only used when writing to .DDS files.
   // Default: Not set.
   cCRNCompFlagDisableEndpointCaching = 32,

   // If enabled, use the cCRNColorEndpointPaletteSize, etc. params to control the CRN palette sizes. Only useful when writing to .CRN files.
   // Default: Not set.
   cCRNCompFlagManualPaletteSizes = 64,

   // If enabled, DXT1A alpha blocks are used to encode single bit transparency.
   // Default: Not set.
   cCRNCompFlagDXT1AForTransparency = 128,

   // If enabled, the DXT1 compressor's color distance metric assumes the pixel shader will be converting the fetched RGB results to luma (Y part of YCbCr).
   // This increases quality when compressing grayscale images, because the compressor can spread the luma error amoung all three channels (i.e. it can generate blocks
   // with some chroma present if doing so will ultimately lead to lower luma error).
   // Only enable on grayscale source images.
   // Default: Not set.
   cCRNCompFlagGrayscaleSampling = 256,

   // If enabled, debug information will be output during compression.
   // Default: Not set.
   cCRNCompFlagDebugging = 0x80000000,

   cCRNCompFlagForceDWORD = 0xFFFFFFFF

} crn_comp_flags;

// Controls DXTn quality vs. speed control - only used when compressing to .DDS.
typedef enum
{
   cCRNDXTQualitySuperFast,
   cCRNDXTQualityFast,
   cCRNDXTQualityNormal,
   cCRNDXTQualityBetter,
   cCRNDXTQualityUber,

   cCRNDXTQualityTotal,

   cCRNDXTQualityForceDWORD = 0xFFFFFFFF

} crn_dxt_quality;

// Which DXTn compressor to use when compressing to plain (non-clustered) .DDS.
typedef enum
{
   cCRNDXTCompressorCRN,      // Use crnlib's ETC1 or DXTc block compressor (default, highest quality, comparable or better than ati_compress or squish, and crnlib's ETC1 is a lot fasterw with similiar quality to Erricson's)
   cCRNDXTCompressorCRNF,     // Use crnlib's "fast" DXTc block compressor
   cCRNDXTCompressorRYG,      // Use RYG's DXTc block compressor (low quality, but very fast)

#if CRNLIB_SUPPORT_ATI_COMPRESS
   cCRNDXTCompressorATI,
#endif

#if CRNLIB_SUPPORT_SQUISH
   cCRNDXTCompressorSquish,
#endif

   cCRNTotalDXTCompressors,

   cCRNDXTCompressorForceDWORD = 0xFFFFFFFF

} crn_dxt_compressor_type;

// Progress callback function.
// Processing will stop prematurely (and fail) if the callback returns false.
// phase_index, total_phases - high level progress
// subphase_index, total_subphases - progress within current phase
typedef crn_bool (*crn_progress_callback_func)(crn_uint32 phase_index, crn_uint32 total_phases, crn_uint32 subphase_index, crn_uint32 total_subphases, void* pUser_data_ptr);

// CRN/DDS compression parameters struct.
typedef struct
{
   crn_uint32                 size_of_obj;

   crn_file_type              file_type;               // Output file type: cCRNFileTypeCRN or cCRNFileTypeDDS.

   crn_uint32                 faces;                   // 1 (2D map) or 6 (cubemap)
   crn_uint32                 width;                   // [1,cCRNMaxLevelResolution], non-power of 2 OK, non-square OK
   crn_uint32                 height;                  // [1,cCRNMaxLevelResolution], non-power of 2 OK, non-square OK
   crn_uint32                 levels;                  // [1,cCRNMaxLevelResolution], non-power of 2 OK, non-square OK

   crn_format                 format;                  // Output pixel format.

   crn_uint32                 flags;                   // see crn_comp_flags enum

   // Array of pointers to 32bpp input images.
   const crn_uint32*          pImages[cCRNMaxFaces][cCRNMaxLevels];

   // Target bitrate - if non-zero, the compressor will use an interpolative search to find the
   // highest quality level that is <= the target bitrate. If it fails to find a bitrate high enough, it'll
   // try disabling adaptive block sizes (cCRNCompFlagHierarchical flag) and redo the search. This process can be pretty slow.
   float                      target_bitrate;

   // Desired quality level.
   // Currently, CRN and DDS quality levels are not compatible with eachother from an image quality standpoint.
   crn_uint32                 quality_level;           // [cCRNMinQualityLevel, cCRNMaxQualityLevel]

   // DXTn compression parameters.
   crn_uint32                 dxt1a_alpha_threshold;
   crn_dxt_quality            dxt_quality;
   crn_dxt_compressor_type    dxt_compressor_type;

   // Alpha channel's component. Defaults to 3.
   crn_uint32                 alpha_component;

   // Various low-level CRN specific parameters.
   float                      crn_adaptive_tile_color_psnr_derating;
   float                      crn_adaptive_tile_alpha_psnr_derating;

   crn_uint32                 crn_color_endpoint_palette_size;  // [cCRNMinPaletteSize,cCRNMaxPaletteSize]
   crn_uint32                 crn_color_selector_palette_size;  // [cCRNMinPaletteSize,cCRNMaxPaletteSize]

   crn_uint32                 crn_alpha_endpoint_palette_size;  // [cCRNMinPaletteSize,cCRNMaxPaletteSize]
   crn_uint32                 crn_alpha_selector_palette_size;  // [cCRNMinPaletteSize,cCRNMaxPaletteSize]

   // Number of helper threads to create during compression. 0=no threading.
   crn_uint32                 num_helper_threads;

   // CRN userdata0 and userdata1 members, which are written directly to the header of the output file.
   crn_uint32                 userdata0;
   crn_uint32                 userdata1;

   // User provided progress callback.
   crn_progress_callback_func pProgress_func;
   void*                      pProgress_func_data;

} crn_comp_params;

// Clear struct to default parameters.
static inline void crn_comp_params_clear(crn_comp_params* p)
{
   p->size_of_obj = sizeof(crn_comp_params);
   p->file_type = cCRNFileTypeCRN;
   p->faces = 1;
   p->width = 0;
   p->height = 0;
   p->levels = 1;
   p->format = cCRNFmtDXT1;
   p->flags = cCRNCompFlagPerceptual | cCRNCompFlagHierarchical | cCRNCompFlagUseBothBlockTypes;

   for (crn_uint32 f = 0; f < cCRNMaxFaces; f++)
      for (crn_uint32 l = 0; l < cCRNMaxLevels; l++)
         p->pImages[f][l] = NULL;

   p->target_bitrate = 0.0f;
   p->quality_level = cCRNMaxQualityLevel;
   p->dxt1a_alpha_threshold = 128;
   p->dxt_quality = cCRNDXTQualityUber;
   p->dxt_compressor_type = cCRNDXTCompressorCRN;
   p->alpha_component = 3;

   p->crn_adaptive_tile_color_psnr_derating = 2.0f;
   p->crn_adaptive_tile_alpha_psnr_derating = 2.0f;
   p->crn_color_endpoint_palette_size = 0;
   p->crn_color_selector_palette_size = 0;
   p->crn_alpha_endpoint_palette_size = 0;
   p->crn_alpha_selector_palette_size = 0;

   p->num_helper_threads = 0;
   p->userdata0 = 0;
   p->userdata1 = 0;
   p->pProgress_func = NULL;
   p->pProgress_func_data = NULL;
}

static inline crn_bool crn_comp_params_comp(const crn_comp_params* lhs, const crn_comp_params* rhs)
{
#define CRNLIB_COMP(x) do { if ((lhs->x) != (rhs->x)) return crn_false; } while(0)
   CRNLIB_COMP(size_of_obj);
   CRNLIB_COMP(file_type);
   CRNLIB_COMP(faces);
   CRNLIB_COMP(width);
   CRNLIB_COMP(height);
   CRNLIB_COMP(levels);
   CRNLIB_COMP(format);
   CRNLIB_COMP(flags);
   CRNLIB_COMP(target_bitrate);
   CRNLIB_COMP(quality_level);
   CRNLIB_COMP(dxt1a_alpha_threshold);
   CRNLIB_COMP(dxt_quality);
   CRNLIB_COMP(dxt_compressor_type);
   CRNLIB_COMP(alpha_component);
   CRNLIB_COMP(crn_adaptive_tile_color_psnr_derating);
   CRNLIB_COMP(crn_adaptive_tile_alpha_psnr_derating);
   CRNLIB_COMP(crn_color_endpoint_palette_size);
   CRNLIB_COMP(crn_color_selector_palette_size);
   CRNLIB_COMP(crn_alpha_endpoint_palette_size);
   CRNLIB_COMP(crn_alpha_selector_palette_size);
   CRNLIB_COMP(num_helper_threads);
   CRNLIB_COMP(userdata0);
   CRNLIB_COMP(userdata1);
   CRNLIB_COMP(pProgress_func);
   CRNLIB_COMP(pProgress_func_data);

   for (crn_uint32 f = 0; f < cCRNMaxFaces; f++)
      for (crn_uint32 l = 0; l < cCRNMaxLevels; l++)
         CRNLIB_COMP(pImages[f][l]);

#undef CRNLIB_COMP
   return crn_true;
}

// Returns true if the input parameters are reasonable.
static inline crn_bool crn_comp_params_check(const crn_comp_params* p)
{
   if ( (p->file_type > cCRNFileTypeDDS) ||
      (((int)p->quality_level < (int)cCRNMinQualityLevel) || ((int)p->quality_level > (int)cCRNMaxQualityLevel)) ||
      (p->dxt1a_alpha_threshold > 255) ||
      ((p->faces != 1) && (p->faces != 6)) ||
      ((p->width < 1) || (p->width > cCRNMaxLevelResolution)) ||
      ((p->height < 1) || (p->height > cCRNMaxLevelResolution)) ||
      ((p->levels < 1) || (p->levels > cCRNMaxLevels)) ||
      ((p->format < cCRNFmtDXT1) || (p->format >= cCRNFmtTotal)) ||
      ((p->crn_color_endpoint_palette_size) && ((p->crn_color_endpoint_palette_size < cCRNMinPaletteSize) || (p->crn_color_endpoint_palette_size > cCRNMaxPaletteSize))) ||
      ((p->crn_color_selector_palette_size) && ((p->crn_color_selector_palette_size < cCRNMinPaletteSize) || (p->crn_color_selector_palette_size > cCRNMaxPaletteSize))) ||
      ((p->crn_alpha_endpoint_palette_size) && ((p->crn_alpha_endpoint_palette_size < cCRNMinPaletteSize) || (p->crn_alpha_endpoint_palette_size > cCRNMaxPaletteSize))) ||
      ((p->crn_alpha_selector_palette_size) && ((p->crn_alpha_selector_palette_size < cCRNMinPaletteSize) || (p->crn_alpha_selector_palette_size > cCRNMaxPaletteSize))) ||
      (p->alpha_component > 3) ||
      (p->num_helper_threads > cCRNMaxHelperThreads) ||
      (p->dxt_quality > cCRNDXTQualityUber) ||
      (p->dxt_compressor_type >= cCRNTotalDXTCompressors) )
   {
      return crn_false;
   }
   return crn_true;
}

// Helper to set/get flags from flags member.
static inline crn_bool crn_comp_params_get_flag(const crn_comp_params* p, crn_comp_flags flag) { return (p->flags & flag) != 0; }
static inline void crn_comp_params_set_flag(crn_comp_params* p, crn_comp_flags flag, crn_bool val) { p->flags &= ~flag; if (val) p->flags |= flag; }

// Mipmap generator's mode.
typedef enum
{
   cCRNMipModeUseSourceOrGenerateMips,       // Use source texture's mipmaps if it has any, otherwise generate new mipmaps
   cCRNMipModeUseSourceMips,                 // Use source texture's mipmaps if it has any, otherwise the output has no mipmaps
   cCRNMipModeGenerateMips,                  // Always generate new mipmaps
   cCRNMipModeNoMips,                        // Output texture has no mipmaps

   cCRNMipModeTotal,

   cCRNModeForceDWORD = 0xFFFFFFFF

} crn_mip_mode;

const char* crn_get_mip_mode_desc(crn_mip_mode m);
const char* crn_get_mip_mode_name(crn_mip_mode m);

// Mipmap generator's filter kernel.
typedef enum
{
   cCRNMipFilterBox,
   cCRNMipFilterTent,
   cCRNMipFilterLanczos4,
   cCRNMipFilterMitchell,
   cCRNMipFilterKaiser,                      // Kaiser=default mipmap filter

   cCRNMipFilterTotal,

   cCRNMipFilterForceDWORD = 0xFFFFFFFF

} crn_mip_filter;

const char* crn_get_mip_filter_name(crn_mip_filter f);

// Mipmap generator's scale mode.
typedef enum
{
   cCRNSMDisabled,
   cCRNSMAbsolute,
   cCRNSMRelative,
   cCRNSMLowerPow2,
   cCRNSMNearestPow2,
   cCRNSMNextPow2,

   cCRNSMTotal,

   cCRNSMForceDWORD = 0xFFFFFFFF

} crn_scale_mode;

const char* crn_get_scale_mode_desc(crn_scale_mode sm);

// Mipmap generator parameters.
typedef struct
{
   crn_uint32     size_of_obj;

   crn_mip_mode   mode;
   crn_mip_filter filter;

   crn_bool       gamma_filtering;
   float          gamma;

   float          blurriness;

   crn_uint32     max_levels;
   crn_uint32     min_mip_size;

   crn_bool       renormalize;
   crn_bool       tiled;

   crn_scale_mode scale_mode;
   float          scale_x;
   float          scale_y;

   crn_uint32     window_left;
   crn_uint32     window_top;
   crn_uint32     window_right;
   crn_uint32     window_bottom;

   crn_bool       clamp_scale;
   crn_uint32     clamp_width;
   crn_uint32     clamp_height;

} crn_mipmap_params;

static inline void crn_mipmap_params_clear(crn_mipmap_params* p)
{
   p->size_of_obj = sizeof(crn_mipmap_params);
   p->mode = cCRNMipModeUseSourceOrGenerateMips;
   p->filter = cCRNMipFilterKaiser;
   p->gamma_filtering = crn_true;
   p->gamma = 2.2f;
   // Default "blurriness" factor of .9 actually sharpens the output a little.
   p->blurriness = .9f;
   p->renormalize = crn_false;
   p->tiled = crn_false;
   p->max_levels = cCRNMaxLevels;
   p->min_mip_size = 1;

   p->scale_mode = cCRNSMDisabled;
   p->scale_x = 1.0f;
   p->scale_y = 1.0f;

   p->window_left = 0;
   p->window_top = 0;
   p->window_right = 0;
   p->window_bottom = 0;

   p->clamp_scale = crn_false;
   p->clamp_width = 0;
   p->clamp_height = 0;
}

static inline crn_bool crn_mipmap_params_check(const crn_mipmap_params* p) { return crn_true; }

static inline crn_bool crn_mipmap_params_comp(const crn_mipmap_params* lhs, const crn_mipmap_params* rhs)
{
#define CRNLIB_COMP(x) do { if ((lhs->x) != (rhs->x)) return crn_false; } while(0)
   CRNLIB_COMP(size_of_obj);
   CRNLIB_COMP(mode);
   CRNLIB_COMP(filter);
   CRNLIB_COMP(gamma_filtering);
   CRNLIB_COMP(gamma);
   CRNLIB_COMP(blurriness);
   CRNLIB_COMP(renormalize);
   CRNLIB_COMP(tiled);
   CRNLIB_COMP(max_levels);
   CRNLIB_COMP(min_mip_size);
   CRNLIB_COMP(scale_mode);
   CRNLIB_COMP(scale_x);
   CRNLIB_COMP(scale_y);
   CRNLIB_COMP(window_left);
   CRNLIB_COMP(window_top);
   CRNLIB_COMP(window_right);
   CRNLIB_COMP(window_bottom);
   CRNLIB_COMP(clamp_scale);
   CRNLIB_COMP(clamp_width);
   CRNLIB_COMP(clamp_height);
   return crn_true;
#undef CRNLIB_COMP
}

// -------- High-level helper function definitions for CDN/DDS compression.

#ifndef CRNLIB_MIN_ALLOC_ALIGNMENT
#define CRNLIB_MIN_ALLOC_ALIGNMENT sizeof(size_t) * 2
#endif

// Function to set an optional user provided memory allocation/reallocation/msize routines.
// By default, crnlib just uses malloc(), free(), etc. for all allocations.
typedef void*  (*crn_realloc_func)(void* p, size_t size, size_t* pActual_size, crn_bool movable, void* pUser_data);
typedef size_t (*crn_msize_func)(void* p, void* pUser_data);
void crn_set_memory_callbacks(crn_realloc_func pRealloc, crn_msize_func pMSize, void* pUser_data);

// Frees memory blocks allocated by crn_compress(), crn_decompress_crn_to_dds(), or crn_decompress_dds_to_images().
void crn_free_block(void *pBlock);

// Compresses a 32-bit/pixel texture to either: a regular DX9 DDS file, a "clustered" (or reduced entropy) DX9 DDS file, or a CRN file in memory.
// Input parameters:
//  comp_params is the compression parameters struct, defined above.
//  compressed_size will be set to the size of the returned memory block containing the output file.
//  The returned block must be freed by calling crn_free_block().
//  *pActual_quality_level will be set to the actual quality level used to compress the image. May be NULL.
//  *pActual_bitrate will be set to the output file's effective bitrate, possibly taking into account LZMA compression. May be NULL.
// Return value:
//  The compressed file data, or NULL on failure.
//  compressed_size will be set to the size of the returned memory buffer.
// Notes:
//  A "regular" DDS file is compressed using normal DXTn compression at the specified DXT quality level.
//  A "clustered" DDS file is compressed using clustered DXTn compression to either the target bitrate or the specified integer quality factor.
//  The output file is a standard DX9 format DDS file, except the compressor assumes you will be later losslessly compressing the DDS output file using the LZMA algorithm.
//  A texture is defined as an array of 1 or 6 "faces" (6 faces=cubemap), where each "face" consists of between [1,cCRNMaxLevels] mipmap levels.
//  Mipmap levels are simple 32-bit 2D images with a pitch of width*sizeof(uint32), arranged in the usual raster order (top scanline first).
//  The image pixels may be grayscale (YYYX bytes in memory), grayscale/alpha (YYYA in memory), 24-bit (RGBX in memory), or 32-bit (RGBA) colors (where "X"=don't care).
//  RGB color data is generally assumed to be in the sRGB colorspace. If not, be sure to clear the "cCRNCompFlagPerceptual" in the crn_comp_params struct!
void *crn_compress(const crn_comp_params *comp_params, crn_uint32 *compressed_size, crn_uint32 *pActual_quality_level, float *pActual_bitrate);

// Like the above function, except this function can also do things like generate mipmaps, and resize or crop the input texture before compression.
// The actual operations performed are controlled by the crn_mipmap_params struct members.
// Be sure to set the "gamma_filtering" member of crn_mipmap_params to false if the input texture is not sRGB.
void *crn_compress_ext(const crn_comp_params *comp_params, const crn_mipmap_params *mip_params, crn_uint32 *compressed_size, crn_uint32 *pActual_quality_level, float *pActual_bitrate);

// Transcodes an entire CRN file to DDS using the crn_decomp.h header file library to do most of the heavy lifting.
// The output DDS file's format is guaranteed to be one of the DXTn formats in the crn_format enum.
// This is a fast operation, because the CRN format is explicitly designed to be efficiently transcodable to DXTn.
// For more control over decompression, see the lower-level helper functions in crn_decomp.h, which do not depend at all on crnlib.
void *crn_decompress_crn_to_dds(const void *pCRN_file_data, crn_uint32 *file_size);

// Decompresses an entire DDS file in any supported format to uncompressed 32-bit/pixel image(s).
// See the crnlib::pixel_format enum in inc/dds_defs.h for a list of the supported DDS formats.
// You are responsible for freeing each image block, either by calling crn_free_all_images() or manually calling crn_free_block() on each image pointer.
typedef struct
{
   crn_uint32 faces;
   crn_uint32 width;
   crn_uint32 height;
   crn_uint32 levels;
   crn_uint32 fmt_fourcc; // Same as crnlib::pixel_format

} crn_texture_desc;
crn_bool crn_decompress_dds_to_images(const void *pDDS_file_data, crn_uint32 dds_file_size, crn_uint32 **ppImages, crn_texture_desc *tex_desc);

// Frees all images allocated by crn_decompress_dds_to_images().
void crn_free_all_images(crn_uint32 **ppImages, const crn_texture_desc *desc);

// -------- crn_format related helpers functions.

// Returns the FOURCC format equivalent to the specified crn_format.
crn_uint32 crn_get_format_fourcc(crn_format fmt);

// Returns the crn_format's bits per texel.
crn_uint32 crn_get_format_bits_per_texel(crn_format fmt);

// Returns the crn_format's number of bytes per block.
crn_uint32 crn_get_bytes_per_dxt_block(crn_format fmt);

// Returns the non-swizzled, basic DXTn version of the specified crn_format.
// This is the format you would supply D3D or OpenGL.
crn_format crn_get_fundamental_dxt_format(crn_format fmt);

// -------- String helpers.

// Converts a crn_file_type to a string.
const char* crn_get_file_type_ext(crn_file_type file_type);

// Converts a crn_format to a string.
const char* crn_get_format_string(crn_format fmt);

// Converts a crn_dxt_quality to a string.
const char* crn_get_dxt_quality_string(crn_dxt_quality q);

// -------- Low-level DXTn 4x4 block compressor API

// crnlib's DXTn endpoint optimizer actually supports any number of source pixels (i.e. from 1 to thousands, not just 16),
// but for simplicity this API only supports 4x4 texel blocks.
typedef void *crn_block_compressor_context_t;

// Create a DXTn block compressor.
// This function only supports the basic/nonswizzled "fundamental" formats: DXT1, DXT3, DXT5, DXT5A, DXN_XY and DXN_YX.
// Avoid calling this multiple times if you intend on compressing many blocks, because it allocates some memory.
crn_block_compressor_context_t crn_create_block_compressor(const crn_comp_params *params);

// Compresses a block of 16 pixels to the destination DXTn block.
// pDst_block should be 8 (for DXT1/DXT5A) or 16 bytes (all the others).
// pPixels should be an array of 16 crn_uint32's. Each crn_uint32 must be r,g,b,a (r is always first) in memory.
void crn_compress_block(crn_block_compressor_context_t pContext, const crn_uint32 *pPixels, void *pDst_block);

// Frees a DXTn block compressor.
void crn_free_block_compressor(crn_block_compressor_context_t pContext);

// Unpacks a compressed block to pDst_pixels.
// pSrc_block should be 8 (for DXT1/DXT5A) or 16 bytes (all the others).
// pDst_pixel should be an array of 16 crn_uint32's. Each uint32 will be r,g,b,a (r is always first) in memory.
// crn_fmt should be one of the "fundamental" formats: DXT1, DXT3, DXT5, DXT5A, DXN_XY and DXN_YX.
// The various swizzled DXT5 formats (such as cCRNFmtDXT5_xGBR, etc.) will be unpacked as if they where plain DXT5.
// Returns false if the crn_fmt is invalid.
crn_bool crn_decompress_block(const void *pSrc_block, crn_uint32 *pDst_pixels, crn_format crn_fmt);

#endif // CRNLIB_H
