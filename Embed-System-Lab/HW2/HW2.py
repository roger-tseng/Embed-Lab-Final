#!/usr/bin/env python3
import socket
import numpy as np
import json
import time
import random
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

      
HOST = '192.168.227.220'      # Standard loopback interface addres
PORT = 65432                # Port to listen on (use ports > 1023)
counter = 0

plt_array = {}
sources = ["Gyro", "Acce"]
axes = ["x", "y", "z"]

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print(f"Listening to {HOST} at port {PORT}...")
    conn, addr = s.accept()
    with conn:
        print(f'Connected by {addr[0]} at {addr[1]}')

        while counter < 5:
            counter +=1
            data = conn.recv(1024).decode('utf-8')
            # print(len(data)) ## 201
            try:
                json_data = json.loads(data)
                print('Received data from socket client:', json_data)
                if not plt_array:
                    plt_array = json_data
                for s in sources:
                    for a in axes:
                        if not isinstance(plt_array[s][a], list):
                            plt_array[s][a] = [plt_array[s][a], json_data[s][a]]
                        else:
                            plt_array[s][a].append(json_data[s][a])

            except:
                print("Error receiving data!")
                break

# plt_array = {'Gyro': {'x': [280.0, 290.0], 'y': [-1890.0, -1789.0], 'z': [770.0, 800.0]}, 'Acce': {'x': [18.0,20.0], 'y': [-27.0, -25.0], 'z': [1001.0, 1011.0]}}
print(plt_array)
fig = plt.figure(figsize = plt.figaspect(0.5))
axes = fig.add_subplot(1, 2, 1, projection='3d')
axes.plot(plt_array["Gyro"]["x"], plt_array["Gyro"]["y"], plt_array["Gyro"]["z"], label = "Gyro", color="blue")
axes.legend()
axes = fig.add_subplot(1, 2, 2, projection='3d')
axes.plot(plt_array["Acce"]["x"], plt_array["Acce"]["y"], plt_array["Acce"]["z"], label = "Acce", color="red")
axes.legend()
plt.show()



                
