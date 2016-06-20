/*======================= files to include ==================================*/
#include <linux/fs.h> /* Needed for device_open */
#include <linux/init.h> /* Needed for macros */
#include <linux/kernel.h> /* Needed for KERN_INFO */
#include <linux/module.h> /* All kernel modules need this */
#include <linux/types.h> /* Needed for dev_t */
#include <linux/device.h> /* Needed for class_create */
#include <linux/export.h> /* Needed for THIS_MODULE */
#include <linux/cdev.h> /* Needed for cdev_add */
#include <asm/uaccess.h> /* Needed for copy_to_user and put_user*/

/*============================ definitions ==================================*/
#define DEVICE_NAME "hello"
#define DEVICE_AUTHOR "Aisha Hasan"
#define NUM_DEVICES 1
#define MINOR_NUM 0
#define BUFFER_SIZE 4000

/* macros to describe driver's author & license */
MODULE_AUTHOR(DEVICE_AUTHOR);
MODULE_LICENSE("GPL"); /* Needed to prevent tainted kernel messages*/
//tainted kernel happens when an unlicensed module is loaded into the kernel

/*====================== function definitions ===============================*/
static int hello_init(void);
static void hello_exit(void);
static int device_open(struct inode *inode, struct file *filep);
static int device_release(struct inode *inode, struct file *filep);
static ssize_t device_read(struct file *filep, char *buffer, size_t length,
         loff_t * offset);

/*======================== structs and global variables =====================*/
static dev_t majorNum; /* Device major number*/
static struct class *helloClass; /*Device Class*/
static struct cdev helloClassDeviceStruct;
static struct device *helloClassDevice; /* Device */
char *message = "cheese\n\0";
char *messagep;
char messageBuf[BUFFER_SIZE];
unsigned long messageLen;

/*
* Holds pointers to functions to be defined by driver, to be used when calling
* "cat /dev/theprocs".
*/
static struct file_operations fops = {
        .owner = THIS_MODULE,
        .open = device_open,
        .release = device_release,
        .read = device_read
};


