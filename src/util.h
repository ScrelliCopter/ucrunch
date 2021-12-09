#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

#define MAX(A, B) ((A > B) ? (A) : (B))
#define MIN(A, B) ((A < B) ? (A) : (B))
#define CLAMP(X, A, B) (MIN(B, MAX(A, X)))

#define LERP(A, B, X) ((A) * (1 - (X)) + (B) * (X))
#define BILERP(A, B, C, D, X, Y) (LERP(LERP((A), (B), (X)), LERP((C), (D), (X)), (Y)))

#define COUNTOF(A) (sizeof(A) / sizeof(*(A)))

#define FOURCC(S) ( \
	((unsigned)S[0] <<  0u) | \
	((unsigned)S[1] <<  8u) | \
	((unsigned)S[2] << 16u) | \
	((unsigned)S[3] << 24u))

#define FINDEND(X) for (; (*(X)) != '\0'; ++(X))
inline char* basename(const char* path)
{
	if (!path) return NULL;
	const char* base = path;
	FINDEND(base);
#ifndef _WIN32
	for (; base > path && (*(base - 1)) != '/'; --base);
#else
	for (; base > path && (*(base - 1)) != '\\'; --base);
#endif
	return (char*)base;
}

inline char* suffix(const char* path)
{
	if (!path) return NULL;
	char* base = basename(path);
	char* end = base;
	FINDEND(end);
	char* ext = end;
	for (;; --ext)
	{
		if (ext == base)
			return end;
		if ((*(ext - 1)) == '.')
			return ext;
	}
}

#endif//UTIL_H
