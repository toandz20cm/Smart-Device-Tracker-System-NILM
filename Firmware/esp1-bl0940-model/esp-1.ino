#include "BL0940.h"
#include "model-7tb.h"
#include <tflm_esp32.h>
#include <eloquent_tinyml.h>
#include <HardwareSerial.h>
#include <scaler_data7tb.h>

#define ARENA_SIZE 1800*1024
#define PF_SCALE 0.01f
#define SERIAL_BAUD 115200
#define RX_PIN 16
#define TX_PIN 17
#define INPUT_SIZE 5

// Global objects
BL0940 bl0940;
Eloquent::TF::Sequential<TF_NUM_OPS, ARENA_SIZE> tf;
HardwareSerial SerialPort(2);

// Polynomial coefficients for PF correction
const float PF_COEFFS[] = {-0.02182f, 5.882f, -525.8f, 15670.0f};

// Function prototypes
float fixPF(float x);
void normalize(float input[], float output[]);
void setupTensorFlow();
void processAndPredict();
float round2(float value);

void setup() {
    Serial.begin(SERIAL_BAUD);
    SerialPort.begin(SERIAL_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);
    bl0940.begin();
    setupTensorFlow();
}

void loop() {
    processAndPredict();
    delay(5000);
}

float round2(float value) {
    return ((int)(value * 100 + 0.5f)) / 100.0f;
}

// Calculate power factor correction using polynomial
float fixPF(float x) {
    return PF_COEFFS[0] * x * x * x + 
           PF_COEFFS[1] * x * x + 
           PF_COEFFS[2] * x + 
           PF_COEFFS[3];
}

// Normalize input data
void normalize(float input[], float output[]) {
    for (uint8_t i = 0; i < INPUT_SIZE; i++) {
        output[i] = (input[i] - mean_values[i]) / scale_values[i];
    }
}

// Setup TensorFlow model
void setupTensorFlow() {
    tf.setNumInputs(INPUT_SIZE);
    tf.setNumOutputs(128);
    
    tf.resolver.AddReshape();
    tf.resolver.AddFullyConnected();
    tf.resolver.AddRelu();
    tf.resolver.AddSoftmax();
    
    while (!tf.begin(model).isOk()) {
        Serial.println(tf.exception.toString());
        delay(1000);
    }
}

// Process measurements and make predictions
void processAndPredict() {
    float v, i, pf;
    
    // Get measurements
    bl0940.getVoltage(&v);
    bl0940.getCurrent(&i);
    bl0940.getPowerFactor(&pf);
    
    // Calculate electrical parameters
    pf = fixPF(pf);
    float p = i * v * pf;
    float s = i * v;
    float q = i * v * sqrtf(1.0f - (pf * PF_SCALE) * (pf * PF_SCALE));

    v = round2(v);
    i = round2(i);
    pf = round2(pf);
    p = round2(p);
    q = round2(q);
    s = round2(s);
    
    float input[INPUT_SIZE] = {i, p, pf, s, q};
    float scaled_input[INPUT_SIZE];
    normalize(input, scaled_input);
    
    // Make prediction
    if (tf.predict(scaled_input).isOk()) {
        int predicted_class = tf.classification;
        String message = String(predicted_class) + "," + String(p);
        
        SerialPort.println(message);
        Serial.println("Sent: " + message);
        Serial.print(F("Predicted class: "));
        Serial.println(predicted_class);
    } else {
        Serial.println(tf.exception.toString());
    }
}