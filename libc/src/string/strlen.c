// __is_libk

#include <string.h>

size_t strlen(const char *str) {
	size_t len = 0;

	while(str && *str != '\0') {
		++str;
		++len;
	}

	return len;
}
