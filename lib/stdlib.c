#include <bathos/stdlib.h>
#include <bathos/types.h>

int32_t atol(const char *s)
{
	int32_t r = 0;
	int i = 0, neg = 0;

	switch (s[0]) {
	case '-':
		neg = 1;
	case '+':
		i++;
		break;
	default:
		break;
	}

	while ((s[i] >= '0') && (s[i] <= '9')) {
		r = r * 10 + s[i] - '0';
		i++;
	}

	return neg ? -r : r;
}
