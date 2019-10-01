#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <asm/io.h>

static int     ccipmu_open(   struct inode *inodep, struct file *fp);
static ssize_t ccipmu_read(   struct file *fp     , char *buffer, size_t len, loff_t * offset);
static ssize_t ccipmu_write(  struct file *fp     , const char *buffer, size_t len, loff_t *);
static long    ccipmu_ioctl(  struct file* fp     , unsigned int ioctl_num, unsigned long ioctl_param);
static int     ccipmu_release(struct inode *inodep, struct file *fp);

#define MAX_NB_COUNTERS 8
unsigned int number_counters = 3;  // set in init function by reading the actual number of counters from the PUM itself

#define CCI_ADDRESS 0xE8100000
#define OFFSET_PMCR 0x00100
#define OFFSET_CNT0 0x10000
#define STRIDE_CNTS 0x10000

#define SI(n) ((n) << 5)
#define MI(n) (((n) + 8) << 5)
#define PMCR_RST (1 << 1)
#define PMCR_CEN (1 << 0)

#define READ_REQUEST_HANDSHAKE 0x00

struct performance_counter {
    uint32_t evnt_sel;
    uint32_t evnt_data;
    uint32_t evnt_ctrl;
    uint32_t evnt_clr_ovfl;
};

void *virtual_addr;
uint32_t *pmcr;
struct performance_counter *counters[MAX_NB_COUNTERS];

static struct file_operations cci_pmu_fops = {
  .owner = THIS_MODULE,
  .open = ccipmu_open,
  .read = ccipmu_read,
  .write = ccipmu_write,
  .unlocked_ioctl = ccipmu_ioctl,
  .release = ccipmu_release,
};

static dev_t first; // Global variable for the first device number
static struct cdev c_dev;
static struct class *cl;

/*static bool test(void *virtual_addr) {
    printk(KERN_INFO "cci_pmu: peripheral_id0 = 0x%x\n", ioread32(virtual_addr + 0x00FE0));
    printk(KERN_INFO "cci_pmu: peripheral_id1 = 0x%x\n", ioread32(virtual_addr + 0x00FE4));
    printk(KERN_INFO "cci_pmu: peripheral_id2 = 0x%x\n", ioread32(virtual_addr + 0x00FE8));
    printk(KERN_INFO "cci_pmu: peripheral_id3 = 0x%x\n", ioread32(virtual_addr + 0x00FEC));
    printk(KERN_INFO "cci_pmu: peripheral_id4 = 0x%x\n", ioread32(virtual_addr + 0x00FD0));
    printk(KERN_INFO "cci_pmu: peripheral_id5 = 0x%x\n", ioread32(virtual_addr + 0x00FD4));
    printk(KERN_INFO "cci_pmu: peripheral_id6 = 0x%x\n", ioread32(virtual_addr + 0x00FD8));
    printk(KERN_INFO "cci_pmu: peripheral_id7 = 0x%x\n", ioread32(virtual_addr + 0x00FDC));
    printk(KERN_INFO "cci_pmu: component_id0 = 0x%x\n", ioread32(virtual_addr + 0x00FF0));
    printk(KERN_INFO "cci_pmu: component_id1 = 0x%x\n", ioread32(virtual_addr + 0x00FF4));
    printk(KERN_INFO "cci_pmu: component_id2 = 0x%x\n", ioread32(virtual_addr + 0x00FF8));
    printk(KERN_INFO "cci_pmu: component_id3 = 0x%x\n", ioread32(virtual_addr + 0x00FFC));

    return false;
}*/

static unsigned int get_nb_counters(void) {
    int nb = (ioread32(pmcr) >> 11) & 0x1F;
    if (nb > MAX_NB_COUNTERS)
        return MAX_NB_COUNTERS;
    else
        return nb;
}

static int ccipmu_open(struct inode *inodep, struct file *fp) {
    return 0;
}

static ssize_t ccipmu_read(struct file *fp, char *buffer, size_t len, loff_t *offset) {
#define ccipmu_read_bufferlen 32
    int minor = MINOR(fp->f_path.dentry->d_inode->i_rdev);

    static char status[ccipmu_read_bufferlen];
    uint32_t value = ioread32(&counters[minor]->evnt_data);
#if DEBUG
    printk(KERN_INFO "cci_pmu: read (offset = %lu): minor = %d, value = %lu\n", *offset, minor, value);
#endif
    int ct = snprintf(status, ccipmu_read_bufferlen, "%u\n", value);

    return simple_read_from_buffer(buffer, len, offset, status, ct);
}

