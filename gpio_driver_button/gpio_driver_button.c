#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h> 

/* rpi pins start 512  */
#define GPIO_BASE  512 
#define LED_PIN    (GPIO_BASE + 21) // 20 and 21 pins
#define BUTTON_PIN (GPIO_BASE + 20) 

static struct gpio_desc *led_dev, *btn_dev;
static int irq_number; // the id for the button click signal

/* this code runs every time you actually click the button */
static irqreturn_t button_isr(int irq, void *data) {
    static int led_state = 0;
    led_state = !led_state; // the light state 
    gpiod_set_value(led_dev, led_state);
    pr_info("gpio_btn: button clicked and led state is now %d\n", led_state);
    return IRQ_HANDLED;
}

static int __init gpio_btn_init(void) {
    int ret;
    
    led_dev = gpio_to_desc(LED_PIN);
    btn_dev = gpio_to_desc(BUTTON_PIN);

    gpiod_direction_output(led_dev, 0); // start led turned off
    gpiod_direction_input(btn_dev);     // button is listening 

    /* get the signal id for button pin */
    irq_number = gpiod_to_irq(btn_dev);

    ret = request_irq(irq_number, button_isr, IRQF_TRIGGER_FALLING, "ulas_button_handler", NULL);
    
    if (ret) {
        pr_err("gpio_btn: something went wrong with the click setup\n");
        return ret;
    }

    pr_info("gpio_btn: driver is ready and click id is %d\n", irq_number);
    return 0;
}

static void __exit gpio_btn_exit(void) {
    free_irq(irq_number, NULL); 
    gpiod_set_value(led_dev, 0); 
    pr_info("gpio_btn: driver is gone\n");
}

module_init(gpio_btn_init);
module_exit(gpio_btn_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ulas kaplan");