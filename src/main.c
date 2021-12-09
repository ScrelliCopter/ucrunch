#include "cmdparse.h"
#include "crnlib.h"
#include "stb_dxt.h"
#include "stb_image.h"
#include "stb_image_resize.h"
#include "dds.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>


/*
Command line usage
-file <filename>

Path/file related parameters
-out <filename>
-outdir <filename>
-outsamedir
-deep
-timestamp
-nooverwrite
-forcewrite
-recreate
-fileformat <filename=cFormatCRN [tga,bmp,dds,ktx,crn,png]>

Modes
-compare
-info

Misc. options
-helperThreads <num=0 0-cCRNMaxHelperThreads>
-noprogress
-paramdebug (hidden)
-debug (hidden)
-quick (hidden)
-quiet
-ignoreerrors
-logfile <file>
-pause
-window <xl=0 0-cCRNMaxLevelResolution> <yl=0 0-cCRNMaxLevelResolution> <xh=0 0-cCRNMaxLevelResolution> <yh=0 0-cCRNMaxLevelResolution>
-clamp <w=1 1-cCRNMaxLevelResolution> <h=1 1-cCRNMaxLevelResolution>
-clampScale <w=1 1-cCRNMaxLevelResolution> <h=1 1-cCRNMaxLevelResolution>
-nostats
-imagestats
-mipstats
-lzmastats
-split
-csvfile (hidden) <file> (bools: imagestats, mipstats, grayscalesampling)
-yflip
-unflip

Image rescaling (mutually exclusive options)
-rescalemode <mode [nearest, hi, lo]>
-rescale <w=-1 1-cCRNMaxLevelResolution> <h=-1 1-cCRNMaxLevelResolution>
-relrescale <w=1 1-256> <h=1 1-256>

DDS/CRN compression quality control
-q (hidden) <level=cDefaultCRNQualityLevel 0-cCRNMaxQualityLevel>
-quality <level=cDefaultCRNQualityLevel 0-cCRNMaxQualityLevel>
-bitrate <rate=0.0 0.1-30.0>

Low-level CRN specific options
-c <cendpts=0 cCRNMinPaletteSize-cCRNMaxPaletteSize>
-s <cselpts=0 cCRNMinPaletteSize-cCRNMaxPaletteSize>
-ca <aendpts=0 cCRNMinPaletteSize-cCRNMaxPaletteSize>
-sa <aendpts=0 cCRNMinPaletteSize-cCRNMaxPaletteSize>

Mipmap filtering options
-mipMode <mode=UseSourceOrGenerate [UseSourceOrGenerate,UseSource,Generate,None]>
-mipFilter <mode=kaiser [box,tent,lanczos4,mitchell,kaiser]>
-gamma <value=2.2 0.1-8.0>
-blurriness <scale=0.9 0.01-8.0>
-wrap
-renormalize
-maxmips <num=cCRNMaxLevels 1-cCRNMaxLevels>
-minmipsize <size=1 1-cCRNMaxLevelResolution>

Compression options
-alphaThreshold <thresh=128 0-255>
-uniformMetrics
-noAdaptiveBlocks
-compressor <mode=CRN [CRN,CRNF,RYG,ATI]>
-dxtQuality <level=cCRNDXTQualityUber [superfast,fast,normal,better,uber]>
-noendpointcaching
-grayscalesampling
-forceprimaryencoding
-usetransparentindicesforblack

???
-converttoluma
-setalphatoluma

Ouptut pixel format options
-usesourceformat

All supported texture formats (Note: .CRN only supports DXTn pixel formats)
-DXT1
-DXT2
-DXT3
-DXT4
-DXT5
-3DC
-DXN
-DXT5A
-DXT5_CCxY
-DXT5_xGxR
-DXT5_xGBR
-DXT5_AGBR
-DXT1A
-ETC1
-R8G8B8
-L8
-A8
-A8L8
-A8R8G8B8
*/

