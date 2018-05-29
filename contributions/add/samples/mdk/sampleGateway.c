/*******************************************************************************
 * Copyright (c) 2016 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Jeffrey Dare - initial implementation and API implementation
 *    Lokesh Haralakatta - Added required changes to use Client Side certificates
 *******************************************************************************/

#include "gatewayclient.h"
#include "cmsis_os2.h"

//Command Handler
void myCallback (char* type, char* id, char* commandName, char *format, void* payload, size_t payloadlen)
{
    printf("------------------------------------\n" );
    printf("Type is : %s\n", type);
    printf("Id is : %s\n", id);
    printf("The command received :: %s\n", commandName);
    printf("format : %s\n", format);
    printf("Payload is : %.*s\n", (int)payloadlen, (char *)payload);
    printf("------------------------------------\n" );
}

int sampleGateway (void)
{
    int rc = -1;
    int count = 0;

    iotfclient client;

    char* configFilePath = "device.cfg";

    rc = initialize_configfile(&client, configFilePath, 1);
    if (rc != SUCCESS) {
        printf("Initialize failed and returned rc = %d.\n Quitting..", rc);
        return -1;
    }

    setKeepAliveInterval(59);

    rc = connectiotf(&client);
    if (rc != SUCCESS) {
        printf("Connection failed and returned rc = %d.\n Quitting..", rc);
        return -1;
    }

    //Registering the function "myCallback" as the command handler.
    setGatewayCommandHandler(&client, myCallback);
    // providing "+" will subscribe to all the command of all formats.
    subscribeToDeviceCommands(&client, "elevator", "elevator-1", "+", "+", 0);

    while (++count <= 10)
    {
        printf("Publishing the event stat with rc ");
        //publishing gateway events
        rc = publishGatewayEvent(&client, "elevatorDevices", "elevatorGateway", "{\"d\" : {\"temp\" : 34 }}", QOS0);
        //publishing device events on behalf of a device
        rc = publishDeviceEvent(&client, "elevator", "elevator-1", "status", "json", "{\"d\" : {\"temp\" : 34 }}", QOS0);

        printf(" %d\n", rc);
        //Yield for receiving commands.
        yield(&client, 1000);
        osDelay(1000);
    }

    printf("Quitting!!\n");

    //Be sure to disconnect the gateway at exit
    disconnectGateway(&client);

    return 0;
}
