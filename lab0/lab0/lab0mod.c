#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
//#include <linux/sched.h>
#include <linux/ktime.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif

static int lab0_show(struct seq_file *m, void *v) {
  int hrs,mins,secs;
  s64 secondsup;

  secondsup = ktime_divns(ktime_get_coarse_boottime(), NSEC_PER_SEC);

  hrs= secondsup / 3600;
  mins = (secondsup % 3600) / 60;
  secs = secondsup % 60;


  seq_printf(m, "System up(%lld):  %d hrs, %d mins, %d seconds\n", secondsup, hrs, mins,secs);
  return 0;
}

static int lab0_open(struct inode *inode, struct  file *file) {
  return single_open(file, lab0_show, NULL);
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops lab0_fops = {
  .proc_open = lab0_open,
  .proc_read = seq_read,
  .proc_lseek = seq_lseek,
  .proc_release = single_release,
};
#else
static const struct file_operations lab0_fops = {
  .owner = THIS_MODULE,
  .open = lab0_open,
  .read = seq_read,
  .llseek = seq_lseek,
  .release = single_release,
};
#endif

static int __init lab0_init(void) {
  proc_create("lab0", 0, NULL, &lab0_fops);
  printk(KERN_INFO "lab0mod in\n");
  return 0;
}

static void __exit lab0_exit(void) {
  remove_proc_entry("lab0", NULL);
  printk(KERN_INFO "lab0mod out\n");
}

MODULE_LICENSE("GPL");
module_init(lab0_init);
module_exit(lab0_exit);
