#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>

#include "message_slot.h"

int main(int argc, char** argv){
    if(argc != 3){
        fprintf (stderr, "%s\n", strerror(errno));
        exit(1);
    }
    char *path = argv[1];
    int channel = atoi(argv[2]);
    int fd, io, rd;
    char buf[BUF_LEN];
    fd = open(path, O_RDONLY);
    if(fd < 0){ 
        fprintf (stderr, "%s\n", strerror(errno));
        exit(1);
    }
    io = ioctl(fd,MSG_SLOT_CHANNEL,channel);
    if(io < 0){ 
        fprintf (stderr, "%s\n", strerror(errno));
        exit(1);
    }
    rd = read(fd, buf, BUF_LEN);
    if(rd<0){
        fprintf (stderr, "%s\n", strerror(errno));
        exit(1);
    }
    close(fd);

    if(write(STDOUT_FILENO, buf, rd)<rd){
        fprintf (stderr, "%s\n", strerror(errno));
        exit(1);
    }
    exit(0);
}