const CmdParamEntry params[] =
{
	{ PT_PAR, PF_PAR_ARGNUM_1,     "file",          "Required input filename, wildcards, multiple -file params OK." },
	{   PT_VAL, PF_VAL_LISTFILE,     "filename",      NULL },

	{ PT_CAT, PF_CAT_LPAD(21),     "file",          "Path/file related parameters" },
	{   PT_PAR, PF_PAR_ARGNUM_1,     "out",           "Output filename" },
	{     PT_VAL, PF_VAL_ESTR,         "filename",      NULL },
	{   PT_PAR, PF_PAR_ARGNUM_1,     "outdir",        "Output directory" },
	{     PT_VAL, PF_VAL_ESTR,         "path",          NULL },
	{   PT_PAR, 0,                   "outsamedir",    "Write output file to input directory" },
	{   PT_PAR, 0,                   "deep",          "Recurse subdirectories, default=false" },
	{   PT_PAR, 0,                   "nooverwrite",   "Don't overwrite existing files" },
	{   PT_PAR, 0,                   "timestamp",     "Update only changed files" },
	{   PT_PAR, 0,                   "forcewrite",    "Overwrite read-only files" },
	{   PT_PAR, 0,                   "recreate",      "Recreate directory structure" },
	{   PT_PAR, PF_PAR_ARGNUM_1,     "fileformat",    "Output file format" },
	{     PT_VAL, PF_VAL_STR,          "type",          "[dds,ktx,crn,tga,bmp,png]" },
	{     PT_DOC, 0,                   NULL,            " default=crn or dds" },

	{ PT_CAT, PF_CAT_LPAD(10),     "modes",         "Modes" },
	{   PT_PAR, 0,                   "compare",       "Compare input and output files (no output files are written)." },
	{   PT_PAR, 0,                   "info",          "Only display input file statistics (no output files are written)." },

	{ PT_CAT, PF_CAT_LPAD(23),     "misc",          "Misc. options" },
	{   PT_PAR, PF_PAR_ARGNUM_1,     "helperThreads", "Set number of helper threads, 0-16, default=(# of CPU's)-1" },
	{     PT_VAL, PF_VAL_INT,          "num",           NULL },
	{   PT_PAR, 0,                   "noprogress",    "Disable progress output" },
	{   PT_PAR, PF_PAR_HIDDEN,       "paramdebug",    NULL },
	{   PT_PAR, PF_PAR_HIDDEN,       "debug",         NULL },
	{   PT_PAR, PF_PAR_HIDDEN,       "quick",         NULL },
	{   PT_PAR, 0,                   "quiet",         "Disable all console output" },
	{   PT_PAR, 0,                   "ignoreerrors",  "Continue processing files after errors. Note: The default" },
	{   PT_DOC, 0,                   NULL,            "                       behavior is to immediately exit whenever an error occurs." },
	{   PT_PAR, PF_PAR_ARGNUM_1,     "logfile",       "Append output to log file" },
	{     PT_VAL, PF_VAL_STR,          "filename",      NULL },
	{   PT_PAR, 0,                   "pause",         "Wait for keypress on error" },
	{   PT_PAR, PF_PAR_ARGNUM_4,     "window",        "Crop window before processing" },
	{     PT_VAL, PF_VAL_INT,          "left",          NULL },
	{     PT_VAL, PF_VAL_INT,          "top",           NULL },
	{     PT_VAL, PF_VAL_INT,          "right",         NULL },
	{     PT_VAL, PF_VAL_INT,          "bottom",        NULL },
	{   PT_PAR, PF_PAR_ARGNUM_2,     "clamp",         "Crop image if larger than width/height" },
	{     PT_VAL, PF_VAL_INT,          "width",         NULL },
	{     PT_VAL, PF_VAL_INT,          "height",        NULL },
	{   PT_PAR, PF_PAR_ARGNUM_2,     "clampscale",    "Scale image if larger than width/height" },
	{     PT_VAL, PF_VAL_INT,          "width",         NULL },
	{     PT_VAL, PF_VAL_INT,          "height",        NULL },
	{   PT_PAR, 0,                   "nostats",       "Disable all output file statistics (faster)" },
	{   PT_PAR, 0,                   "imagestats",    "Print various image qualilty statistics" },
	{   PT_PAR, 0,                   "mipstats",      "Print statistics for each mipmap, not just the top mip" },
	{   PT_PAR, 0,                   "lzmastats",     "Print size of output file compressed with LZMA codec" },
	{   PT_PAR, 0,                   "split",         "Write faces/mip levels to multiple separate output PNG files" },
	{   PT_PAR, PF_PAR_HIDDEN | PF_PAR_ARGNUM_1, "csvfile", NULL },
	{     PT_VAL, PF_VAL_STR,          "filename",      NULL },
	{   PT_PAR, 0,                   "yflip",         "Always flip texture on Y axis before processing" },
	{   PT_PAR, 0,                   "unflip",        "Unflip texture if read from source file as flipped" },

	{ PT_CAT, PF_CAT_LPAD(35),     "rescale",       "Image rescaling (mutually exclusive options)" },
	{   PT_PAR, PF_PAR_ARGNUM_2,     "rescale",       "Rescale image to specified resolution" },
	{     PT_VAL, PF_VAL_INT,          "w",             "<int>" },
	{     PT_VAL, PF_VAL_INT,          "h",             "<int>" },
	{   PT_PAR, PF_PAR_ARGNUM_2,     "relrescale",    "Rescale image to specified relative resolution" },
	{     PT_VAL, PF_VAL_FLT,          "w",             "<float>" },
	{     PT_VAL, PF_VAL_FLT,          "h",             "<float>" },
	{   PT_PAR, PF_PAR_ARGNUM_1,     "rescalemode",   "Auto-rescale non-power of two images" },
	{     PT_VAL, PF_VAL_ESTR,         "mode",          "<nearest | hi | lo>" },
	{     PT_DOC, 0,                   NULL,            " nearest - Use nearest power of 2" },
	{     PT_DOC, 0,                   NULL,            " hi      - Use next" },
	{     PT_DOC, 0,                   NULL,            " lo      - Use previous" },

	{ PT_CAT, 0,                   "quality",       "DDS/CRN compression quality control" },
	{   PT_PAR, PF_PAR_HIDDEN | PF_PAR_ARGNUM_1, "q", NULL },
	{     PT_VAL, PF_VAL_INT,          "level",         NULL },
	{   PT_PAR, PF_PAR_ARGNUM_1,     "quality",       "Set Clustered DDS/CRN quality factor [0-255] 255=best" },
	{     PT_VAL, PF_VAL_INT,          "level",         "<level> (or /q <level>)" },
	{     PT_DOC, 0,                   NULL,            "                                   DDS default quality is best possible." },
	{     PT_DOC, 0,                   NULL,            "                                   CRN default quality is 128." },
	{   PT_PAR, PF_PAR_ARGNUM_1,     "bitrate",       "Set the desired output bitrate of DDS or CRN output files." },
	{     PT_VAL, PF_VAL_FLT,          "rate",          NULL },
	{     PT_DOC, 0,                   NULL,            "                  This option causes crunch to find the quality factor" },
	{     PT_DOC, 0,                   NULL,            "                  closest to the desired bitrate using a binary search." },

	{ PT_CAT, PF_CAT_LPAD(16),     "crnlib",        "Low-level CRN specific options" },
	{   PT_PAR, PF_PAR_ARGNUM_1,     "c",             "Color endpoint palette size, 32-8192, default=3072" },
	{     PT_VAL, PF_VAL_INT,          "cendpts",       NULL },
	{   PT_PAR, PF_PAR_ARGNUM_1,     "s",             "Color selector palette size, 32-8192, default=3072" },
	{     PT_VAL, PF_VAL_INT,          "cselpts",       NULL },
	{   PT_PAR, PF_PAR_ARGNUM_1,     "ca",            "Alpha endpoint palette size, 32-8192, default=3072" },
	{     PT_VAL, PF_VAL_INT,          "aendpts",       NULL },
	{   PT_PAR, PF_PAR_ARGNUM_1,     "sa",            "Alpha selector palette size, 32-8192, default=3072" },
	{     PT_VAL, PF_VAL_INT,          "aendpts",       NULL },

	{ PT_CAT, PF_CAT_LPAD(22),     "mipmap",        "Mipmap filtering options" },
	{   PT_PAR, PF_PAR_ARGNUM_1,     "mipMode",       NULL },
	{     PT_VAL, PF_VAL_STR,          "mode",          "[UseSourceOrGenerate,UseSource,Generate,None]" },
	{     PT_DOC, 0,                   NULL,            "         Default mipMode is UseSourceOrGenerate" },
	{     PT_DOC, 0,                   NULL,            "         UseSourceOrGenerate: Use source mipmaps if possible, or create new mipmaps." },
	{     PT_DOC, 0,                   NULL,            "         UseSource: Always use source mipmaps, if any (never generate new mipmaps)" },
	{     PT_DOC, 0,                   NULL,            "         Generate: Always generate a new mipmap chain (ignore source mipmaps)" },
	{     PT_DOC, 0,                   NULL,            "         None: Do not output any mipmaps" },
	{   PT_PAR, PF_PAR_ARGNUM_1,     "mipFilter",     NULL },
	{     PT_VAL, PF_VAL_STR,          "mode",          "[box,tent,lanczos4,mitchell,kaiser], default=kaiser" },
	{   PT_PAR, PF_PAR_ARGNUM_1,     "gamma",         "Mipmap gamma correction value, default=2.2, use 1.0 for linear" },
	{     PT_VAL, PF_VAL_FLT,          "value",         NULL },
	{   PT_PAR, PF_PAR_ARGNUM_1,     "blurriness",    "Scale filter kernel, >1=blur, <1=sharpen, .01-8, default=.9" },
	{     PT_VAL, PF_VAL_FLT,          "scale",         NULL },
	{   PT_PAR, 0,                   "wrap",          "Assume texture is tiled when filtering, default=clamping" },
	{   PT_PAR, 0,                   "renormalize",   "Renormalize filtered normal map texels, default=disabled" },
	{   PT_PAR, PF_PAR_ARGNUM_1,     "maxmips",       "Limit number of generated texture mipmap levels, 1-16, default=16" },
	{     PT_VAL, PF_VAL_INT,          "num",           NULL },
	{   PT_PAR, PF_PAR_ARGNUM_1,     "minmipsize",    "Smallest allowable mipmap resolution, default=1" },
	{     PT_VAL, PF_VAL_INT,          "size",          NULL },

	{ PT_CAT, PF_CAT_LPAD(33),     "compression",   "Compression options" },
	{   PT_PAR, PF_PAR_ARGNUM_1,     "alphaThreshold", "Set DXT1A alpha threshold, 0-255, default=128" },
	{     PT_VAL, PF_VAL_INT,          "thresh",        NULL },
	{   PT_DOC, 0,                   NULL,            " Note: -alphaThreshold also changes the compressor's behavior to" },
	{   PT_DOC, 0,                   NULL,            " prefer DXT1A over DXT5 for images with alpha channels (.DDS only)." },
	{   PT_PAR, 0,                   "uniformMetrics", "Use uniform color metrics, default=use perceptual metrics" },
	{   PT_PAR, 0,                   "noAdaptiveBlocks", "Disable adaptive block sizes (i.e. disable macroblocks)." },
	{   PT_PAR, PF_PAR_ARGNUM_1,     "compressor",    "Set DXTn compressor, default=CRN" },
	{     PT_VAL, PF_VAL_STR,          "mode",          "[CRN,CRNF,RYG]" },
	{   PT_PAR, PF_PAR_ARGNUM_1,     "dxtQuality",    "Endpoint optimizer speed." },
	{     PT_VAL, PF_VAL_STR,          "level",         "[superfast,fast,normal,better,uber]" },
	{     PT_DOC, 0,                   NULL,            "            Sets endpoint optimizer's max iteration depth. Default=uber." },
	{   PT_PAR, 0,                   "noendpointcaching", "Don't try reusing previous DXT endpoint solutions." },
	{   PT_PAR, 0,                   "grayscalesampling", "Assume shader will convert fetched results to luma (Y)." },
	{   PT_PAR, 0,                   "forceprimaryencoding", "Only use DXT1 color4 and DXT5 alpha8 block encodings." },
	{   PT_PAR, 0,                   "usetransparentindicesforblack", "Try DXT1 transparent indices for dark pixels." },

	{ PT_CAT, 0,                   "output",        "Ouptut pixel format options" },
	{   PT_PAR, 0,                   "usesourceformat", "Use input file's format for output format (when possible)." },

	{ PT_CAT, 0,                   "format",        "All supported texture formats" },
	{   PT_DOC, 0,                   NULL,            "  (Note: .CRN only supports DXTn pixel formats)" },
	{   PT_PAR, 0,                   "DXT1",          NULL },
	{   PT_PAR, 0,                   "DXT2",          NULL },
	{   PT_PAR, 0,                   "DXT3",          NULL },
	{   PT_PAR, 0,                   "DXT4",          NULL },
	{   PT_PAR, 0,                   "DXT5",          NULL },
	{   PT_PAR, 0,                   "3DC",           NULL },
	//{   PT_PAR, 0,                   "DXN",           NULL },
	{   PT_PAR, 0,                   "DXT5A",         NULL },
	//{   PT_PAR, 0,                   "DXT5_CCxY",     NULL },
	//{   PT_PAR, 0,                   "DXT5_xGxR",     NULL },
	//{   PT_PAR, 0,                   "DXT5_xGBR",     NULL },
	//{   PT_PAR, 0,                   "DXT5_AGBR",     NULL },
	//{   PT_PAR, 0,                   "DXT1A",         NULL },
	//{   PT_PAR, 0,                   "ETC1",          NULL },
	{   PT_PAR, 0,                   "R8G8B8",        NULL },
	{   PT_PAR, 0,                   "L8",            NULL },
	{   PT_PAR, 0,                   "A8",            NULL },
	{   PT_PAR, 0,                   "A8L8",          NULL },
	{   PT_PAR, 0,                   "A8R8G8B8",      NULL },

	{ PT_END, 0, NULL, NULL }
};

