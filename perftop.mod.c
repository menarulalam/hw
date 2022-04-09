#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
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
__used __section(__versions) = {
	{ 0x581b1365, "module_layout" },
	{ 0xb0448582, "proc_remove" },
	{ 0x7413dd1c, "proc_create" },
	{ 0x6b2941b2, "__arch_copy_to_user" },
	{ 0xc5850110, "printk" },
	{ 0xf3ecac14, "cpu_hwcap_keys" },
	{ 0xdaca81bf, "cpu_hwcaps" },
	{ 0x14b89635, "arm64_const_caps_ready" },
	{ 0x1fdc7df2, "_mcount" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "9D7DEA601C986C514B0B791");
