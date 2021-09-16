/****************************************
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
 *    Hari Hara,              - Initial implementation and API implementation
 *    Hari prasada Reddy P
 *    Hari Prasada Reddy P    - Added implementation for Device actions support
 *    Lokesh Haralakatta      - Added SSL/TLS support
 *    Lokesh Haralakatta      - Added Client Side Certificates support
 *                            - Added logging feature
 ****************************************/


#include "devicemanagementclient.h"
#include "cJSON.h"

//Character strings to hold log header and log message to be dumped.
extern char logHdr[LOG_BUF];
extern char logStr[LOG_BUF];

ManagedDevice dmClient;

//Callabcks
commandCallback cb;
commandCallback cbReboot;
commandCallback cbFactoryReset;
actionCallback cbFirmwareDownload;
actionCallback cbFirmwareUpdate;

const char* dmUpdate = "iotdm-1/device/update";
const char* dmObserve = "iotdm-1/observe";
const char* dmCancel = "iotdm-1/cancel";
const char* dmReboot = "iotdm-1/mgmt/initiate/device/reboot";
const char* dmFactoryReset = "iotdm-1/mgmt/initiate/device/factory_reset";
const char* dmFirmwareDownload = "iotdm-1/mgmt/initiate/firmware/download";
const char* dmFirmwareUpdate = "iotdm-1/mgmt/initiate/firmware/update";

static char currentRequestID[40];
volatile int interrupt = 0;

// Handle signal interrupt
/*void sigHandler(int signo) {
	printf("SigINT received.\n");
	interrupt = 1;
}*/

/*
* Function used to initialize the IBM Watson IoT client using the config file which is generated when you register your device
*
* @param configFilePath - File path to the configuration file
*
* @return int return code
* error codes
* CONFIG_FILE_ERROR -3 - Config file not present or not in right format
*/
int initialize_configfile_dm(char *configFilePath)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	int rc = -1;
	rc = initialize_configfile(&dmClient.deviceClient, configFilePath,0);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
        LOG_STR("rc = %d",rc);
        LOG(logHdr,logStr);
        LOG(logHdr,"exit::");

	return rc;
}

/**
* Function used to initialize the IBM Watson IoT client
*
* @param org - Your organization ID
*
* @param type - The type of your device
*
* @param id - The ID of your device
*
* @param auth-method - Method of authentication (the only value currently supported is â€œtokenâ€�)
*
* @param auth-token - API key token (required if auth-method is â€œtokenâ€�)
*
* @return int return code
*/
int initialize_dm(char *orgId, char* domainName, char *deviceType, char *deviceId,
		  char *authmethod, char *authToken, char *serverCertPath, int useCerts,
		  char *rootCACertPath, char *clientCertPath, char *clientKeyPath)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	int rc = -1;
	rc = initialize(&dmClient.deviceClient, orgId, domainName, deviceType, deviceId,
			authmethod, authToken,serverCertPath,useCerts, rootCACertPath,
			clientCertPath,clientKeyPath,0);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("rc = %d",rc);
	LOG(logHdr,logStr);
	LOG(logHdr,"exit::");

	return rc;
}

/*
* Function used to initialize the IBM Watson IoT client
*
* @return int return code
*/
int connectiotf_dm()
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	int rc = isConnected(&dmClient.deviceClient);
	if(rc){ //if connected return
		printf("Client is connected\n");
		return rc;
	}

	rc = connectiotf(&dmClient.deviceClient);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("rc = %d",rc);
	LOG(logHdr,logStr);
	LOG(logHdr,"exit::");

	return rc;
}

/*
* Function used to Publish events from the device to the IBM Watson IoT service
*
* @param eventType - Type of event to be published e.g status, gps
*
* @param eventFormat - Format of the event e.g json
*
* @param data - Payload of the event
*
* @param QoS - qos for the publish event. Supported values : QOS0, QOS1, QOS2
*
* @return int return code from the publish
*/

int publishEvent_dm(char *eventType, char *eventFormat, unsigned char* data, enum QoS qos)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	int rc = -1;
	rc = publishEvent(&dmClient.deviceClient, eventType, eventFormat, (char *)data, qos);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("rc = %d",rc);
	LOG(logHdr,logStr);
	LOG(logHdr,"exit::");

	return rc;
}

/*
* Function used to set the Command Callback function. This must be set if you to recieve commands.
*
* @param handler Function pointer to the commandCallback. Its signature - void (*commandCallback)(char* commandName, char* payload)
*
*/
void setCommandHandler_dm(commandCallback handler)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	setCommandHandler(&dmClient.deviceClient,handler );//handler
	cb = handler;

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG(logHdr,"exit::");
}

/**
 * Register Callback function to managed request response
 *
 * @param handler Function pointer to the commandCallback. Its signature - void (*commandCallback)(char* Status, char* requestId,            void*       payload)
 *
*/

void setManagedHandler_dm(commandCallback handler)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	cb = handler;

	if(cb != NULL){
                LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
                LOG_STR("Registered Manage callabck");
                LOG(logHdr,logStr);
        }
        else{
                LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
                LOG_STR("Manage callabck not registered");
                LOG(logHdr,logStr);
        }

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG(logHdr,"exit::");
}

/**
 * Register Callback function to Reboot request
 *
 * @param handler Function pointer to the commandCallback. Its signature - void (*commandCallback)(char* Status, char* requestId,            void*       payload)
 *
*/

