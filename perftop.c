#include "perftop.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Menarul Alam");
MODULE_DESCRIPTION("perftop");

static char func_name[NAME_MAX] = "pick_next_task_fair";
module_param_string(func, func_name, NAME_MAX, S_IRUGO);
MODULE_PARM_DESC(func, "Function to kretprobe; this module will report the function's execution time");


DEFINE_SPINLOCK(xxx_lock);



int entry_count = 0;
int ret_count = 0;

int ctx_switch_count = 0;

int task;


 struct my_data {
        u64 entry_stamp;
    };

struct hm_node{
    int key;
    struct hlist_node node;
	u64  val;
	u64 rb_key;
};

DEFINE_HASHTABLE(hm, 9);



typedef struct RBNode{
    size_t val;
	int pid;
    struct rb_node node;
} RBNode;

struct rb_root * rbroot;



void rb_insert(RBNode * new_node){
    struct rb_node ** p = &(rbroot->rb_node);
    struct rb_node * prev = NULL;
    int val = new_node->val;
            while(*p!=NULL){
                prev = *p;
                RBNode * entry = rb_entry(*p, RBNode, node);
                if(entry->val>val)
                    p = &(*p)->rb_left;
                else
                    p = &(*p)->rb_right;
            }
            rb_link_node(&new_node->node, prev, p);
            rb_insert_color(&new_node->node, rbroot);
}

RBNode * rb_find(int val){
    struct rb_node * p = rbroot->rb_node;
    while(p){
        RBNode * entry = rb_entry(p, RBNode, node);
        if(entry->val==val){
         //   pr_info("Found RB Node entry: %d\n", val);
            return entry;
        }
        else if(entry->val>val)
            p = p->rb_left;
        else
            p = p->rb_right;
    }
    return NULL;
}


void rb_delete(int val){
    RBNode * node = rb_find(val);
    if(node==NULL){
     //   pr_info("Node not found for deletion for value: %d\n", val);
    }
    rb_erase(&node->node, rbroot);
    kfree(node);
   // pr_info("Erased and freed %d node from RB tree\n", val);
}


static int proc_show(struct seq_file *m, void *v) {
  seq_printf(m, "Hello world\n");
  seq_printf(m, "entry count: %d\nret count: %d\nctx switch count: %d\n", entry_count, ret_count, ctx_switch_count);
  struct rb_node * p = rb_last(rbroot);
  int count = 10;
  while(p&&count>0){
	  count--;
	  RBNode * entry = rb_entry(p, RBNode, node);
	  seq_printf(m, "%d) PID: %d time: %lu\n", 10-count, entry->pid, entry->val);
	  p = rb_prev(p);
  }
  return 0;
}

static int proc_open(struct inode *inode, struct  file *file) {
  return single_open(file, proc_show, NULL);
}

static const struct file_operations ops = {
  .owner = THIS_MODULE,
  .open = proc_open,
  .read = seq_read,
  .llseek = seq_lseek,
  .release = single_release,
};
static struct proc_dir_entry *ent;



/* Here we use the entry_hanlder to timestamp function entry */
static int entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	if(regs==NULL) return 0;
    entry_count++;
    int pid = regs->sp;
	task = pid;
	u64 time;
    asm volatile("mrs %0, cntvct_el0" : "=r" (time));
	spin_lock(&xxx_lock);
	int bkt;
    struct hm_node * hmp;
	bool found = false;
   	hash_for_each_possible(hm, hmp, node, pid){
		if(hmp->key==pid){
			found = true;
			//pr_info("Entry found for key: %d\n", p->val);
			hmp->val = time;
		}
	}
 	if(!found){
// 		//printk(KERN_ALERT "Allocating for hm node\n");
 		struct hm_node * hmn = kmalloc(sizeof(struct hm_node), GFP_ATOMIC);
		memset(hmn, 0, sizeof(struct hm_node));
		if(hmn==NULL){
			return -1;
		}
		hmn->key = pid;
		hmn->val = time;
		hmn->rb_key = pid;
		hash_add(hm, &hmn->node, hmn->key);
		//printk(KERN_ALERT "Allocating for rb node\n");
		RBNode * node = kmalloc(sizeof(RBNode), GFP_ATOMIC);
		memset(node, 0, sizeof(RBNode));
		if(node==NULL) return -1;
		node->val = pid;
		node->pid =pid;
		rb_insert(node);
// 		printk(KERN_ALERT "Inserted \n");
}
	spin_unlock(&xxx_lock);

	return 0;
}
NOKPROBE_SYMBOL(entry_handler);

