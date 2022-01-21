/* Sockets Example
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
#include "wifi_helper.h"
#include "mbed-trace/mbed_trace.h"

#include "BufferedSerial.h"
#include "EthernetInterface.h" //

#include "stm32l475e_iot01_gyro.h"
#include "stm32l475e_iot01_accelero.h"
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

        /* opening the socket only allocates resources */
        result = _socket.open(_net);
        if (result != 0) {
            printf("Error! _socket.open() returned: %d\r\n", result);
            return;
        }

        SocketAddress a;
        const char addr[] = "192.168.227.220";
        a.set_ip_address(addr);
        a.set_port(REMOTE_PORT);
        result = _socket.connect(a);

        /* we are connected to the network but since we're using a connection oriented
         * protocol we still need to open a connection on the socket */
        if (result != 0) {
            printf("Error! _socket.connect() returned: %d\r\n", result);
            return;
        } else {
            printf("Socket connect to %s || Port: %d\r\n", a.get_ip_address(), REMOTE_PORT);
        }

        /* exchange an HTTP request and response */
        from_sensor();
        _socket.close();
        printf("Complete from sensor\r\n");
        _net->disconnect();
        printf("Demo concluded successfully \r\n");
    }

private:
    void from_sensor()
    {
        int16_t pDataXYZ[3] = {0};
        float pGyroDataXYZ[3] = {0};
        int sample_num = 0;
        char acc_json[MAX_MESSAGE_RECEIVED_LENGTH];
        printf("Start sensor init\n");
        BSP_GYRO_Init();
        BSP_ACCELERO_Init();

        while(sample_num < 5) {
            printf("\nLoop %d\n", sample_num);
            ++sample_num;

            BSP_GYRO_GetXYZ(pGyroDataXYZ);
            printf("\nGYRO_X = %.2f\n", pGyroDataXYZ[0]);
            printf("GYRO_Y = %.2f\n", pGyroDataXYZ[1]);
            printf("GYRO_Z = %.2f\n", pGyroDataXYZ[2]);

            BSP_ACCELERO_AccGetXYZ(pDataXYZ);
            printf("\nACCELERO_X = %d\n", pDataXYZ[0]);
            printf("ACCELERO_Y = %d\n", pDataXYZ[1]);
            printf("ACCELERO_Z = %d\n", pDataXYZ[2]);

            ThisThread::sleep_for(1s);

            int len = sprintf(acc_json, "{\"Gyro\":{\"x\":%f,\"y\":%f,\"z\":%f},\
                                          \"Acce\":{\"x\":%f,\"y\":%f,\"z\":%f},\
                                          \"s\":%d}",
                            (float)((int)(pGyroDataXYZ[0]*10000))/10000,
                            (float)((int)(pGyroDataXYZ[1]*10000))/10000, 
                            (float)((int)(pGyroDataXYZ[2]*10000))/10000, 
                            (float)((int)(pDataXYZ[0]*10000))/10000,
                            (float)((int)(pDataXYZ[1]*10000))/10000, 
                            (float)((int)(pDataXYZ[2]*10000))/10000,
                            sample_num);

            // while (len) {
            int response = _socket.send(acc_json, len);
            if (response < 0) {
                printf("Error! _socket.send() returned: %d\r\n", response);
                return;
            } else {
                printf("sent %d bytes\r\n", response);
            }

            // len -= response;
            ThisThread::sleep_for(1s);
            // }

        }
        printf("Complete 5 sample message sent\r\n");
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
    printf("\r\nStarting socket demo\r\n\r\n");

#ifdef MBED_CONF_MBED_TRACE_ENABLE
    mbed_trace_init();
#endif

    SocketDemo *example = new SocketDemo();
    MBED_ASSERT(example);
    example->run();

    return 0;
}
