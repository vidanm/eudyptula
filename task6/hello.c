// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/miscdevice.h>
#define MYUSERID 11121998
MODULE_LICENSE("Dual BSD/GPL");

static int eudyptula_misc_open(struct inode *inode, struct file *file)
{
	pr_info("Eudyptula misc device open\n");
	return 0;
}

static int eudyptula_misc_close(struct inode *inodep, struct file *filp)
{
	pr_info("Eudyptula misc device close\n");
	return 0;
}

static ssize_t eudyptula_misc_write(struct file *file, const char __user *buf,
	size_t len, loff_t *ppos)
{
	pr_info("Eudyptula misc device write\n");
	int ret;
	unsigned long long res;

	ret = kstrtoull_from_user(buf, len, 10, &res);
	if (ret) {
		pr_info("ko = %d\n", ret);
		return ret;
	}

	if (res == MYUSERID) {
		pr_info("data written to eudyptula is user id\n");
		return 8;
	}

	pr_info("data written to eudyptula is not user id\n");
	return -EINVAL;

}

static ssize_t eudyptula_misc_read(struct file *filp, char __user *buf,
	size_t count, loff_t *f_pos)
{
	pr_info("EtX misc device read\n");

	return MYUSERID;
}

static const struct file_operations fops = {
	.owner          = THIS_MODULE,
	.write          = eudyptula_misc_write,
	.read           = eudyptula_misc_read,
	.open           = eudyptula_misc_open,
	.release        = eudyptula_misc_close,
	.llseek         = no_llseek,
};

struct miscdevice eudyptula_miscdevice = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "eudyptula",
	.fops = &fops
};

static int hello_init(void)
{
	int error;

	error = misc_register(&eudyptula_miscdevice);
	if (error) {
		pr_err("Registering my misc device failed\n");
		return error;
	}
	pr_info("Registering my misc device succeed !\n");

	return 0;
}

static void hello_exit(void)
{
	misc_deregister(&eudyptula_miscdevice);
	pr_alert("Unregistering my misc device !\n");
}

module_init(hello_init);
module_exit(hello_exit);
