/* Modified from Sockets Example at https://github.com/ARMmbed/mbed-os-example-sockets
 * Copyright (c) 2016-2020 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "ble/BLE.h"
#include "ble/Gap.h"
#include <events/mbed_events.h>

#include "wifi_helper.h"
#include "mbed-trace/mbed_trace.h"

#include "BufferedSerial.h"
#include "EthernetInterface.h" //

#include "stm32l475e_iot01_magneto.h"
#include "stm32l475e_iot01_accelero.h"
#include <cstdint>
#include <cstdio>

static BufferedSerial serial_port(USBTX, USBRX);
FileHandle*mbed::mbed_override_console(int fd){
    return &serial_port;
}

#if MBED_CONF_APP_USE_TLS_SOCKET
#include "root_ca_cert.h"

#ifndef DEVICE_TRNG
#error "mbed-os-example-tls-socket requires a device which supports TRNG"
#endif
#endif // MBED_CONF_APP_USE_TLS_SOCKET

InterruptIn button(BUTTON1);
volatile bool button_switch = 0;
void button_pressed()
{
    button_switch = !button_switch;
}

class SocketDemo {
    static constexpr size_t MAX_NUMBER_OF_ACCESS_POINTS = 10;
    static constexpr size_t MAX_MESSAGE_RECEIVED_LENGTH = 224;  // should be larger than sent bit length

#if MBED_CONF_APP_USE_TLS_SOCKET
    static constexpr size_t REMOTE_PORT = 443; // tls port
#else
    static constexpr size_t REMOTE_PORT = 65432;
#endif // MBED_CONF_APP_USE_TLS_SOCKET

public:
    SocketDemo() : _net(NetworkInterface::get_default_instance())
    {
    }

    ~SocketDemo()
    {
        if (_net) {
            _net->disconnect();
        }
    }

    void run()
    {
        if (!_net) {
            printf("Error! No network interface found.\r\n");
            return;
        }

        WiFiInterface *wifi = _net->wifiInterface();
        /* connect will perform the action appropriate to the interface type to connect to the network */

        printf("Connecting to the network...\r\n");

        nsapi_size_or_error_t result = _net->connect();
        if (result != 0) {
            printf("Error! _net->connect() returned: %d\r\n", result);
            return;
        }

        //print_network_info();

        /* opening the socket only allocates resources */
        result = _socket.open(_net);
        if (result != 0) {
            printf("Error! _socket.open() returned: %d\r\n", result);
            return;
        }

        SocketAddress a;
        const char addr[] = "192.168.141.220";
        a.set_ip_address(addr);
        a.set_port(REMOTE_PORT);

        /* we are connected to the network but since we're using a connection oriented
         * protocol we still need to open a connection on the socket */

        printf("Opening connection to remote port %d\r\n", REMOTE_PORT);

        result = _socket.connect(a);
        if (result != 0) {
            printf("Error! _socket.connect() returned: %d\r\n", result);
            return;
        } else {
            printf("Socket connect to %s || Port: %d\r\n", a.get_ip_address(), REMOTE_PORT);
        }

        BSP_ACCELERO_Init();
        BSP_MAGNETO_Init();

        int16_t pDataXYZ[3] = {0};
        float pGyroDataXYZ[3] = {0};
        int16_t pMagXYZ[3] = {0};
        int response;

        while(true){
            char acc_json[MAX_MESSAGE_RECEIVED_LENGTH];
            BSP_ACCELERO_AccGetXYZ(pDataXYZ);
            BSP_MAGNETO_GetXYZ(pMagXYZ);

            printf("\nMag_X = %d\n", pMagXYZ[0]);
            printf("Mag_Y = %d\n", pMagXYZ[1]);
            printf("Mag_Z = %d\n", pMagXYZ[2]);

            int dir = compass(pDataXYZ[0], pDataXYZ[1], pDataXYZ[2], pMagXYZ[0], pMagXYZ[1], pMagXYZ[2]);

            printf("direction = %d \n", dir);

            //int len = sprintf(acc_json, "%d", dir);
            //response = _socket.send(acc_json, len);

            printf("\nACCELERO_X = %d\n", pDataXYZ[0]);
            printf("ACCELERO_Y = %d\n", pDataXYZ[1]);
            printf("ACCELERO_Z = %d\n", pDataXYZ[2]);
            if(pDataXYZ[1] < -600){
                int len = sprintf(acc_json, "{\"Tilt\": \"R\"}");
                response = _socket.send(acc_json,len);
            }
            else if(pDataXYZ[1] > 600){
                int len = sprintf(acc_json, "{\"Tilt\": \"L\"}");
                response = _socket.send(acc_json,len);
            }
            else if(pDataXYZ[0] < -800){
                int len = sprintf(acc_json, "{\"Tilt\": \"U\"}");
                response = _socket.send(acc_json,len);
            }
            else if(pDataXYZ[0] > 800){
                int len = sprintf(acc_json, "{\"Tilt\": \"D\"}");
                response = _socket.send(acc_json,len);
            }    
            else{
                int len = sprintf(acc_json, "{\"Angle\": %d}", dir);
                response = _socket.send(acc_json,len);
            }

                
            if (response < 0) {
                printf("Error! _socket.send() returned: %d\r\n", response);
                return;
            } else {
                printf("sent %d bytes\r\n", response);
            }

            ThisThread::sleep_for(500ms);
            button.fall(&button_pressed);
            if(button_switch){
                send_press();
                button_switch = !button_switch;
            }
        }

        printf("Demo concluded successfully \r\n");
    }

