#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/jiffies.h>
#include <linux/sched.h>

#define BUFFER_SIZE 1280
#define PROC_NAME "porcc"
long  int *res;

ssize_t proc_read(struct file *file, char __user *usr_buf, size_t count, loff_t *pos);
ssize_t proc_write(struct file *file, const char __user *user_buf, size_t count, loff_t *pos);

static const struct proc_ops proc_ops = {
    .proc_read = proc_read,
    .proc_write = proc_write,
};

int proc_init(void)
{
    res = kmalloc(sizeof(long int), GFP_KERNEL);
    if (!res) {
        printk(KERN_ERR"Failed to allocate memory for res \n ");
        return -ENOMEM;
    }

    proc_create(PROC_NAME, 0666, NULL, &proc_ops);
    printk(KERN_INFO "kernel uploaded \n");
    return 0;
}


void proc_exit(void)
{
    kfree(res);
    remove_proc_entry(PROC_NAME, NULL);
    printk(KERN_INFO "kernel removed ");
}
ssize_t proc_write(struct file *file, const char __user *user_buf, size_t count, loff_t *pos)
{
    int rv = 0;
    char *k_mem;
    unsigned int base = 10;
    int suc ;
    /* allocate kernel memory */
    k_mem = kmalloc(count, GFP_KERNEL);/*like malloc*/
    /* copies user space usr buf to kernel memory */
    copy_from_user(k_mem, user_buf, count);
    /*conv the string value of user_buf in longstr*/
    suc = kstrtol(k_mem, base, res);

    /*print the k_mem*/
    printk(KERN_INFO "str value k_mem:%s \n", k_mem);
    printk(KERN_INFO "int value res is :%ld \n", *res);
    /* return kernel memory */
    kfree(k_mem);
    return count;
}

ssize_t proc_read(struct file *file, char __user *usr_buf, size_t count, loff_t *pos)
{
    extern unsigned long volatile jiffies;
    int rv;
    char buffer[BUFFER_SIZE];
    static int completed = 0;
    struct pid *pid;
    struct task_struct *struct_pid;


    if (completed) {
        completed = 0;
        return 0;
    }

    completed = 1;

    pid = find_vpid(*res);
    struct_pid = pid_task(pid, PIDTYPE_PID);
    if (struct_pid == NULL) {
        return 0;
    }

    rv = snprintf(buffer, BUFFER_SIZE, "PID: %d, Command: %s, State: %u\n",
                  struct_pid->pid, struct_pid->comm, struct_pid->__state);//here maybe there is a problem the sge fault
    printk(KERN_INFO "PID: %d, Command: %s, State: %u\n",
           struct_pid->pid, struct_pid->comm, struct_pid->__state);
    copy_to_user(usr_buf, buffer, rv);
    if (rv < 0 || rv > count) {
        return -EINVAL;
    }

    return rv;
}


module_init(proc_init);
module_exit(proc_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TaskInfo");
MODULE_AUTHOR("SGG");