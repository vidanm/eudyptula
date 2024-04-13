// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/miscdevice.h>
#include <linux/jiffies.h>
#include <linux/debugfs.h>
#define MY_USER_ID "11121998"
#define USER_ID_LENGTH 9 //\0

MODULE_LICENSE("Dual BSD/GPL");

/* fid file fops */
static ssize_t fid_read(struct file *filp, char __user *buf, size_t len, loff_t *f_pos) {
	char *user_id = MY_USER_ID;

	if (*f_pos != 0)
		return 0;

	if ((len < USER_ID_LENGTH) || (copy_to_user(buf,user_id,USER_ID_LENGTH)))
			return -EINVAL;

	*f_pos += len;
	return len;
}

static ssize_t fid_write(struct file *file, const char __user *buf, size_t len, loff_t  *f_pos) {
	int ret;
	char *user_id = MY_USER_ID;
	char input[USER_ID_LENGTH];

	ret = copy_from_user(input,buf,USER_ID_LENGTH);
	
	if (len != USER_ID_LENGTH || ret || strncmp(user_id,input,USER_ID_LENGTH-1))
		return -EINVAL;
	else {
		pr_info("Data written to eudyptula/id is user id\n");
		return len;
	}
}

static int fid_open(struct inode *inode, struct file *file){
	pr_info("eudyptula/id open\n");
	return 0;
}

static int fid_close(struct inode *inode, struct file *file){
	pr_info("eudyptula/id close\n");
	return 0;
}

static const struct file_operations fid_fops = {
	.owner = THIS_MODULE,
	.write = fid_write,
	.read = fid_read,
	.open = fid_open,
	.release = fid_close,
	.llseek = no_llseek,
};

/* end of fid file ops */

static struct dentry *eud_dentry;
static int hello_init(void)
{
	eud_dentry = debugfs_create_dir("eudyptula",NULL);
	if (eud_dentry == ERR_PTR(-ENODEV)){
		pr_alert("No debugfs support\n");
		return -ENODEV;
	}

	
	struct dentry* eud_id = debugfs_create_file("id",0666,eud_dentry,NULL,&fid_fops);
	debugfs_create_u32("jiffies",0444,eud_dentry,(u32*)&jiffies);

	if (!eud_id)
	{
		pr_alert("Files could not be created\n");
		return -ENODEV;
	}

	pr_alert("eudyptula task 8 module : added\n");
	return 0;
}

static void hello_exit(void)
{
	pr_alert("eudyptula task 8 module : removed\n");
	debugfs_remove_recursive(eud_dentry);
}

module_init(hello_init);
module_exit(hello_exit);
