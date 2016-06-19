/*
*	hello.c - The simplest kernel module
*/
#include <linux/fs.h> /* Needed for device_open */
#include <linux/init.h> /* Needed for macros */
#include <linux/kernel.h> /* Needed for KERN_INFO */
#include <linux/module.h> /* All kernel modules need this */

#define DEVICE_NAME "hello"
#define DEVICE_AUTHOR "Aisha Hasan"

/* macro to describe who the driver's author is */
//MODULE_AUTHOR(DEVICE_AUTHOR);

static int hello_init(void);
static void hello_exit(void);
//static void device_open(void);

int major_num;

/*
* Holds pointers to functions to be defined by driver, to be used when calling
* "cat /dev/theprocs".
*/
struct file_operations fops = {
//	.open = device_open
};

//void device_open(void){
	//do stuff
//}

/*
* Registers device with the kernel, with an (unused) major number automatically
* assigned by the kernel.
*
* If there is an error while registering the device, the return value of the
* function is returned, otherwise 0 (success) is returned. 
*/
static int hello_init(void){
	printk(KERN_INFO "Hello world");

	major_num = register_chrdev(0, DEVICE_NAME, &fops);

	if(major_num < 0){
		printk(KERN_INFO "Could not register device\n");
		return major_num;
	}

	printk(KERN_INFO "Device \"%s\" was assigned major number %d\n",
		DEVICE_NAME, major_num);
	return 0;
}

/* Unregisters device and exits */
static void hello_exit(void){
	unregister_chrdev(major_num, DEVICE_NAME);
	printk(KERN_INFO "Goodbye world\n");
}


module_init(hello_init);
module_exit(hello_exit);