void setRebootHandler(commandCallback handler)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	cbReboot = handler;

	if(cbReboot != NULL){
                LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
                LOG_STR("Registered Reboot callabck");
                LOG(logHdr,logStr);
        }
        else{
                LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
                LOG_STR("Reboot callabck not registered");
                LOG(logHdr,logStr);
        }

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG(logHdr,"exit::");
}

/**
 * Register Callback function to Factory reset request
 *
 * @param handler Function pointer to the commandCallback. Its signature - void (*commandCallback)(char* Status, char* requestId,            void*       payload)
 *
*/

void setFactoryResetHandler(commandCallback handler)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	cbFactoryReset = handler;

	if(cbFactoryReset != NULL){
                LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
                LOG_STR("Registered FactoryReset callabck");
                LOG(logHdr,logStr);
        }
        else{
                LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
                LOG_STR("FactoryReset callabck not registered");
                LOG(logHdr,logStr);
        }

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG(logHdr,"exit::");
}

/**
 * Register Callback function to Download Firmware
 *
 * @param handler Function pointer to the actionCallback. Its signature - void (*actionCallback)()
 *
*/

void setFirmwareDownloadHandler(actionCallback handler)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	cbFirmwareDownload = handler;

	if(cbFirmwareDownload != NULL){
                LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
                LOG_STR("Registered FirmwareDownload callabck");
                LOG(logHdr,logStr);
        }
        else{
                LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
                LOG_STR("FirmwareDownload callabck not registered");
                LOG(logHdr,logStr);
        }

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG(logHdr,"exit::");
}

/**
 * Register Callback function to update Firmware
 *
 * @param handler Function pointer to the actionCallback. Its signature - void (*actionCallback)()
 *
*/

void setFirmwareUpdateHandler(actionCallback handler)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	cbFirmwareUpdate = handler;

	if(cbFirmwareDownload != NULL){
                LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
                LOG_STR("Registered FirmwareUpdate callabck");
                LOG(logHdr,logStr);
        }
        else{
                LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
                LOG_STR("FirmwareUpdate callabck not registered");
                LOG(logHdr,logStr);
        }

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG(logHdr,"exit::");
}

/*
* Function used to subscribe to all commands. This function is by default called when in registered mode.
*
* @return int return code
*/
int subscribeCommands_dm()
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	int rc = -1;

	rc = subscribeCommands(&dmClient.deviceClient);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("rc from subscribeCommands = %d",rc);
	LOG(logHdr,logStr);

	if(rc >=0){
		MQTTClient *c;
		c= &(dmClient.deviceClient.c);
		// Call back handles all the requests and responses received from the Watson IoT platform
		rc = MQTTSubscribe(c, "iotdm-1/#", QOS0, onMessage);
		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("rc from MQTTSubscribe = %d",rc);
		LOG(logHdr,logStr);
	}

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("rc = %d",rc);
	LOG(logHdr,logStr);
	LOG(logHdr,"exit::");

	return rc;
}

/*
* Function used to Yield for commands.
*
* @param time_ms - Time in milliseconds
*
* @return int return code
*/
int yield_dm(int time_ms)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	int rc = 0;
	rc = yield(&dmClient.deviceClient, time_ms);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("rc = %d",rc);
	LOG(logHdr,logStr);
	LOG(logHdr,"exit::");

	return rc;
}

/*
* Function used to check if the client is connected
*
* @return int return code
*/
int isConnected_dm()
{
	return isConnected(&dmClient.deviceClient);
}

/*
* Function used to disconnect from the IBM Watson IoT service
*
* @return int return code
*/

int disconnect_dm()
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	int rc = 0;
	rc = disconnect(&dmClient.deviceClient);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("rc = %d",rc);
	LOG(logHdr,logStr);
	LOG(logHdr,"exit::");

	return rc;
}

/**
* <p>Send a device manage request to Watson IoT Platform</p>
*
* <p>A Device uses this request to become a managed device.
* It should be the first device management request sent by the
* Device after connecting to the IBM Watson IoT Platform.
* It would be usual for a device management agent to send this
* whenever is starts or restarts.</p>
*
* <p>This method connects the device to Watson IoT Platform connect if its not connected already</p>
*
* @param lifetime The length of time in seconds within
*        which the device must send another Manage device request.
*        if set to 0, the managed device will not become dormant.
*        When set, the minimum supported setting is 3600 (1 hour).
*
* @param supportFirmwareActions Tells whether the device supports firmware actions or not.
*        The device must add a firmware handler to handle the firmware requests.
*
* @param supportDeviceActions Tells whether the device supports Device actions or not.
*        The device must add a Device action handler to handle the reboot and factory reset requests.
*
* @param reqId Function returns the reqId if the publish Manage request is successful.
*
* @return
*/
void publishManageEvent(long lifetime, int supportFirmwareActions,int supportDeviceActions, char* reqId)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	char uuid_str[40];
	generateUUID(uuid_str);
	strcpy(currentRequestID,uuid_str);
	char* strPayload = "{\"d\": {\"metadata\":%s ,\"lifetime\":%ld ,\"supports\": {\"deviceActions\":%d,\"firmwareActions\":%d},\"deviceInfo\": {\"serialNumber\":\"%s\",\"manufacturer\":\"%s\",\"model\":\"%s\",\"deviceClass\":\"%s\",\"description\":\"%s\",\"fwVersion\":\"%s\",\"hwVersion\":\"%s\",\"descriptiveLocation\":\"%s\"}},\"reqId\": \"%s\"}" ;//cJSON_Print(jsonPayload);
	char payload[1500];
	sprintf(payload,strPayload,dmClient.DeviceData.metadata.metadata,lifetime, supportDeviceActions, supportFirmwareActions, dmClient.DeviceData.deviceInfo.serialNumber,dmClient.DeviceData.deviceInfo.manufacturer, dmClient.DeviceData.deviceInfo.model,dmClient.DeviceData.deviceInfo.deviceClass,dmClient.DeviceData.deviceInfo.description,dmClient.DeviceData.deviceInfo.fwVersion,dmClient.DeviceData.deviceInfo.hwVersion,dmClient.DeviceData.deviceInfo.descriptiveLocation,uuid_str);
	int rc = -1;
	rc = publish(MANAGE, payload);
	if(rc == SUCCESS){
		strcpy(reqId, uuid_str);

		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("reqId = %s",reqId);
		LOG(logHdr,logStr);
	}

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("rc = %d",rc);
	LOG(logHdr,logStr);
	LOG(logHdr,"exit::");
}

