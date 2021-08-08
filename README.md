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

Once you are satisfied with the performace of your model, follow the directions to [deploy your impulse](https://docs.edgeimpulse.com/docs/running-your-impulse-arduino) locally on your Arduino. This means that you will be using the Arduino IDE to load your impulse as an Arduino library. It will run on your microcontroller upon bootup. Confirm that your impulse runs properly on the Arduino before proceeding to the next section.



### 2. Create a new fleet in balenaCloud
Once you have signed up for an account, you can use the deploy button below to create a new fleet (a fleet can be one or more devices) and then download an OS image which you can burn to an SD card and insert into your Raspberry Pi. When you power on the Pi, it will download the project in this repo. Alternatively, you can clone this repository, (and modify it locally if you wish) and the use the [balena CLI](https://www.balena.io/docs/reference/balena-cli/) to push the code to your device.


## Going further
