/*******************************************************************************
 * Copyright (c) 2015 IBM Corp.
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

#include "deviceclient.h"
#include "cmsis_os2.h"

void myCallback (char* commandName, char* format, void* payload)
{
    printf("------------------------------------\n" );
    printf("The command received :: %s\n", commandName);
    printf("format : %s\n", format);
    printf("Payload is : %s\n", (char *)payload);

    printf("------------------------------------\n" );
}

int sampleDevice (void)
{
    int rc = -1;
    int count = 0;

    iotfclient client;

    char* configFilePath = "device.cfg";

    rc = initialize_configfile(&client, configFilePath, 0);
    if (rc != SUCCESS) {
        printf("Initialize failed and returned rc = %d.\n Quitting..", rc);
        return -1;
    }

    rc = connectiotf(&client);
    if (rc != SUCCESS) {
        printf("Connection failed and returned rc = %d.\n Quitting..", rc);
        return -1;
    }

    if (!client.isQuickstart) {
        subscribeCommands(&client);
        setCommandHandler(&client, myCallback);
    }

    while (++count <= 10)
    {
        printf("Publishing the event stat with rc ");
        rc= publishEvent(&client, "status","json", "{\"d\" : {\"temp\" : 34 }}", QOS0);
        printf(" %d\n", rc);
        yield(&client,1000);
        osDelay(2000);
    }

    printf("Quitting!!\n");

    disconnect(&client);

    return 0;
}
