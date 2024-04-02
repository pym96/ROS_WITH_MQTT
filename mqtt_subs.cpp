#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 将字节数组转换回浮点数
float bytesToFloat(unsigned char *bytes) {
    union {
        float a;
        unsigned char bytes[4];
    } thing;
    
    memcpy(thing.bytes, bytes, sizeof(thing.bytes));
    return thing.a;
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message) {
    if(message->payloadlen == 7) { // 确保接收到的消息长度正确
        unsigned char *msg = (unsigned char *)message->payload;
        
        // 读取功能码
        unsigned char funcCode = msg[1];
        
        // 反序列化浮点数
        float sensorValue = bytesToFloat(&msg[2]);
        
        switch(funcCode) {
            case 0x01: // 温度
                printf("Temperature: %.2f\n", sensorValue);
                break;
            case 0x02: // 湿度
                printf("Humidity: %.2f\n", sensorValue);
                break;
            case 0x03: // 光照强度
                printf("Light intensity: %.2f\n", sensorValue);
                break;
            default:
                printf("Unknown sensor type\n");
        }
    } else {
        printf("Received message with unexpected length: %d\n", message->payloadlen);
    }
}

int main(int argc, char *argv[]) {
    struct mosquitto *mosq;
    int rc;

    mosquitto_lib_init();

    mosq = mosquitto_new("subscriber-test", true, NULL);
    if(mosq == NULL){
        fprintf(stderr, "Error: Out of memory.\n");
        return 1;
    }

    mosquitto_message_callback_set(mosq, on_message);

    rc = mosquitto_connect(mosq, "111.230.18.220", 1883, 60);
    if(rc != 0){
        fprintf(stderr, "Unable to connect (%s).\n", mosquitto_strerror(rc));
        return 1;
    }

    mosquitto_subscribe(mosq, NULL, "test/topic", 0);

    mosquitto_loop_forever(mosq, -1, 1);

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}