/*
 * Return-probe handler: Log the return value and duration. Duration may turn
 * out to be zero consistently, depending upon the granularity of time
 * accounting on the platform.
 */
static int ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    ret_count++;
	int prev_task;
	if(regs==NULL)
		return 0;
    if(task != regs->sp){
		prev_task = task;
		int curr_task =  regs_get_register(regs, 14);
        u64 curr;
    asm volatile("mrs %0, cntvct_el0" : "=r" (curr));
		ctx_switch_count++;
		u64 elapsed;
		
    
		bool found = false;
		spin_lock(&xxx_lock);
		int bkt;
    	struct hm_node * hmp;
		hash_for_each_possible(hm, hmp, node, prev_task){
				if(hmp->key==prev_task){
					found = true;
					//pr_info("Entry found for key: %d\n", p->val);
					elapsed = curr - hmp->val;
					RBNode * node = rb_find(hmp->rb_key);
					if(node!=NULL){
						elapsed += node->val;
						rb_delete(node->val);
					}
					RBNode * nnode = kmalloc(sizeof(RBNode), GFP_ATOMIC);
					memset(nnode, 0, sizeof(RBNode));
					if(node==NULL) return -1;
					nnode->val = elapsed;
					nnode->pid = prev_task;
					hmp->rb_key = elapsed;
					rb_insert(nnode);
			}
	
		}
		found = false;
		hash_for_each_possible(hm, hmp, node, curr_task){
			if(hmp->key==curr_task){
				hmp->val = curr;
			}
			found = true;
		
		}
		if(!found){
			struct hm_node * hmn = kmalloc(sizeof(struct hm_node), GFP_ATOMIC);
			memset(hmn, 0, sizeof(struct hm_node));
			if(hmn==NULL){
				return -1;
			}
			hmn->key = curr_task;
			hmn->val = curr;
			hmn->rb_key = curr_task;
			hash_add(hm, &hmn->node, hmn->key);
			RBNode * node= kmalloc(sizeof(RBNode), GFP_ATOMIC);
			memset(node, 0, sizeof(RBNode));
			if(node==NULL) return -1;
			node->val = curr_task;
			node->pid =curr_task;
			rb_insert(node);
		}
		spin_unlock(&xxx_lock);
	}
	
	return 0;
}
NOKPROBE_SYMBOL(ret_handler);

static struct kretprobe my_kretprobe = {
	.handler		= ret_handler,
	.entry_handler		= entry_handler,
	/* Probe up to 20 instances concurrently. */
	.maxactive		= 20,
};



int pr_init(void){
	int ret;
	rbroot = kmalloc(sizeof(struct rb_root), GFP_KERNEL);
    *rbroot = RB_ROOT;
    ent=proc_create("perftop",0660,NULL,&ops);
	my_kretprobe.kp.symbol_name = func_name;
	ret = register_kretprobe(&my_kretprobe);
	if (ret < 0) {
		pr_err("register_kretprobe failed, returned %d\n", ret);
		return -1;
	}
	
	//pr_info("Planted return probe at %s: %p\n",
		//	my_kretprobe.kp.symbol_name, my_kretprobe.kp.addr);
	return 0;
}



 void pr_cleanup(void){
	unregister_kretprobe(&my_kretprobe);
	//pr_info("kretprobe at %p unregistered\n", my_kretprobe.kp.addr);

	/* nmissed > 0 suggests that maxactive was set too low. */
    proc_remove(ent);
}

module_init(pr_init);
module_exit(pr_cleanup);