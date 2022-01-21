# Lab 6: DHT11 one-wire serial protocol and device drivers

In this lab, we connect a DHT11 sensor to a Raspberry Pi device, and read temperature and humidity values of our surroundings. 

Using ZeroPlus Logic Cube Analyzer, we also observe how it transmits data with its one-wire two-way signal protocol.

# Usage
1. Connect the DHT11 sensor to a Raspberry Pi device.
2. Install required software for ZeroPlus Logic Cube Analyzer by following instructions given [here](https://www.youtube.com/watch?v=gULjmHH_nKE)
3. Use a breadboard to connect two wires (data and ground) from the logic analyzer to data and ground channel of the DHT11 sensor and Raspberry Pi.
4. Execute the following commands in Raspberry Pi terminal:
```bash
sudo apt-get update
sudo apt-get install build-essential python-dev
git clone https://github.com/adafruit/Adafruit_Python_DHT.git 
cd Adafruit_Python_DHT
sudo python setup.py install
cd examples
```
5. Execute ```sudo ./AdafruitDHT.py 11 4``` in Raspberry Pi terminal, and observe the generated waveform in the logic analyzer.