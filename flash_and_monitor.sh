#!/bin/bash

# ESP32-S3 QR Code Demo Flash and Monitor Script
# For Freenove WROOM Board

echo "ESP32-S3 QR Code Demo - Flash and Monitor"
echo "=========================================="

# Check if ESP-IDF is set up
if ! command -v idf.py &> /dev/null; then
    echo "Error: ESP-IDF not found. Please run:"
    echo "source ~/esp/v5.5/esp-idf/export.sh"
    exit 1
fi

# Build the project
echo "Building project..."
idf.py build

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Build successful!"

# Flash the project
echo "Flashing to ESP32-S3..."
idf.py flash

if [ $? -ne 0 ]; then
    echo "Flash failed!"
    exit 1
fi

echo "Flash successful!"

# Monitor the output
echo "Starting monitor (press Ctrl+] to exit)..."
echo "=========================================="
echo "Expected output:"
echo "- Camera initialization"
echo "- QR code detection running"
echo "- When QR codes are detected, you'll see the decoded content"
echo "=========================================="

idf.py monitor 