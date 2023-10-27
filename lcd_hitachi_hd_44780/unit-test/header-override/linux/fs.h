#ifndef FS_H_
#define FS_H_

  #include <sys/types.h>

  //typedef long long loff_t;

  struct file {};
  struct inode {};

  struct file_operations {
    ssize_t (*read)(struct file *, char *, size_t, loff_t *);
    ssize_t (*write)(struct file *, const char *, size_t, loff_t *);
    int (*open) (struct inode *, struct file *);
    int (*release) (struct inode *, struct file *);
  };
#endif
