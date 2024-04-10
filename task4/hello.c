// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
MODULE_LICENSE("Dual BSD/GPL");

static char *whom = "world";
static int howmany = 1;
module_param(howmany, int, 0444);
module_param(whom, charp, 0444);

static int hello_init(void)
{
	int i = 0;

	for (i = 0; i < howmany; i++)
		pr_alert("Hello %s\n", whom);
	return 0;
}

static void hello_exit(void)
{
	pr_alert("Goodbye, cruel world\n");
}

module_init(hello_init);
module_exit(hello_exit);
