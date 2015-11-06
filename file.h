extern struct file_operations mfs_file_operations;
extern struct file_operations mfs_dir_operations;

struct mfs_LFN_entry {
	u8_t id;
	s16_t name[30];
	u8_t reserved[1];
};

int read_file(struct mfs_volume*, struct mfs_dirent*, char*,
	   	unsigned int, unsigned int);

int write_file(struct mfs_volume*, struct mfs_dirent*, char*,
		unsigned int, unsigned int);

int __mfs_create(struct mfs_volume*, ps16_t, ps16_t);

BOOL is_normal_file(u8_t);

BOOL is_long_file_name(u8_t);

BOOL is_deleted_file(u8_t);

void set_normal_file_attribute(struct mfs_dirent*);
