#include "cmdparse.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <math.h>

#ifdef _WIN32
 #define CMDLINE_SLASH_PARAMS 1
#endif

static unsigned usageParam(FILE* f, char switchc, const CmdParamEntry* p, int lpad);
static void usageBasic(FILE* f, char switchc);
static bool usageAdvanced(FILE* f, char switchc, const char* cat);
static bool searchParam(const char* cmd, const CmdParamEntry** out);
static bool parseValue(const CmdParamEntry* p, const CmdParamEntry* a, const char* token, int* outInt, float* outFlt, const char** outStr);


unsigned usageParam(FILE* f, char switchc, const CmdParamEntry* p, int lpad)
{
	unsigned argnum = GET_ARGNUM(p->flags);
	if (p->flags & PF_PAR_HIDDEN)
		return argnum;

	int linelen = fprintf(f, "  %c%s ", switchc, p->name);
	const char* help = p->help;
	for (unsigned i = 0; i < argnum; ++i)
	{
		++p;
		//printf("%d/%d: %d\n", i, argnum, p->kind);
		assert(p->kind == PT_VAL);

		if (p->help)
			linelen += fprintf(f, "%s ", p->help);
		else if (p->flags & PF_VAL_OPTIONAL)
			linelen += fprintf(f, "[%s] ", p->name);
		else
			linelen += fprintf(f, "<%s> ", p->name);
	}
	for (int i = 0; i < lpad - linelen; ++i)
		fputc(' ', f);
	if (help)
		fprintf(f, "- %s", help);
	fputc('\n', f);
	return argnum;
}

void usageBasic(FILE* f, char switchc)
{
	fprintf(f, "Command line usage:\n");
	const CmdParamEntry* p;
	int lpad = 0;
	for (p = params; ; ++p)
	{
		if (p->kind == PT_END || p->kind == PT_CAT)
			break;
		if (p->kind == PT_PAR)
			p += usageParam(f, switchc, p, lpad);
	}
	usageParam(f, switchc, (const CmdParamEntry[])
	{
		{ PT_PAR, PF_PAR_ARGNUM_1, "help", "Show this help." },
		{ PT_VAL, 0, "category", NULL }
	}, lpad);

	fprintf(f, "\n  Categories (use %chelp \"category\" to view full parameter list):\n", switchc);
	fprintf(f, "    \"all\" - Show help for all categories\n");
	for (;; ++p)
	{
		if (p->kind == PT_END)
			break;
		if (p->kind == PT_CAT)
		{
			lpad = GET_LPAD(p->flags);
			fprintf(f, "    \"%s\" - %s\n", p->name, p->help);
		}
	}
}

bool usageAdvanced(FILE* f, char switchc, const char* cat)
{
	const CmdParamEntry* p = params;
	int lpad = 0;
	bool all = strcmp(cat, "all") ? false : true;
	if (!all)
	{
		for (;; ++p)
		{
			if (p->kind == PT_END)
			{
				fprintf(stderr, "Error: No such help category \"%s\".\n", cat);
				fprintf(stderr, "type %chelp (with no argument) to see a full list.\n", switchc);
				return false;
			}
			if (p->kind == PT_CAT)
				if (!strcmp(p->name, cat))
					break;
		}

		lpad = GET_LPAD(p->flags);
		fprintf(f, "%s:\n", p->help);
		++p;
	}
	else
	{
		fprintf(f, "Command line usage:\n");
	}

	for (;; ++p)
	{
		if (p->kind == PT_END)
			break;
		if (p->kind == PT_CAT)
		{
			if (all)
			{
				lpad = GET_LPAD(p->flags);
				fprintf(f, "\n%s:\n", p->help);
				continue;
			}
			break;
		}
		if (p->kind == PT_PAR)
		{
			p += usageParam(f, switchc, p, lpad);
		}
		else if (p->kind == PT_DOC)
		{
			fprintf(f, "  %s\n", p->help ? p->help : "");
		}
	}

	return true;
}

bool searchParam(const char* cmd, const CmdParamEntry** out)
{
	for (const CmdParamEntry* p = params;; ++p)
	{
		if (p->kind == PT_END)
			return false;
		if (p->kind == PT_PAR && !strcmp(p->name, cmd))
		{
			(*out) = p;
			return true;
		}
	}
}

