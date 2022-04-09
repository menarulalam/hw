#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>   
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>

static ssize_t pr_write(struct file *file, const char __user *ubuf,size_t count, loff_t *ppos);

unsigned long copy_to_user(void __user *to,const void *from, unsigned long n);
unsigned long copy_from_user(void *to,const void __user *from,unsigned long n);

