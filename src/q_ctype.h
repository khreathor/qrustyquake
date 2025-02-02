// Locale insensitive ctype.h functions taken from the RPM library -
// RPM is Copyright (c) 1998 by Red Hat Software, Inc.
// GPLv3 See LICENSE for details.


#ifndef Q_CTYPE_H
#define Q_CTYPE_H

static inline int q_isascii(int c)
{
	return ((c & ~0x7f) == 0);
}

static inline int q_islower(int c)
{
	return (c >= 'a' && c <= 'z');
}

static inline int q_isupper(int c)
{
	return (c >= 'A' && c <= 'Z');
}

static inline int q_isalpha(int c)
{
	return (q_islower(c) || q_isupper(c));
}

static inline int q_isdigit(int c)
{
	return (c >= '0' && c <= '9');
}

static inline int q_isxdigit(int c)
{
	return (q_isdigit(c) || (c >= 'a' && c <= 'f') ||
				(c >= 'A' && c <= 'F'));
}

static inline int q_isalnum(int c)
{
	return (q_isalpha(c) || q_isdigit(c));
}

static inline int q_isblank(int c)
{
	return (c == ' ' || c == '\t');
}

static inline int q_isspace(int c)
{
	switch(c) {
	case ' ':  case '\t':
	case '\n': case '\r':
	case '\f': case '\v': return 1;
	}
	return 0;
}

static inline int q_isgraph(int c)
{
	return (c > 0x20 && c <= 0x7e);
}

static inline int q_isprint(int c)
{
	return (c >= 0x20 && c <= 0x7e);
}

static inline int q_toascii(int c)
{
	return (c & 0x7f);
}

static inline int q_tolower(int c)
{
	return ((q_isupper(c)) ? (c | ('a' - 'A')) : c);
}

static inline int q_toupper(int c)
{
	return ((q_islower(c)) ? (c & ~('a' - 'A')) : c);
}

#endif /* Q_CTYPE_H */
