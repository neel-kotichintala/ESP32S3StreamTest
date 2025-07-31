# ESP32-S3 QR Code Demo for Freenove WROOM Board

This is a modified version of the ESP32-S3 QR code recognition demo adapted for the Freenove WROOM board.

## Hardware Requirements

- ESP32-S3 Freenove WROOM board
- Camera module (OV2640 or similar)
- USB-C cable for programming and power

## Camera Pin Connections

Connect your camera module to the ESP32-S3 using the following pin mapping:

| Camera Pin | ESP32-S3 Pin | Description |
|------------|--------------|-------------|
| PWDN       | NC           | Power down (not used) |
| RESET      | NC           | Reset (not used) |
| XCLK       | GPIO 10      | System clock |
| SIOD       | GPIO 40      | I2C data |
| SIOC       | GPIO 39      | I2C clock |
| D7         | GPIO 48      | Data bit 7 |
| D6         | GPIO 11      | Data bit 6 |
| D5         | GPIO 12      | Data bit 5 |
| D4         | GPIO 14      | Data bit 4 |
| D3         | GPIO 16      | Data bit 3 |
| D2         | GPIO 18      | Data bit 2 |
| D1         | GPIO 17      | Data bit 1 |
| D0         | GPIO 15      | Data bit 0 |
| VSYNC      | GPIO 38      | Vertical sync |
| HREF       | GPIO 47      | Horizontal reference |
| PCLK       | GPIO 13      | Pixel clock |
| VCC        | 3.3V         | Power supply |
| GND        | GND          | Ground |

## Building and Flashing

1. Make sure you have ESP-IDF v5.0 or later installed and configured.

2. Navigate to the project directory:
   ```bash
   cd qrcode-demo
   ```

3. Build the project:
   ```bash
   idf.py build
   ```

4. Flash the project to your board:
   ```bash
   idf.py flash
   ```

5. Monitor the output:
   ```bash
   idf.py monitor
   ```

## Expected Output

When the demo is running, you should see output like this in the serial monitor:

```
I (1234) example: Starting QR Code Demo for Freenove WROOM Board
I (2345) example: Camera Init done
I (3456) example: Processing task started
I (4567) example: QR code detection initialized
I (5678) example: QR count: 0   Heap: 8086720  Stack free: 11692  time: 22 ms
```

When a QR code is detected, you'll see:

```
I (64823) example: QR count: 1   Heap: 8086720  Stack free: 11692  time: 229 ms
I (64827) example: Decoded in 3 ms
I (64827) example: QR code: 62 bytes: 'https://www.espressif.com/en/products/devkits/esp-eye/overview'
```

## Troubleshooting

### Camera Not Working
- Check all pin connections
- Verify the camera module is powered (3.3V)
- Make sure the camera module is compatible (OV2640 recommended)

### Build Errors
- Ensure ESP-IDF is properly installed and configured
- Run `idf.py fullclean` and try building again
- Check that all dependencies are installed

### No QR Codes Detected
- Ensure good lighting conditions
- Hold the QR code steady and close to the camera
- Try different QR code sizes
- Check that the QR code is not damaged or partially obscured

## Customization

### Changing Camera Pins
If you need to use different pins for your camera module, modify the pin definitions in `main/qrcode_demo_main.c`:

```c
#define CAM_PIN_XCLK    10  // Change to your desired pin
#define CAM_PIN_SIOD    40  // Change to your desired pin
// ... etc
```

### Adjusting Image Size
To change the camera resolution, modify these definitions:

```c
#define IMG_WIDTH 240   // Change to desired width
#define IMG_HEIGHT 240  // Change to desired height
#define CAM_FRAME_SIZE FRAMESIZE_240X240  // Change to match
```

## License

This project is based on the original ESP32-S3 QR code demo by Espressif Systems and is licensed under Apache 2.0. 