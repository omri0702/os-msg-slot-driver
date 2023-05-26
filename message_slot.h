#ifndef MESSAGE_SLOT_H
#define MESSAGE_SLOT_H

#include <linux/ioctl.h> 

#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned int)
#define DEVICE_RANGE_NAME "char_dev"
#define MAJOR_NUM 235
#define BUF_LEN 128
#define DEVICE_FILE_NAME "message_slot"

typedef struct channel_node {
    int chanel_number;
    int len;
    char data[BUF_LEN]; 
    struct channel_node* next;
} channel_node;

typedef struct file_node {
    int minor;
    struct file_node* next;
    channel_node* head;
} file_node;

#endif
