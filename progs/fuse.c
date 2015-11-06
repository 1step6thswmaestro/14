#define FUSE_USE_VERSION 30

#include <fuse/fuse_lowlevel.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

#include "../volume.h"
#include "../dir.h"

#define DEBUG

struct mfs_volume* volume;

static int mfs_getattr(const char *path, struct stat *stbuf)
{

	int res;
	res = lstat(path, stbuf);
	if (res == -1)
			return -errno;
	return 0;

//	int ret;
//	s16_t route[128] = {0, };
//	s16_t file_name[64] = {0, };
//	int cluster_number;
//	struct mfs_dirent dentry_mfs;
//
//	printf("getattr : %s\n", path);
//
//	get_file_name(path, file_name);
//	get_dir_path(path, route);
//
//	cluster_number = get_cluster_number(volume, route);
//	get_dentry(volume, cluster_number, file_name, &dentry_mfs);
//
//	ret = lstat(path, stbuf);
//
//	stbuf->st_size = dentry_mfs.size;
//	printk("mode - %d\n", stbuf->st_mode);
//	stbuf->st_mode|=0777;
//
//	return ret;
}

static int mfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi)
{

	printf("readdir path : %s\n", path	);

	DIR *dp;
	struct dirent *de;
	(void) offset;
	(void) fi;
	dp = opendir(path);
	if (dp == NULL)
			return -errno;
	while ((de = readdir(dp)) != NULL) {
			struct stat st;
			memset(&st, 0, sizeof(st));
			st.st_ino = de->d_ino;
			st.st_mode = de->d_type << 12;
			if (filler(buf, de->d_name, &st, 0))
					break;
	}
	closedir(dp);
	return 0;
}

static int mfs_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;
	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	if (S_ISREG(mode)) {
			res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
			if (res >= 0)
					res = close(res);
	} else if (S_ISFIFO(mode))
			res = mkfifo(path, mode);
	else
			res = mknod(path, mode, rdev);
	if (res == -1)
			return -errno;
	return 0;
}

static int mfs_mkdir(const char *path, mode_t mode)
{
	int res;
	res = mkdir(path, mode);
	if (res == -1)
			return -errno;
	return 0;
}

static int mfs_unlink(const char *path)
{
	int res;
	res = unlink(path);
	if (res == -1)
			return -errno;
	return 0;
}

static int mfs_link(const char *from, const char *to)
{
	int res;
	res = link(from, to);
	if (res == -1)
			return -errno;
	return 0;
}

static int mfs_chmod(const char *path, mode_t mode)
{
	int res;
	res = chmod(path, mode);
	if (res == -1)
			return -errno;
	return 0;
}

static int mfs_chown(const char *path, uid_t uid, gid_t gid)
{
	int res;
	res = lchown(path, uid, gid);
	if (res == -1)
			return -errno;
	return 0;
}

static int mfs_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi)
{

	printf("read path : %s\n", path	);

	int fd;
	int res;
	(void) fi;
	fd = open(path, O_RDONLY);
	if (fd == -1)
			return -errno;
	res = pread(fd, buf, size, offset);
	if (res == -1)
			res = -errno;
	close(fd);
	return res;
}

static int mfs_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;
	(void) fi;
	fd = open(path, O_WRONLY);
	if (fd == -1)
			return -errno;
	res = pwrite(fd, buf, size, offset);
	if (res == -1)
			res = -errno;
	close(fd);
	return res;
}

static int mfs_statfs(const char *path, struct statvfs *stbuf)
{

	printf("stat path : %s\n", path	);

	int res;
	res = statvfs(path, stbuf);
	if (res == -1)
			return -errno;
	return 0;
}

static int mfs_open(const char *path, struct fuse_file_info *fi)
{

	printf("open path : %s\n", path	);

    (void) fi;


    if(strcmp(path, "/") != 0)
        return -ENOENT;

    return 0;
}

static struct fuse_operations mfs_fuse_operations = {
	.open			= mfs_open,
	.getattr        = mfs_getattr,
	.readdir        = mfs_readdir,
	.mknod          = mfs_mknod,
	.mkdir          = mfs_mkdir,
	.link           = mfs_link,
	.read           = mfs_read,
	.write          = mfs_write,
	.statfs         = mfs_statfs,
};

static void print_usage(int ret)
{
	fprintf(stderr, "Usage: mfs-fuse <mount-file> <mount-point>\n");
	exit(ret);
}

int main(int argc, char *argv[])
{

	char *argv_fuse[argc - 1];
	int argc_fuse = 0;

	// Maybe the parameter is..
	// [mount_file] [mount_point]

	char* mount_file = argv[1];		// argv[0] is file name..
	char* mount_point = argv[2];

	if (argc != 3) {
		print_usage(1);
	}

	volume = open_volume(mount_file, "r+b");

	if (volume == NULL) {
		printf("ERROR: %s doesn't exist\n", mount_file);
		exit(1);
	}

	argv_fuse[argc_fuse++] = argv[0];
	argv_fuse[argc_fuse++] = mount_point;
#ifdef DEBUG
	argv_fuse[argc_fuse++] = "-d";
#endif

	struct mfs_sb_info sb;
	read_sb(volume, &sb);
	printf("%s volume is mounted!\n", sb.volume_label);

	return fuse_main(argc_fuse, argv_fuse, &mfs_fuse_operations, NULL);
}
