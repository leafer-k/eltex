#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <asm/uaccess.h>

#define DEV_NAME "myDev"
#define DEV_CLASS_NAME "myDevClass"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Module to communicate with user-space through chardev. Returns an inverted input.");
MODULE_AUTHOR("Aleksandr Korovin");

static char msg[100]={0};
static short readPos=0;
static int times = 0;
static int major;
static struct class*  myClass  = NULL;
static struct device* myDevice = NULL;

static int dev_open(struct inode *, struct file *);
static int dev_rls(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops = {
    .read = dev_read,
    .open = dev_open,
    .write = dev_write,
    .release = dev_rls,
};

int init_module(void) {
    major = register_chrdev(0, DEV_NAME, &fops);
    if (major < 0) {
        printk(KERN_ALERT "Device registration failed...\n");
        return major;
    }
    printk(KERN_INFO "Device registered with major number %d\n", major);

    myClass = class_create(DEV_CLASS_NAME);
    if (IS_ERR(myClass)) {
        unregister_chrdev(major, DEV_NAME);
        printk(KERN_ALERT "Failed to create device class\n");
        return PTR_ERR(myClass);
    }

    myDevice = device_create(myClass, NULL, MKDEV(major, 0), NULL, DEV_NAME);
    if (IS_ERR(myDevice)) {
        class_destroy(myClass);
        unregister_chrdev(major, DEV_NAME);
        printk(KERN_ALERT "Failed to create device\n");
        return PTR_ERR(myDevice);
    }

    printk(KERN_INFO "/dev/%s device created\n", DEV_NAME);
    return 0;
}

void cleanup_module(void) {
    device_destroy(myClass, MKDEV(major, 0));
    class_unregister(myClass);
    class_destroy(myClass);
    unregister_chrdev(major, DEV_NAME);
    printk(KERN_INFO "Device unregistered and removed\n");
}

static int dev_open(struct inode *inod, struct file *fil) {
    times++;
    printk(KERN_ALERT "Device opened %d times\n", times);
    return 0;
}

static ssize_t dev_read(struct file *filp, char *buff, size_t len, loff_t *off) {
    short count = 0;
    while (len && (msg[readPos] != 0)) {
        put_user(msg[readPos], buff++);
        count++;
        len--;
        readPos++;
    }
    return count;
}

static ssize_t dev_write(struct file *filp, const char *buff, size_t len, loff_t *off) {
    ssize_t i;
    if (len >= sizeof(msg)) len = sizeof(msg) - 1;

    memset(msg, 0, sizeof(msg));
    readPos = 0;

    char tmp[100] = {0};
    if (copy_from_user(tmp, buff, len)) {
        return -EFAULT;
    }

    for (i = 0; i < len; i++) {
        msg[i] = tmp[len - i - 1];
    }

    return len;
}

static int dev_rls(struct inode *inod, struct file *fil) {
    printk(KERN_ALERT "Device closed\n");
    return 0;
}
