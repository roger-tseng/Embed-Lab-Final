import socket
import json
import time
import random
import time 
import math

import paho.mqtt.client as mqtt
from scipy.optimize import minimize
import numpy as np

# Socket Settings
HOST = '192.168.141.220'    # your IP address
PORT = 65432                # Port to listen for STM32 data (use ports > 1023)

from mcpi_e.minecraft import Minecraft
from mcpi_e import block

# Anchor point locations
ap_locations_list = [[0.1,0], [4,0], [0,2.5], [4,2.5]]

# Post-processing cutoff coordinates
max_x = max(ap_locations_list[1][0], ap_locations_list[3][0]) + 1
min_x = min(ap_locations_list[0][0], ap_locations_list[2][0]) - 1
max_y = max(ap_locations_list[2][1], ap_locations_list[3][1]) + 0.5
min_y = min(ap_locations_list[0][1], ap_locations_list[1][1]) - 0.5

# Minecraft Settings
block_ids = [ block.STONE.id,       \
              block.WOOD_PLANKS.id, \
              block.WOOL.id,        \
              block.ICE.id ]
ID = 0                                  # current block index
ip   =   "127.0.0.1"                    # Minecraft server IP (localhost here)
port =   4711                           # RaspberryJuice port (default is 4711)
name =   "RogerT"                       # Your Minecraft username
mc = Minecraft.create(ip, port, name)

# Configurations
ratio = 0.5             # post-processing averaging ratio
magnify_by = 8          # Minecraft in-game viewing ratio
pitch_change = 10       # ange of rotation per upwards/downwards tilt
tp = 10                 # num of steps in telport to simulate walking
follow = True           # rotate based on magnetometer data or not

# Declare variables
received_message = {}
raw_dist = {}
filtered_dist = {}
prev_uncertainty = {}
prev_state = {}
dist_ready = []
ap_locations = np.array(ap_locations_list)
error = 0 
prev_error = 0
prev_result = None
coord_x = []
coord_y = []
angle = 0

def Kalman_filter(measure_state, measure_uncertainty, prev_state, prev_uncertainty):
    ## use Kalman Filter
    if prev_uncertainty == 0:
        prev_uncertainty = 2 ## avoid divide 0
    Kv = prev_uncertainty / (prev_uncertainty + measure_uncertainty)
    estimate_state = prev_state + Kv * (measure_state - prev_state)
    estimate_uncertainty = (1 - Kv) * prev_uncertainty
    
    return estimate_state, estimate_uncertainty

def triangulate(distances_to_ap, ap_locations):
	def error(x, c, d):
		return sum([(np.linalg.norm(x - c[i]) - d[i]) ** 2 for i in range(len(c))])
	num_ap = len(ap_locations)
	dist_sum = sum(distances_to_ap)
	global prev_result
	if prev_result is None:
		weight = [((num_ap - 1) * dist_sum) / (dist_sum - w) for w in distances_to_ap]   # compute weight vector for initial guess
		x0 = sum([weight[i] * ap_locations[i] for i in range(num_ap)])                   # get initial guess of point location
	else:
		x0 = prev_result
	return minimize(error, x0, args=(ap_locations, distances_to_ap), method='Nelder-Mead').x

def on_connect(client, userdata, flags, rc):
    global connected
    connected = True

