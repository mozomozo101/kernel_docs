/*
 * wait test program
 * for linux-2.6 kernel
 */

#define MY_MAJOR   240
#define MY_MINOR   0
#define MY_MAX_MINOR 1
#define DEVNAME    "so2_cdev"

#include <linux/module.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/major.h>
#include <linux/device.h>
#include <linux/atomic.h>

struct my_device_data {
	struct cdev my_cdev;
};

struct my_device_data devs[MY_MAX_MINOR];

// accessフラグ初期化
static atomic_t access_flag = ATOMIC_INIT(0);

static int my_open(struct inode *inode, struct file *filp)
{
	int minor;

	// フラグが0なら1に、1なら1のまま。評価前のaccess_flag値を返す
	if (atomic_cmpxchg(&access_flag, 0, 1))
		return -EBUSY;

	minor = iminor(inode);
	filp->private_data = (void *)minor;
	pr_info("open my device. minor=%d\n", minor);
	return 0;
}

static int my_read(struct file *filp, char *user_buf, size_t count, loff_t *ppos)
{
	int minor;
	minor = (int)(filp->private_data);
	pr_info("read my device. minor=%d\n", minor);
	return 0;
}

const struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.open = my_open,
	.read = my_read,
};

static int reg(void){
	int i;
	int ret;
	pr_info("register device\n");

	// デバイス識別子を確保
	ret = register_chrdev_region(MKDEV(MY_MAJOR, MY_MINOR), MY_MAX_MINOR, DEVNAME); 

	// cdevを初期化し、先ほど確保したデバイス識別子と紐付け、カーネルに登録
	for (i=0; i<MY_MAX_MINOR; i++) {
		cdev_init(&devs[i].my_cdev, &my_fops);
		ret = cdev_add(&devs[i].my_cdev, MKDEV(MY_MAJOR, i), 1);
	}

	return 0;
}

static void unreg(void){
	int i;
	pr_info("unregister device\n");
	for (i=0; i<MY_MAX_MINOR; i++) {
		cdev_del(&devs[i].my_cdev);
	}
	unregister_chrdev_region(MKDEV(MY_MAJOR, MY_MINOR), MY_MAX_MINOR);
}

module_init(reg);
module_exit(unreg);

MODULE_LICENSE("GPL");
