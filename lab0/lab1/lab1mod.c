#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/ktime.h>


#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif

static int lab1_show(struct seq_file *m, void *v) {
  struct task_struct *cur_task = current; 
  const struct cred *cred = cur_task->cred;

  seq_printf(m, "Current Process PCB Information\n");
  seq_printf(m, "Name = %s\n", cur_task->comm);
  seq_printf(m, "PID = %d\n", cur_task->pid);
  seq_printf(m, "PPID = %d\n", task_ppid_nr(cur_task));
  if (cur_task->state == TASK_RUNNING){
    seq_printf(m, "State = Running\n");
  } else if (cur_task->state == TASK_INTERRUPTIBLE || cur_task->state == TASK_UNINTERRUPTIBLE) {
    seq_printf(m, "State = Waiting\n");
  } else {
    seq_printf(m, "State = Stopped\n");
  }
  
  seq_printf(m, "Real UID = %d\n", cred->uid.val);
  seq_printf(m, "Effective UID = %d\n", cred->euid.val);
  seq_printf(m, "Saved UID = %d\n", cred->suid.val);
  seq_printf(m, "Real GID = %d\n", cred->gid.val);
  seq_printf(m, "Effective GID = %d\n", cred->egid.val);
  seq_printf(m, "Saved GID = %d\n", cred->sgid.val);
  return 0;
}

static int lab1_open(struct inode *inode, struct  file *file) {
  return single_open(file, lab1_show, NULL);
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops lab1_fops = {
  /* operation mapping */
};
#else
static const struct file_operations lab1_fops = {
  .owner = THIS_MODULE,
  .open = lab1_open,
  .read = seq_read,
  .llseek = seq_lseek,
  .release = single_release,
};
#endif

static int __init lab1_init(void) {
  proc_create("lab1", 0, NULL, &lab1_fops); 
  printk(KERN_INFO "lab1mod in\n");
  return 0;
}

static void __exit lab1_exit(void) {
  remove_proc_entry("lab1", NULL);
  printk(KERN_INFO "lab1mod out\n");
}

MODULE_LICENSE("GPL");
module_init(lab1_init);
module_exit(lab1_exit);
