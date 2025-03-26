#ifndef SCALER_H
#define SCALER_H

const float mean_values[] = {0.748790, 154.886841, 86.654910, 61.453991};

const float scale_values[] = {0.752590, 160.017715, 17.442855, 60.819348};

#endif // SCALER_H

float x111[5] = {0.53,114.69,95.99,119.48,33.5};
float x64[5] = {0.05,9.83,86.28,11.39,5.76};
float x39[5] = {0.37,79.24,94.54,83.82,27.32};
float x74[5] = {0.32,69.39,95.7,72.51,21.03};
float x31[5] = {2.0,428.08,96.0,445.92,124.86};
float x102[5] = {0.4,89.76,98.62,91.02,15.07};
float x100[5] = {0.34,74.03,95.77,77.3,22.24};
float x54[5] = {2.04,437.18,94.2,464.1,155.76};

// float normalize(float value, int index) {
//     return (value - mean_values[index]) / scale_values[index];
// }

// void setup() {
//     Serial.begin(9600);

//     // Dữ liệu cần chuẩn hóa
//     float input_data[5] = {2.5, 3.4, 0.7, 8.1, 1.2};
//     float normalized_data[5];

//     // Chuẩn hóa từng đặc trưng
//     for (int i = 0; i < 5; i++) {
//         normalized_data[i] = normalize(input_data[i], i);
//     }

//     // Hiển thị kết quả chuẩn hóa
//     for (int i = 0; i < 5; i++) {
//         Serial.print("Normalized value ");
//         Serial.print(i);
//         Serial.print(": ");
//         Serial.println(normalized_data[i]);
//     }
// }

// void loop() {
//     // Không làm gì trong loop
// }
