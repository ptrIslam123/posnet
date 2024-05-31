#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x92997ed8, "_printk" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0xb41d8294, "cdev_init" },
	{ 0xe5bf1632, "cdev_add" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xc4057e8, "__class_create" },
	{ 0x4afe073f, "cdev_del" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x24328ec, "device_create" },
	{ 0x2e94ddc2, "init_net" },
	{ 0x54b06dbc, "nf_register_net_hook" },
	{ 0xb1695cee, "nf_unregister_net_hook" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x37a0cba, "kfree" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x6093f1a5, "module_layout" },
};

MODULE_INFO(depends, "");

