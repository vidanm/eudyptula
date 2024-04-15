// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/jiffies.h>
#include <linux/debugfs.h>
#include <linux/semaphore.h>
#define MY_USER_ID "11121998"
#define USER_ID_LENGTH 9 //\0

MODULE_LICENSE("Dual BSD/GPL");
				    //Linux Device Drivers Chapter 05
static DEFINE_SEMAPHORE(foo_sem,1); //DECLARE_MUTEX HAS BEEN RENAMED TO DEFINE_SEMAPHORE 

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
	else
		return len;

}

static int fid_open(struct inode *inode, struct file *file){
	return 0;
}

static int fid_close(struct inode *inode, struct file *file){
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

/* start of foo file ops */

static char foo_data[PAGE_SIZE];
static int foo_len;

static ssize_t foo_read(struct file *filp, char __user *buf, size_t len, loff_t *f_pos) {
	int ret;

	if (*f_pos != 0)
		return 0;

	if (down_interruptible(&foo_sem))
		return -EINVAL;

	ret = copy_to_user(buf,foo_data,foo_len);
	up(&foo_sem);

	if (ret)
		return -EINVAL;

	*f_pos += len;
	return len;

}

static ssize_t foo_write(struct file *file, const char __user *buf, size_t len, loff_t  *f_pos) {
	int ret;

	if (down_interruptible(&foo_sem))
		goto fail;

	ret = copy_from_user(foo_data,buf,len);
	up(&foo_sem);

	if (ret)
		goto fail;

	foo_len = len;
	return len;

	fail: foo_len = 0;
	      return -EINVAL;
}

static int foo_open(struct inode *inode, struct file *file){
	return 0;
}

static int foo_close(struct inode *inode, struct file *file){
	return 0;
}

static const struct file_operations foo_fops = {
	.owner = THIS_MODULE,
	.write = foo_write,
	.read = foo_read,
	.open = foo_open,
	.release = foo_close,
	.llseek = no_llseek,
};

/* end of foo file ops */

static struct dentry *eud_dentry;
static int hello_init(void)
{
	eud_dentry = debugfs_create_dir("eudyptula",NULL);
	if (eud_dentry == ERR_PTR(-ENODEV)){
		pr_alert("No debugfs support\n");
		return -ENODEV;
	}

	struct dentry* eud_id = debugfs_create_file("id",0666,eud_dentry,NULL,&fid_fops);
	debugfs_create_ulong("jiffies",0444,eud_dentry,(unsigned long *)&jiffies); 
	struct dentry* eud_foo = debugfs_create_file("foo",0744,eud_dentry,NULL,&foo_fops);

	if (!eud_id || !eud_foo) 
	{
		debugfs_remove_recursive(eud_dentry);
		pr_alert("Files could not be created\n");
		return -ENODEV;
	}

	pr_alert("Eudyptula module, task 8 : added\n");
	return 0;
}

static void hello_exit(void)
{
	pr_alert("Eudyptula module, task 8 : removed\n");
	debugfs_remove_recursive(eud_dentry);
}

module_init(hello_init);
module_exit(hello_exit);
