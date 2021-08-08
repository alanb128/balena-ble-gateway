# balena BLE Gateway
Use a Raspberry Pi as a BLE gateway to receive data from an Arduino running Edge Impulse. This example uses the [Continuous Motion Recognition project](https://docs.edgeimpulse.com/docs/continuous-motion-recognition) but could easily be adapted to others.

## Getting started

### Hardware required
- An [Arduino Nano 33 BLE Sense](https://store.arduino.cc/usa/nano-33-ble-sense) - or any microcontroller that has BLE and is fully supported by Edge Impulse
- A Raspberry Pi that supports BLE: Model 3 A+, 3 B, 3 B+, 4 B, 400, CM4, Zero W, Zero WH.

### Software required
- A free [Edge Impulse](https://www.edgeimpulse.com/) account
- A free [balenaCloud](https://dashboard.balena-cloud.com/signup) account

### 1. Build your Arduino project
Follow the [Continuous Motion Recognition project](https://docs.edgeimpulse.com/docs/continuous-motion-recognition) directions to set up your microcontroller on Edge Impulse, collect data, build your model and then test it on your microcontroller. Be sure to include anomaly detection in your model.

Once you are satisfied with the performace of your model, follow the directions to [deploy your impulse](https://docs.edgeimpulse.com/docs/running-your-impulse-arduino) locally on your Arduino. Once you've done this, open the File > Examples > (Your project name) > nano_ble33_sense_accelerometer sketch. This is the sketch that takes a reading from the onboard accelerometer every two seconds and performs an inference which you can observe using the serial monitor. This is the sketch we'll be modifying to send data using BLE.

Note the project name in the very first `#include` line in your sketch, which will look something like this:
```
#include <my_project_name.h>
```
Download the `nano_ble33_sense_motion.ino` sketch fom this repository and replace `your-project-name` in the first `#include` with the name noted in your include file above. Now upload this sketch to your Arduino and confirm that it runs. Using the Arduino IDE serial monitor, you should see some output such as this:

```
Insert Here
```

Get the address...

The additional code creates a new BLE service and two characteristics: AnomalyScore and TopClass. AnomalyScore has a UUID of `2101` and is a boolean value inidcating if the AnomalyScore is above the value of 0.3. TopClass has a UUID of `4101` and is an integer representing the highest scoring class from the most recent inference. You can change these UUIDs in the code, but they must match the UUIDs in the Python code (`ble.py`) on the gateway device used to read the data and convert it to MQTT.

### 2. Create a new fleet in balenaCloud
Once you have signed up for an account, you can use the deploy button below to create a new fleet (a fleet can be one or more devices) and then download an OS image which you can burn to an SD card and insert into your Raspberry Pi. When you power on the Pi, it will download the project from this repo. Alternatively, you can clone this repository, (and modify it locally if you wish) and the use the [balena CLI](https://www.balena.io/docs/reference/balena-cli/) to push the code to your device. For more information about using the balena platform, check out the [Getting Started](https://www.balena.io/docs/learn/getting-started/raspberrypi3/python/) guide.

[![balena deploy button](https://www.balena.io/deploy.svg)](https://dashboard.balena-cloud.com/deploy?repoUrl=https://github.com/alanb128/balena-ble-gateway)

When your Raspberry Pi is online and the code is running, power up your Arduino (it does not need to be connected to a computer!)

You'll need to set a few [environment variables](https://www.balena.io/docs/learn/manage/serv-vars/) in order for your Raspberry Pi to communicate with your Arduino over BLE:

`BLE_UUID` - the 128 bit UUID for our custom BLE service, defaults to the value `0aa84a74-d4cd-4a2e-add5-a34c52c8b19c` in our Arduino sketch. You can leave it as-is or generate your own, using an online tool [such as this](https://www.guidgenerator.com/online-guid-generator.aspx). (If you use a different UUID, it must match in both the Arduino sketch and the balenaCloud environment variable.

`BLE_ADDRESS` - 

`MQTT_ADDRESS` - the address of the MQTT broker you want to send the data to.

## Going further