/**
 * Moves the device from managed state to unmanaged state
 *
 * A device uses this request when it no longer needs to be managed.
 * This means Watson IoT Platform will no longer send new device management requests
 * to this device and device management requests from this device will
 * be rejected apart from a Manage device request
 *
 * @param reqId Function returns the reqId if the Unmanage request is successful.
 */
void publishUnManageEvent(char* reqId)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	char uuid_str[40];
	int rc = -1;
	generateUUID(uuid_str);
	strcpy(currentRequestID,uuid_str);
	char data[70];
	sprintf(data,"{\"reqId\":\"%s\"}",uuid_str);
	rc = publish(UNMANAGE, data);
	if(rc == SUCCESS){
		strcpy(reqId, uuid_str);

		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("reqId = %s",reqId);
		LOG(logHdr,logStr);
	}

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("rc = %d",rc);
	LOG(logHdr,logStr);
	LOG(logHdr,"exit::");
}

/*
 * Update the location of the device. This method converts the
 * date in the required format. The caller needs to pass the date in string in ISO8601 format.
 *
 * @param latitude	Latitude in decimal degrees using WGS84
 *
 * @param longitude Longitude in decimal degrees using WGS84
 *
 * @param elevation	Elevation in meters using WGS84
 *
 * @param measuredDateTime When the location information is retrieved
 *
 * @param accuracy	Accuracy of the position in meters
 *
 * @param reqId Function returns the reqId if the updateLocation request is successful.
 *
 * @return code indicating whether the update is successful or not
 *        (200 means success, otherwise unsuccessful)

 */
void updateLocation(double latitude, double longitude, double elevation, char* measuredDateTime, double accuracy, char* reqId)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	int rc = -1;

	char uuid_str[40];
	generateUUID(uuid_str);
	strcpy(currentRequestID,uuid_str);

	char data[500];
	sprintf(data,"{\"d\":{\"longitude\":%f,\"latitude\":%f,\"elevation\":%f,\"measuredDateTime\":\"%s\",\"accuracy\":%f},\"reqId\":\"%s\"}",
	                latitude, longitude, elevation, measuredDateTime, accuracy, uuid_str);

	rc = publish(UPDATE_LOCATION, data);
	if(rc == SUCCESS){
		strcpy(reqId, uuid_str);

		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("reqId = %s",reqId);
		LOG(logHdr,logStr);
	}

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("rc = %d",rc);
	LOG(logHdr,logStr);
	LOG(logHdr,"exit::");
}

/*
 * Update the location of the device. This method converts the
 * date in the required format. The caller needs to pass the date in string in ISO8601 format.
 *
 * @param latitude	Latitude in decimal degrees using WGS84
 *
 * @param longitude Longitude in decimal degrees using WGS84
 *
 * @param elevation	Elevation in meters using WGS84
 *
 * @param measuredDateTime When the location information is retrieved
 * *
 * @param updatedDateTime When the location information is updated
 *
 * @param accuracy	Accuracy of the position in meters
 *
 * @param reqId Function returns the reqId if the updateLocation request is successful.
 *
 * @return code indicating whether the update is successful or not
 *        (200 means success, otherwise unsuccessful)

 */
void updateLocationEx(double latitude, double longitude, double elevation, char* measuredDateTime,char* updatedDateTime, double accuracy, char* reqId)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	int rc = -1;
	char uuid_str[40];
	generateUUID(uuid_str);
	strcpy(currentRequestID,uuid_str);

	char data[500];
	sprintf(data,"{\"d\":{\"longitude\":%f,\"latitude\":%f,\"elevation\":%f,\"measuredDateTime\":\"%s\",\"updatedDateTime\":\"%s\",\"accuracy\":%f},\"reqId\":\"%s\"}",
	        latitude, longitude, elevation, updatedDateTime, updatedDateTime, accuracy, currentRequestID);

	rc = publish(UPDATE_LOCATION, data);

	if(rc == SUCCESS){
		strcpy(reqId, uuid_str);

		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("reqId = %s",reqId);
		LOG(logHdr,logStr);
	}

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("rc = %d",rc);
	LOG(logHdr,logStr);
	LOG(logHdr,"exit::");
}

/**
 * Adds the current errorcode to IBM Watson IoT Platform.
 *
 * @param errorCode The "errorCode" is a current device error code that
 * needs to be added to the Watson IoT Platform.
 *
 * @param reqId Function returns the reqId if the addErrorCode request is successful.
 *
 * @return code indicating whether the update is successful or not
 *        (200 means success, otherwise unsuccessful)
 */
