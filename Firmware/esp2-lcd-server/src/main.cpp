#include <Arduino.h>
#include "D:\Lab\NILM\LCD_NILM\lib\model-7tb.h"
// #include <tflm_esp32.h>
#include <eloquent_tinyml.h>
// #include <scaler_data7tb.h>
// #include"D:\Lab\NILM\LCD_NILM\lib\eloquent_tinyml.h"
#include"D:\Lab\NILM\LCD_NILM\lib\tflm_esp32.h"
#include"D:\Lab\NILM\LCD_NILM\lib\scaler_data7tb.h"


#define ARENA_SIZE 80000
#define INPUT_SIZE 5

Eloquent::TF::Sequential<TF_NUM_OPS, ARENA_SIZE> tf;

void normalize(float input[], float output[]) {
    for (uint8_t i = 0; i < INPUT_SIZE; i++) {
        output[i] = (input[i] - mean_values[i]) / scale_values[i];
    }
}

void setupTensorFlow() {
    tf.setNumInputs(INPUT_SIZE);
    tf.setNumOutputs(128);
    
    tf.resolver.AddFullyConnected();
    tf.resolver.AddSoftmax();
    
    while (!tf.begin(model).isOk()) {
        Serial.println("TF Error: " + String(tf.exception.toString()));
        delay(1000);
    }
    Serial.println("TensorFlow initialization successful!");
}

void runPrediction(float input[], const char* name) {
    float scaled_input[INPUT_SIZE];
    normalize(input, scaled_input);
    
    Serial.print("\nTesting ");
    Serial.println(name);
    Serial.println("Raw inputs: ");
    for (int i = 0; i < INPUT_SIZE; i++) {
        Serial.print(input[i]);
        Serial.print(" ");
    }
    Serial.println("\nNormalized inputs: ");
    for (int i = 0; i < INPUT_SIZE; i++) {
        Serial.print(scaled_input[i]);
        Serial.print(" ");
    }
    
    if (tf.predict(scaled_input).isOk()) {
        Serial.print("\nPredicted class: ");
        Serial.println(tf.classification);
    } else {
        Serial.println("\nPrediction Error: " + String(tf.exception.toString()));
    }
    Serial.println("------------------------");
}

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(100);
    
    Serial.println("Starting model test...");
    setupTensorFlow();
    
    
    Serial.println("Test complete!");
}

void loop() {
    runPrediction(x111, "x111");
    runPrediction(x64, "x64");
    runPrediction(x39, "x39");
    runPrediction(x74, "x74");
    runPrediction(x31, "x31");
    runPrediction(x102, "x102");
    runPrediction(x100, "x100");
    runPrediction(x54, "x54");
    // Nothing to do in loop
    delay(1000);
}