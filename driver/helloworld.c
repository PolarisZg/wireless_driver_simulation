#include<linux/module.h>
#include<linux/init.h>
#include<linux/types.h>

static int hello_init(void){
    printk("hello world \r\n");
    return 0;
}

static void hello_exit(void){
    printk("hello world over \r\n");
}

MODULE_LICENSE("GPL");
module_init(hello_init);
module_exit(hello_exit);