#ifndef DHT11_H
#define DHT11_H

#define DHT11_PIN                0
#define DHT11_TASK_PRIO          24
#define DHT11_TASK_STACK_SIZE    0x2000
#define DHT11_READ_INTERVAL_MS   2000
#define STACK_GUARD_SIZE         128

// 传感器类型标识
#define SENSOR_DHT11             3

typedef struct {
    float temperature;
    float humidity;
    uint8_t checksum;
} dht11_data_t;

void dht11_task(const char *arg);

#endif // DHT11_H