typedef enum
{
	F_NONE = 0u,

	F_OUTSAMEDIR      =      0x1u,
	F_DEEP            =      0x2u,
	F_NOOVERWRITE     =      0x4u,
	F_TIMESTAMP       =      0x8u,
	F_FORCEWRITE      =     0x10u,
	F_RECREATE        =     0x20u,
	F_COMPAREMODE     =     0x40u,
	F_INFOMODE        =     0x80u,
	F_NOPROGRESS      =    0x100u,
	F_PARAMDEBUG      =    0x200u,
	F_DEBUG           =    0x400u,
	F_QUICK           =    0x800u,
	F_QUIET           =   0x1000u,
	F_IGNOREERRORS    =   0x2000u,
	F_PAUSE           =   0x4000u,
	F_NOSTATS         =   0x8000u,
	F_IMAGESTATS      =  0x10000u,
	F_MIPSTATS        =  0x20000u,
	F_LZMASTATS       =  0x40000u,
	F_SPLIT           =  0x80000u,
	F_YFLIP           = 0x100000u,
	F_UNFLIP          = 0x200000u,
	F_USESOURCEFORMAT = 0x400000u

} Flags;

typedef enum
{
	TXFM_INVALID,

	TXFM_DDS,
	TXFM_CRN,
	TXFM_KTX,

	// mipmapped
	TXFM_TGA,
	TXFM_PNG,
	TXFM_JPG,
	TXFM_JPEG,
	TXFM_BMP,
	TXFM_GIF,
	TXFM_TIF,
	TXFM_TIFF,
	TXFM_PPM,
	TXFM_PGM,
	TXFM_PSD,
	TXFM_JP2,

	NUM_TXFM

} FileTypes;

