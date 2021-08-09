# balena BLE Gateway
Use a Raspberry Pi as a BLE gateway to receive data from an Arduino running Edge Impulse. This example uses the [Continuous Motion Recognition project](https://docs.edgeimpulse.com/docs/continuous-motion-recognition) but could easily be adapted to others.

![](https://raw.githubusercontent.com/alanb128/balena-ble-gateway/main/arduino-hvac.jpg)

In my use case, I attached the Arduino to an HVAC unit and trained it to recognize four different vibration patterns under normal operation. An inference runs every two seconds, and the result of that inference is sent via BLE to a Raspberry Pi running balenaOS. The code running on the Pi then converts that data to MQTT so it can be used by other applications, such as Home Assistant.

## Getting started

### Hardware required
- An [Arduino Nano 33 BLE Sense](https://store.arduino.cc/usa/nano-33-ble-sense) - or any microcontroller that has BLE and is fully supported by Edge Impulse
- A Raspberry Pi that supports BLE: Model 3 A+, 3 B, 3 B+, 4 B, 400, CM4, Zero W, Zero WH.

### Software required
- A free [Edge Impulse](https://www.edgeimpulse.com/) account
- A free [balenaCloud](https://dashboard.balena-cloud.com/signup) account

### 1. Build your Arduino project
Follow the [Continuous Motion Recognition project](https://docs.edgeimpulse.com/docs/continuous-motion-recognition) instructions to set up your microcontroller on Edge Impulse, collect data, build your model and then test it on your microcontroller. Be sure to include anomaly detection in your model.

Once you are satisfied with the performace of your model, follow the directions to [deploy your impulse](https://docs.edgeimpulse.com/docs/running-your-impulse-arduino) locally on your Arduino. Once you've done this, open the File > Examples > (Your project name) > nano_ble33_sense_accelerometer sketch in the Arduino IDE. This is the sketch that takes a reading from the onboard accelerometer every two seconds and performs an inference which you can observe using the serial monitor. This is the sketch we'll be modifying to send data using BLE.

Note the project name in the very first `#include` line in your sketch, which will look something like this:
```
#include <my_project_name.h>
```
Download the `nano_ble33_sense_motion.ino` sketch fom this repository and replace `your-project-name` in the first `#include` with the name noted in your include file above. Now upload this sketch to your Arduino and confirm that it runs. Using the Arduino IDE serial monitor, you should see some output such as this:

```
Sampling...
Predictions (DSP: 24 ms., Classification: 1 ms., Anomaly: 5 ms.):
    idle: 0.16797
    running: 0.83203
    running2: 0.00000
    startup: 0.00000
running    anomaly score: 0.464
Sending BLE data...8d:e3:fd:54:f8:69
Starting inferencing in 2 seconds...

```

Note the address after the "Sending BLE data..." text (`8d:e3:fd:54:f8:69`) in the above example. You'll need this address for one of the steps below.

The additional code creates a new BLE service and two characteristics: AnomalyScore and TopClass. AnomalyScore has a UUID of `2101` and is a boolean value that indicates if the AnomalyScore is above the value of 0.3. TopClass has a UUID of `4101` and is an integer representing the highest scoring class from the most recent inference. You can change these UUIDs in the code, but they must match the UUIDs in the Python code (`ble.py`) on the gateway device used to read the data and convert it to MQTT.

### 2. Create a new fleet in balenaCloud
Once you have signed up for an account, you can use the deploy button below to create a new fleet (a fleet can be one or more devices) and then download an OS image which you can burn to an SD card and insert into your Raspberry Pi. When you power on the Pi, it will download the project from this repo. Alternatively, you can clone this repository, (and modify it locally if you wish) and the use the [balena CLI](https://www.balena.io/docs/reference/balena-cli/) to push the code to your device. For more information about using the balena platform, check out the [Getting Started](https://www.balena.io/docs/learn/getting-started/raspberrypi3/python/) guide.

[![balena deploy button](https://www.balena.io/deploy.svg)](https://dashboard.balena-cloud.com/deploy?repoUrl=https://github.com/alanb128/balena-ble-gateway)

When your Raspberry Pi is online and the code is running, power up your Arduino (it does not need to be connected to a computer!) and place the devices within a few feet of each other. Your useful distance may vary, but about 10 feet was my range. (Make sure they are at least a foot apart!)

You'll need to set a few [environment variables](https://www.balena.io/docs/learn/manage/serv-vars/) in order for your Raspberry Pi to communicate with your Arduino over BLE:

`BLE_UUID` - the 128 bit UUID for our custom BLE service, defaults to the value `0aa84a74-d4cd-4a2e-add5-a34c52c8b19c` in our Arduino sketch. You can leave it as-is or generate your own, using an online tool [such as this](https://www.guidgenerator.com/online-guid-generator.aspx). (If you use a different UUID, it must match in both the Arduino sketch and the balenaCloud environment variable.

`BLE_ADDRESS` - This is the unique address for your Arduino. You can obtain it from the step outlined in #1 above. It's printed to the serial console in the Arduino IDE which you can view when the Arduino is still connected to your computer right after upload.

`MQTT_ADDRESS` - the address of the MQTT broker you want to send the data to. The MQTT topic is hard-coded to `HVACanomaly` in the `ble.py` code but you can change it to suit your needs. It is named `HVACanomaly` due to the original purpose of this project, which is outlined here. (coming soon!)

If all is running well, you should see the Arduino data being received in the logs on the balenaCloud dashboard:
![](https://raw.githubusercontent.com/alanb128/balena-ble-gateway/main/logs.png)
The first value displayed after the device address is AnomalyScore, followed by TopClass.

## Going further
This project is merely a simple example of how data can be transferred from a microcontroller to another device via BLE. A useful modification would be to allow the Raspberry Pi to receive data from multiple microcontrollers simultaneously. Another useful feature would be for the Pi to automatically discover any BLE signals currently in range. If you decide to make these or other improvements, PRs are welcome!
