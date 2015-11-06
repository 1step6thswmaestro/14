#include "util.h"

#ifdef __KERNEL__
#include <linux/string.h>

char * strtok_r(char *s, const char *delim, char **last);
	char *
strtok(char *s, const char *delim)
{
	static char *last;

	return strtok_r(s, delim, &last);
}

	char *
strtok_r(char *s, const char *delim, char **last)
{
	char *spanp;
	int c, sc;
	char *tok;


	if (s == NULL && (s = *last) == NULL)
		return (NULL);

	/*
	 * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
	 */
cont:
	c = *s++;
	for (spanp = (char *)delim; (sc = *spanp++) != 0;) {
		if (c == sc)
			goto cont;
	}

	if (c == 0) {           /* no non-delimiter characters */
		*last = NULL;
		return (NULL);
	}
	tok = s - 1;

	/*
	 * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
	 * Note that delim must have one NUL; we stop if we see that, too.
	 */
	for (;;) {
		c = *s++;
		spanp = (char *)delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*last = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}


/**
	@brief	mfs의 dentry를 기반으로 경로 추출

	mfs가 /mnt/mfs에 마운트 되어있을 때 /mnt/mfs/dir1/a.out의 dentry를 인자로 줬을 경우
	"/dir1/a.out"을 buf에 써준다.
	@param
			struct dentry	*dentry	경로를 알고자 하는 파일/디렉토리의 VFS dentry
			char			*buf	경로를 저장 할 버퍼
			int				len		버퍼buf에 저장 할 수 있는 최대 길이
	@return	none
*/
void get_file_path_from_dentry(struct dentry *dentry, char* buf, int len)
{
	static char tmp[1024] = {0};
	struct super_block* sb = dentry->d_sb;

	struct dentry* walker = dentry;
	buf[0] = '\0';

	while(walker != sb->s_root){
		sprintf(tmp, "/%s%s", walker->d_name.name, buf);
		strncpy(buf, tmp, len);
		printk("%s/", walker->d_name.name);
		walker = walker->d_parent;
	}

	printk(" : %s; fullpath : %s\n", walker->d_name.name, buf);
	return ;
}

/**
	@brief	mfs의 dentry를 기반으로 부모 디렉토리까지의 경로 추출

	mfs가 /mnt/mfs에 마운트 되어있을 때 /mnt/mfs/dir1/a.out의 dentry를 인자로 줬을 경우
	"/dir1/"을 buf에 써준다.
	@param
			struct dentry	*dentry	경로를 알고자 하는 파일/디렉토리의 VFS dentry
			char			*buf	경로를 저장 할 버퍼
			int				len		버퍼buf에 저장 할 수 있는 최대 길이
	@return	none
*/
void get_dir_path_from_dentry(struct dentry *dentry, char* buf, int len)
{
	static char tmp[1024] = {0};
	struct super_block* sb = dentry->d_sb;

	struct dentry* walker = dentry->d_parent;
	buf[0] = '\0';

	while(walker != sb->s_root){
		sprintf(tmp,"/%s%s", walker->d_name.name, buf);
		strncpy(buf, tmp, len);
		printk("%s/", walker->d_name.name);
		walker = walker->d_parent;
	}

	if(strlen(buf) == 0){
		strcpy(buf, "/");
	}

	printk(" : %s; fullpath : %s\n", walker->d_name.name, buf);
	return ;
}

#else

#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

int fls64(u64_t x)
{
	int i;

	for (i = 0; i <64; i++)
		if (x << i & (1ULL << 63))
			return 64 - i;
	return 64 - i;
}

u64_t parse_size(char *s)
{
	char c;
	char *endptr;
	u64_t mult = 1;
	u64_t ret;

	if (!s) {
		fprintf(stderr, "ERROR: Size value is empty\n");
		exit(1);
	}
	if (s[0] == '-') {
		fprintf(stderr,
			"ERROR: Size value '%s' is less equal than 0\n", s);
		exit(1);
	}
	ret = strtoull(s, &endptr, 10);
	if (endptr == s) {
		fprintf(stderr, "ERROR: Size value '%s' is invalid\n", s);
		exit(1);
	}
	if (endptr[0] && endptr[1]) {
		fprintf(stderr, "ERROR: Illegal suffix contains character '%c' in wrong position\n",
			endptr[1]);
		exit(1);
	}

	if (errno == ERANGE && ret == ULLONG_MAX) {
		fprintf(stderr,
			"ERROR: Size value '%s' is too large for u64\n", s);
		exit(1);
	}
	if (endptr[0]) {
		c = tolower(endptr[0]);
		switch (c) {
		case 'e':
			mult *= 1024;
		case 'p':
			mult *= 1024;
		case 't':
			mult *= 1024;
		case 'g':
			mult *= 1024;
		case 'm':
			mult *= 1024;
		case 'k':
			mult *= 1024;
		case 'b':
			break;
		default:
			fprintf(stderr, "ERROR: Unknown size descriptor '%c'\n",
				c);
			exit(1);
		}
	}

	if (fls64(ret) + fls64(mult) - 1 > 64) {
		fprintf(stderr,
			"ERROR: Size value '%s' is too large for u64\n", s);
		exit(1);
	}
	ret *= mult;
	return ret;
}

#endif