static FileTypes filetypeFromOutname(const char* filename)
{
	const char* ext = suffix(filename);
	if (strlen(ext) > 4 || strlen(ext) < 3)
		return TXFM_INVALID;
	const char* extensions[] =
	{
		[TXFM_DDS] = "dds",
		[TXFM_CRN] = "crn",
		[TXFM_KTX] = "ktx",

		[TXFM_TGA]  = "tga",
		[TXFM_PNG]  = "png",
		[TXFM_JPG]  = "jpg",
		[TXFM_JPEG] = "jpeg",
		[TXFM_BMP]  = "bmp",
		[TXFM_GIF]  = "gif",
		[TXFM_TIF]  = "tif",
		[TXFM_TIFF] = "tiff",
		[TXFM_PPM]  = "ppm",
		[TXFM_PGM]  = "pgm",
		[TXFM_PSD]  = "psd",
		[TXFM_JP2]  = "jp2"
	};

	for (FileTypes i = TXFM_INVALID + 1; i < NUM_TXFM; ++i)
		if (extensions[i] && !strcmp(extensions[i], ext))
			return i;
	return TXFM_INVALID;
}

typedef struct
{
	crn_mipmap_params mipParams;
	crn_comp_params compParams;
	PixelFormat pixFmt;
	Flags flags;
	FileTypes outFiletype;
	const char* inFilename;
	const char* outFilename;
	const char* outDir;
	const char* logFilename;
	const char* csvFilename;

} Params;

