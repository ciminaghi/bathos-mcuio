#include <stdarg.h>
#include <pp-printf.h>

/*
 * empty vsprintf: only the format string. Public domain
 */
int pp_vsprintf(char *buf, const char * PROGMEM fmt, va_list args)
{
	char *str = buf;

	for (; ; ++fmt) {
		char c;
		c = __get_fmt_char(*fmt);
		*str++ = c;
		if (!c)
			break;
	}
	*str++ = '\0';
	return str - buf;
}
