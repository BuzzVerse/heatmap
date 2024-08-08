#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdcard.h"
#include "gps.h"
#include "lora.h"
#include <string.h>
#include <inttypes.h>

#define TAG "Main"
#define PROJECT_VER "0.0.0"
#define PROJECT_NAME "BuzzVerse"

  
typedef union {
  int32_t deserialized;
  uint8_t serialized[4];
} coordinates_t;

typedef union {
  uint64_t deserialized;
  uint8_t serialized[8];
} axis_t;

typedef union {
  uint16_t deserialized;
  uint8_t serialized[2];
} altitude_t;

static packet_t packet;

void print_logo(void)
{
  ESP_LOGI(TAG, "     ,     ,");
  ESP_LOGI(TAG, "    (\\____/)");
  ESP_LOGI(TAG, "     (_oo_)");
  ESP_LOGI(TAG, "       (O)");
  ESP_LOGI(TAG, "     __||__    \\)");
  ESP_LOGI(TAG, "  []/______\\[] /");
  ESP_LOGI(TAG, "  / \\______/ \\/");
  ESP_LOGI(TAG, " /    /__\\");
  ESP_LOGI(TAG, "(\\   /____\\");
}

void test_gps_packet(void)
{
  coordinates_t latitude, longitude;
  altitude_t altitude;
  uint8_t status = 0;
  static uint8_t test_gps_msg_id = 0;
  
  packet.version = (CONFIG_PACKET_VERSION << 4) | 0; // Reserved 4 bits set to 0
  packet.id = (CONFIG_CLASS_ID << 4) | CONFIG_DEVICE_ID;
  packet.msgID = test_gps_msg_id++;
  packet.msgCount++;
  packet.dataType = PACKET_TYPE_GPS;

  gps_get_pos(&latitude.deserialized,
	      &longitude.deserialized,
	      &altitude.deserialized);
  gps_get_status(&status);

  packet.data[0] = status;
  packet.data[1] = altitude.serialized[0];
  packet.data[2] = altitude.serialized[1];
  packet.data[3] = latitude.serialized[0];
  packet.data[4] = latitude.serialized[1];
  packet.data[5] = latitude.serialized[2];
  packet.data[6] = latitude.serialized[3];
  packet.data[7] = longitude.serialized[0];
  packet.data[8] = longitude.serialized[1];
  packet.data[9] = longitude.serialized[2];
  packet.data[10] = longitude.serialized[3];
}

void test_sms_packet(void)
{
  uint8_t i;
  static char start_chr = '!';
  static uint8_t test_sms_msg_id = 0;

  packet.version = (CONFIG_PACKET_VERSION << 4) | 0; // Reserved 4 bits set to 0
  packet.id = (CONFIG_CLASS_ID << 4) | CONFIG_DEVICE_ID;
  packet.msgID = test_sms_msg_id++;
  packet.msgCount++;
  packet.dataType = PACKET_TYPE_SMS;

  for (i = 0; i < 59; i++) {
    packet.data[i] = start_chr + i;
  }
  if (start_chr == '!') {
    start_chr = 'A';
  } else {
    start_chr = '!';
  }
}

void test_bme280_packet(void)
{
  static uint8_t test_bme280_msg_id = 0;
  static uint8_t bme280_temp, bme280_hum, bme280_press;

  packet.version = (CONFIG_PACKET_VERSION << 4) | 0; // Reserved 4 bits set to 0
  packet.id = (CONFIG_CLASS_ID << 4) | CONFIG_DEVICE_ID;
  packet.msgID = test_bme280_msg_id++;
  packet.msgCount++;
  packet.dataType = PACKET_TYPE_BME280;

  bme280_temp++;
  bme280_hum += 3;
  bme280_press += 7;
  packet.data[0] = bme280_temp;
  packet.data[1] = bme280_hum;
  packet.data[2] = bme280_press;
}

void test_bma400_packet(void)
{
  static uint8_t test_bma400_msg_id = 0;
  static axis_t axis_x, axis_y, axis_z;

  packet.version = (CONFIG_PACKET_VERSION << 4) | 0; // Reserved 4 bits set to 0
  packet.id = (CONFIG_CLASS_ID << 4) | CONFIG_DEVICE_ID;
  packet.msgID = test_bma400_msg_id++;
  packet.msgCount++;
  packet.dataType = PACKET_TYPE_BMA400;

  axis_x.deserialized++;
  axis_y.deserialized += 19;// Increase by prime.
  axis_z.deserialized += 113;// Increase by prime.

  packet.data[0] = axis_x.serialized[0];
  packet.data[1] = axis_x.serialized[1];
  packet.data[2] = axis_x.serialized[2];
  packet.data[3] = axis_x.serialized[3];
  packet.data[4] = axis_x.serialized[4];
  packet.data[5] = axis_x.serialized[5];
  packet.data[6] = axis_x.serialized[6];
  packet.data[7] = axis_x.serialized[7];  

  packet.data[8] = axis_y.serialized[0];
  packet.data[9] = axis_y.serialized[1];
  packet.data[10] = axis_y.serialized[2];
  packet.data[11] = axis_y.serialized[3];
  packet.data[12] = axis_y.serialized[4];
  packet.data[13] = axis_y.serialized[5];
  packet.data[14] = axis_y.serialized[6];
  packet.data[15] = axis_y.serialized[7];  
  
  packet.data[16] = axis_z.serialized[0];
  packet.data[17] = axis_z.serialized[1];
  packet.data[18] = axis_z.serialized[2];
  packet.data[19] = axis_z.serialized[3];
  packet.data[20] = axis_z.serialized[4];
  packet.data[21] = axis_z.serialized[5];
  packet.data[22] = axis_z.serialized[6];
  packet.data[23] = axis_z.serialized[7];  
}

