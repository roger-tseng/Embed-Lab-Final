# Lab 3: BLE programming - Python/C/C++ BLE central 

In this lab, we connect to a GATT peripheral hosted on an Android phone with a Bluetooth Low Energy (BLE) central on Raspberry Pi. The BLE central is implemented in Python using the ```bluepy``` library.

We demonstrate read/write capabilities on the value of a specific characteristic, and our code can easily be extended to connect to different services/characteristics.

# Usage
1. With an USB-TTL serial cable, you can connect your host computer to your Raspberry Pi device and use ```minicom``` to see boot and console messages. Alternatively, we connected our device to a monitor, keyboard, and mouse for direct development. 
2. Download ```rpi.py``` to your Raspberry Pi device.
3. Install the ```bluepy``` Python library with the following commands on target terminal. 
```bash
sudo apt install python-pip
sudo apt install libglib2.0-dev
sudo pip3 install bluepy
```
4. On an Android phone, install [BLE Tool](https://play.google.com/store/apps/details?id=com.lapis_semi.bleapp&hl=zh_TW&gl=US). 
After installing, open the application, choose "GATT server", and click "Start Advertising". In "Show Advertiser Settings", check the complete local name of your GATT server. This name will be useful for finding your address later.
5. Execute ```sudo python3 rpi.py```.