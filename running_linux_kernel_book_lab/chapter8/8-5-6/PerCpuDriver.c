#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>

#define    MAJOR_NUM    231
#define    DEVICE_NAME  "hellodr"

DEFINE_PER_CPU( long, gUsage ) = 0;

static int     DriverOpen(struct inode *inode, struct file *filp);
static ssize_t DriverWrite(struct file *filp, const char __user *buf, size_t cnt, loff_t *ppos);
static int     DriverClose(struct inode *inode, struct file *filp);
static long    DriverIOControl(struct file *filp, unsigned int cmd, unsigned long arg);

int DriverOpen( struct inode *pslINode, struct file *pslFileStruct )
{
    printk( KERN_ALERT DEVICE_NAME " hello open.\n" );
    return(0);
}


ssize_t DriverWrite( struct file *pslFileStruct, const char __user *pBuffer, size_t nCount, loff_t *pOffset )
{
    printk( KERN_ALERT DEVICE_NAME " hello write.\n" );
    return(0);
}


int DriverClose( struct inode *pslINode, struct file *pslFileStruct )
{
    int     i       = 0;
    unsigned long   ulBaseAddr  = 0;
    unsigned long   ulOffset    = 0;
    long        *pUsage     = NULL;
    long        usageSum    = 0;

    ulOffset = (unsigned long) (&gUsage);
    for_each_online_cpu( i )
    {
        ulBaseAddr  = __per_cpu_offset[i];
        pUsage      = (long *) (ulBaseAddr + ulOffset);
        usageSum    += (*pUsage);
        printk( KERN_ALERT DEVICE_NAME " pUsage = %lx, *pUsage = %ld\n", (unsigned long) pUsage, *pUsage );
    }
    printk( KERN_ALERT DEVICE_NAME " %ld\n", usageSum );

    return(0);
}


long DriverIOControl( struct file *pslFileStruct, unsigned int uiCmd, unsigned long ulArg )
{
    long *pUsage = NULL;
    /* printk( KERN_ALERT DEVICE_NAME ": pUsage = 0x%lx %lx %ld", (unsigned long) pUsage, (unsigned long) (&gUsage), (*pUsage) ); */
    preempt_disable();
    pUsage = this_cpu_ptr( (long *) (&gUsage) );
    (*pUsage)++;
    preempt_enable();
    return(0);
}


struct file_operations hello_flops = {
    .owner      = THIS_MODULE,
    .open       = DriverOpen,
    .write      = DriverWrite,
    .release    = DriverClose,
    .unlocked_ioctl = DriverIOControl
};

static int __init hello_init( void )
{
    int ret;

    ret = register_chrdev( MAJOR_NUM, DEVICE_NAME, &hello_flops );
    if ( ret < 0 )
    {
        printk( KERN_ALERT DEVICE_NAME " can't register major number.\n" );
        return(ret);
    }
    printk( KERN_ALERT DEVICE_NAME " initialized.\n" );
    return(0);
}


static void __exit hello_exit( void )
{
    printk( KERN_ALERT DEVICE_NAME " removed.\n" );
    unregister_chrdev( MAJOR_NUM, DEVICE_NAME );
}


module_init( hello_init );
module_exit( hello_exit );
MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Sean Depp" );
