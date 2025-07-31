# ESP32-S3 QR Code Demo Setup Guide for Freenove WROOM Board

This guide will help you set up and run the ESP32-S3 QR code recognition demo on your Freenove WROOM board.

## Prerequisites

1. **ESP-IDF v5.0 or later** - Already installed on your system
2. **ESP32-S3 Freenove WROOM board**
3. **Camera module** (OV2640 or similar)
4. **USB-C cable** for programming and power
5. **Breadboard and jumper wires** for camera connections

## Hardware Setup

### Camera Module Connections

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

### Connection Tips

1. **Power**: Make sure the camera module gets 3.3V power, not 5V
2. **Ground**: Connect all ground pins together
3. **Signal integrity**: Keep wires as short as possible
4. **Pull-up resistors**: Some camera modules may need pull-up resistors on I2C lines (SDA/SDL)

## Software Setup

### 1. Set up ESP-IDF Environment

```bash
source ~/esp/v5.5/esp-idf/export.sh
```

### 2. Navigate to Project Directory

```bash
cd qrcode-demo
```

### 3. Build the Project

```bash
idf.py build
```

### 4. Flash to ESP32-S3

```bash
idf.py flash
```

### 5. Monitor Output

```bash
idf.py monitor
```

Or use the provided script:

```bash
./flash_and_monitor.sh
```

## Testing the System

### Generate Test QR Codes

1. Install Python dependencies:
   ```bash
   pip install -r requirements.txt
   ```

2. Generate test QR codes:
   ```bash
   python3 generate_test_qr.py
   ```

3. This will create a `test_qr_codes` directory with various test QR codes.

### Test Procedure

1. **Flash the ESP32-S3** with the QR code demo
2. **Connect the camera module** according to the pin mapping above
3. **Power up the ESP32-S3** via USB
4. **Open the serial monitor** to see the output
5. **Display a QR code** on your phone/computer screen
6. **Point the camera** at the QR code
7. **Check the serial output** for decoded results

## Expected Output

### Normal Operation
```
I (1234) example: Starting QR Code Demo for Freenove WROOM Board
I (2345) example: Camera Init done
I (3456) example: Processing task started
I (4567) example: QR code detection initialized
I (5678) example: QR count: 0   Heap: 8086720  Stack free: 11692  time: 22 ms
```

### QR Code Detected
```
I (64823) example: QR count: 1   Heap: 8086720  Stack free: 11692  time: 229 ms
I (64827) example: Decoded in 3 ms
I (64827) example: QR code: 62 bytes: 'https://www.espressif.com/en/products/devkits/esp-eye/overview'
```

## Troubleshooting

### Camera Not Working

1. **Check connections**: Verify all pins are connected correctly
2. **Power supply**: Ensure camera gets 3.3V, not 5V
3. **I2C issues**: Try adding 4.7kΩ pull-up resistors to SDA and SCL lines
4. **Pin conflicts**: Make sure no other devices are using the same pins

### Build Errors

1. **ESP-IDF not set up**: Run `source ~/esp/v5.5/esp-idf/export.sh`
2. **Dependencies missing**: Run `idf.py reconfigure`
3. **Clean build**: Run `idf.py fullclean` then `idf.py build`

### No QR Codes Detected

1. **Lighting**: Ensure good, even lighting
2. **Distance**: Hold QR code 10-30cm from camera
3. **Focus**: Make sure QR code is in focus
4. **Size**: QR code should be large enough (at least 2cm square)
5. **Quality**: Use high-quality, undamaged QR codes

### Serial Monitor Issues

1. **Port not found**: Check USB connection and port settings
2. **Baud rate**: Default is 115200 baud
3. **Driver issues**: Install appropriate USB-to-serial drivers

## Customization

### Changing Camera Pins

If you need to use different pins, edit `main/qrcode_demo_main.c`:

```c
#define CAM_PIN_XCLK    10  // Change to your desired pin
#define CAM_PIN_SIOD    40  // Change to your desired pin
// ... etc
```

### Adjusting Image Size

To change resolution, modify these definitions:

```c
#define IMG_WIDTH 240   // Change to desired width
#define IMG_HEIGHT 240  // Change to desired height
#define CAM_FRAME_SIZE FRAMESIZE_240X240  // Change to match
```

### Performance Tuning

1. **Frame rate**: Adjust the delay in the main loop
2. **Memory**: Monitor heap usage in the output
3. **CPU frequency**: Can be adjusted in sdkconfig

## Advanced Features

### Adding WiFi Support

You can extend the demo to send QR code data over WiFi:

1. Add WiFi initialization code
2. Send decoded data to a server
3. Implement MQTT or HTTP client

### Adding Display Support

If you add a display module:

1. Initialize the display driver
2. Show camera preview
3. Display decoded QR code content

### Adding SD Card Support

For storing QR code data:

1. Initialize SD card
2. Log decoded data to files
3. Store configuration

## Support

If you encounter issues:

1. Check the troubleshooting section above
2. Verify all connections
3. Test with known good QR codes
4. Check ESP-IDF documentation
5. Review the original demo documentation

## License

This project is based on the original ESP32-S3 QR code demo by Espressif Systems and is licensed under Apache 2.0. 