bool parseValue(const CmdParamEntry* p, const CmdParamEntry* a, const char* token, int* outInt, float* outFlt, const char** outStr)
{
	char* end;
	switch (GET_VAL_TYPE(a->flags))
	{
	case (PF_VAL_INT):
		errno = 0;
		*outInt = (int)strtol(token, &end, 10);
		if (errno == ERANGE)
		{
			fprintf(stderr, "Error: Invalid value for \"%s\" arg \"%s\": Numerical input of range\n", p->name, a->name);
			return false;
		}
		else if (token == end || (*end) != '\0')
		{
			fprintf(stderr, "Error: Invalid value for \"%s\" arg \"%s\": Expected an integer, got string\n", p->name, a->name);
			return false;
		}
		return true;

	case (PF_VAL_FLT):
		errno = 0;
		*outFlt = (float)strtof(token, &end);
		if (errno == ERANGE)
		{
			fprintf(stderr, "Error: Invalid value for \"%s\" arg \"%s\": Numerical input of range\n", p->name, a->name);
			return false;
		}
		else if (token == end || (*end) != '\0')
		{
			fprintf(stderr, "Error: Invalid value for \"%s\" arg \"%s\": Expected an float, got string\n", p->name, a->name);
			return false;
		}
		else if (isnan(*outFlt))
		{
			fprintf(stderr, "Error: Invalid value for \"%s\" arg \"%s\": Expected an float, got NaN\n", p->name, a->name);
			return false;
		}
		return true;

	case (PF_VAL_STR):
		*outStr = token;
		return true;

	default:
		return false;
	}
}


CmdReturnCode cmdparse(int argc, char** argv, int* i, CmdParam* param)
{
	if (argc == 1)
	{
		fprintf(stderr, "Error: No command line parameters specified!\n\n");
		usageBasic(stderr, '-');
		return EXIT_FAILURE;
	}
	if ((*i) >= argc)
		return CR_FINISH;

	const char* token = argv[*i];

#ifdef CMDLINE_SLASH_PARAMS
	bool iscmd = (token[0] == '-' || token[0] == '/') ? true : false;
#else
	bool iscmd = token[0] == '-' ? true : false;
#endif
	if (!iscmd)
	{
		fprintf(stderr, "Error: Unexpected token: \"%s\"\n", token);
		return CR_EXIT_ERROR;
	}

	if (!strcmp(token + 1, "help"))
	{
		char switchc = token[0];
		if (argc - (*i) == 1)
		{
			usageBasic(stdout, switchc);
			return CR_EXIT_USAGE;
		}
		else if (argc - (*i) == 2)
		{
			return usageAdvanced(stdout, switchc, argv[(*i) + 1]) ? EXIT_SUCCESS : EXIT_FAILURE;
		}
		else
		{
			fprintf(stderr, "Error: Too many arguments passed to %chelp (takes zero or one at most).\n", switchc);
			return CR_EXIT_ERROR;
		}
	}

	const CmdParamEntry* p;
	if (!searchParam(token + 1, &p))
	{
		fprintf(stderr, "Error: Invalid command line parameter: \"%s\"\n", token);
		return CR_EXIT_ERROR;
	}

	unsigned argnum = GET_ARGNUM(p->flags);
	for (unsigned j = 0; j < argnum; ++j)
	{
		if (++(*i) == argc)
		{
			fprintf(stderr,
				"Error: Expected %d value%s after command line parameter: \"%s\"\n",
				argnum, argnum > 1 ? "s" : "", token);
			return CR_EXIT_USAGE;
		}

		const CmdParamEntry* a = p + 1 + j;
		assert(a->kind == PT_VAL);

		if (!parseValue(p, a, argv[*i],
			&param->intVals[j],
			&param->fltVals[j],
			&param->strVals[j]))
		{
			return CR_EXIT_ERROR;
		}
	}

	param->name = p->name;
	return CR_CONTINUE;
}

int cmdclampint(const char* key, int x, int min, int max)
{
	if (x < min)
	{
		fprintf(stderr, "Warning: Value %d for parameter \"%s\" is out of range, clamping to %d\n", x, key, min);
		return min;
	}
	else if (x > max)
	{
		fprintf(stderr, "Warning: Value %d for parameter \"%s\" is out of range, clamping to %d\n", x, key, max);
		return max;
	}

	return x;
}

float cmdclampflt(const char* key, float x, float min, float max)
{
	if (x < min)
	{
		fprintf(stderr, "Warning: Value %f for parameter \"%s\" is out of range, clamping to %f\n", x, key, min);
		return min;
	}
	else if (x > max)
	{
		fprintf(stderr, "Warning: Value %f for parameter \"%s\" is out of range, clamping to %f\n", x, key, max);
		return max;
	}

	return x;
}
