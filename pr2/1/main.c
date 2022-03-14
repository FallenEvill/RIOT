// Подключение библиотек
#include "xtimer.h"
#include "timex.h"
#include "periph/gpio.h"

uint32_t timing = 0;

// Обработчик прерывания по нажатию кнопки
void btn_handler(void *arg)
{
  // Прием аргументов, передаваемых из главного потока.
  (void)arg;
    if ((xtimer_usec_from_ticks(xtimer_now()) - timing) > 250000){ 
        gpio_toggle(GPIO_PIN(PORT_C, 8));
        timing = xtimer_usec_from_ticks(xtimer_now());
    }
}


int main(void)
{
  // Инициализация пина PA0, к которому подключена кнопка, как источника прерывания.
  // GPIO_RISING - прерывание срабатывает по фронту
  // btn_handler - имя функции обработчика прерывания
  // NULL - ничего не передаем в аргументах
  gpio_init_int(GPIO_PIN(PORT_A, 0), GPIO_IN, GPIO_RISING, btn_handler, NULL);
  // Инициализация пина PC8 на выход
    gpio_init(GPIO_PIN(PORT_C, 8), GPIO_OUT);

  while(1){
  }
  return 0;
}
