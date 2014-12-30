MQTT_DIR=./paho/MQTTPacket
MQTT_SRC_DIR=$(MQTT_DIR)/src
MQTT_SAMPLE_DIR=$(MQTT_DIR)/samples

MQTTCD_SRCS=mqttcd.c mqttcd_logger.c mqttcd_arg.c

MQTT_SRCS=$(MQTT_SRC_DIR)/MQTTConnectClient.c $(MQTT_SRC_DIR)/MQTTSerializePublish.c $(MQTT_SRC_DIR)/MQTTPacket.c $(MQTT_SRC_DIR)/MQTTSubscribeClient.c $(MQTT_SRC_DIR)/MQTTDeserializePublish.c $(MQTT_SRC_DIR)/MQTTConnectServer.c $(MQTT_SRC_DIR)/MQTTSubscribeServer.c $(MQTT_SRC_DIR)/MQTTUnsubscribeServer.c $(MQTT_SRC_DIR)/MQTTUnsubscribeClient.c

CFLAGS=-I$(MQTT_SRC_DIR) -I$(MQTT_SAMPLE_DIR)
LDLIBS=

ifdef DEBUG
DEBUG_MACRO=-DDEBUG
endif

all:
	gcc -Wall -c $(MQTT_DIR)/samples/transport.c -Os
	gcc $(MQTTCD_SRCS) transport.o -o mqttcd $(MQTT_SRCS) $(CFLAGS) $(LDLIBS) $(DEBUG_MACRO)

clean:
	rm -f transport.o mqttcd
