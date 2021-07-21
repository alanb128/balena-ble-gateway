import struct
import time
import os
import paho.mqtt.client as mqtt
from bluepy.btle import UUID,Peripheral

mqtt_address = os.getenv('MQTT_ADDRESS', 'none')
ble_uuid = os.getenv('BLE_UUID', 'none')
ble_address = os.getenv('BLE_ADDRESS', 'none')
mqtt_topic = "HVACanomaly"
mqtt_address = "192.168.1.65"
client = mqtt.Client()
try:
    client.connect(mqtt_address, 1883, 60)
except Exception as e:
    print("Error connecting to mqtt. ({0})".format(str(e)))
    mqtt_address = "none"
else:
    client.loop_start()

temp_uuid = UUID("0aa84a74-d4cd-4a2e-add5-a34c52c8b19c")
p = Peripheral("EC:C0:0A:8A:4D:B5","public")
services = p.getServices()

try:
    ch = p.getCharacteristics(uuid=UUID(0x2101))[0]  # Anomaly
    bh = p.getCharacteristics(uuid=UUID(0x4101))[0]  # Class
    while 1:
        val = ch.read()
        a = struct.unpack_from('?',val)[0]  # Boolean, 1 byte
        if a:
            aa= "ON"
        else:
            aa = "OFF"

        val = bh.read()
        b = struct.unpack_from('b',val)[0]  # signed char, 1 byte
        print("BLE data received from {0}... {1}, {2}".format(ble_address, a, b))
        if mqtt_address != 'none':
            client.publish(mqtt_topic + "/ble_anomaly", aa)
            client.publish(mqtt_topic + "/ble_class", b)
except Exception as e:
    print("Error, conatiner will restart and try again. ({0})".format(str(e)))

try:
    p.disconnect()
except Exception as e:
    print("(No clean disconnect possible.)")
