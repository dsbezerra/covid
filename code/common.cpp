void
strcpy(char *dest, char *src) {
	while (*src) {
		*dest++ = *src++;
	}
	*dest++ = 0;
}

void
strcpy(unsigned char *dest, unsigned char *src) {
	while (*src) {
		*dest++ = *src++;
	}
	*dest++ = 0;
}

void
strncpy(char *dest, char *src, int count) {
	while (*src && count--) {
		*dest++ = *src++;
	}
	if (count) {
		*dest++ = 0;
    }
}

void
strncpy(unsigned char *dest, unsigned char *src, int count) {
	while (*src && count--) {
		*dest++ = *src++;
	}
	if (count) {
		*dest++ = 0;
    }
}

int
strlen(char *str) {
	int count = 0;
	while (str[count])
		count++;
	return count;
}

void
strcat(char *dest, char *src) {
    dest += strlen(dest);
	strcpy(dest, src);
}

int
strcmp(char *s1, char *s2) {
	while (1) {
		if (*s1 != *s2)
			return -1;              // strings not equal
		if (!*s1)
			return 0;               // strings are equal
		s1++;
		s2++;
	}
	
	return -1;
}