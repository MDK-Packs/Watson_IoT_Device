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
 *    Jeffrey Dare            - initial implementation
 *    Lokesh Haralakatta      - Added SSL/TLS support
 *    Lokesh Haralakatta      - Added Client Side Certificates support
 *    Lokesh Haralakatta      - Separated out device client and gateway client specific code.
 *                            - Retained gateway specific code here.
 *                            - Added logging feature
 *******************************************************************************/

#include "gatewayclient.h"

//Command Callback
commandCallback cbGateway;

//Character strings to hold log header and log message to be dumped.
extern char logHdr[LOG_BUF];
extern char logStr[LOG_BUF];

//Subscription details storage
char* subscribeTopics[5];
int subscribeCount = 0;

/**
* Function used to Publish events from the device to the Watson IoT
* @param client - Reference to the GatewayClient
* @param deviceType - The type of your device
* @param deviceId - The ID of your deviceId
* @param eventType - Type of event to be published e.g status, gps
* @param eventFormat - Format of the event e.g json
* @param data - Payload of the event
* @param QoS - qos for the publish event. Supported values : QOS0, QOS1, QOS2
*
* @return int return code from the publish
*/
int publishDeviceEvent(iotfclient  *client, char *deviceType, char *deviceId, char *eventType, char *eventFormat, char* data, enum QoS qos)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	int rc = -1;

	char publishTopic[strlen(eventType) + strlen(eventFormat) + strlen(deviceType) + strlen(deviceId)+25];

	sprintf(publishTopic, "iot-2/type/%s/id/%s/evt/%s/fmt/%s", deviceType, deviceId, eventType, eventFormat);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
        LOG_STR("Calling publishData to publish to topic - %s",publishTopic);
        LOG(logHdr,logStr);

	rc = publishData(&client->c, publishTopic , data, qos);

	if(rc != SUCCESS) {
		printf("connection lost.. %d \n",rc);
		retry_connection(client);
		rc = publishData(&client->c, publishTopic, data, qos);
	}

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
        LOG_STR("rc = %d",rc);
        LOG(logHdr,logStr);
        LOG(logHdr,"exit::");

	return rc;

}

/**
* Function used to Publish events from the device to the Watson IoT
* @param client - Reference to the GatewayClient
* @param eventType - Type of event to be published e.g status, gps
* @param eventFormat - Format of the event e.g json
* @param data - Payload of the event
* @param QoS - qos for the publish event. Supported values : QOS0, QOS1, QOS2
*
* @return int return code from the publish
*/
int publishGatewayEvent(iotfclient  *client, char *eventType, char *eventFormat, char* data, enum QoS qos)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	int rc = -1;

	char publishTopic[strlen(eventType) + strlen(eventFormat) + strlen(client->cfg.id) + strlen(client->cfg.type)+25];

	sprintf(publishTopic, "iot-2/type/%s/id/%s/evt/%s/fmt/%s", client->cfg.type, client->cfg.id, eventType, eventFormat);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
        LOG_STR("Calling publishData to publish to topic - %s",publishTopic);
        LOG(logHdr,logStr);

	rc = publishData(&client->c, publishTopic , data, qos);

	if(rc != SUCCESS) {
		printf("connection lost.. \n");
		retry_connection(client);
		rc = publishData(&client->c, publishTopic , data, qos);
	}

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
        LOG_STR("rc = %d",rc);
        LOG(logHdr,logStr);
        LOG(logHdr,"exit::");

	return rc;

}

/**
* Function used to subscribe to all commands for gateway.
*
* @return int return code
*/
int subscribeToGatewayCommands(iotfclient  *client)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	int rc = -1;

	char* subscribeTopic = NULL;

	subscribeTopic = (char*) malloc(strlen(client->cfg.id) + strlen(client->cfg.type) + 28);

	sprintf(subscribeTopic, "iot-2/type/%s/id/%s/cmd/+/fmt/+", client->cfg.type, client->cfg.id);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
        LOG_STR("Calling MQTTSubscribe for subscribing to gateway commands");
        LOG(logHdr,logStr);

	rc = MQTTSubscribe(&client->c, subscribeTopic, QOS2, gatewayMessageArrived);

	subscribeTopics[subscribeCount++] = subscribeTopic;

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
        LOG_STR("RC from MQTTSubscribe - %d",rc);
        LOG(logHdr,logStr);
        LOG(logHdr,"exit::");

	return rc;
}

