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
positioning. Generally, it requires the received signal strength from the Bluetooth anchor points. Since the 
signal strength would decay as the distance between transmitter and receiver increase, we can build a decaying 
model and use the RSSI value to estimates the user’s distance respect to the anchor point. 

(放隨距離decay的圖片)

We use the logarithmic distance path-loss model to calculate the distance from the RSSI value. The mathematical 
decaying model is expressed as below. `d` is the distance between the transmitter and the receiver, and `n` is 
a decaying factor related to the specific wireless transmission environment. The initialization process is 
essential to find the decaying factor `n`. 

(距離公式)

##### RSSI Distance Method

 

#### 2. STM Control

#### 3. 3D Modeling