static ssize_t ccipmu_write(struct file *fp, const char *buffer, size_t len, loff_t *offset) {
    uint32_t event;
    int status;
    int minor = MINOR(fp->f_path.dentry->d_inode->i_rdev);

    status = kstrtou32_from_user(buffer, len, 0, &event);
    if (status != 0)
    {
        return status;
    }

    if (event & 0xFFFFFE00) {
        return -EINVAL;
    }
    printk(KERN_INFO "cci_pmu: write: minor = %d: ", minor);
    int interface = (event >> 5) & 0xF;
    if (interface == 15) {
        printk(KERN_CONT "global event, ");
    } else if (interface > 8) {
        printk(KERN_CONT "master interface %d, ", interface - 8);
    } else {
        printk(KERN_CONT "slave interface %d,  ", interface);
    }
    printk(KERN_CONT "event %x\n", event & 0x1F);

    iowrite32(event, &counters[minor]->evnt_sel);

    return len;
}

static long ccipmu_ioctl(struct file* fp, unsigned int ioctl_num, unsigned long ioctl_param) {
    int minor = MINOR(fp->f_path.dentry->d_inode->i_rdev);

    if (ioctl_num == 0xC0) {
        printk(KERN_INFO "cci_pmu: ioctl: minor = %d, reset counter\n", minor);
        iowrite32(0, &counters[minor]->evnt_data);  // reset counter
    } else if (ioctl_num == 0xC1) {
        printk(KERN_INFO "cci_pmu: ioctl: minor = %d, enable counter\n", minor);
        iowrite32(1, &counters[minor]->evnt_ctrl);  // enable counter
    } else if (ioctl_num == 0xC2) {
        printk(KERN_INFO "cci_pmu: ioctl: minor = %d, disable counter\n", minor);
        iowrite32(0, &counters[minor]->evnt_ctrl);  // disable counter
    } else {
        printk(KERN_INFO "cci_pmu: ioctl: minor = %d, unknown param %lu\n", minor, ioctl_param);
        return -1;
    }
    return 0;
}

static int ccipmu_release(struct inode *inodep, struct file *fp) {
    return 0;
}

static int __init cci_pmu_init(void) {
    int subdevice, i;

    printk(KERN_INFO "cci_pmu: init called\n");

    virtual_addr = ioremap_nocache(CCI_ADDRESS, 0x00100000);
    if (virtual_addr == NULL) {
        printk(KERN_INFO "cci_pmu [ERROR] could not map IO memory region\n");
        goto iomap_failed;
    }

    printk(KERN_INFO "cci_pmu: successfully mapped IO memory region\n");
    pmcr = virtual_addr + OFFSET_PMCR;
    number_counters = get_nb_counters();
    printk(KERN_INFO "cci_pmu: detected %d counters\n", number_counters);
    for (i = 0; i < number_counters ; i++) {
        counters[i] = (struct performance_counter *)(virtual_addr + OFFSET_CNT0 + i * STRIDE_CNTS);
    }
    iowrite32(ioread32(pmcr) | PMCR_RST | PMCR_CEN, pmcr);  // reset all counters and enable counting
    printk(KERN_INFO "cci_pmu: successfully initialized PMU\n");

    if (alloc_chrdev_region(&first, 0, number_counters, "cci_pmu") < 0)
    {
        goto chrdev_region_failed;
    }
    if ((cl = class_create(THIS_MODULE, "chardrv")) == NULL)
    {
        goto class_failed;
    }
    cdev_init(&c_dev, &cci_pmu_fops);
    if (cdev_add(&c_dev, first, number_counters) == -1)
    {
        goto cdev_add_failed;
    }

    for (subdevice = 0; subdevice < number_counters; subdevice++)
    {
        if (device_create(cl, NULL, MKDEV(MAJOR(first), MINOR(first)+subdevice), NULL, "cci_pmu_%d_cntr", subdevice) == NULL)
        {
            goto device_create_failed;
        }
    }

    return 0;

device_create_failed:
    for(i = 0; i < subdevice; i++) {
        device_destroy(cl, MKDEV(MAJOR(first), MINOR(first)+i));
    }

cdev_add_failed:
    class_destroy(cl);
class_failed:
    unregister_chrdev_region(first, number_counters);
chrdev_region_failed:
    iounmap(virtual_addr);
iomap_failed:
    return -1;
}

static void __exit cci_pmu_exit(void) {
    int i;
    printk(KERN_INFO "cci_pmu: exit called\n");

    for (i = 0; i < number_counters; i++) {
        device_destroy(cl, MKDEV(MAJOR(first), MINOR(first)+i));
    }
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    iounmap(virtual_addr);
    //printk(KERN_INFO "cci_pmu: unmapped IO memory region\n");
}

module_init(cci_pmu_init);
module_exit(cci_pmu_exit);

MODULE_LICENSE("GPL");
