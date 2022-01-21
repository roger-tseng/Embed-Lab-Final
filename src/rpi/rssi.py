from bluepy.btle import Peripheral, UUID
from bluepy.btle import Scanner, DefaultDelegate
import time
import numpy as np
import json
import paho.mqtt.client as mqtt
import math

n_factor = 1
PL_d0 = 0
d0 = 1.2

class NotificationDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)
    def handleNotification(self, cHandle, data):
        print(f"Received {int.from_bytes(data, 'big')} from characteristic with handle {cHandle}")

def Kalman_filter(measure_state, measure_uncertainty, prev_state, prev_uncertainty):
    ## use Kalman Filter
    if prev_uncertainty == 0:
        prev_uncertainty = 1e-8 ## avoid divide 0
    Kv = prev_uncertainty / (prev_uncertainty + measure_uncertainty)
    estimate_state = prev_state + Kv * (measure_state - prev_state)
    estimate_uncertainty = (1 - Kv) * prev_uncertainty
    
    return estimate_state, estimate_uncertainty

def scan_3sec(verbose=False):
    scanner = Scanner()
    raw_rssi = None
    filtered_rssi = None
    scan_time_start = time.time()
    miss = 0
    while time.time() - scan_time_start <= 3:
        devices = list(scanner.scan(0.2))
        found = False
        for d in devices:
            for (adtype, desc, value) in d.getScanData():
                if value=="Button":
                    found = True
                    print( f"RSSI={d.rssi} dB")
                    measure_state = d.rssi
                    break
        if found==False:
            miss += 1
            continue

        if filtered_rssi is None:
            filtered_rssi = np.array([measure_state]) ## initialize
            raw_rssi = np.array([measure_state]) ## initialize
            prev_uncertainty = np.std(raw_rssi)
            prev_state = measure_state
        else:
            raw_rssi = np.append(raw_rssi, measure_state)
            measure_uncertainty = np.std(raw_rssi)
            estimate_state, estimate_uncertainty = Kalman_filter(measure_state, measure_uncertainty, prev_state, prev_uncertainty)
            filtered_rssi = np.append(filtered_rssi, estimate_state)
            prev_uncertainty = measure_uncertainty
            prev_state = estimate_state
    
    # if can't scan anything
    if raw_rssi is None: 
        return None, miss

    ## scanning ends
    original_mean = np.mean(raw_rssi)
    original_std = np.std(raw_rssi)
    filtered_mean = np.mean(filtered_rssi)
    filtered_std = np.std(filtered_rssi)
    
    if verbose:
        print("-----------------------")
        print("original mean:", original_mean)
        print("original std:", original_std)
        print("-----------------------")
        print("mean after kalman filter:", filtered_mean)
        print("std after kalman filter:", filtered_std)
        print("-----------------------")

    return filtered_mean, miss

def initialize():
    print("Start initializing... ")
    d = [1.2, 2.4, 3.6]
    initial = {}
    n = []
    for dd in d:
        while True:
            print(f"Please put Rpi at m = {dd}")
            ready = input("start scanning (y / n) ")
            if ready == "y":
                break
        rssi, miss = scan_3sec()
        print("RSSI: ", rssi, " | Miss: ", miss)
        initial[dd] = {"rssi": rssi, "miss": miss}
    
    for i in range(1, len(d)):
        nn = (initial[d[0]]["rssi"] - initial[d[i]]["rssi"]) / 10.0 / math.log10(d[i]/d[0])
        n.append(nn)
    print("\nCalculate n: ", n)

    global n_factor
    n_factor = np.mean(n)
    print("average:", n_factor, "\n")  

    global PL_d0, d0
    PL_d0 = initial[d[0]]["rssi"]
    d0 = d[0]


def publish_mqtt(topic, dist, rssi, miss):
    while True:
        t = str(time.time()).split(".")[0]
        if int(t) % 4 == 0:
            break
    payload = {"time": t, "dist": dist, "rssi": rssi, "miss": miss}
    print(json.dumps(payload))
    client.publish(topic, json.dumps(payload))

broker_ip = "192.168.141.220"
topic = "rssi/b"

client = mqtt.Client()
client.connect(broker_ip)
client.loop_start()

init = input("initialize ? (y / n) ")
if init == "y":
    initialize()
elif init == "n":
    d0 = float(input("input d0: "))
    PL_d0 = float(input("input the rssi value measured at d0: "))
    n_factor = float(input("input n factor: "))

while True:
    print("\nFinish initializing!\n")
    ready = input("start scanning (y / n) ")
    if ready == "y":
        break

while True:    
    rssi, miss = scan_3sec()
    # print(rssi, miss)
    if rssi is not None:
        logdist = (PL_d0 - rssi) / 10.0 / n_factor
        dist = pow(10.0, logdist) * d0
        print(dist)

        publish_mqtt(topic, dist, rssi, miss)
    time.sleep(0.1)