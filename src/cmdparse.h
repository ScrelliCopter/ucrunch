#ifndef CMDPARSE_H
#define CMDPARSE_H

typedef enum { PT_END, PT_CAT, PT_PAR, PT_VAL, PT_DOC } CmdParamType;
typedef enum
{
	PF_PAR_HIDDEN   = (1u << 0u),
	PF_PAR_ARGNUM_1 = (1u << 1u),
	PF_PAR_ARGNUM_2 = (2u << 1u),
	PF_PAR_ARGNUM_4 = (3u << 1u),
	PF_PAR_ARGNUM_MASK = 0x6u,

	PF_VAL_INT  = (1u << 2u),
	PF_VAL_FLT  = (2u << 2u),
	PF_VAL_STR  = (3u << 2u),
	//PF_VAL_ESTR = (4u << 2u),
	PF_VAL_ESTR = PF_VAL_STR,
	PF_VAL_TYPES_MASK = 0x1Cu,
	PF_VAL_OPTIONAL = (1u << 0u),
	PF_VAL_LISTFILE = (1u << 1u) | PF_VAL_STR

} CmdParamFlags;
#define GET_ARGNUM(N)   ((unsigned[]){0, 1, 2, 4}[((N) & PF_PAR_ARGNUM_MASK) >> 1u])
#define GET_VAL_TYPE(N) ((N) & PF_VAL_TYPES_MASK)
#define PF_CAT_LPAD(N)  (((N) & 0x3Fu) << 0u)
#define GET_LPAD(N)     (((N) >> 0u) & 0x3Fu)
typedef struct
{
	CmdParamType kind;
	CmdParamFlags flags;
	const char* name;
	const char* help;

} CmdParamEntry;

typedef enum
{
	CR_EXIT_USAGE = 0,
	CR_EXIT_ERROR = 1,
	CR_CONTINUE,
	CR_FINISH

} CmdReturnCode;
typedef struct
{
	const char* name;

	int         intVals[4];
	float       fltVals[4];
	const char* strVals[4];

} CmdParam;
CmdReturnCode cmdparse(int argc, char** argv, int* i, CmdParam* param);

int   cmdclampint(const char* key, int x, int min, int max);
float cmdclampflt(const char* key, float x, float min, float max);

#define CMD_SEL_BLOCK_BEGIN(FUNCNAME, TYPE) \
	bool FUNCNAME(const CmdParam* p, TYPE* o) {
#define CMD_SEL_BLOCK_END() return true; }

#define CMD_SEL_PAR(PAR) if (!strcmp(p->name, PAR))
#define CMD_SEL_FLAGS(VAR, PAR, FLAGS) \
	CMD_SEL_PAR(PAR) { o->VAR |= FLAGS; }
#define CMD_SEL_SET(VAR, PAR, SET) \
	CMD_SEL_PAR(PAR) { o->VAR = SET; }
#define CMD_SEL_STRING(VAR, PAR) \
	CMD_SEL_PAR(PAR) { o->VAR = p->strVals[0]; }
#define CMD_SEL_INT(VAR, PAR, MIN, MAX) \
	CMD_SEL_PAR(PAR) { o->VAR = cmdclampint(PAR, p->intVals[0], MIN, MAX); }
#define CMD_SEL_FLOAT(VAR, PAR, MIN, MAX) \
	CMD_SEL_PAR(PAR) { o->VAR = cmdclampflt(PAR, p->fltVals[0], MIN, MAX); }

#define CMD_SEL_MODE(VAR, PAR, SET) \
	if (!strcmp(p->strVals[0], PAR)) { o->VAR = SET; }
#define CMD_SEL_MODE_ERR(MSG) \
	{ fprintf(stderr, "Error: "MSG": \"%s\"\n", p->strVals[0]); return false; }

extern const CmdParamEntry params[];

#endif//CMDPARSE_H
