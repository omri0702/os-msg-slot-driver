#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>

#include "message_slot.h"

int main(int argc, char** argv){
    if(argc != 4){
        fprintf (stderr, "%s\n", strerror(errno));
        exit(1);
    }
    char *path = argv[1];
    int channel = atoi(argv[2]);
    char *msg = argv[3];
    int fd, io, wr;
    fd = open(path, O_WRONLY);
    if(fd < 0){ 
        fprintf (stderr, "%s\n", strerror(errno));
        exit(1);
    }
    io = ioctl(fd,MSG_SLOT_CHANNEL,channel);
    if(io < 0){ 
        fprintf (stderr, "%s\n", strerror(errno));
        exit(1);
    }
    wr = write(fd, msg, strlen(msg));
    if(wr < 0){ 
        fprintf (stderr, "%s\n", strerror(errno));
        exit(1);
    }
    close(fd); 
    exit(0);
}
