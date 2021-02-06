#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
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
__used
__attribute__((section("__versions"))) = {
	{ 0x8b44049c, "module_layout" },
	{ 0xe409a39a, "param_ops_int" },
	{ 0x9ac69f20, "pps_unregister_source" },
	{ 0xaaee28d0, "hrtimer_cancel" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0xdb7305a1, "__stack_chk_fail" },
	{ 0x2c805a3c, "hrtimer_init" },
	{ 0x8678a082, "pps_register_source" },
	{ 0xfe990052, "gpio_free" },
	{ 0xdf35b9d6, "gpiod_direction_input" },
	{ 0x47229b5c, "gpio_request" },
	{ 0x5f754e5a, "memset" },
	{ 0x2196324, "__aeabi_idiv" },
	{ 0x7c32d0f0, "printk" },
	{ 0xd697e69a, "trace_hardirqs_on" },
	{ 0xa4710d9c, "pps_event" },
	{ 0xb43f9365, "ktime_get" },
	{ 0x1e1e140e, "ns_to_timespec64" },
	{ 0xdafcdc3a, "ktime_get_snapshot" },
	{ 0xec3d2e1b, "trace_hardirqs_off" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0xd46ff06d, "hrtimer_start_range_ns" },
	{ 0xed0ccfb5, "gpiod_get_raw_value" },
	{ 0x4d790e8d, "gpio_to_desc" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=pps_core";


MODULE_INFO(srcversion, "1BEA4AA63383571784250FE");
