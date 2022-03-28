#include "thread.h"
#include "periph/gpio.h"
#include "xtimer.h"
#include "msg.h"

uint32_t toggle = 0;
uint32_t timing = 0;
uint32_t val = 1;
uint32_t tock = 1000000;
uint32_t freq = 1000000;

static msg_t rcv_queue[8];

// Выделение памяти под стеки двух тредов
char thread_one_stack[THREAD_STACKSIZE_DEFAULT];
char thread_two_stack[THREAD_STACKSIZE_DEFAULT];

// Глобально объявляется структура, которая будет хранить PID (идентификатор треда)
// Нам нужно знать этот идентификатор, чтобы посылать сообщения в тред
static kernel_pid_t thread_one_pid;
static kernel_pid_t thread_two_pid;

// Обработчик прерывания с кнопки
void btn_handler(void *arg)
{
  // Прием аргументов из главного потока
  (void)arg;
  // Создание объекта для хранения сообщения
  if (((xtimer_usec_from_ticks(xtimer_now())-timing)>250000)) 
  {



  msg_t msg1;
  msg_t msg2;
  if(val == 6)
    val = 1;
  if(tock == 0)
    tock = 1000000;
  msg1.content.value = val;
  msg2.content.value = tock;
  tock -= 100000;
  val++;
  // Поскольку прерывание вызывается и на фронте, и на спаде, нам нужно выяснить, что именно вызвало обработчик
  // для этого читаем значение пина с кнопкой. Если прочитали высокое состояние - то это был фронт.  
  if (gpio_read(GPIO_PIN(PORT_A,0)) > 0){
    // Отправка сообщения в тред по его PID
    // Сообщение остается пустым, поскольку нам сейчас важен только сам факт его наличия, и данные мы не передаем
    msg_send(&msg1, thread_one_pid);  
  }
  else
  {
    msg_send(&msg2, thread_two_pid);
  }
  timing = xtimer_usec_from_ticks(xtimer_now());  
}

}

// Первый поток
void *thread_one(void *arg)
{
  // Прием аргументов из главного потока
  (void) arg;
  // Создание объекта для хранения сообщения 
  msg_t msg;
  // Инициализация пина PC8 на выход
  gpio_init(GPIO_PIN(PORT_C,8),GPIO_OUT);
  msg_init_queue(rcv_queue, 8);
  while(1){
    // Ждем, пока придет сообщение
    // msg_receive - это блокирующая операция, поэтому задача зависнет в этом месте, пока не придет сообщение
  	msg_receive(&msg);
    for(uint32_t i = 0; i < msg.content.value; i++)
    {
      gpio_toggle(GPIO_PIN(PORT_C,8));
      xtimer_usleep(333333);
      gpio_toggle(GPIO_PIN(PORT_C,8));
      xtimer_usleep(333333);
    }
  }
  // Хотя код до сюда не должен дойти, написать возврат NULL все же обязательно надо    
  return NULL;
}

// Второй поток
void *thread_two(void *arg)
{

  // Прием аргументов из главного потока
  (void) arg;
  msg_t msg;
  // Инициализация пина PC9 на выход
  gpio_init(GPIO_PIN(PORT_C,9),GPIO_OUT);
  msg_init_queue(rcv_queue, 8);
  
  while(1){
    if(msg_avail() > 0)
    {
      msg_receive(&msg);
      freq = msg.content.value;
    }
    // Включаем светодиод
    gpio_set(GPIO_PIN(PORT_C, 9));
    xtimer_usleep(freq);
    gpio_clear(GPIO_PIN(PORT_C, 9));
    xtimer_usleep(freq);
    }
    return NULL;
}


int main(void)
{
  // Инициализация прерывания с кнопки (подробнее в примере 02button)
	gpio_init_int(GPIO_PIN(PORT_A,0),GPIO_IN,GPIO_BOTH,btn_handler,NULL);

  // Создение потоков (подробнее в примере 03threads)
  // Для первого потока мы сохраняем себе то, что возвращает функция thread_create,
  // чтобы потом пользоваться этим как идентификатором потока для отправки ему сообщений
  thread_one_pid = thread_create(thread_one_stack, sizeof(thread_one_stack),
                  THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
                  thread_one, NULL, "thread_one");
  // обратите внимание, что у потоков разный приоритет: на 1 и на 2 выше, чем у main
  thread_two_pid = thread_create(thread_two_stack, sizeof(thread_two_stack),
                  THREAD_PRIORITY_MAIN - 2, THREAD_CREATE_STACKTEST,
                  thread_two, NULL, "thread_two");

  while (1){

    }

    return 0;
}