def on_message(client, userdata, msg):
    topic = msg.topic.split("/")[-1]
    global raw_dist, filtered_dist, prev_uncertainty, prev_state, prev_error, error

    payload = msg.payload
    payload = json.loads(payload)
    print(topic, payload)
    time_stamp = payload["time"]
    measure_state = payload["dist"]
    if measure_state > 5:
        measure_state = 5
    prev_error = error
    error = 0

    if topic not in filtered_dist.keys():
        filtered_dist[topic] = np.array([measure_state]) ## initialize
        raw_dist[topic] = np.array([measure_state]) ## initialize
        prev_uncertainty[topic] = 2
        prev_state[topic] = measure_state
    else:
        raw_dist[topic] = np.append(raw_dist[topic], measure_state)
        measure_uncertainty = np.std(raw_dist[topic])
        if len(raw_dist[topic]) > 2:
            if abs(measure_state - np.mean(raw_dist[topic][:-1])) > max(2, 2*np.std(raw_dist[topic][:-1])):  # move too much >> error
                # measure_uncertainty = max(np.std(raw_dist[topic]), 4)
                #print(topic, "error")
                error = 1
                #print("====", measure_state, np.mean(raw_dist[topic][:-1]))
                #print("====", raw_dist[topic], 2*np.std(raw_dist[topic][:-1]))
                if measure_state > np.mean(raw_dist[topic][:-1]):
                    measure_state = np.mean(raw_dist[topic][:-1]) + max(2, 2*np.std(raw_dist[topic][:-1]))
                else:
                    measure_state = np.mean(raw_dist[topic][:-1]) - max(2, 2*np.std(raw_dist[topic][:-1]))
                if prev_error:
                    raw_dist[topic] = raw_dist[topic][-2:]
                    prev_error = 0
                    error = 0
                    prev_state[topic] = raw_dist[topic][0]
                    filtered_dist[topic] = np.array([np.mean(raw_dist[topic])])
                    print(topic, "restart")
            elif np.mean(filtered_dist[topic]) - np.max(raw_dist[topic]) > np.std(raw_dist[topic]) or \
                 np.min(raw_dist[topic]) - np.mean(filtered_dist[topic]) > np.std(raw_dist[topic]):
                #print(topic, "move!")
                #print("====", measure_state, np.mean(filtered_dist[topic]))
                #print("====", raw_dist[topic], measure_uncertainty)
                prev_state[topic] = np.mean(raw_dist[topic])
                filtered_dist[topic] = np.array([np.mean(raw_dist[topic])])
                measure_uncertainty = np.std(raw_dist[topic])
                prev_uncertainty[topic] = 1
        estimate_state, estimate_uncertainty = Kalman_filter(measure_state, measure_uncertainty, prev_state[topic], prev_uncertainty[topic])
            
        filtered_dist[topic] = np.append(filtered_dist[topic], estimate_state)
        prev_uncertainty[topic] = measure_uncertainty
        prev_state[topic] = estimate_state
    
    filtered_dist[topic] = filtered_dist[topic][-3:]
    raw_dist[topic] = raw_dist[topic][-3:]
    dist = np.mean(filtered_dist[topic])
    #if topic == "a":
    #    print(dist)

    global received_message
    if time_stamp not in received_message:
        received_message[time_stamp] = {topic: dist}
    else:
        received_message[time_stamp][topic] = dist
        if len(received_message[time_stamp]) == 4:
            distances_to_ap = [ 
                received_message[time_stamp]["a"], 
                received_message[time_stamp]["b"], 
                received_message[time_stamp]["c"],
                received_message[time_stamp]["d"],
            ]
            global dist_ready
            dist_ready.append(distances_to_ap)

# Connect to anchor network
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
connected = False
client.connect(HOST, port=1883) # MQTT default port is 1833
client.loop_start()
while connected != True:
    print("Not Connected...")
    time.sleep(0.1)
topic = "rssi/#"
client.subscribe(topic)

# Create in-game "pillars" at anchor points
# Note that player coords are relative to world spawn instead of absolute coords
x,y,z = mc.player.getPos()
#print(x,y,z)
for count, i in enumerate(ap_locations_list):
    for j in range(-100,100):
        mc.setBlock(int(magnify_by*i[0]), int(y+j), -int(magnify_by*i[1]), block_ids[count])

