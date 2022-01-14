## Embedded System Lab - 夢想家園

### I. Author
曾元 <br>
林佩潁 B07901102 <br>
邵家澤 <br>

### II. Demo slides and video
https://docs.google.com/presentation/d/1oxG7fQ4YjdOL_rXL2Y6k44hEikNMTCKHO84AEx_ngOA/edit?usp=sharing

### III. Abstract
We use STM32 and Rpi to create an interactive way for interior design. By setting up RSSI network, we 
can do the triangulation and find the user's location. The user can press the button on the STM32 board 
to trigger IRQ function for setting furniture. The STM32 board also supports accelerometer and magnetometer 
for motion and heading detection, which allows user to switch object and change facing direction. The user 
can walk around the room and place the furniture at anywhere he/she wants. This project is demonstrated on 
MineCraft engine.

### IV. Motivation
It's not a easy task for people to design the interior decoration. Traditional interior design model are 
usually accessed by mouses and keyboards. All the furnitures must be set up on the computer remotely, which 
may be less straightforward.... Thus, we want to provide a more interactive and straightforward way, so that 
the user can directly walk inside the new house and place the furniture at their wish. 

### V. Contents

#### 1. Interior Positioning 

##### Indoor positioning
Compared with outdoor localization, the difficulty of indoor localization lies in the higher precision 
requirement since we need to differentiate two points with small distance, which make it impractical to use 
GPS for localization. After surveying several papers, we adopt the RSSI distance method for our positioning.

##### RSSI Distance Method
The received signal strength indication (RSSI) distance method is one of the common choices for interior 
positioning. The RSSI value represents the power of a received radio signal. The higher the RSSI value, 
the higher the signal strength. Moreover, since no additional sensors are required to measure RSSI values. 
It becomes our first choice.

RSSI distance method requires the received signal strength from the Bluetooth anchor points. The signal 
strength would decay as the distance between transmitter and receiver increase. Thus, we can build a decaying 
model and use the RSSI value to estimates the user’s distance respect to the anchor point. 

(放隨距離decay的圖片)

We use the mainstream logarithmic distance path-loss model to calculate the distance from the RSSI value. 
The mathematical decaying model is expressed as below. `d` is the distance between the transmitter and 
the receiver, and `n` is a decaying factor related to the specific wireless transmission environment. The 
initialization process is essential to find the decaying factor `n`. 

(距離公式)

##### RSSI Variation
Our model assumes that the RSSI value is only dependent on the distance between the two devices. However, 
in reality, RSSI values are significantly influenced by the environment and noise caused by multi-path 
fading. There exist multiple reflection paths for the signal to transmit from the transmitter to the 
receiver, which may cause some interference so the actual received strength might be quite different from 
the ideal model. As a result, the variation of the received RSSI are unstable even in a well-controlled 
indoor scenario. Thus, some filter and post-processing is needed.

##### Kalman Filter
We adopt Kalman filter on both received RSSI value and the calculated distance to remove some noise and 
eliminate the large variation. The Kalman filter is a state estimator that makes an estimate based on 
noisy measurements. The key is that it takes the history values as well as the uncertainty of measurements 
into account. 

(機率圖 大K小K)

##### Triangulation
After we obtain the distance from each AP with known location, we can do the triangulation to find the 
coordinate of the user. We use `scipy.optimize` library to find the optimized coordinate and minimize 
the error.

(放MATLAB圖)

##### MQTT
We use MQTT for the communication between Rpi and the server. MQTT is a publish-subscribe network 
protocol that transports messages between devices, which is a great choice for communication between 
different IOT sensors. 

Rpi act as MQTT publisher and the server act as MQTT subscriber. The benefit of using MQTT is that the 
message is organized in a hierarchy of topics. We can specify the topic for the publisher and subscriber. 
In other words, it allows us to differentiate the message from different Rpi simply by the topic. The 
subscriber can subscribe to multiple topic at the same time, and the publisher can publish their message 
no matter the subscriber exist or not. This allows a great flexibility for our setting. 

(放MQTT圖)

##### Localization Procedure
In conclusion, we set four Rpi at each corner of the room. Each AP would publish time stamp and the 
calculated distance to the server every four seconds. When the server receives four distances with the 
same time stamp, it would do triangulation and find the coordinate. 

In other words, the syncronization among four Rpi are achieved by only send the message when the time 
stamp is the multiples of 4. So that the server can check the time stamp to find the corresponding 
distance pair. 

Moreover, the average window of three values is used for post-processing on calculated result.

##### Path Following Result

 

#### 2. STM Control
(資料處理)

#### 3. 3D Modeling
(port 解法)

### Reference
[1] Chai, Song & An, Renbo & Du, Zhengzhong. (2016). "An Indoor Positioning Algorithm using Bluetooth Low Energy RSSI." 10.2991/amsee-16.2016.72. 
[2] Kalman Filter: https://www.kalmanfilter.net/kalman1d.html