void addErrorCode(int errNum, char* reqId)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	char uuid_str[40];
	generateUUID(uuid_str);
	strcpy(currentRequestID,uuid_str);
	int rc = -1;
	char data[125];
	sprintf(data,"{\"d\":{\"errorCode\":%d},\"reqId\":\"%s\"}", errNum, uuid_str);

	rc = publish(CREATE_DIAG_ERRCODES, data);
	if(rc == SUCCESS){
		strcpy(reqId, uuid_str);

		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("reqId = %s",reqId);
		LOG(logHdr,logStr);
	}

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("rc = %d",rc);
	LOG(logHdr,logStr);
	LOG(logHdr,"exit::");
}

/**
 * Clear the Error Codes from IBM Watson IoT Platform for this device
 *
 * @param reqId Function returns the reqId if the clearErrorCodes request is successful.
 *
 * @return code indicating whether the clear operation is successful or not
 *        (200 means success, otherwise unsuccessful)
 */
void clearErrorCodes(char* reqId)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	char uuid_str[40];
	int rc = -1;
	generateUUID(uuid_str);
	strcpy(currentRequestID,uuid_str);

	char data[125];
	sprintf(data,"{\"reqId\":\"%s\"}", uuid_str);

	rc = publish(CLEAR_DIAG_ERRCODES, data);
	if(rc == SUCCESS){
		strcpy(reqId, uuid_str);

		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("reqId = %s",reqId);
		LOG(logHdr,logStr);
	}

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("rc = %d",rc);
	LOG(logHdr,logStr);
	LOG(logHdr,"exit::");
}

/**
 * The Log message that needs to be added to the Watson IoT Platform.
 *
 * @param message The Log message that needs to be added to the Watson IoT Platform.
 *
 * @param timestamp The Log timestamp
 *
 * @param data The optional diagnostic string data
 *
 * @param severity The Log severity
 *
 * @param reqId Function returns the reqId if the addLog request is successful.
 *
 * @return code indicating whether the update is successful or not
 *        (200 means success, otherwise unsuccessful)
 */
void addLog(char* message, char* data ,int severity, char* reqId)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	char uuid_str[40];
	int rc = -1;
	generateUUID(uuid_str);
	strcpy(currentRequestID,uuid_str);

        LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
        LOG_STR("currentRequestID = %s",currentRequestID);
        LOG(logHdr,logStr);

	time_t t = 0;
	char updatedDateTime[50];//"2016-03-01T07:07:56.323Z"
	strftime(updatedDateTime, sizeof(updatedDateTime), "%Y-%m-%dT%TZ", localtime(&t));
	char payload[125];
	sprintf(payload,"{\"d\":{\"message\":\"%s\",\"timestamp\":\"%s\",\"data\":\"%s\",\"severity\":%d},\"reqId\":\"%s\"}",message,updatedDateTime,data,severity, uuid_str);

        LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
        LOG_STR("payload = %s",payload);
        LOG(logHdr,logStr);

	rc = publish(ADD_DIAG_LOG, payload );
	if(rc == SUCCESS){
		strcpy(reqId, uuid_str);

		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("reqId = %s",reqId);
		LOG(logHdr,logStr);
	}

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("rc = %d",rc);
	LOG(logHdr,logStr);
	LOG(logHdr,"exit::");
}

/**
 * Clear the Logs from IBM Watson IoT Platform for this device
 *
 * @param reqId Function returns the reqId if the clearLogs request is successful.
 *
 * @return code indicating whether the clear operation is successful or not
 *        (200 means success, otherwise unsuccessful)
 */
void clearLogs(char* reqId){
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	char uuid_str[40];
	int rc = -1;
	generateUUID(uuid_str);
	strcpy(currentRequestID,uuid_str);

	char data[125];
	sprintf(data,"{\"reqId\":\"%s\"}", uuid_str);

	rc = publish(CLEAR_DIAG_LOG, data);
	if(rc == SUCCESS){
		strcpy(reqId, uuid_str);

		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("reqId = %s",reqId);
		LOG(logHdr,logStr);
	}

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("rc = %d",rc);
	LOG(logHdr,logStr);
	LOG(logHdr,"exit::");
}

/**
 * Notifies the IBM Watson IoT Platform response for action
 *
 * @param reqId request Id of the request that is received from the IBM Watson IoT Platform
 *
 * @param state state of the request that is request received from the IBM Watson IoT Platform
 *
 * @return int return code
 *
 */
int changeState(int rc)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	char response[100];
	char msg[100] ;
	getMessageFromReturnCode(rc,msg);
	sprintf(response, "{\"rc\":\"%d\",\"message\":\"%s\",\"reqId\":\"%s\"}",rc,msg,currentRequestID);
	int res = publishActionResponse(RESPONSE, response);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("publishActionResponse = %d",res);
	LOG(logHdr,logStr);
	LOG(logHdr,"exit::");

	return res;
}

/**
 * Update the firmware state while downloading firmware and
 * Notifies the IBM Watson IoT Platform with the updated state
 *
 * @param state Download state update received from the device
 *
 * @return int return code
 *
 */
int changeFirmwareState(int state)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	char firmwareMsg[300];
	int rc = -1;
	if (dmClient.bObserve) {
		dmClient.DeviceData.mgmt.firmware.state = state;
		sprintf(firmwareMsg,
				"{\"d\":{\"fields\":[{\"field\" : \"mgmt.firmware\",\"value\":{\"state\":%d}}]}}",
				state);
		rc = publishActionResponse(NOTIFY, firmwareMsg);

		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("publishActionResponse = %d",rc);
		LOG(logHdr,logStr);
	} else{
		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("mgmt.firmware is not in observe state");
		LOG(logHdr,logStr);
	}

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("rc = %d",rc);
	LOG(logHdr,logStr);
	LOG(logHdr,"exit::");

	return rc;
}

