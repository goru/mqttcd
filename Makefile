MQTT_DIR=./paho/MQTTPacket
MQTT_SRC_DIR=$(MQTT_DIR)/src
MQTT_SAMPLE_DIR=$(MQTT_DIR)/samples

MQTT_SRCS=$(MQTT_SRC_DIR)/MQTTConnectClient.c $(MQTT_SRC_DIR)/MQTTSerializePublish.c $(MQTT_SRC_DIR)/MQTTPacket.c $(MQTT_SRC_DIR)/MQTTSubscribeClient.c $(MQTT_SRC_DIR)/MQTTDeserializePublish.c $(MQTT_SRC_DIR)/MQTTConnectServer.c $(MQTT_SRC_DIR)/MQTTSubscribeServer.c $(MQTT_SRC_DIR)/MQTTUnsubscribeServer.c $(MQTT_SRC_DIR)/MQTTUnsubscribeClient.c

CFLAGS=-I$(MQTT_SRC_DIR) -I$(MQTT_SAMPLE_DIR)
LDLIBS=

all:
	gcc -Wall -c $(MQTT_DIR)/samples/transport.c -Os
	gcc mqttcd.c transport.o -o mqttcd $(MQTT_SRCS) $(CFLAGS) $(LDLIBS)

clean:
	rm -f transport.o mqttcd
