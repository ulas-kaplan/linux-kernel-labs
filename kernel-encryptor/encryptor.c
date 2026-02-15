#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/string.h>

#define DEVICE_NAME "Encryptor"
#define BUFFER_SIZE 1024

static int major_number;
static char kernel_buffer[BUFFER_SIZE];
static int stored_data_len = 0; 
static char key = 0x64; 

MODULE_LICENSE("GPL");

static int dev_open(struct inode *inodep, struct file *filep) {
    return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    int errors = 0;

    // DEBUG: Okuma isteği geldiğinde durumu bildir
    printk(KERN_INFO "Encryptor read request offset: %lld, Len: %zu, Stored: %d\n", *offset, len, stored_data_len);

    if (*offset >= stored_data_len) {
        return 0;
    }

    if (len > (stored_data_len - *offset)) {
        len = stored_data_len - *offset;
    }

    errors = copy_to_user(buffer, kernel_buffer + *offset, len);

    if (errors == 0) {
        *offset += len;
        printk(KERN_INFO "Encryptor successfully sent %zu bytes\n", len);
        return len;
    } else {
        printk(KERN_ERR "Encryptorf ailed to send data to user\n");
        return -EFAULT;
    }
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    int i;
    
    stored_data_len = (len > BUFFER_SIZE) ? BUFFER_SIZE : len;

    memset(kernel_buffer, 0, BUFFER_SIZE);
    
    if (copy_from_user(kernel_buffer, buffer, stored_data_len)) {
        printk(KERN_ERR "Encryptor: Failed to receive data from user\n");
        return -EFAULT;
    }

    for (i = 0; i < stored_data_len; i++) {
        kernel_buffer[i] = kernel_buffer[i] ^ key;
    }

    printk(KERN_INFO "Encryptor received and encrypted %d bytes\n", stored_data_len);
    return len; 
}

static int dev_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "Encryptor device closed\n");
    return 0;
}

static struct file_operations fops = {
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};


static int __init encryptor_init(void) {
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) return major_number;
    printk(KERN_INFO "Encryptor loaded with major number %d\n", major_number);
    return 0;
}

static void __exit encryptor_exit(void) {
    unregister_chrdev(major_number, DEVICE_NAME);
}

module_init(encryptor_init);
module_exit(encryptor_exit);