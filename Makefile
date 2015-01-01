# common
OBJS_DIR=./obj

# mqtt directories
MQTT_DIR=./paho/MQTTPacket
MQTT_SRC_DIR=$(MQTT_DIR)/src
MQTT_SAMPLE_DIR=$(MQTT_DIR)/samples

# mqtt objects
MQTT_PACKET_OBJS=MQTTConnectClient.o MQTTSerializePublish.o MQTTPacket.o MQTTSubscribeClient.o MQTTDeserializePublish.o MQTTConnectServer.o MQTTSubscribeServer.o MQTTUnsubscribeServer.o MQTTUnsubscribeClient.o
MQTT_TRANSPORT_OBJS=transport.o
MQTT_OBJS=$(patsubst %,$(OBJS_DIR)/%,$(MQTT_PACKET_OBJS)) $(patsubst %,$(OBJS_DIR)/%,$(MQTT_TRANSPORT_OBJS))

# flags
CFLAGS=-I$(MQTT_SRC_DIR) -I$(MQTT_SAMPLE_DIR)
LDLIBS=

# macros
ifdef ENABLE_DEBUG
MACROS+= -DENABLE_DEBUG
endif
ifdef ENABLE_SYSLOG
MACROS+= -DENABLE_SYSLOG
endif

# mqttcd objects
MQTTCD_OBJS=$(patsubst %,$(OBJS_DIR)/%,mqttcd.o mqttcd_logger.o mqttcd_arg.o mqttcd_wrapper.o mqttcd_fork.o)

all: $(MQTT_OBJS) $(MQTTCD_OBJS)
	gcc -o mqttcd $^

$(OBJS_DIR)/%.o: $(MQTT_SRC_DIR)/%.c
	gcc -o $@ -c $<

$(OBJS_DIR)/%.o: $(MQTT_SAMPLE_DIR)/%.c
	gcc -o $@ -c $<

$(OBJS_DIR)/%.o: %.c
	gcc -o $@ $(CFLAGS) $(LDLIBS) $(MACROS) -c $<

clean:
	rm -f mqttcd $(OBJS_DIR)/*.o

