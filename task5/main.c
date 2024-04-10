/*                                                     
 * $Id: hello.c,v 1.5 2004/10/26 03:32:21 corbet Exp $ 
 */                                                    
#include <linux/init.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/hid.h>

MODULE_LICENSE("Dual BSD/GPL");

static int eud_probe(struct usb_interface *interface,const struct usb_device_id *id){
	printk("Keyboard connected\n");
	return 0;
}

static void eud_disconnect(struct usb_interface *interface){
	printk("Keyboard disconnected\n");
}

static struct usb_device_id eud_table [] = {
        { USB_INTERFACE_INFO
	 (USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
	  USB_INTERFACE_PROTOCOL_KEYBOARD) },
        { }                      /* Terminating entry Parce qu'une entrée se termine à Null*/
};

/* Permets de définir à quel groupe de device USB ce driver s'addresse */
MODULE_DEVICE_TABLE (usb, eud_table);

static struct usb_driver keyboard_driver = {
	.name = "Eudyptula_task5",
	.probe = eud_probe,
	.disconnect = eud_disconnect,
	.id_table = eud_table
};

static int hello_init(void)
{	
	int result;
	result = usb_register(&keyboard_driver);
	if (result < 0) {
		pr_err("usb_register failed for the %s driver. Error number %d\n",
                       keyboard_driver.name, result);
		return -1;
	}
	return 0;
}

static void hello_exit(void)
{
	usb_deregister(&keyboard_driver);
}

module_init(hello_init);
module_exit(hello_exit);