# Connect to STM
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((HOST, PORT))
    s.listen()
    print(f"Listening to {HOST} at port {PORT} for STM signals...")
    conn, addr = s.accept()
    with conn:
        time.sleep(1)
        print(f'Connected by {addr[0]} at {addr[1]}')
        try:
            while(True):
                data_seq = conn.recv(1024).decode('utf-8')
                # data_seq examples: {'Angle': 75}{'Angle': 76}{'Angle': 74}    ,
                #                    {'Angle': 75}                              ,
                #                    {'Tilt': 'U'}                              ,
                #                    {'Angle': 75}{press button}{'Angle': 76}   etc.

                # Hacky way to resolve multiple inputs in one string
                data_seq = str(data_seq).split("}{")
                if data_seq[0][0] == "{":
                    data_seq[0] = data_seq[0][1:]
                if data_seq[-1][-1] == "}":
                    data_seq[-1] = data_seq[-1][:-1]
                #print("Sequence:", data_seq)
                
                for data in data_seq:

                    # STM Button Pressed
                    if str(data)=='press button':
                        x,y,z = mc.player.getPos()
                        mc.setBlock(x+1, y, z+1, block_ids[ID])

                    # Tilt or Angle
                    else:
                        data = json.loads("{"+data+"}")
                        #print("Data:", data)
                        pitch = mc.player.getPitch()
                        rotate = mc.player.getRotation() % 360
                        tilt = data.get('Tilt')
                        new_angle = data.get('Angle')

                        # Board Tilt (determined by accelerometer)
                        # If board tilts to the left/right, switch between available blocks
                        if tilt=='R':
                            ID = (ID+1)%len(block_ids)
                        elif tilt=='L':
                            ID = (ID-1)%len(block_ids)
                        # If board tilts up/down, update player to look up/down by pitch_change degrees in-game 
                        elif tilt=='U':
                            mc.player.setPitch(pitch - pitch_change)
                        elif tilt=='D':
                            mc.player.setPitch(pitch + pitch_change)
                        
                        # Board Rotation (determined by magnetometer)
                        # If angle is received and angle of rotation is larger than 2 degrees, update player rotation
                        if new_angle != None and abs(new_angle-angle)>2:
                            angle = new_angle
                            mc.player.setRotation(360-angle%360)

                # Run when distance data from anchor points are received
                if len(dist_ready):
                    dist = dist_ready.pop(0)
                    #print("dist", dist)

                    # triangulate board position from distance
                    result = triangulate(dist, ap_locations)

                    # Post-processing: remove outliers to far from anchors
                    if result[0] > max_x: result[0] = max_x
                    elif result[0] < min_x: result[0] = min_x
                    if result[1] > max_y: result[1] = max_y
                    elif result[1] < min_y: result[1] = min_y
                    #print("result", result)
                    
                    # Magnify position coordinates for ease of viewing in-game
                    result = result * magnify_by
                    #print("result", result)

                    # Average change of position with previous position if position fluctuates quickly
                    if prev_result is not None:
                        step = math.dist(prev_result, result)
                        #print("step: ", step)
                        if step > magnify_by * 2:
                            result[0] = ratio * result[0] + (1 - ratio) * prev_result[0]
                            result[1] = ratio * result[1] + (1 - ratio) * prev_result[1]
                    coord_x.append(result[0])
                    coord_y.append(result[1])

                    # Average last i+1 coordinates
                    i = 1
                    if len(coord_x) > i:
                        coord_x = coord_x[-i:]
                        coord_y = coord_y[-i:]
                        #print(coord_x, coord_y)
                    new_coord = np.array([np.mean(coord_x), np.mean(coord_y)])
                    #print("new coord", new_coord)

                    # Set initial position
                    if prev_result is None:
                        prev_result = new_coord
                        mc.player.setPos(new_coord[0], y, new_coord[1])

                    # Move player to new position by repeated telporting 
                    x,y,z = mc.player.getPos()
                    dx, dz = new_coord - prev_result
                    dz = -dz # switch z direction
                    #new_rotate = math.atan2(-dx, dz)/math.pi*180%360
                    for step in range(tp):
                        time.sleep(0.001)
                        x, z = x+dx/tp, z+dz/tp
                        mc.player.setPos(x, y, z)
                        #if follow and not (dx==0 and dz==0) and step < r_step:
                        #    mc.player.setRotation((new_rotate-rotate)/r_step*(step+1)+rotate)
                    
                    prev_result = new_coord

        # Close connection and remove in-game anchor points on exit
        except:
            conn.close()
            x,y,z = mc.player.getPos()
            #print(x,y,z)
            for count, i in enumerate(ap_locations_list):
                for j in range(-100,100):
                    #print(magnify_by*i[0], int(y+j), -magnify_by*i[1])
                    mc.setBlock(int(magnify_by*i[0]), int(y+j), -int(magnify_by*i[1]), block.AIR.id)
            raise KeyboardInterrupt