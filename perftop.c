
#include "perftop.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Menarul Alam");
MODULE_DESCRIPTION("perftop");

#define NAME "perftop"

char * hw = "Hello World";



static ssize_t pr_write(struct file *file, const char __user *ubuf,size_t count, loff_t *ppos){
	return -1;
}

static ssize_t pr_read(struct file *file, char __user *ubuf,size_t count, loff_t *ppos){
	printk( KERN_DEBUG "read for %s\n", NAME);
    printk( KERN_DEBUG "args: %ld\n", count);
    if (sizeof(hw)> count){
        printk(KERN_DEBUG "Here\n");
		return 0;
}
	else {
        if(copy_to_user(ubuf,hw,sizeof(hw)))
		return -EFAULT;
        return sizeof(hw);
	}

}


static struct file_operations ops={
	.owner = THIS_MODULE,
	.read = pr_read,
	.write = pr_write,
};
static struct proc_dir_entry *ent;

int static pr_init(void){
	ent=proc_create("perftop",0660,NULL,&ops);
	return 0;
}



static void pr_cleanup(void){
	proc_remove(ent);
}

module_init(pr_init);
module_exit(pr_cleanup);