#include <linux/module.h>	// Included for all kernel modules
#include <linux/kernel.h>	// Included for KERN_INFO
#include <linux/init.h>		// included for __init and __exit macros
#include <linux/fs.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alek Mabry");
MODULE_DESCRIPTION("A TFT Display Driver");
MODULE_VERSION("1");

#define DEVICE_NAME "tft_display"
#define EXAMPLE_MSG "TFT Driver Feedback"
#define MSG_BUFFER_LEN 32

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

static int major_num;
static int device_open_count = 0;
static char msg_buffer[MSG_BUFFER_LEN];
static char *msg_ptr;

/* This structure points to all the device functions */
static struct file_operations file_ops = 
{
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

/* When a process reads from our device, this gets called */
static ssize_t device_read(struct file *flip, char *buffer, size_t len, loff_t *offset)
{
	int bytes_read = 0;
	// If we're at the end, loop back to the beginning
	if (*msg_ptr == 0)
	{
		msg_ptr = msg_buffer;
	}
	// Put data in the buffer
	while (len && *msg_ptr)
	{
		// Buffer is in user data, not kernel, so it can't be referenced with a pointer.
		// The function put_user handles this.
		put_user(*(msg_ptr++), buffer++);
		len--;
		bytes_read++;
	}
	return bytes_read;
}

/* Called when a process tries to write to the device */
static ssize_t device_write(struct file *flip, const char *buffer, size_t len, loff_t *offset)
{
	// This is a read only device
	printk(KERN_ALERT "This operation is not supported.\n");
	return -EINVAL;
}

/* Called when a process opens the device */
static int device_open(struct inode *inode, struct file *file)
{
	// If the device is open, return busy
	if (device_open_count)
	{
		return -EBUSY;
	}
	device_open_count++;
	try_module_get(THIS_MODULE);
	return 0;
}

/* Called when a process closes the device */
static int device_release(struct inode *inode, struct file *file)
{
	// Decrement the open counter and usage count. Without this the module
	// would not unload
	device_open_count--;
	module_put(THIS_MODULE);
	return 0;
}

static int __init tft_display_init(void)
{
	// Fill buffer with message
	strncpy(msg_buffer, EXAMPLE_MSG, MSG_BUFFER_LEN);
	// Set the msg_ptr to the buffer
	msg_ptr = msg_buffer;
	// Try to register character device
	major_num = register_chrdev(0, "tft_display", &file_ops);
	if (major_num < 0)
	{
		printk(KERN_ALERT "Could not register device: %d\n", major_num);
		return major_num;
	}
	else
	{
		printk(KERN_INFO "TFT display module loaded with device major number %d\n", major_num);
		return 0;	// Non-zero means it can't be loaded
	}
}

static void __exit tft_display_cleanup(void)
{
	unregister_chrdev(major_num, DEVICE_NAME);
	printk(KERN_INFO "Removed TFT display module.\n");
}

module_init(tft_display_init);
module_exit(tft_display_cleanup);