/**
 * Update the firmware update state while updating firmware and
 * Notifies the IBM Watson IoT Platform with the updated state
 *
 * @param state Update state received from the device while updating the Firmware
 *
 * @return int return code
 *
 */
int changeFirmwareUpdateState(int state)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	char firmwareMsg[300];
	int rc = -1;
	if (dmClient.bObserve) {
		dmClient.DeviceData.mgmt.firmware.updateStatus = state;
		sprintf(firmwareMsg,
				"{\"d\":{\"fields\":[{\"field\" : \"mgmt.firmware\",\"value\":{\"state\":%d,\"updateStatus\":%d}}]}}",
				dmClient.DeviceData.mgmt.firmware.state,state);
		rc = publishActionResponse(NOTIFY, firmwareMsg);

		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("publishActionResponse = %d",rc);
		LOG(logHdr,logStr);
	} else{
		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("mgmt.firmware is not in observe state");
		LOG(logHdr,logStr);
	}

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("rc = %d",rc);
	LOG(logHdr,logStr);
	LOG(logHdr,"exit::");

	return rc;
}

// Utility function to publish the message to Watson IoT
int publish(char* publishTopic, char* data)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	int rc = -1;
	MQTTMessage pub;
	pub.qos = QOS1;
	pub.retained = '0';
	pub.payload = data;
	pub.payloadlen = strlen(data);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("Topic - %s Payload - %s",publishTopic,data);
	LOG(logHdr,logStr);

	//signal(SIGINT, sigHandler);
	//signal(SIGTERM, sigHandler);
	interrupt =0;
	while(!interrupt)
	{
		rc = MQTTPublish(&dmClient.deviceClient.c, publishTopic , &pub);

		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("RC from MQTTPublish = %d",rc);
		LOG(logHdr,logStr);

		if(rc == SUCCESS) {
			rc = yield(&dmClient.deviceClient, 100);
		}
		if(!interrupt)
			osDelay(2000U);
	}

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("rc = %d",rc);
	LOG(logHdr,logStr);
	LOG(logHdr,"exit::");

	return rc;
}

//Publish actions response to IoTF platform
int publishActionResponse(char* publishTopic, char* data)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	int rc = -1;
	MQTTMessage pub;
	pub.qos = QOS1;
	pub.retained = '0';
	pub.payload = data;
	pub.payloadlen = strlen(data);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("Topic - %s Payload - %s",publishTopic,data);
	LOG(logHdr,logStr);


	rc = MQTTPublish(&dmClient.deviceClient.c, publishTopic , &pub);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("RC from MQTTPublish = %d",rc);
	LOG(logHdr,logStr);

	if(rc == SUCCESS) {
		rc = yield(&dmClient.deviceClient, 100);
	}

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("rc = %d",rc);
	LOG(logHdr,logStr);
	LOG(logHdr,"exit::");

	return rc;
}

//Utility for LocationUpdate Handler
void updateLocationHandler(double latitude, double longitude, double elevation, char* measuredDateTime,char* updatedDateTime, double accuracy)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

        int rc = -1;
        char data[500];
        sprintf(data,"{\"d\":{\"longitude\":%f,\"latitude\":%f,\"elevation\":%f,\"measuredDateTime\":\"%s\",\"updatedDateTime\":\"%s\",\"accuracy\":%f},\"reqId\":\"%s\"}",
	        latitude, longitude, elevation, updatedDateTime, updatedDateTime, accuracy, currentRequestID);

        rc = publish(UPDATE_LOCATION, data);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("rc = %d",rc);
	LOG(logHdr,logStr);
	LOG(logHdr,"exit::");
}

// Utility function to generate Unique Identifier
void generateUUID(char* uuid_str)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	char GUID[40];
	int t = 0;
	char *szTemp = "xxxxxxxx-xxxxy-4xxxx-yxxxy-xxxxxxxxxxxx";
	char *szHex = "0123456789ABCDEF-";
	int nLen = strlen (szTemp);

	for (t=0; t<nLen+1; t++)
	{
	    int r =( rand ())% 16;
	    char c = ' ';

	    switch (szTemp[t])
	    {
	        case 'x' : { c = szHex [r]; } break;
	        case 'y' : { c = szHex [((r & 0x03) | 0x08)]; } break;
	        case '-' : { c = '-'; } break;
	        case '4' : { c = '4'; } break;
	    }

	    GUID[t] = ( t < nLen ) ? c : 0x00;
	}
	strcpy(uuid_str , GUID);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("uuid_str = %s",uuid_str);
	LOG(logHdr,logStr);
	LOG(logHdr,"exit::");
}

// Utility function to get message from the return code
void getMessageFromReturnCode(int rc, char* msg)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	switch(rc)
	{
	case 202:
		strcpy(msg ,"Device action initiated immediately");
		break;
	case 500:
		strcpy(msg ,"Device action attempt fails");
		break;
	case 501:
		strcpy(msg, "Device action is not supported");
		break;
	}

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("msg = %s",msg);
	LOG(logHdr,logStr);
	LOG(logHdr,"exit::");
}

