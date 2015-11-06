#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x1fc32c62, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x4c4fef19, __VMLINUX_SYMBOL_STR(kernel_stack) },
	{ 0xda3e43d1, __VMLINUX_SYMBOL_STR(_raw_spin_unlock) },
	{ 0xb33e4c62, __VMLINUX_SYMBOL_STR(simple_link) },
	{ 0xd6ee688f, __VMLINUX_SYMBOL_STR(vmalloc) },
	{ 0x1d7dbee0, __VMLINUX_SYMBOL_STR(kill_anon_super) },
	{ 0x34184afe, __VMLINUX_SYMBOL_STR(current_kernel_time) },
	{ 0xe232cc89, __VMLINUX_SYMBOL_STR(generic_delete_inode) },
	{ 0x52cbb014, __VMLINUX_SYMBOL_STR(lockref_get) },
	{ 0x6c369136, __VMLINUX_SYMBOL_STR(inc_nlink) },
	{ 0x999e8297, __VMLINUX_SYMBOL_STR(vfree) },
	{ 0x51d11ca3, __VMLINUX_SYMBOL_STR(mount_nodev) },
	{ 0x91715312, __VMLINUX_SYMBOL_STR(sprintf) },
	{ 0xe2d5255a, __VMLINUX_SYMBOL_STR(strcmp) },
	{ 0x4f8b5ddb, __VMLINUX_SYMBOL_STR(_copy_to_user) },
	{ 0x327ee395, __VMLINUX_SYMBOL_STR(vfs_read) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xe14471e8, __VMLINUX_SYMBOL_STR(d_rehash) },
	{ 0x9166fada, __VMLINUX_SYMBOL_STR(strncpy) },
	{ 0x61651be, __VMLINUX_SYMBOL_STR(strcat) },
	{ 0x1d15c5c0, __VMLINUX_SYMBOL_STR(simple_getattr) },
	{ 0x9f984513, __VMLINUX_SYMBOL_STR(strrchr) },
	{ 0xf85d987b, __VMLINUX_SYMBOL_STR(simple_unlink) },
	{ 0xf0fdf6cb, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
	{ 0xd52bf1ce, __VMLINUX_SYMBOL_STR(_raw_spin_lock) },
	{ 0x96c5c192, __VMLINUX_SYMBOL_STR(register_filesystem) },
	{ 0x34cca3e9, __VMLINUX_SYMBOL_STR(iput) },
	{ 0x13ec5d93, __VMLINUX_SYMBOL_STR(d_make_root) },
	{ 0x7b030abc, __VMLINUX_SYMBOL_STR(simple_statfs) },
	{ 0x95d156e0, __VMLINUX_SYMBOL_STR(unregister_filesystem) },
	{ 0x5a70a3d9, __VMLINUX_SYMBOL_STR(init_special_inode) },
	{ 0xa093bcd3, __VMLINUX_SYMBOL_STR(new_inode) },
	{ 0x98fae3d3, __VMLINUX_SYMBOL_STR(simple_rename) },
	{ 0x4f6b400b, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0x10ac0100, __VMLINUX_SYMBOL_STR(d_instantiate) },
	{ 0x5a41bb89, __VMLINUX_SYMBOL_STR(simple_rmdir) },
	{ 0xba948b50, __VMLINUX_SYMBOL_STR(vfs_write) },
	{ 0xe914e41e, __VMLINUX_SYMBOL_STR(strcpy) },
	{ 0x2f627c13, __VMLINUX_SYMBOL_STR(filp_open) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "E34CC56373A5F2671F73232");
