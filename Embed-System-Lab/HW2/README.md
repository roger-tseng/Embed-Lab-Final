# Lab 2: Socket Programming and Data Visualization

In this lab, we use the B-L475E-IOT01 board as a socket client, and send 3D gyrocope & accelerometer data through Wi-fi to our socket server, implemented in Python.

We visualize the received data using the ```mplot3d``` toolkit in ```matplotlib```.

# Usage
1. Using Mbed Studio, import a new program from [here](https://github.com/ARMmbed/mbed-os-example-sockets). Update the Mbed OS version to 6.15.0, and import the required library [(here)](https://os.mbed.com/teams/ST/code/BSP_B-L475E-IOT01/) for the B-L475E-IOT01 board.
2. Replace the ```main.cpp``` file under the ```source``` directory with the file of the same name here.
3. Change the library used for ```printf``` in ```./mbed-os/targets/targets.json``` folder by changing ```printf-lib``` from ```minimal-printf``` to ```std```.
4. Replace the Wi-Fi SSID and password in ```./mbed_app.json```, and connect your computer to the same network.
5. Use the ```ipconfig``` command in terminal to determine the current IP address of your computer. Change the address in line 89 of ```main.cpp``` and line 9 of ```HW2.py``` to your IP address.
6. Run ```HW2.py```. If the program successfully connects to the port at the given address, a message is printed to show that the program is listening.
7. Build and run ```main.cpp```.