//Handler for all requests and responses from the server. This function routes the
//right handlers
void onMessage(MessageData* md)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	if (md) {
		//MQTTMessage* message = md->message;

		char *topic = malloc(md->topicName->lenstring.len + 1);

		sprintf(topic, "%.*s", md->topicName->lenstring.len,
				md->topicName->lenstring.data);

		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("onMessage topic = %s",topic);
		LOG(logHdr,logStr);

		if(!strcmp(topic,DMRESPONSE)){
			LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
			LOG_STR("Calling messageResponse from onMessage");
			LOG(logHdr,logStr);

			messageResponse(md);
		}

		if (!strcmp(topic, dmUpdate)) {
			LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
			LOG_STR("Calling messageUpdate from onMessage");
			LOG(logHdr,logStr);

			messageUpdate(md);
		}

		if (!strcmp(topic, dmObserve)) {
			LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
			LOG_STR("Calling messageObserve from onMessage");
			LOG(logHdr,logStr);

			messageObserve(md);
		}

		if (!strcmp(topic, dmCancel)) {
			LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
			LOG_STR("Calling messageCancel from onMessage");
			LOG(logHdr,logStr);

			messageCancel(md);
		}

		if (!strcmp(topic, dmReboot)) {
			LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
			LOG_STR("Calling messageForAction from onMessage");
			LOG(logHdr,logStr);

			messageForAction(md,1);
		}

		if (!strcmp(topic, dmFactoryReset)) {
			LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
			LOG_STR("Calling dmFactoryReset from onMessage");
			LOG(logHdr,logStr);

			messageForAction(md,0);
		}

		if (!strcmp(topic, dmFirmwareDownload)) {
			LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
			LOG_STR("Calling dmFirmwareDownload from onMessage");
			LOG(logHdr,logStr);

			messageFirmwareDownload(md);
		}

		if (!strcmp(topic, dmFirmwareUpdate)) {
			LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
			LOG_STR("Calling dmFirmwareUpdate from onMessage");
			LOG(logHdr,logStr);

			messageFirmwareUpdate(md);
		}

		free(topic);
	}

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG(logHdr,"exit::");
}

//Handler for Firmware Download request
void messageFirmwareDownload(MessageData* md)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	int rc = RESPONSE_ACCEPTED;
	//char msg[100];
	char respmsg[300];

	MQTTMessage* message = md->message;
	void *payload = message->payload;
	cJSON * jsonPayload = cJSON_Parse(payload);
	strcpy(currentRequestID, cJSON_GetObjectItem(jsonPayload, "reqId")->valuestring);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("messageFirmwareDownload with reqId:%s",currentRequestID);
	LOG(logHdr,logStr);

	if(dmClient.DeviceData.mgmt.firmware.state != FIRMWARESTATE_IDLE)
	{
		rc = BAD_REQUEST;

		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("Cannot download as the device is not in the idle state");
		LOG(logHdr,logStr);
	}
	else
	{
		rc = RESPONSE_ACCEPTED;

		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("Firmware Download Initiated");
		LOG(logHdr,logStr);
	}

	sprintf(respmsg,"{\"rc\":%d,\"reqId\":%s}",rc,currentRequestID);
	publishActionResponse(RESPONSE, respmsg);

	if(rc == RESPONSE_ACCEPTED){
		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("Calling Firmware Download callback");
		LOG(logHdr,logStr);

		(*cbFirmwareDownload)();
	}

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG(logHdr,"exit::");
}

//Handler for Firmware update request
void messageFirmwareUpdate(MessageData* md)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	int rc;
	char respmsg[300];

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("Update Firmware Request called, Firmware State: %d",
	        dmClient.DeviceData.mgmt.firmware.state);
	LOG(logHdr,logStr);

	if (dmClient.DeviceData.mgmt.firmware.state != FIRMWARE_DOWNLOADED) {
		rc = BAD_REQUEST;

		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("Firmware state is not in Downloaded state while updating");
		LOG(logHdr,logStr);

	} else {
		rc = RESPONSE_ACCEPTED;

		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("Firmware Update Initiated");
		LOG(logHdr,logStr);
	}

	sprintf(respmsg, "{\"rc\":%d,\"reqId\":%s}", rc, currentRequestID);
	publishActionResponse(RESPONSE, respmsg);

	if(rc == RESPONSE_ACCEPTED){
		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("Calling Firmware Update callback");
		LOG(logHdr,logStr);

		(*cbFirmwareUpdate)();
	}

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG(logHdr,"exit::");
}

//Handler for Observe request
void messageObserve(MessageData* md)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	int i = 0;
	MQTTMessage* message = md->message;
	void *payload = message->payload;
	char* respMsg;
	cJSON *resPayload, *resd, *resFields;
	resPayload = cJSON_CreateObject();
	cJSON_AddItemToObject(resPayload, "rc",
			cJSON_CreateNumber(RESPONSE_SUCCESS));
	cJSON * jsonPayload = cJSON_Parse(payload);
	cJSON* jreqId = cJSON_GetObjectItem(jsonPayload, "reqId");
	strcpy(currentRequestID, jreqId->valuestring);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("Observe reqId: %s", currentRequestID);

	cJSON_AddItemToObject(resPayload, "reqId",
			cJSON_CreateString(currentRequestID));

	cJSON_AddItemToObject(resPayload, "d", resd = cJSON_CreateObject());

	cJSON_AddItemToObject(resd, "fields", resFields =
			cJSON_CreateArray());

	cJSON *d = cJSON_GetObjectItem(jsonPayload, "d");

	cJSON *fields = cJSON_GetObjectItem(d, "fields");

	//cJSON *fields = cJSON_GetObjectItem(jsonPayload,"fields");
	for (i = 0; i < cJSON_GetArraySize(fields); i++) {
		cJSON * field = cJSON_GetArrayItem(fields, i);

		cJSON* fieldName = cJSON_GetObjectItem(field, "field");

		cJSON * value = cJSON_GetArrayItem(fields, i);

		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("Observe called for fieldName:%s", fieldName->valuestring);

		if (!strcmp(fieldName->valuestring, "mgmt.firmware")) {
			dmClient.bObserve = true;
			cJSON* resValue;
			cJSON* resField = cJSON_CreateObject();
			cJSON_AddItemToObject(resField, "field", cJSON_CreateString("mgmt.firmware"));
			cJSON_AddItemToObject(resField, "value", resValue = cJSON_CreateObject());
			cJSON_AddItemToObject(resValue, "state",cJSON_CreateNumber(dmClient.DeviceData.mgmt.firmware.state));
			cJSON_AddItemToObject(resValue, "updateStatus",cJSON_CreateNumber(dmClient.DeviceData.mgmt.firmware.updateStatus));
			cJSON_AddItemToArray(resFields,resField);

		}
	}
	respMsg = cJSON_Print(resPayload);
	cJSON_Delete(resPayload);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("Response Message:%s", respMsg);

	//Publish the response to the IoTF
	publishActionResponse(RESPONSE, respMsg);

	cJSON_Delete(jsonPayload);
	free(respMsg);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG(logHdr,"exit::");
}