CMD_SEL_BLOCK_BEGIN(hello, Params)
	// File stuff
	     CMD_SEL_PAR("file")
	{
		//TODO
		o->inFilename = p->strVals[0];
		printf("file %s\n", p->strVals[0]);
	}
	else CMD_SEL_STRING(outFilename, "out")
	else CMD_SEL_STRING(outDir,      "outdir")
	else CMD_SEL_FLAGS(flags, "outsamedir",  F_OUTSAMEDIR)
	else CMD_SEL_FLAGS(flags, "deep",        F_DEEP)
	else CMD_SEL_FLAGS(flags, "nooverwrite", F_NOOVERWRITE)
	else CMD_SEL_FLAGS(flags, "timestamp",   F_TIMESTAMP)
	else CMD_SEL_FLAGS(flags, "forcewrite",  F_FORCEWRITE)
	else CMD_SEL_FLAGS(flags, "recreate",    F_RECREATE)
	else CMD_SEL_PAR("fileformat")
	{
		     CMD_SEL_MODE(outFiletype, "tga", TXFM_TGA)
		else CMD_SEL_MODE(outFiletype, "bmp", TXFM_BMP)
		else CMD_SEL_MODE(outFiletype, "dds", TXFM_DDS)
		else CMD_SEL_MODE(outFiletype, "ktx", TXFM_KTX)
		else CMD_SEL_MODE(outFiletype, "crn", TXFM_CRN)
		else CMD_SEL_MODE(outFiletype, "png", TXFM_PNG)
		else CMD_SEL_MODE_ERR("Unsupported output file type")
	}

	// Modes
	else CMD_SEL_FLAGS(flags, "compare", F_COMPAREMODE)
	else CMD_SEL_FLAGS(flags, "info",    F_INFOMODE)

	// Misc
	else CMD_SEL_INT(compParams.num_helper_threads, "helperThreads", 0, cCRNMaxHelperThreads)
	//else CMD_SEL_FLAGS(flags, "noprogress",   F_NOPROGRESS && "quiet")
	else CMD_SEL_FLAGS(flags, "paramdebug",   F_PARAMDEBUG)
	else CMD_SEL_FLAGS(flags, "debug",        F_DEBUG)
	else CMD_SEL_FLAGS(flags, "quick",        F_QUICK)
	else CMD_SEL_FLAGS(flags, "quiet",        F_QUIET)
	else CMD_SEL_FLAGS(flags, "ignoreerrors", F_IGNOREERRORS)
	else CMD_SEL_STRING(logFilename, "logfile")
	else CMD_SEL_FLAGS(flags, "pause", F_PAUSE)
	else CMD_SEL_PAR("window")
	{
		uint32_t xl = (uint32_t)cmdclampint("window", p->intVals[0], 0, cCRNMaxLevelResolution);
		uint32_t yl = (uint32_t)cmdclampint("window", p->intVals[1], 0, cCRNMaxLevelResolution);
		uint32_t xh = (uint32_t)cmdclampint("window", p->intVals[2], 0, cCRNMaxLevelResolution);
		uint32_t yh = (uint32_t)cmdclampint("window", p->intVals[3], 0, cCRNMaxLevelResolution);

		o->mipParams.window_left   = MIN(xl, xh);
		o->mipParams.window_top    = MIN(yl, yh);
		o->mipParams.window_right  = MAX(xl, xh);
		o->mipParams.window_bottom = MAX(yl, yh);
	}
	else CMD_SEL_PAR("clamp")
	{
		o->mipParams.clamp_scale  = false;
		o->mipParams.clamp_width  = (uint32_t)cmdclampint("clamp", p->intVals[0], 1, cCRNMaxLevelResolution);
		o->mipParams.clamp_height = (uint32_t)cmdclampint("clamp", p->intVals[1], 1, cCRNMaxLevelResolution);
	}
	else CMD_SEL_PAR("clampScale")
	{
		o->mipParams.clamp_scale  = true;
		o->mipParams.clamp_width  = (uint32_t)cmdclampint("clampScale", p->intVals[0], 1, cCRNMaxLevelResolution);
		o->mipParams.clamp_height = (uint32_t)cmdclampint("clampScale", p->intVals[1], 1, cCRNMaxLevelResolution);
	}
	else CMD_SEL_FLAGS(flags, "nostats",    F_NOSTATS)
	else CMD_SEL_FLAGS(flags, "imagestats", F_IMAGESTATS)
	else CMD_SEL_FLAGS(flags, "mipstats",   F_MIPSTATS)
	else CMD_SEL_FLAGS(flags, "lzmastats",  F_LZMASTATS)
	else CMD_SEL_FLAGS(flags, "split",      F_SPLIT)
	else CMD_SEL_STRING(csvFilename, "csvfile")
	else CMD_SEL_FLAGS(flags, "yflip",  F_YFLIP)
	else CMD_SEL_FLAGS(flags, "unflip", F_UNFLIP)

	// Rescaling
	else CMD_SEL_PAR("rescale")
	{
		o->mipParams.scale_mode = cCRNSMAbsolute;
		o->mipParams.scale_x = (float)cmdclampint("rescale", p->intVals[0], 1, cCRNMaxLevelResolution);
		o->mipParams.scale_y = (float)cmdclampint("rescale", p->intVals[1], 1, cCRNMaxLevelResolution);
	}
	else CMD_SEL_PAR("relrescale")
	{
		o->mipParams.scale_mode = cCRNSMRelative;
		o->mipParams.scale_x = cmdclampflt("relrescale", p->fltVals[0], 1, 256);
		o->mipParams.scale_y = cmdclampflt("relrescale", p->fltVals[1], 1, 256);
	}
	else CMD_SEL_PAR("rescalemode")
	{
		     CMD_SEL_MODE(mipParams.scale_mode, "nearest", cCRNSMNearestPow2)
		else CMD_SEL_MODE(mipParams.scale_mode, "hi",      cCRNSMNextPow2)
		else CMD_SEL_MODE(mipParams.scale_mode, "lo",      cCRNSMLowerPow2)
		else CMD_SEL_MODE_ERR("Invalid rescale mode")
	}

	// Quality
	else if (!strcmp(p->name, "q") || !strcmp(p->name, "quality"))
	{
		o->compParams.quality_level = (uint32_t)cmdclampint(p->name, p->intVals[0], cCRNMinQualityLevel, cCRNMaxQualityLevel);
	}
	else CMD_SEL_FLOAT(compParams.target_bitrate, "bitrate", 0.1, 30.0)

	// crnlib
	else CMD_SEL_FLOAT(compParams.crn_color_endpoint_palette_size, "c",  cCRNMinPaletteSize, cCRNMaxPaletteSize)
	else CMD_SEL_FLOAT(compParams.crn_color_selector_palette_size, "s",  cCRNMinPaletteSize, cCRNMaxPaletteSize)
	else CMD_SEL_FLOAT(compParams.crn_alpha_endpoint_palette_size, "ca", cCRNMinPaletteSize, cCRNMaxPaletteSize)
	else CMD_SEL_FLOAT(compParams.crn_alpha_selector_palette_size, "sa", cCRNMinPaletteSize, cCRNMaxPaletteSize)

	// Mipmap generation
	else CMD_SEL_PAR("mipMode")
	{
		     CMD_SEL_MODE(mipParams.mode, "UseSourceOrGenerate", cCRNMipModeUseSourceOrGenerateMips)
		else CMD_SEL_MODE(mipParams.mode, "UseSource",           cCRNMipModeUseSourceMips)
		else CMD_SEL_MODE(mipParams.mode, "Generate",            cCRNMipModeGenerateMips)
		else CMD_SEL_MODE(mipParams.mode, "None",                cCRNMipModeNoMips)
		else CMD_SEL_MODE_ERR("Invalid MipMode")
	}
	else CMD_SEL_PAR("mipFilter")
	{
		     CMD_SEL_MODE(mipParams.filter, "box",      cCRNMipFilterBox)
		else CMD_SEL_MODE(mipParams.filter, "tent",     cCRNMipFilterTent)
		else CMD_SEL_MODE(mipParams.filter, "lanczos4", cCRNMipFilterLanczos4)
		else CMD_SEL_MODE(mipParams.filter, "mitchell", cCRNMipFilterMitchell)
		else CMD_SEL_MODE(mipParams.filter, "kaiser",   cCRNMipFilterKaiser)
	    else CMD_SEL_MODE_ERR("Invalid MipFilter")
	}
	else CMD_SEL_FLOAT(mipParams.gamma,      "gamma",      0.1f, 8.0f)
	else CMD_SEL_FLOAT(mipParams.blurriness, "blurriness", 0.01f, 8.0f)
	else CMD_SEL_SET(mipParams.tiled,       "wrap",        crn_true)
	else CMD_SEL_SET(mipParams.renormalize, "renormalize", crn_true)
	else CMD_SEL_INT(mipParams.max_levels,  "maxmips",    1, cCRNMaxLevels)
	else CMD_SEL_INT(mipParams.min_mip_size,"minmipsize", 1, cCRNMaxLevelResolution)

	// Compression
	else CMD_SEL_INT(compParams.dxt1a_alpha_threshold, "alphaThreshold", 0, 255)
	else CMD_SEL_FLAGS(compParams.flags, "uniformMetrics",   cCRNCompFlagPerceptual)
	else CMD_SEL_FLAGS(compParams.flags, "noAdaptiveBlocks", cCRNCompFlagHierarchical)
	else CMD_SEL_PAR("compressor")
	{
		     CMD_SEL_MODE(compParams.dxt_compressor_type, "CRN",  cCRNDXTCompressorCRN)
		else CMD_SEL_MODE(compParams.dxt_compressor_type, "CRNF", cCRNDXTCompressorCRNF)
		else CMD_SEL_MODE(compParams.dxt_compressor_type, "RYG",  cCRNDXTCompressorRYG)
		//else CMD_SEL_MODE(compParams.dxt_compressor_type, "ATI",  cCRNDXTCompressorATI)
		else CMD_SEL_MODE_ERR("Invalid compressor")
	}
	else CMD_SEL_PAR("dxtQuality")
	{
		     CMD_SEL_MODE(compParams.dxt_quality, "superfast", cCRNDXTQualitySuperFast)
		else CMD_SEL_MODE(compParams.dxt_quality, "fast",      cCRNDXTQualityFast)
		else CMD_SEL_MODE(compParams.dxt_quality, "normal",    cCRNDXTQualityNormal)
		else CMD_SEL_MODE(compParams.dxt_quality, "better",    cCRNDXTQualityBetter)
		else CMD_SEL_MODE(compParams.dxt_quality, "uber",      cCRNDXTQualityUber)
		else CMD_SEL_MODE_ERR("Invalid DXT quality")

	}
	else CMD_SEL_FLAGS(compParams.flags, "noendpointcaching",             cCRNCompFlagDisableEndpointCaching)
	else CMD_SEL_FLAGS(compParams.flags, "grayscalesampling",             cCRNCompFlagGrayscaleSampling)
	//else CMD_SEL_FLAGS(compParams.flags, "forceprimaryencoding",          !cCRNCompFlagUseBothBlockTypes)
	//else CMD_SEL_FLAGS(compParams.flags, "usetransparentindicesforblack", cCRNCompFlagUseBothBlockTypes && "usetransparentindicesforblack")

	// Output pixel format
	else CMD_SEL_SET(flags, "usesourceformat", F_USESOURCEFORMAT)

	// Texture formats
	else CMD_SEL_SET(pixFmt, "DXT1",     PXFM_BC1_DXT1)
	else CMD_SEL_SET(pixFmt, "DXT3",     PXFM_BC2_DXT3)
	else CMD_SEL_SET(pixFmt, "DXT5",     PXFM_BC3_DXT5)
	else CMD_SEL_SET(pixFmt, "3DC",      PXFM_BC5_3DC)
	else CMD_SEL_SET(pixFmt, "DXT5A",    PXFM_BC4_DXT5A)
	else CMD_SEL_SET(pixFmt, "R8G8B8",   PXFM_R8G8B8)
	else CMD_SEL_SET(pixFmt, "L8",       PXFM_L8)
	else CMD_SEL_SET(pixFmt, "A8",       PXFM_A8)
	else CMD_SEL_SET(pixFmt, "A8L8",     PXFM_A8L8)
	else CMD_SEL_SET(pixFmt, "A8R8G8B8", PXFM_A8R8G8B8)

