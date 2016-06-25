/*
*	hello.c - The simplest kernel module
*/

#include "hello.h"


/* 
* Called when the device file is opened.
* 
* Write list of processes to message buffer. Resize buffer by twice the current
* size if number of lines will exceed current buffer size. ("14" in assignment
* of lineLen are extra characters to formatting printing processes).
*
* Set filep's private_data to point to messageBuf.
* Point messagep to private_data (used in device_read to read message buffer to user)
*/
static int device_open(struct inode *fileSysObj, struct file *filep){
	char *tempBuf;
	int lineLen;
	int bufLines;	
	int numLines;
	size_t numBytes;
	struct task_struct *task;
	
	lineLen = (sizeof task->comm) + (sizeof task->pid) + 14;
	numLines = 40;
	bufLines = 0;
	numBytes = 0;

	printk(KERN_INFO "device name \"%s\" request to open\n", DEVICE_NAME);

	/* ================== write list of processes to message buffer ======= */
	messageBuf = kmalloc(numLines*lineLen, GFP_KERNEL);

	for_each_process(task){
		//printk(KERN_INFO "all Procs: %s, %d\n", task->comm, task->pid);
		//printk(KERN_INFO "msgBuf length: %zu", strlen(messageBuf));
		numBytes += snprintf(messageBuf + strlen(messageBuf), lineLen,
				"all Procs: %s, %d\n", task->comm, task->pid); 
		bufLines++;
	
		if(bufLines == numLines){
			printk(KERN_INFO "bufLines %d, numLines %d: Reallocating buffer....\n",
				bufLines, numLines);
			numLines *= 2;
			tempBuf = krealloc(messageBuf, numLines * lineLen, GFP_KERNEL);

			if(tempBuf == NULL){
				printk(KERN_INFO "Error reallocating buffer\n");
				kfree(messageBuf);
				return -1;
			}
			
			messageBuf = tempBuf;
		}
	}

	printk(KERN_INFO "bufLines: %d\n", bufLines);
	printk(KERN_INFO "device \"%s\" message = %zu bytes\n", DEVICE_NAME, numBytes);
	
	filep->private_data = messageBuf;
	messagep = filep->private_data;

	return 0;
}

/* Frees message buffer if not already free */
static int device_release(struct inode *inode, struct file *filep){
	if(messageBuf != NULL){
		printk("Freeing memory\n");
		kfree(messageBuf);
	}
	return 0;
}

/*
* Called when a process, which already opened the dev file, attempts to
* read from it.
* Puts characters (bytes) from messageBuf into the users buffer, using messagep
* to iterate through messageBuf. If the user has already reached the end of the
* messageBuf, return 0 to signal we're done reading;
*/
static ssize_t device_read(struct file *filep,	/* see include/linux/fs.h   */
			   char *buffer,	/* buffer to fill with data */
			   size_t length,	/* length of the buffer     */
			   loff_t *offset){

	int bytes_read = 0;
	printk(KERN_INFO "device name \"%s\" request to read\n", DEVICE_NAME);

	if(*messagep == 0) return 0; //if at the end of msg, return 0
	
	//put data in the buffer
	while(length && *messagep){
		//printk(KERN_INFO "to user: %s, bytes read: %d\n", messagep, bytes_read);
		put_user(*(messagep++), buffer++);
		length --;
		bytes_read++;
	}

	printk(KERN_INFO "/dev/%s read succesfully\n", DEVICE_NAME);
	return bytes_read;
}

/*
* Registers device with the kernel, with an (unused) major number automatically
* assigned by the kernel.
*
* If there is an error while registering the device, the returned error value 
* is returned, otherwise 0 (success) is returned. 
*/
static int hello_init(void){

	int err;
	printk(KERN_INFO "Hello world\n");
	
	err = alloc_chrdev_region(&majorNum, MINOR_NUM, NUM_DEVICES,
		DEVICE_NAME);	

	if(err < 0){
		printk(KERN_INFO "Error registering device\n");
		return err;
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
	helloClassDeviceStruct.owner = THIS_MODULE;
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