//Handler for cancel observation request
void messageCancel(MessageData* md)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	int i = 0;
	char respMsg[100];
	MQTTMessage* message = md->message;
	void *payload = message->payload;
	cJSON * jsonPayload = cJSON_Parse(payload);
	cJSON* jreqId = cJSON_GetObjectItem(jsonPayload, "reqId");
	strcpy(currentRequestID, jreqId->valuestring);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("Cancel reqId: %s", currentRequestID);

	cJSON *d = cJSON_GetObjectItem(jsonPayload, "d");
	cJSON *fields = cJSON_GetObjectItem(d, "fields");
	//cJSON *fields = cJSON_GetObjectItem(jsonPayload,"fields");
	for (i = 0; i < cJSON_GetArraySize(fields); i++) {
		cJSON * field = cJSON_GetArrayItem(fields, i);
		cJSON* fieldName = cJSON_GetObjectItem(field, "field");

		cJSON * value = cJSON_GetArrayItem(fields, i);

		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("Cancel called for fieldName:%s", fieldName->valuestring);

		if (!strcmp(fieldName->valuestring, "mgmt.firmware")) {
			dmClient.bObserve = false;
			sprintf(respMsg,"{\"rc\":%d,\"reqId\":%s}",RESPONSE_SUCCESS,currentRequestID);

			LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
			LOG_STR("Response Message:%s", respMsg);

			//Publish the response to the IoTF
			publishActionResponse(RESPONSE, respMsg);
		}
	}

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG(logHdr,"exit::");
}

//Handler for update location request
void updateLocationRequest(cJSON* value)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	double latitude, longitude, elevation,accuracy;
	char* measuredDateTime;
	char* updatedDateTime;

	latitude = cJSON_GetObjectItem(value,"latitude")->valuedouble;
	longitude = cJSON_GetObjectItem(value,"longitude")->valuedouble;
	elevation = cJSON_GetObjectItem(value,"elevation")->valuedouble;
	accuracy = cJSON_GetObjectItem(value,"accuracy")->valuedouble;
	measuredDateTime = cJSON_GetObjectItem(value,"measuredDateTime")->valuestring;
	updatedDateTime = cJSON_GetObjectItem(value,"updatedDateTime")->valuestring;

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("Calling updateLocationHandler");

	updateLocationHandler(latitude, longitude, elevation,measuredDateTime,updatedDateTime,accuracy);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG(logHdr,"exit::");
}

//Handler for update Firmware request
void updateFirmwareRequest(cJSON* value) {
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	char response[100];

	strcpy(dmClient.DeviceData.mgmt.firmware.version,
			cJSON_GetObjectItem(value, "version")->valuestring);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("Firmware Version: %s",dmClient.DeviceData.mgmt.firmware.version);
	LOG(logHdr,logStr);

	strcpy(dmClient.DeviceData.mgmt.firmware.name,
			cJSON_GetObjectItem(value, "name")->valuestring);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("Name: %s",dmClient.DeviceData.mgmt.firmware.name);
	LOG(logHdr,logStr);

	strcpy(dmClient.DeviceData.mgmt.firmware.url,
			cJSON_GetObjectItem(value, "uri")->valuestring);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("URI: %s",dmClient.DeviceData.mgmt.firmware.url);
	LOG(logHdr,logStr);

	strcpy(dmClient.DeviceData.mgmt.firmware.verifier,
			cJSON_GetObjectItem(value, "verifier")->valuestring);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("Verifier: %s",dmClient.DeviceData.mgmt.firmware.verifier);
	LOG(logHdr,logStr);

	dmClient.DeviceData.mgmt.firmware.state = cJSON_GetObjectItem(value,"state")->valueint;

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("State: %d",dmClient.DeviceData.mgmt.firmware.state);
	LOG(logHdr,logStr);

	dmClient.DeviceData.mgmt.firmware.updateStatus = cJSON_GetObjectItem(value,"updateStatus")->valueint;

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("updateStatus: %d",dmClient.DeviceData.mgmt.firmware.updateStatus);
	LOG(logHdr,logStr);

	strcpy(dmClient.DeviceData.mgmt.firmware.updatedDateTime,
			cJSON_GetObjectItem(value, "updatedDateTime")->valuestring);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("updatedDateTime: %s",dmClient.DeviceData.mgmt.firmware.updatedDateTime);
	LOG(logHdr,logStr);

	sprintf(response, "{\"rc\":%d,\"reqId\":\"%s\"}", UPDATE_SUCCESS,
			currentRequestID);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("Response: %s",response);
	LOG(logHdr,logStr);

	publishActionResponse(RESPONSE, response);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG(logHdr,"exit::");
}