/**
* Function used to subscribe to device commands for gateway.
*
* @return int return code
*/
int subscribeToDeviceCommands(iotfclient  *client, char* deviceType, char* deviceId, char* command, char* format, int qos)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	int rc = -1;

	char* subscribeTopic = NULL;

	subscribeTopic = (char*)malloc(strlen(deviceType) + strlen(deviceId) + strlen(command) + strlen(format) + 26);

	sprintf(subscribeTopic, "iot-2/type/%s/id/%s/cmd/%s/fmt/%s", deviceType, deviceId, command, format);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
        LOG_STR("Calling MQTTSubscribe for subscribing to device commands");
        LOG(logHdr,logStr);

	rc = MQTTSubscribe(&client->c, subscribeTopic, (enum QoS)qos, gatewayMessageArrived);

	subscribeTopics[subscribeCount++] = subscribeTopic;

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
        LOG_STR("RC from MQTTSubscribe - %d",rc);
        LOG(logHdr,logStr);
        LOG(logHdr,"exit::");

	return rc;
}

/**
* Function used to disconnect from the IoTF service
*
* @return int return code
*/

int disconnectGateway(iotfclient  *client)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	int rc = 0;
	int count;

	//Disconnect from IoT Service
	rc = disconnect(client);

	//free memory for subscriptions
	for(count = 0; count < subscribeCount ; count++)
		free(subscribeTopics[count]);

	LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	LOG_STR("RC from iotf disconnect function - %d",rc);
	LOG(logHdr,logStr);
	LOG(logHdr,"exit::");

	return rc;
}

//Handler for all commands. Invoke the callback.
void gatewayMessageArrived(MessageData* md)
{
       LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
       LOG(logHdr,"entry::");

       if(cbGateway != 0) {
	       MQTTMessage* message = md->message;

	       char *topic = malloc(md->topicName->lenstring.len+1);

	       sprintf(topic,"%.*s",md->topicName->lenstring.len,md->topicName->lenstring.data);

	       void *payload = message->payload;

	       size_t payloadlen = message->payloadlen;

	       strtok(topic, "/");
	       strtok(NULL, "/");

	       char *type = strtok(NULL, "/");
	       strtok(NULL, "/");
	       char *id = strtok(NULL, "/");
	       strtok(NULL, "/");
	       char *commandName = strtok(NULL, "/");
	       strtok(NULL, "/");
	       char *format = strtok(NULL, "/");

	       free(topic);

	       LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	       LOG_STR("Calling registered callabck to process the arrived message");
	       LOG(logHdr,logStr);

	       (*cbGateway)(type,id,commandName, format, payload,payloadlen);
       }
       else{
	       LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
	       LOG_STR("No registered callback function to process the arrived message");
	       LOG(logHdr,logStr);
       }

       LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
       LOG_STR("Returning from %s",__func__);
       LOG(logHdr,logStr);
       LOG(logHdr,"exit::");
}

/**
* Function used to set the Command Callback function. This must be set if you to recieve commands.
*
* @param cb - A Function pointer to the commandCallback. Its signature - void (*commandCallback)(char* commandName, char* payload)
* @return int return code
*/
void setGatewayCommandHandler(iotfclient *client, commandCallback handler)
{
	LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

	cbGateway = handler;

	if(cbGateway != NULL){
                LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
                LOG_STR("Registered callabck to process the arrived message");
                LOG(logHdr,logStr);
        }
        else{
                LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
                LOG_STR("Callabck not registered to process the arrived message");
                LOG(logHdr,logStr);
        }

        LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
        LOG_STR("Returning from %s",__func__);
        LOG(logHdr,logStr);
        LOG(logHdr,"exit::");
}
