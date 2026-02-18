#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio/consumer.h>

/* gpio base is 512 as confirmed by debugfs */
#define GPIO_BASE  512 
#define LED_PIN    (GPIO_BASE + 21)
#define BUTTON_PIN (GPIO_BASE + 20)

static struct gpio_desc *led_dev, *btn_dev;

/**
 * @brief module initialization function
 */
static int __init gpio_init(void)
{
    int ret;

    /* request descriptor for pin 533 */
    led_dev = gpio_to_desc(LED_PIN);
    if (!led_dev) {
        pr_err("gpio: failed for pin %d. check base!\n", LED_PIN);
        return -ENODEV;
    }

    /* request descriptor for pin 532 */
    btn_dev = gpio_to_desc(BUTTON_PIN);
    if (!btn_dev) {
        pr_err("gpio: failed for pin %d. check base!\n", BUTTON_PIN);
        return -ENODEV;
    }

    /* set led to output and turn on to verify */
    ret = gpiod_direction_output(led_dev, 1);
    if (ret) return ret;

    /* set button to input */
    ret = gpiod_direction_input(btn_dev);
    if (ret) return ret;

    pr_info("gpio: module loaded successfully. base: %d, led: %d, button status: %d\n", 
            GPIO_BASE, LED_PIN, gpiod_get_value(btn_dev));

    return 0;
}

/**
 * @brief module exit function
 */
static void __exit gpio_exit(void)
{
    if (led_dev)
        gpiod_set_value(led_dev, 0);
    
    pr_info("gpio: module unloaded. led cleared.\n");
}

module_init(gpio_init);
module_exit(gpio_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ulas kaplan");
MODULE_DESCRIPTION("gpio driver for rpi 3b+ with offset 512");