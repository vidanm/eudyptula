/*                                                     
 * $Id: hello.c,v 1.5 2004/10/26 03:32:21 corbet Exp $ 
 */                                                    
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
MODULE_LICENSE("Dual BSD/GPL");

static char* whom = "world";
static int howmany = 1;
module_param(howmany,int,S_IRUGO);
module_param(whom,charp,S_IRUGO);

static int hello_init(void)
{	
	int i = 0;
	for (i = 0;i<howmany;i++)
		printk(KERN_ALERT "Hello %s",whom);
	return 0;
}

static void hello_exit(void)
{
	printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(hello_init);
module_exit(hello_exit);
