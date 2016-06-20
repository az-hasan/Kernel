/*
*	hello.c - The simplest kernel module
*/
#include <linux/fs.h> /* Needed for device_open */
#include <linux/init.h> /* Needed for macros */
#include <linux/kernel.h> /* Needed for KERN_INFO */
#include <linux/module.h> /* All kernel modules need this */

#include <linux/types.h> /* Needed for dev_t */
//#include <linux/kdev_t.h> /* Needed for MAJOR(dev) */
#include <linux/device.h> /* Needed for class_create */
#include <linux/export.h> /* Needed for THIS_MODULE */
#include <linux/cdev.h> /* Needed for cdev_add */

#define DEVICE_NAME "hello"
#define DEVICE_AUTHOR "Aisha Hasan"
#define NUM_DEVICES 1
#define MINOR_NUM 0

/* macros to describe driver's author & license */
MODULE_AUTHOR(DEVICE_AUTHOR);
MODULE_LICENSE("GPL");

static int hello_init(void);
static void hello_exit(void);
//static void device_open(void);

static dev_t majorNum; /* Device major number*/
static struct class *helloClass; /*Device Class*/
static struct cdev helloClassDeviceStruct;
static struct device *helloClassDevice; /* Device */

/*
* Holds pointers to functions to be defined by driver, to be used when calling
* "cat /dev/theprocs".
*/
struct file_operations fops = {
	.owner = THIS_MODULE
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

	int err;
	printk(KERN_INFO "Hello world\n");
	
	err = alloc_chrdev_region(&majorNum, MINOR_NUM, NUM_DEVICES,
		DEVICE_NAME);	

	if(err < 0){
		printk(KERN_INFO "Error registering device\n");
	}

	//get allocated major number from "majorNum" & try to create a class
	// with our device name
	printk(KERN_INFO "device \"%s\" got allocated major number %d\n", 
		DEVICE_NAME, MAJOR(majorNum));

	helloClass = class_create(THIS_MODULE, DEVICE_NAME);
	err = IS_ERR(helloClass);
	
	if(err){
		printk(KERN_INFO "Error creating class\n");
		unregister_chrdev_region(majorNum, NUM_DEVICES);
		return err;
	}

	//initialize and add device to system
	cdev_init(&helloClassDeviceStruct, &fops);
	err = cdev_add(&helloClassDeviceStruct, majorNum, NUM_DEVICES);
	
	if(err < 0){
		printk(KERN_INFO "Error adding device\n");
		unregister_chrdev_region(majorNum, NUM_DEVICES);
		return err;
	}

	//create device and register with sysfs
	helloClassDevice = device_create(helloClass, NULL, majorNum, NULL, DEVICE_NAME);
	err = IS_ERR(helloClassDevice);
	
	if(err){
		printk(KERN_INFO "Error creating device\n");
		cdev_del(&helloClassDeviceStruct);
		class_destroy(helloClass);
		unregister_chrdev_region(majorNum, NUM_DEVICES);
		return err;
	}	

	return 0;
}

/* Unregisters device and exits */
static void hello_exit(void){
	printk(KERN_INFO ".........destroying device, class, and unregistering\n");
	device_destroy(helloClass, majorNum);
	cdev_del(&helloClassDeviceStruct);
	class_destroy(helloClass);
	unregister_chrdev_region(majorNum, NUM_DEVICES);
	printk(KERN_INFO "Goodbye world\n");
}


module_init(hello_init);
module_exit(hello_exit);