void test_mq2_packet(void)
{
  static uint8_t test_mq2_msg_id = 0;
  static uint8_t status, v0, v1, v2, v3, v4;

  packet.version = (CONFIG_PACKET_VERSION << 4) | 0; // Reserved 4 bits set to 0
  packet.id = (CONFIG_CLASS_ID << 4) | CONFIG_DEVICE_ID;
  packet.msgID = test_mq2_msg_id++;
  packet.msgCount++;
  packet.dataType = PACKET_TYPE_MQ2;

  packet.data[0] = status++;
  packet.data[1] = v0++;
  packet.data[2] = 0;
  packet.data[3] = 0;
  packet.data[4] = 0;
  packet.data[5] = v1++;
  packet.data[6] = 0;
  packet.data[7] = 0;
  packet.data[8] = 0;
  packet.data[9] = v2++;
  packet.data[10] = 0;
  packet.data[11] = 0;
  packet.data[12] = 0;
  packet.data[13] = v3++;
  packet.data[14] = 0;
  packet.data[15] = v4++;
  packet.data[16] = 0;
}

void lora_task(){

    vTaskDelay(100);
    lora_init();
    lora_dump_registers();
    vTaskDelay(100);
    lora_close();
    lora_get_config();

    while (1) {
      // GPS
      test_gps_packet();
      lora_status_t send_status = lora_send(&packet);
      if (LORA_OK != send_status) {
	ESP_LOGE(TAG, "GPS Packet send failed");
      } else {
	ESP_LOGI(TAG, "GPS Packet sent successfully");
      }
      vTaskDelay(200);

      // SMS
      test_sms_packet();
      send_status = lora_send(&packet);
      if (LORA_OK != send_status) {
	ESP_LOGE(TAG, "SMS Packet send failed");
      } else {
	ESP_LOGI(TAG, "SMS Packet sent successfully");
      }
      vTaskDelay(200);

      // BME280
      test_bme280_packet();
      send_status = lora_send(&packet);
      if (LORA_OK != send_status) {
	ESP_LOGE(TAG, "BME280 Packet send failed");
      } else {
	ESP_LOGI(TAG, "BME280 Packet sent successfully");
      }
      vTaskDelay(200);

      // BMA400
      test_bma400_packet();
      send_status = lora_send(&packet);
      if (LORA_OK != send_status) {
	ESP_LOGE(TAG, "BMA400 Packet send failed");
      } else {
	ESP_LOGI(TAG, "BMA400 Packet sent successfully");
      }
      vTaskDelay(200);

      test_mq2_packet();
      send_status = lora_send(&packet);
      if (LORA_OK != send_status) {
	ESP_LOGE(TAG, "MQ2 Packet send failed");
      } else {
	ESP_LOGI(TAG, "MQ2 Packet sent successfully");
      }
      vTaskDelay(200);

    }
}


void app_main(void){
  BaseType_t xReturned;
  TaskHandle_t xHandle = NULL;

  print_logo();
  gps_warm_start();
  xReturned = xTaskCreate(
			  &gps_task,       /* Function that implements the task. */
			  "GPS",           /* Text name for the task. */
			  4*1024,            /* Stack size in words, not bytes. */
			  ( void * ) 1,    /* Parameter passed into the task. */
			  tskIDLE_PRIORITY,/* Priority at which the task is created. */
			  &xHandle );      /* Used to pass out the created task's handle. */

  if( xReturned != pdPASS ) {
    vTaskDelete( xHandle );
    ESP_LOGE(TAG, "[FATAl] Could not create GPS task!");
  }

  xReturned = xTaskCreate(
  			  &lora_task,      /* Function that implements the task. */
  			  "LoRa",          /* Text name for the task. */
  			  4*1024,          /* Stack size in words, not bytes. */
			  ( void * ) 1,    /* Parameter passed into the task. */
			  tskIDLE_PRIORITY,/* Priority at which the task is created. */
			  &xHandle );      /* Used to pass out the created task's handle. */

  if( xReturned != pdPASS ) {
    vTaskDelete( xHandle );
    ESP_LOGE(TAG, "[FATAl] Could not create LoRa task!");
  }

  while (1) {
    vTaskDelay(100);
  }
}
