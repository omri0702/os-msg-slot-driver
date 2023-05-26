#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include <linux/slab.h>
#include <linux/kernel.h>   
#include <linux/module.h>   
#include <linux/fs.h>       
#include <linux/uaccess.h>  
#include <linux/string.h> 

#include "message_slot.h"

MODULE_LICENSE("GPL");


static file_node *my_driver = NULL;


static int device_open(struct inode* inode,struct file* file){
    file_node *scan_list, *tmp, *new;
    int minor_num;
    scan_list = my_driver;
    tmp = NULL;
    minor_num = iminor(inode);
    if(my_driver == NULL){
        new = kmalloc(sizeof(file_node), GFP_KERNEL);
        if(new == NULL){return -1;}
        new->minor = minor_num;
        new->head = NULL;
        new->next = NULL;
        my_driver = new;
        return 0;
    }
    while(scan_list != NULL){
        if(scan_list->minor == minor_num){
            return 0;
        }
        tmp = scan_list;
        scan_list = scan_list->next;
    }
    new = kmalloc(sizeof(file_node), GFP_KERNEL);
    if(new == NULL){return -1;}
    new->minor = minor_num;
    new->head = NULL;
    new->next = NULL;
    tmp->next = new;
    return 0;
}

static long device_ioctl(struct file* file, unsigned int ioctl_command_id, long unsigned int channel_num){
    file_node *scan_list;
    channel_node *tmp, *chanel, *new;
    int minor_num;
    scan_list = my_driver;
    tmp = NULL;
    minor_num = iminor(file->f_inode);
    if(ioctl_command_id != MSG_SLOT_CHANNEL || channel_num == 0){ return -EINVAL;}
    while(scan_list != NULL && scan_list->minor != minor_num){
        scan_list = scan_list->next;
    }
    if (scan_list == NULL){
        return -1;
    }
    chanel = scan_list->head;
    while(chanel != NULL && chanel->chanel_number != channel_num){
        tmp = chanel;
        chanel = chanel->next;
    }
    if(chanel != NULL){
        file->private_data = (void*)chanel;
        return 0;
    }
    new = kmalloc(sizeof(channel_node), GFP_KERNEL);
    if(new == NULL){return -1;}
    new->chanel_number = channel_num;
    new->next = NULL;
    new->len = 0;
    if(scan_list->head == NULL || tmp == NULL){
        scan_list->head = new;
    }
    else {    
        tmp->next = new;
    }
    
    file->private_data = (void*)new; 
    return 0;
}


static ssize_t device_write(struct file* file, const char __user* buffer, size_t length, loff_t* offset){
    ssize_t i;
    int j;
    char msg[BUF_LEN];
    channel_node *update;
    if(length > 128 || length <= 0){ return -EMSGSIZE;}
    if(file->private_data == NULL){return -EINVAL;}
    for(i = 0; i < length && i < BUF_LEN; ++i ) {
        if(get_user(msg[i], &buffer[i])<0){
            return -1;
        }
    }
    update = file->private_data;
    update->len = i;
    for(j=0; j<update->len; j++){
        update->data[j] = msg[j];
    }
    return i;
}

static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loff_t* offset){
    ssize_t i;
    int j;
    channel_node *read_from;
    char tmp_buffer[BUF_LEN];
    if(file->private_data == NULL){return -EINVAL;}
    read_from = file->private_data;
    if(read_from->data == NULL || read_from->len == 0){return -EWOULDBLOCK;}
    if(length < read_from->len){return -ENOSPC;}
    for(i=0; i<read_from->len; i++){
        if(get_user(tmp_buffer[i], &buffer[i])<0){
            for(j=0;j<i;j++){
                put_user(tmp_buffer[j], &buffer[j]);
            }
            return -1;
        }
        put_user(read_from->data[i], &buffer[i]);
    }
    return i;
}



//==================== DEVICE SETUP =============================


struct file_operations Fops = {
  .owner	  = THIS_MODULE, 
  .read           = device_read,
  .write          = device_write,
  .open           = device_open,
  .unlocked_ioctl = device_ioctl,
};

static int __init simple_init(void){
    int rc = -1;
    rc = register_chrdev( MAJOR_NUM, DEVICE_RANGE_NAME, &Fops );
    if( rc < 0 ) {
        printk( KERN_ERR "%s registraion failed for  %d\n", DEVICE_FILE_NAME, MAJOR_NUM);
        return rc;
    }
    return 0;
}

static void __exit simple_cleanup(void)
{
    file_node *scan_file;
    file_node *tmp_file;
    channel_node *scan_channel, *tmp_channel;
    scan_file = my_driver;
    unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);  
    while(scan_file != NULL){
        scan_channel = scan_file->head;
        while(scan_channel != NULL){
            tmp_channel = scan_channel;
            scan_channel = scan_channel->next;
            kfree(tmp_channel);
        }
        tmp_file = scan_file;
        scan_file = scan_file->next;
        kfree(tmp_file);
    }
}


module_init(simple_init);
module_exit(simple_cleanup);