//Handler for update request from the server.
//It receives all the update requests like location, mgmt.firmware
//Currently only location and firmware updates are supported.
void messageUpdate(MessageData* md)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	int i = 0;
	MQTTMessage* message = md->message;
	void *payload = message->payload;
	cJSON * jsonPayload = cJSON_Parse(payload);
	if (jsonPayload) {
		cJSON* jreqId = cJSON_GetObjectItem(jsonPayload, "reqId");
		strcpy(currentRequestID, jreqId->valuestring);

		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("Update reqId: %s",currentRequestID);
		LOG(logHdr,logStr);

		cJSON *d = cJSON_GetObjectItem(jsonPayload, "d");
		cJSON *fields = cJSON_GetObjectItem(d, "fields");

		for (i = 0; i < cJSON_GetArraySize(fields); i++) {
			cJSON * field = cJSON_GetArrayItem(fields, i);
			cJSON* fieldName = cJSON_GetObjectItem(field, "field");

			LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
			LOG_STR("Update request received for fieldName: %s",fieldName->valuestring);
			LOG(logHdr,logStr);

			cJSON * value = cJSON_GetObjectItem(field, "value");

			if (!strcmp(fieldName->valuestring, "location")){
				LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
				LOG_STR("Calling updateLocationRequest");
				LOG(logHdr,logStr);

				updateLocationRequest(value);
			}
			else if (!strcmp(fieldName->valuestring, "mgmt.firmware")){
				LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
				LOG_STR("Calling updateFirmwareRequest");
				LOG(logHdr,logStr);

				updateFirmwareRequest(value);
			}
			else if (!strcmp(fieldName->valuestring, "metadata")){
				LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
				LOG_STR("METADATA not supported");
				LOG(logHdr,logStr);
			}
			else if (!strcmp(fieldName->valuestring, "deviceInfo")){
				LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
				LOG_STR("deviceInfo not supported");
				LOG(logHdr,logStr);
			}
			else{
				LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
				LOG_STR("Fieldname = %s",fieldName->valuestring);
				LOG(logHdr,logStr);
			}
		}
		cJSON_Delete(jsonPayload);//Needs to delete the parsed pointer
	} else{
		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("Error in parsing Json");
		LOG(logHdr,logStr);
	}

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG(logHdr,"exit::");
}

//Handler for responses from the server . Invoke the callback for the response.
//Callback needs to be invoked only if the request Id is matched. While yielding we
//receives the response for old request Ids from the platform. But we are interested only
//with the request Id action was initiated.
void messageResponse(MessageData* md)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	if(cb != 0) {
		MQTTMessage* message = md->message;
		void *payload = message->payload;
		int sz = message->payloadlen;
		char *pl = (char*) malloc(sizeof(char)*sz+1);
		strcpy(pl,message->payload);
		char *reqID;
		char *status;

		reqID = strtok(pl, ",");
		status= strtok(NULL, ",");

		reqID = strtok(reqID, ":\"");
		reqID = strtok(NULL, ":\"");
		reqID = strtok(NULL, ":\"");

		status= strtok(status, "}");
		status= strtok(status, ":");
		status= strtok(NULL, ":");

		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("Status: %s reqID: %s payload: %s",status,reqID,pl);
		LOG(logHdr,logStr);

		if(!strcmp(currentRequestID,reqID))
		{
			LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
			LOG_STR("%s == %s, Calling the callback",currentRequestID,reqID);
			LOG(logHdr,logStr);

			interrupt = 1;
			(*cb)(status, reqID, payload);
		}
		else
		{
			LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
			LOG_STR("%s != %s, Calling the callback",currentRequestID,reqID);
			LOG(logHdr,logStr);
		}
		free(pl);
	}

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG(logHdr,"exit::");
}

//Handler for Reboot and Factory reset action requests received from the platform.
//Invoke the respective callback for action.
void messageForAction(MessageData* md, bool isReboot)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	if(cbReboot != 0 ){

		MQTTMessage* message = md->message;

		char *topic = malloc(md->topicName->lenstring.len + 1);

		sprintf(topic, "%.*s", md->topicName->lenstring.len,
				md->topicName->lenstring.data);

		void *payload = message->payload;
		char *pl = (char*) malloc(sizeof(char)*message->payloadlen+1);
		strcpy(pl,message->payload);

		strtok(topic, "/");
		strtok(NULL, "/");

		strtok(NULL, "/");
		strtok(NULL, "/");
		char *action = strtok(NULL, "/");

		char *reqID;

		reqID = strtok(pl, ":\"");
		reqID = strtok(NULL, ":\"");
		reqID = strtok(NULL, ":\"");

		strcpy(currentRequestID,reqID);

		LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
		LOG_STR("reqId: %s action: %s payload: %s",reqID, action,pl);
		LOG(logHdr,logStr);

		if(isReboot){
			LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
			LOG_STR("Calling Reboot callback");
			LOG(logHdr,logStr);

			(*cbReboot)(reqID, action, payload);
		}
		else {
			LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
			LOG_STR("Calling Factory Reset callback");
			LOG(logHdr,logStr);

			(*cbFactoryReset)(reqID, action, payload);
		}

		free(topic);
		free(pl);
	}

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG(logHdr,"exit::");
}
