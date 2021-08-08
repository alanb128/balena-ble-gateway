/* Edge Impulse Arduino examples
 * Copyright (c) 2021 EdgeImpulse Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Includes ---------------------------------------------------------------- */
#include <hvac-motion-classifier_inferencing.h>
#include <Arduino_LSM9DS1.h>
#include <ArduinoBLE.h>

/* Constant defines -------------------------------------------------------- */
#define CONVERT_G_TO_MS2    9.80665f
#define RED 23
#define BLUE 24
#define GREEN 22
#define LED_PWR 25

/* Private variables ------------------------------------------------------- */
static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal


BLEService HVACanomaly("0aa84a74-d4cd-4a2e-add5-a34c52c8b19c");
BLEBooleanCharacteristic AnomalyScore("2101", BLERead | BLENotify);
BLEByteCharacteristic TopClass("4101", BLERead | BLENotify);

/**
* @brief      Arduino setup function
*/
void setup()
{
    // put your setup code here, to run once:
    //Serial.begin(115200);
    //Serial.println("Edge Impulse Inferencing Demo");
    Serial.begin(9600);
    delay(2500);
    //while (!Serial);
        //pinMode(LED_BUILTIN, OUTPUT);
        //if (!BLE.begin()) {
            //Serial.println("starting BLE failed!");
            //while (1);
        //}

    if (!BLE.begin()) {
        Serial.println("starting BLE failed!");
    }
    else {
        digitalWrite(BLUE, HIGH);
        delay(1500);
        digitalWrite(BLUE, LOW);
    }

    BLE.setLocalName("HVACMonitor");
    BLE.setAdvertisedService(HVACanomaly);
    HVACanomaly.addCharacteristic(AnomalyScore);
    HVACanomaly.addCharacteristic(TopClass);
    BLE.addService(HVACanomaly);

    BLE.advertise();
    Serial.println("Bluetooth device active, waiting for connections...");

    pinMode(RED, OUTPUT);
    pinMode(BLUE, OUTPUT);
    pinMode(GREEN, OUTPUT);
    
    if (!IMU.begin()) {
        ei_printf("Failed to initialize IMU!\r\n");
    }
    else {
        ei_printf("IMU initialized\r\n");
    }

    if (EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != 3) {
        ei_printf("ERR: EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME should be equal to 3 (the 3 sensor axes)\n");
        return;
    }
}

/**
* @brief      Printf function uses vsnprintf and output using Arduino Serial
*
* @param[in]  format     Variable argument list
*/
void ei_printf(const char *format, ...) {
   static char print_buf[1024] = { 0 };

   va_list args;
   va_start(args, format);
   int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
   va_end(args);

   if (r > 0) {
       Serial.write(print_buf);
   }
}

/**
* @brief      Get data and run inferencing
*
* @param[in]  debug  Get debug info if true
*/
void loop()
{
    Serial.print(BLE.address());
    
    ei_printf("\nStarting inferencing in 2 seconds...\n");
    
    delay(2000);

    ei_printf("Sampling...\n");

    // Allocate a buffer here for the values we'll read from the IMU
    float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0 };

    for (size_t ix = 0; ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; ix += 3) {
        // Determine the next tick (and then sleep later)
        uint64_t next_tick = micros() + (EI_CLASSIFIER_INTERVAL_MS * 1000);

        IMU.readAcceleration(buffer[ix], buffer[ix + 1], buffer[ix + 2]);

        buffer[ix + 0] *= CONVERT_G_TO_MS2;
        buffer[ix + 1] *= CONVERT_G_TO_MS2;
        buffer[ix + 2] *= CONVERT_G_TO_MS2;

        delayMicroseconds(next_tick - micros());
    }

    // Turn the raw buffer in a signal which we can the classify
    signal_t signal;
    int err = numpy::signal_from_buffer(buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    if (err != 0) {
        ei_printf("Failed to create signal from buffer (%d)\n", err);
        return;
    }

    // Run the classifier
    ei_impulse_result_t result = { 0 };

    err = run_classifier(&signal, &result, debug_nn);
    if (err != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", err);
        return;
    }

    // print the predictions
    ei_printf("Predictions ");
    ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
        result.timing.dsp, result.timing.classification, result.timing.anomaly);
    ei_printf(": \n");
    
    int topix = 0;
    float topclass = result.classification[topix].value;
    
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
        if (result.classification[ix].value > topclass) {
            topclass = result.classification[ix].value;
            topix = ix; 
        }
    }

    Serial.print(result.classification[topix].label);
    float anomaly = 0;
    boolean anomalybool = false;
    
#if EI_CLASSIFIER_HAS_ANOMALY == 1
    ei_printf("    anomaly score: %.3f\n", result.anomaly);
    anomaly = result.anomaly;
    if (anomaly > 0.3) {
       anomalybool = true;
       digitalWrite(GREEN, LOW); 
       digitalWrite(RED, HIGH); 
    }  else {
       anomalybool = false;
       digitalWrite(GREEN, HIGH);
       digitalWrite(RED, LOW); 
    }
#endif

   
    // send values via BLE
    BLEDevice central = BLE.central();
    if (central.connected()) {
        //int batteryLevel = 57;
        //int batteryLevel = map(battery, 0, 1023, 0, 100);
        //Serial.print("Battery Level % is now: ");
        //Serial.println(batteryLevel);
        Serial.print("Sending BLE data...");
        AnomalyScore.writeValue(anomalybool);
        TopClass.writeValue(topix);

    }
    
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_ACCELEROMETER
#error "Invalid model for current sensor"
#endif
