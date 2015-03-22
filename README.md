mqttcd
======

[Paho](https://eclipse.org/paho/clients/c/embedded/) based lightweight MQTT client daemon for embedded linux

How to build
------------

```
$ git clone https://github.com/lostman-github/mqttcd
$ cd mqttcd
$ git submodule init
$ git submodule update
$ make
```

if execute make command with `ENABLE_SYSLOG=1` , destination of output is changed to syslog when daemonized.

Command line arguments
----------------------

| argument      | required | description           |
| ------------- | -------- | --------------------- |
| --host        | x        |                       |
| --port        |          | default is 1883.      |
| --version     |          | protocol version of MQTT.<br>3 ([3.1](http://public.dhe.ibm.com/software/dw/webservices/ws-mqtt/mqtt-v3r1.html)) or 4 ([3.1.1](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html)). default is 4. |
| --client_id   |          | if this argument is not specified, hostname is used as client id. |
| --username    |          | default is empty.     |
| --password    |          | default is empty.     |
| --topic       | x        |                       |
| --daemonize   |          | if this argument is not specified, received message is written to current terminal.<br>otherwise, message is written to /var/tmp/mqttcd.log or syslog (if enabled) |
| --handler     |          | handler for received payload.<br>"nop": default behavior. mqttcd does not execute handler.<br>"string": mqttcd pass topic name and received payload to "default". |
| --handler_dir |          | default is /path/to/current/dir/handlers |
