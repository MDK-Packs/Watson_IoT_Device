/*******************************************************************************
 * Copyright (c) 2017 IBM Corp.
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
 *    Lokesh Haralakatta  -  Initial implementation
 *                        -  Contains the device client specific functions
 *                        -  Added logging feature
 *******************************************************************************/

 #include "deviceclient.h"

 //Command Callback
 commandCallback cbDevice;

//Character strings to hold log header and log message to be dumped.
 extern char logHdr[LOG_BUF];
 extern char logStr[LOG_BUF];

 /**
 * Function used to Publish events from the device to the IBM Watson IoT service
 * @param eventType - Type of event to be published e.g status, gps
 * @param eventFormat - Format of the event e.g json
 * @param data - Payload of the event
 * @param QoS - qos for the publish event. Supported values : QOS0, QOS1, QOS2
 *
 * @return int return code from the publish
 */

 int publishEvent(iotfclient  *client, char *eventType, char *eventFormat, char* data, enum QoS qos)
 {
        LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

        int rc = -1;

 	char publishTopic[strlen(eventType) + strlen(eventFormat) + 16];

 	sprintf(publishTopic, "iot-2/evt/%s/fmt/%s", eventType, eventFormat);

        LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
        LOG_STR("Calling publishData to publish to topic - %s",publishTopic);
        LOG(logHdr,logStr);

 	rc = publishData(&(client->c),publishTopic,data,qos);

 	if(rc != SUCCESS) {
 		printf("\nConnection lost, retry the connection \n");
 		retry_connection(client);
 		rc = publishData(&(client->c),publishTopic,data,qos);
 	}

        LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
        LOG_STR("rc = %d",rc);
        LOG(logHdr,logStr);
        LOG(logHdr,"exit::");

 	return rc;

 }

 /**
 * Function used to subscribe to all device commands.
 *
 * @return int return code
 */
 int subscribeCommands(iotfclient  *client)
 {
        LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

        int rc = -1;

        LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
        LOG_STR("Calling MQTTSubscribe for subscribing to device commands");
        LOG(logHdr,logStr);

        rc = MQTTSubscribe(&client->c, "iot-2/cmd/+/fmt/+", QOS0, messageArrived);

        LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
        LOG_STR("RC from MQTTSubscribe - %d",rc);
        LOG(logHdr,logStr);
        LOG(logHdr,"exit::");

        return rc;
 }

 //Handler for all commands. Invoke the callback.
 void messageArrived(MessageData* md)
 {
        LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

 	if(cbDevice != 0) {
 		MQTTMessage* message = md->message;

 		char *topic = malloc(md->topicName->lenstring.len+1);

 		sprintf(topic,"%.*s",md->topicName->lenstring.len,md->topicName->lenstring.data);

 		void *payload = message->payload;

 		strtok(topic, "/");
 		strtok(NULL, "/");

 		char *commandName = strtok(NULL, "/");
 		strtok(NULL, "/");
 		char *format = strtok(NULL, "/");

                LOG_HDR("%s:%d:%s",__FILE__,__LINE__,__func__);
                LOG_STR("Calling registered callabck to process the arrived message");
                LOG(logHdr,logStr);

 		(*cbDevice)(commandName, format, payload);

 		free(topic);

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
 void setCommandHandler(iotfclient  *client, commandCallback handler)
 {
        LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

        cbDevice = handler;

        if(cbDevice != NULL){
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
