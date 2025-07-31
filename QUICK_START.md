# Quick Start Guide - ESP32-S3 QR Code Demo

## What We've Set Up

✅ **Modified the original ESP32-S3-EYE demo** for your Freenove WROOM board  
✅ **Removed display dependencies** (since your board doesn't have a built-in display)  
✅ **Configured camera pins** for your specific board  
✅ **Added serial console output** for QR code detection results  
✅ **Created test QR code generator** for easy testing  
✅ **Built successfully** - ready to flash!  

## Next Steps

### 1. Hardware Setup
Connect your camera module to the ESP32-S3 using the pin mapping in `README_FREENOVE.md`

### 2. Flash the Demo
```bash
cd qrcode-demo
./flash_and_monitor.sh
```

### 3. Generate Test QR Codes
```bash
pip install -r requirements.txt
python3 generate_test_qr.py
```

### 4. Test the System
- Point the camera at a QR code
- Check the serial monitor for decoded output

## Key Files

- `main/qrcode_demo_main.c` - Main application (modified for your board)
- `README_FREENOVE.md` - Detailed setup instructions
- `SETUP_GUIDE.md` - Comprehensive guide with troubleshooting
- `flash_and_monitor.sh` - Easy flash and monitor script
- `generate_test_qr.py` - Test QR code generator

## Expected Output

When running, you should see:
```
I (1234) example: Starting QR Code Demo for Freenove WROOM Board
I (2345) example: Camera Init done
I (3456) example: Processing task started
I (4567) example: QR code detection initialized
I (5678) example: QR count: 0   Heap: 8086720  Stack free: 11692  time: 22 ms
```

When a QR code is detected:
```
I (64823) example: QR count: 1   Heap: 8086720  Stack free: 11692  time: 229 ms
I (64827) example: Decoded in 3 ms
I (64827) example: QR code: 62 bytes: 'https://www.espressif.com/en/products/devkits/esp-eye/overview'
```

## Need Help?

- Check `SETUP_GUIDE.md` for detailed troubleshooting
- Verify camera connections
- Ensure good lighting for QR code detection
- Test with the generated QR codes first

Good luck with your ESP32-S3 QR code demo! 🚀 