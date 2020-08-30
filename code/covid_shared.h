
internal void
copy_string(char *dest, char *src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest++ = 0;
}

internal void
copy_string_n(char *dest, char *src, int count) {
	while (*src && count--) {
		*dest++ = *src++;
	}
	if (count) {
		*dest++ = 0;
    }
}

internal int
string_length(char *str) {
	int count = 0;
	while (str[count])
		count++;
	return count;
}

internal void
string_concat(char *dest, char *src) {
    dest += string_length(dest);
    copy_string(dest, src);
}

internal b32
strings_are_equal(char *a, char *b) {
    while (1) {
		if (*a != *b)
			return -1;              // strings not equal
		if (!*a)
			return 0;               // strings are equal
		a++;
		b++;
	}
	return -1;
}