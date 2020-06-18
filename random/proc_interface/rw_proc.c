/**
Kernel version: 5.7.0-rc6linux-5.7-dirty
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/seq_file.h>

struct proc_dir_entry *p_valor;
#define PROCFS_MAX_SIZE 30
char proc_buf[PROCFS_MAX_SIZE];
int abc=100;

static ssize_t p_valor_read(struct file *fp, char *buf, size_t len, loff_t *off) {
	static int finished =0;
	//int xyz;
	if(finished){
		finished=0;
		return 0;
	}
	finished = 1;

	//xyz=100;
	//strcpy(buf, "I am from /proc file\n");
	//sprintf(buf, "xyz: %d\n", xyz);
	//memset(buf, 0xaa, 6);

	//when write
	sprintf(buf, "abc: %d\n", abc);
	return strlen(buf);
	//return 6;
}

/*
static const struct file_operations p_valor_fops = {
	.owner=THIS_MODULE,
	.read=p_valor_read,
};
*/

static ssize_t p_valor_write(struct file *fp, const char *buf, size_t len, loff_t * off) {

	if(len > PROCFS_MAX_SIZE) {
		return -EFAULT;
	}
	if(copy_from_user(proc_buf, buf, len)) {
		return -EFAULT;
	}

	abc=simple_strtoul(proc_buf,NULL,10);
	return len;
}

// https://lore.kernel.org/netdev/20191225172546.GB13378@avx2/
static struct proc_ops p_valor_fops = {
	.proc_read=p_valor_read,
	.proc_write=p_valor_write,
};

// create file on kernel module
int __init valor_init(void) {
	p_valor = proc_create("p_valor", 0444, NULL, &p_valor_fops);
	if(p_valor == NULL) {
		printk(KERN_ALERT "ERROR: Could not initialize %s\n", "p_valor");
	}
	return 0;
}

// remove file on rmmod
void __exit valor_exit(void) {
	remove_proc_entry("p_valor", NULL);
}

module_init(valor_init);
module_exit(valor_exit);