private:

int16_t compass(int16_t ax, int16_t ay, int16_t az, int16_t mx, int16_t my, int16_t mz){
        // Compute rotation base on magnetometer data
        // See: https://stackoverflow.com/questions/3120954/calculating-magnetic-heading-using-raw-accelerometer-and-magnetometer-data
        //      https://stackoverflow.com/questions/35061294/how-to-calculate-heading-using-gyro-and-magnetometer
        float roll = atan2(ax, az);
        float pitch = -atan(ay/(ax*sin(roll)+az*cos(roll)));
        float yaw = atan2(mx*sin(roll)*sin(pitch) + mz*cos(roll)*sin(pitch) + my*cos(pitch), mx*cos(roll) - mz*sin(roll));

        yaw = yaw *180 /3.14159;

        if(yaw > 0) yaw = -180 + (yaw - 180);
        yaw = -yaw;
        yaw += 90;
        yaw -= 2;       // compensate
        while(yaw > 360) {yaw -= 360;}

        int dir = int(yaw);

        return dir;
    }

    void send_press()
    {
        const char buffer[] = "{press button}";
            nsapi_size_t bytes_to_send = strlen(buffer);
            nsapi_size_or_error_t bytes_sent = 0;
        while (bytes_to_send) {
            bytes_sent = _socket.send(buffer + bytes_sent, bytes_to_send);
            bytes_to_send -= bytes_sent;
        }
    }

    void print_network_info()
    {
        /* print the network info */
        SocketAddress a;
        _net->get_ip_address(&a);
        printf("IP address: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
        _net->get_netmask(&a);
        printf("Netmask: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
        _net->get_gateway(&a);
        printf("Gateway: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
    }

private:
    NetworkInterface *_net;

#if MBED_CONF_APP_USE_TLS_SOCKET
    TLSSocket _socket;
#else
    TCPSocket _socket;
#endif // MBED_CONF_APP_USE_TLS_SOCKET
};


int main() {
    printf("\r\nStarting demo\r\n\r\n");

#ifdef MBED_CONF_MBED_TRACE_ENABLE
    mbed_trace_init();
#endif

    SocketDemo *example = new SocketDemo();
    MBED_ASSERT(example);
    example->run();

    return 0;
}
