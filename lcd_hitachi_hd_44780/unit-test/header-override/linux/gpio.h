#ifndef GPIO_H_
#define GPIO_H_

  #include <sys/types.h>

  #define GPIOF_DIR_OUT	(0 << 0)
  #define GPIOF_DIR_IN	(1 << 0)

  #define GPIOF_INIT_LOW	(0 << 1)
  #define GPIOF_INIT_HIGH	(1 << 1)

  #define GPIOF_IN		(GPIOF_DIR_IN)
  #define GPIOF_OUT_INIT_LOW	(GPIOF_DIR_OUT | GPIOF_INIT_LOW)
  #define GPIOF_OUT_INIT_HIGH	(GPIOF_DIR_OUT | GPIOF_INIT_HIGH)

  struct gpio {
    unsigned	gpio;
    unsigned long	flags;
    const char	*label;
  };

  static inline void gpio_set_value(unsigned int gpio, int value){}

  static inline void gpio_free_array(const struct gpio *array, size_t num){}
#endif