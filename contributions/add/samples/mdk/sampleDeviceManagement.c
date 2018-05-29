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
 *    Hari hara prasad  - initial implementation and API implementation
 *    Lokesh Haralakatta - Added required changes to use Client Side certificates
 *******************************************************************************/

#include "devicemanagementclient.h"

void myCallback (char* commandName, char* format, void* payload)
{
    printf("------------------------------------\n" );
    printf("The command received :: %s\n", commandName);
    printf("format : %s\n", format);
    printf("Payload is : %s\n", (char *)payload);
    printf("------------------------------------\n" );
}

void managedCallBack (char* Status, char* requestId, void* payload)
{
    printf("\n------------------------------------\n" );
    printf("Status :: %s\n", Status);
    printf("requestId : %s\n", requestId);
    printf("Payload is : %s\n", (char *)payload);

    printf("------------------------------------\n" );
}

void printOptions (void)
{
    printf("\n========================== Device Management Sample ==========================");
    printf("\n Please select one of the following options:");
    printf("\n 1: Make managed request");
    printf("\n 2: Make unmanaged request");
    printf("\n 3: Make addErrorCode request");
    printf("\n 4: Make clearErrorCodes request");
    printf("\n 5: Make addLog request");
    printf("\n 6: Make clearLog request");
    printf("\n 7: Make updateLocation request");
    printf("\n 8: Publish Event To WIoTP");
    printf("\n 0: exit");
    printf("\n Enter your option:");
}

void populateMgmtConfig (void)
{
    strcpy(dmClient.DeviceData.deviceInfo.serialNumber, "10087");
    strcpy(dmClient.DeviceData.deviceInfo.manufacturer, "IBM");
    strcpy(dmClient.DeviceData.deviceInfo.model, "7865");
    strcpy(dmClient.DeviceData.deviceInfo.deviceClass, "A");
    strcpy(dmClient.DeviceData.deviceInfo.description, "My Ras");
    strcpy(dmClient.DeviceData.deviceInfo.fwVersion, "1.0.0");
    strcpy(dmClient.DeviceData.deviceInfo.hwVersion, "1.0");
    strcpy(dmClient.DeviceData.deviceInfo.descriptiveLocation, "EGL C");
    strcpy(dmClient.DeviceData.metadata.metadata, "{}");
}

void publishEventToIot (void)
{
    int rc =-1;
    rc = publishEvent_dm("status", "json", (unsigned char *)"{\"d\" : {\"temp\" : 34 }}", QOS0);
    if (rc == SUCCESS)
        printf("Event has been published \n");
    else
        printf("event publish failed \n");
}

int sampleDeviceManagement (void)
{
    int rc = -1;

    char* configFilePath = "device.cfg";

    rc = initialize_configfile_dm(configFilePath);
    if (rc != SUCCESS) {
        printf("Initialize failed and returned rc = %d.\n Quitting..", rc);
        return -1;
    }

    rc = connectiotf_dm();
    if (rc != SUCCESS) {
        printf("Connection; failed and returned rc = %d.\n Quitting..", rc);
        return -1;
    }

    setCommandHandler_dm(myCallback);
    setManagedHandler_dm(managedCallBack);
    subscribeCommands_dm();
    populateMgmtConfig();

    int exit = 0;
    char reqId[40];
    time_t t = time(NULL);
    while (!exit){
        printOptions();
        int val;
        scanf("%d",&val);
        switch (val) {
            case 1:
                printf("\n publish manage ..\n");
                publishManageEvent(4000, 1, 1, reqId);
                printf("\n Manage Event Exited: %s", reqId);
                break;
            case 2:
                printf("\n publish unmanaged..\n");
                publishUnManageEvent(reqId);
                printf("\nunmanaged Request Exit : %s", reqId);
                break;
            case 3:
                printf("\n publish addErrorCode ..\n");
                addErrorCode(121, reqId);
                printf("\n addErrorCode Request Exit : %s", reqId);
                break;
            case 4:
                printf("\n publish clearErrorCodes ..\n");
                clearErrorCodes(reqId);
                printf("\n clearErrorCodes Request Exit :");// %s", reqId);
                break;
            case 5:
                printf("\n publish addLog ..\n");
                addLog("test", "", 1, reqId);
                printf("\n addLog Request Exit : %s", reqId);
                break;
            case 6:
                printf("\n publish clearLogs ..\n");
                clearLogs(reqId);
                printf("\n clearLogs Request Exit : %s", reqId);
                break;
            case 7:
                printf("\n publish updateLocation ..\n");
                char updatedDateTime[50];//"2016-03-01T07:07:56.323Z"
                strftime(updatedDateTime, sizeof(updatedDateTime), "%Y-%m-%dT%TZ", localtime(&t));
                updateLocation(77.5667,12.9667, 0, updatedDateTime, 0, reqId);
                printf("updateLocation Request Exit : %s", reqId);
                break;
            case 8:
                printf("\n publish Event To WIoTP ..\n");
                publishEventToIot();
                break;
            default:
                disconnect_dm();
                printf("\n Quitting!!\n");
                exit = 1;
                break;
        }
    }

    return 0;
}
