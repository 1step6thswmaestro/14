#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

typedef char s8_t;
typedef char* ps8_t;
typedef unsigned char u8_t;
typedef unsigned char* pu8_t;
typedef char s16_t;
typedef char * ps16_t;
typedef unsigned short u16_t;
typedef unsigned short* pu16_t;
typedef unsigned int u32_t;
typedef unsigned int* pu32_t;
typedef unsigned long long u64_t;
typedef unsigned long long* pu64_t;
#if __x86_64__
 typedef __uint128_t u128;
#else
typedef int64_t u128;
#endif
typedef unsigned char BOOL;

#ifndef __KERNEL__
#define LINUX_VERSION_CODE 0
#define KERNEL_VERSION(major, minor, micro) 0
#define printk printf
#endif

#ifdef __KERNEL__

#include <linux/fs.h>

char* strtok(char *, const char *);
void get_file_path_from_dentry(struct dentry *dentry, char* buf, int len);
void get_dir_path_from_dentry(struct dentry *dentry,char* buf,int len);

#else

#define	ERANGE 34

#define GETOPT_VAL_HELP 270

int fls64(u64_t);
u64_t parse_size(char *);

#endif