CMD_SEL_BLOCK_END()


static inline void paramsClear(Params* o)
{
	crn_mipmap_params_clear(&o->mipParams);
	//o->mipParams;

	crn_comp_params_clear(&o->compParams);
	crn_comp_params_set_flag(&o->compParams, cCRNCompFlagPerceptual,        crn_false);
	crn_comp_params_set_flag(&o->compParams, cCRNCompFlagHierarchical,      crn_false);
	crn_comp_params_set_flag(&o->compParams, cCRNCompFlagUseBothBlockTypes, crn_false);

	o->pixFmt = PXFM_INVALID;
	o->flags = F_NONE;
	o->inFilename  = NULL;
	o->outFiletype = TXFM_INVALID;
	o->outFilename = NULL;
	o->outDir      = NULL;
	o->csvFilename = NULL;
}

int main(int argc, char** argv)
{
	Params o;
	paramsClear(&o);

	for (int i = 1;; ++i)
	{
		CmdParam param;
		CmdReturnCode code = cmdparse(argc, argv, &i, &param);
		if (code == CR_FINISH)
			break;
		if (code == CR_EXIT_USAGE || code == CR_EXIT_ERROR)
			return code;

		if (!hello(&param, &o))
			return CR_EXIT_ERROR;
	}

	// if outsamedir
	//   out = inpath/infname.outext
	// elif out != null
	//   out = out
	//   if numfiles > 1
	//     oname = oname_fileidx
	//     out = outpath/oname/
	//   if fileformat == null
	//     outftype = determine_file_format(out)
	// else
	//

	if (o.flags & F_OUTSAMEDIR)
	{
	//	const char* inbase = basename(o.inFilename);
	//	const char* ext = suffix(inbase);
	//	printf("%.*s%.*s%s\n",
	//		(int)(inbase - o.inFilename), o.inFilename,
	//		(int)(ext - inbase), inbase,
	//		ext);
	//	//o.outFilename = malloc();
		const char* ext = suffix(o.inFilename);
		printf("%.*s%.*s%s\n", "");
	}

	if (o.outFilename)
		if (o.outFiletype == TXFM_INVALID)
			o.outFiletype = filetypeFromOutname(o.outFilename);

	return 0;

#define PRINT_FLAG(STRUCT, PAD, FLAG) if (STRUCT.flags & FLAG) printf("%*s%s\n", PAD, "", #FLAG);

	printf("o.compParams.file_type:                       %d\n", o.compParams.file_type);
	printf("o.compParams.faces:                           %d\n", o.compParams.faces);
	printf("o.compParams.width:                           %d\n", o.compParams.width);
	printf("o.compParams.height:                          %d\n", o.compParams.height);
	printf("o.compParams.levels:                          %d\n", o.compParams.levels);
	printf("o.compParams.format:                          %ld\n", o.compParams.format);
	printf("o.compParams.flags:                           %u\n", o.compParams.flags);
	PRINT_FLAG(o.compParams, 46, cCRNCompFlagPerceptual)
	PRINT_FLAG(o.compParams, 46, cCRNCompFlagHierarchical)
	PRINT_FLAG(o.compParams, 46, cCRNCompFlagQuick)
	PRINT_FLAG(o.compParams, 46, cCRNCompFlagUseBothBlockTypes)
	PRINT_FLAG(o.compParams, 46, cCRNCompFlagUseTransparentIndicesForBlack)
	PRINT_FLAG(o.compParams, 46, cCRNCompFlagDisableEndpointCaching)
	PRINT_FLAG(o.compParams, 46, cCRNCompFlagManualPaletteSizes)
	PRINT_FLAG(o.compParams, 46, cCRNCompFlagDXT1AForTransparency)
	PRINT_FLAG(o.compParams, 46, cCRNCompFlagGrayscaleSampling)
	PRINT_FLAG(o.compParams, 46, cCRNCompFlagDebugging)
	printf("o.compParams.target_bitrate:                  %f\n", o.compParams.target_bitrate);
	printf("o.compParams.quality_level:                   %d\n", o.compParams.quality_level);
	printf("o.compParams.dxt1a_alpha_threshold:           %d\n", o.compParams.dxt1a_alpha_threshold);
	printf("o.compParams.dxt_quality:                     %d\n", o.compParams.dxt_quality);
	printf("o.compParams.dxt_compressor_type:             %d\n", o.compParams.dxt_compressor_type);
	printf("o.compParams.alpha_component:                 %d\n", o.compParams.alpha_component);
	printf("o.compParams.crn_color_endpoint_palette_size: %d\n", o.compParams.crn_color_endpoint_palette_size);
	printf("o.compParams.crn_color_selector_palette_size: %d\n", o.compParams.crn_color_selector_palette_size);
	printf("o.compParams.crn_alpha_endpoint_palette_size: %d\n", o.compParams.crn_alpha_endpoint_palette_size);
	printf("o.compParams.crn_alpha_selector_palette_size: %d\n", o.compParams.crn_alpha_selector_palette_size);
	printf("o.compParams.num_helper_threads:              %d\n", o.compParams.num_helper_threads);
	printf("\n");
	printf("mipParams.mode:            %d\n", o.mipParams.mode);
	printf("mipParams.filter:          %d\n", o.mipParams.filter);
	printf("mipParams.gamma_filtering: %d\n", o.mipParams.gamma_filtering);
	printf("mipParams.gamma:           %f\n", o.mipParams.gamma);
	printf("mipParams.blurriness:      %f\n", o.mipParams.blurriness);
	printf("mipParams.max_levels:      %d\n", o.mipParams.max_levels);
	printf("mipParams.min_mip_size:    %d\n", o.mipParams.min_mip_size);
	printf("mipParams.renormalize:     %s\n", o.mipParams.renormalize ? "true" : "false");
	printf("mipParams.tiled:           %s\n", o.mipParams.tiled ? "true" : "false");
	printf("mipParams.scale_mode:      %d\n", o.mipParams.scale_mode);
	printf("mipParams.scale_x:         %f\n", o.mipParams.scale_x);
	printf("mipParams.scale_y:         %f\n", o.mipParams.scale_y);
	printf("mipParams.window_left:     %d\n", o.mipParams.window_left);
	printf("mipParams.window_top:      %d\n", o.mipParams.window_top);
	printf("mipParams.window_right:    %d\n", o.mipParams.window_right);
	printf("mipParams.window_bottom:   %d\n", o.mipParams.window_bottom);
	printf("mipParams.clamp_scale:     %d\n", o.mipParams.clamp_scale);
	printf("mipParams.clamp_width:     %d\n", o.mipParams.clamp_width);
	printf("mipParams.clamp_height:    %d\n", o.mipParams.clamp_height);
	printf("\n");
	printf("pixFmt:      %d\n", o.pixFmt);
	printf("flags:       %u\n", o.flags);
	PRINT_FLAG(o, 13, F_OUTSAMEDIR)
	PRINT_FLAG(o, 13, F_DEEP)
	PRINT_FLAG(o, 13, F_NOOVERWRITE)
	PRINT_FLAG(o, 13, F_TIMESTAMP)
	PRINT_FLAG(o, 13, F_FORCEWRITE)
	PRINT_FLAG(o, 13, F_RECREATE)
	PRINT_FLAG(o, 13, F_COMPAREMODE)
	PRINT_FLAG(o, 13, F_INFOMODE)
	PRINT_FLAG(o, 13, F_NOPROGRESS)
	PRINT_FLAG(o, 13, F_PARAMDEBUG)
	PRINT_FLAG(o, 13, F_DEBUG)
	PRINT_FLAG(o, 13, F_QUICK)
	PRINT_FLAG(o, 13, F_QUIET)
	PRINT_FLAG(o, 13, F_IGNOREERRORS)
	PRINT_FLAG(o, 13, F_PAUSE)
	PRINT_FLAG(o, 13, F_NOSTATS)
	PRINT_FLAG(o, 13, F_IMAGESTATS)
	PRINT_FLAG(o, 13, F_MIPSTATS)
	PRINT_FLAG(o, 13, F_LZMASTATS)
	PRINT_FLAG(o, 13, F_SPLIT)
	PRINT_FLAG(o, 13, F_YFLIP)
	PRINT_FLAG(o, 13, F_UNFLIP)
	PRINT_FLAG(o, 13, F_USESOURCEFORMAT)
	printf("outFiletype: %d\n", o.outFiletype);
	printf("outFilename: \"%s\"\n", o.outFilename);
	printf("outDir:      \"%s\"\n", o.outDir);
	printf("csvFilename: \"%s\"\n", o.csvFilename);

	return 0;
}
