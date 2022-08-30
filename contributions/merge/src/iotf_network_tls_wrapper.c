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
 *    Lokesh Haralakatta  -  Added Logging Feature
 *******************************************************************************/

#define MBEDTLS_ALLOW_PRIVATE_ACCESS
#include "iotf_network_tls_wrapper.h"

//Character strings to hold log header and log message to be dumped.
extern char logHdr[LOG_BUF];
extern char logStr[LOG_BUF];

/** Function to initialize Network structure with default values and network functions
* @param - Address of Network Structure Variable
* @return - void
**/
void NewNetwork(Network* n)
{
       LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
       LOG(logHdr,"entry::");

       n->my_socket = 0;
       n->mqttread = network_read;
       n->mqttwrite = network_write;
       n->disconnect = network_disconnect;
       n->TLSConnectData.pServerCertLocation = NULL;
       n->TLSConnectData.pRootCACertLocation = NULL;
       n->TLSConnectData.pDeviceCertLocation = NULL;
       n->TLSConnectData.pDevicePrivateKeyLocation = NULL;
       n->TLSConnectData.pDestinationURL = NULL;

       LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
       LOG(logHdr,"exit::");
}

static uint32_t TickFreq;

/** Function to check whether the given timer is expired or not
* @param - Address of Timer
* @return - 0 or 1
**/
 char expired(Timer* timer)
 {
 	uint32_t tick, left;
 	tick = osKernelGetTickCount();
 	left = timer->end_time - tick;
 	return (left >= 0x80000000U);
 }

 /** Function to update timer with given timeout value in milliseconds
 * @param - Address of Timer and timeout in milliseconds
 * @return - void
 **/
 void countdown_ms(Timer* timer, unsigned int timeout)
 {
 	uint32_t tick;
 	tick = osKernelGetTickCount();
 	timer->end_time = tick + ((timeout * TickFreq) / 1000U);
 }

 /** Function to update timer with given timeout value in seconds
 * @param - Address of Timer and timeout in seconds
 * @return - void
 **/
 void countdown(Timer* timer, unsigned int timeout)
 {
 	uint32_t tick;
 	tick = osKernelGetTickCount();
 	timer->end_time = tick + (timeout * TickFreq);
 }

 /** Function to find and return left out time in Timer
 * @param - Address of Timer
 * @return - Left out time in milliseconds
 **/
 int left_ms(Timer* timer)
 {
 	uint32_t tick, left;
 	tick = osKernelGetTickCount();
 	left = timer->end_time - tick;
 	return (left >= 0x80000000U) ? 0U : (int)left;
 }

 /** Function to initialize the Timer Structure variable
 * @param - Address of Timer
 * @return - void
 **/
 void InitTimer(Timer* timer)
 {
 	TickFreq = osKernelGetTickFreq();
 	timer->end_time =  0U;
 }

 /** Function to establish connection to given host address and port without SSL/TLS
 * @param - Address of Network Structure
 *        - Host Address to connect
 *        - Port number to connect
 * @return - 0 for SUCCESS
 *         - -1 for FAILURE
 **/
 int ConnectNetwork(Network* n, char* addr, int port)
 {
        LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

 	unsigned char ip[4];
 	unsigned int  ip_len = sizeof(ip);
 	int rc;

 	rc = iotSocketGetHostByName(addr, IOT_SOCKET_AF_INET, &ip[0], &ip_len);
 	if (rc == 0)
 	{
                LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
                LOG_STR("%s","ADDR FAMILY: AF_INET");
                LOG(logHdr,logStr);
 	}

 	if (rc == 0)
 	{
 		n->my_socket = iotSocketCreate(IOT_SOCKET_AF_INET, IOT_SOCKET_SOCK_STREAM, 0);

                LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
                LOG_STR("Socket FD: %d",n->my_socket);
                LOG(logHdr,logStr);

 		if (n->my_socket >= 0)
 		{
 			rc = iotSocketConnect(n->my_socket, &ip[0], ip_len, (unsigned short)port);

                        LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
                        LOG_STR("RC from connect - %d :",rc);
                        LOG(logHdr,logStr);
 		}
 	}

        LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG_STR("rc = %d ",rc);
        LOG(logHdr,logStr);
        LOG(logHdr,"exit::");

 	return rc;
 }

 /** Function to read data from the socket opened into provided buffer
 * @param - Address of Network Structure
 *        - Buffer to store the data read from socket
 *        - Expected number of bytes to read from socket
 *        - Timeout in milliseconds
 * @return - Number of Bytes read on SUCCESS
 *         - -1 on FAILURE
 **/
 int network_read(Network* n, unsigned char* buffer, int len, int timeout_ms)
 {
        LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

 	int bytes = 0;
 	int rc;

 	if (timeout_ms == 0)
 	{
 		timeout_ms = 10;
 	}

 	iotSocketSetOpt(n->my_socket, IOT_SOCKET_SO_RCVTIMEO, &timeout_ms, sizeof(int));

 	while (bytes < len)
 	{
 		rc = iotSocketRecv(n->my_socket, &buffer[bytes], (uint32_t)(len - bytes));
 		if (rc < 0)
 		{
 			if (rc != IOT_SOCKET_EAGAIN)
 			{
                                LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
                                LOG_STR("network_read failed while calling recv with return code - %d\n",rc);
                                LOG(logHdr,logStr);
 				bytes = -1;
 			}
 			break;
 		}
 		else
 			bytes += rc;
 	}

        LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG_STR("bytes - %d ",bytes);
        LOG(logHdr,logStr);
        LOG(logHdr,"exit::");

 	return bytes;
 }

 /** Function to write data to the socket opened present in provided buffer
 * @param - Address of Network Structure
 *        - Buffer storing the data to write to socket
 *        - Number of bytes of data to write to socket
 *        - Timeout in milliseconds
 * @return - Number of Bytes written on SUCCESS
 *         - -1 on FAILURE
 **/
 int network_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
 {
        LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

 	int bytes = 0;
 	int rc;

 	iotSocketSetOpt(n->my_socket, IOT_SOCKET_SO_SNDTIMEO, &timeout_ms, sizeof(int));

 	while (bytes < len)
 	{
 		rc = iotSocketSend(n->my_socket, &buffer[bytes], (uint32_t)(len - bytes));
 		if (rc < 0)
 		{
 			if (rc != IOT_SOCKET_EAGAIN)
 			{
                                LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
                                LOG_STR("network_write failed while calling write with return code - %d\n",rc);
                                LOG(logHdr,logStr);
 				bytes = -1;
 			}
 			break;
 		}
 		else
 			bytes += rc;
 	}

        LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG_STR("bytes - %d ",bytes);
        LOG(logHdr,logStr);
        LOG(logHdr,"exit::");

 	return bytes;
 }

 /** Function used to close the opened socket for communication. If the given mode is quick start,
 it just closes the socket opened otherwise it calls teardown_tls function to cleanup mbedtls structures.
 * @param - Address of Network Structure
 *        - Connected Mode - Quickstart or Not
 * @return - void
 **/
 void network_disconnect(Network* n, int qsMode)
 {
        if(qsMode)
 	  iotSocketClose(n->my_socket);
        else
          teardown_tls(&n->TLSInitData,&n->TLSConnectData);
 }

 /** Function to initialize mbedtls structures. If useClientCerts flag is 1,
 * then certificate related structure gets initialized.
 * @param - Address of tls_init Structure
 *        - Flag to indicate whether to use client certificates or not
 * @return - 0 on SUCCESS
 *         - -1 on FAILURE
 **/
 int initialize_tls(tls_init_params *tlsInitParams, int useClientCerts){
        LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

        int rc=-1;
        mbedtls_net_init( &(tlsInitParams->server_fd) );
        mbedtls_ssl_init( &(tlsInitParams->ssl) );
        mbedtls_ssl_config_init( &(tlsInitParams->conf) );
        mbedtls_x509_crt_init( &(tlsInitParams->cacert) );
        mbedtls_ctr_drbg_init( &(tlsInitParams->ctr_drbg) );
        mbedtls_entropy_init( &(tlsInitParams->entropy) );
//      if(useClientCerts){
            mbedtls_x509_crt_init(&(tlsInitParams->clicert));
            mbedtls_pk_init(&(tlsInitParams->pkey));
//      }
        strcpy(tlsInitParams->clientName,"mbed_tls_client");

        if((rc = mbedtls_ctr_drbg_seed( &(tlsInitParams->ctr_drbg), mbedtls_entropy_func, &(tlsInitParams->entropy),
                            (const unsigned char *) tlsInitParams->clientName,
                            strlen( tlsInitParams->clientName) ) )!= 0){
                LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
                LOG_STR("mbedtls_ctr_drbg_seed failed with return code = 0x%x",rc);
                LOG(logHdr,logStr);
        }

        LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG_STR("rc = %d ",rc);
        LOG(logHdr,logStr);
        LOG(logHdr,"exit::");

        return rc;
 }

 #ifndef MBEDTLS_DEBUG_LEVEL
 #define MBEDTLS_DEBUG_LEVEL    0
 #endif

 /** Function to display SSL related debug messages from mbedtls library
 * @param - Address of Context
 *        - Debug level in the range 0 to 4
 *        - File Handle
 *        - Line number in the code
 *        - Message to display
 * @return - Number of Bytes written on SUCCESS
 *         - -1 on FAILURE
 **/
 void tls_debug( void *ctx, int level,
                      const char *file, int line, const char *str )
 {
    ((void) level);
    fprintf( (FILE *) ctx, "%s:%04d: %s", file, line, str );
    fflush(  (FILE *) ctx  );
 }

 /** Function to connect to given server using SSL/TLS secured connection. If useClientCerts Flag
 * is set, then it uses the specified Client Side Certificates for communication.
 * @param - Address of tls_init_larams structure
 *        - Address of tls_connect_params structure
 *        - Server Address
 *        - Port Number
 *        - whether to use client side certificates or not
 * @return - 1 on SUCCESS
 *         - -1 on FAILURE
 **/
 int tls_connect(tls_init_params *tlsInitData,tls_connect_params *tlsConnectData,
                const char *server, const int port, int useClientCerts){

        LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

        int rc=-1;
        char str_port[10];
        sprintf(str_port,"%d",port);

        mbedtls_debug_set_threshold(MBEDTLS_DEBUG_LEVEL);

        if((rc = initialize_tls(tlsInitData,useClientCerts))!=0)
        {
            LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
            LOG_STR("initialize_tls failed with return code = 0x%x",-rc);
            LOG(logHdr,logStr);
            goto exit;
        }
        if((rc = mbedtls_net_connect(&(tlsInitData->server_fd), server, str_port, MBEDTLS_NET_PROTO_TCP )) != 0){
            LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
            LOG_STR("mbedtls_net_connect failed with return code = 0x%x",-rc);
            LOG(logHdr,logStr);
            goto exit;
        }
        if((rc = mbedtls_net_set_block(&(tlsInitData->server_fd)))!=0){
            LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
            LOG_STR("mbedtls_net_set_block failed with return code = 0x%x",-rc);
            LOG(logHdr,logStr);
            goto exit;
        }

        if((rc = mbedtls_x509_crt_parse_file(&(tlsInitData->cacert),tlsConnectData->pServerCertLocation))!=0)
        {
            LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
            LOG_STR("mbedtls_x509_crt_parse_file failed for Server CA certificate with return code = 0x%x",-rc);
            LOG(logHdr,logStr);
            goto exit;
        }

        if(useClientCerts){
          if((rc = mbedtls_x509_crt_parse_file(&(tlsInitData->cacert),tlsConnectData->pRootCACertLocation))!=0)
          {
            LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
            LOG_STR("mbedtls_x509_crt_parse_file failed for Root CA certificate with return code = 0x%x",-rc);
            LOG(logHdr,logStr);
            goto exit;
          }
          if((rc = mbedtls_x509_crt_parse_file(&(tlsInitData->clicert), tlsConnectData->pDeviceCertLocation))!=0)
          {
            LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
            LOG_STR("mbedtls_x509_crt_parse_file failed for Device Certificate with return code = 0x%x",-rc);
            LOG(logHdr,logStr);
            goto exit;
          }
          if((rc = mbedtls_pk_parse_keyfile(&(tlsInitData->pkey), tlsConnectData->pDevicePrivateKeyLocation, "", mbedtls_ctr_drbg_random, &(tlsInitData->ctr_drbg)))!=0)
          {
            LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
            LOG_STR("mbedtls_pk_parse_keyfile failed for Device Private Key with return code = 0x%x",-rc);
            LOG(logHdr,logStr);
            goto exit;
          }
          if((rc = mbedtls_ssl_conf_own_cert(&(tlsInitData->conf), &(tlsInitData->clicert),
                       &(tlsInitData->pkey)))!=0)
          {
              LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
              LOG_STR("mbedtls_ssl_conf_own_cert failed with return code = 0x%x",-rc);
              LOG(logHdr,logStr);
              goto exit;
          }
        }
        if((rc = mbedtls_ssl_set_hostname( &(tlsInitData->ssl), tlsConnectData->pDestinationURL)) != 0 )
        {
                LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
                LOG_STR("mbedtls_ssl_set_hostname failed with rc = 0x%x",-rc);
                LOG(logHdr,logStr);
                goto exit;
        }
        if((rc = mbedtls_ssl_config_defaults(&(tlsInitData->conf),MBEDTLS_SSL_IS_CLIENT,
                      MBEDTLS_SSL_TRANSPORT_STREAM,MBEDTLS_SSL_PRESET_DEFAULT ))!=0)
        {
                LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
                LOG_STR("mbedtls_ssl_config_defaults failed with return code = 0x%x",-rc);
                LOG(logHdr,logStr);
                goto exit;
        }
        mbedtls_ssl_conf_max_version(&(tlsInitData->conf),MBEDTLS_SSL_MAJOR_VERSION_3,MBEDTLS_SSL_MINOR_VERSION_3);
        mbedtls_ssl_conf_min_version(&(tlsInitData->conf),MBEDTLS_SSL_MAJOR_VERSION_3,MBEDTLS_SSL_MINOR_VERSION_3);
        mbedtls_ssl_conf_authmode(&(tlsInitData->conf), MBEDTLS_SSL_VERIFY_REQUIRED);
        mbedtls_ssl_conf_ca_chain(&(tlsInitData->conf), &(tlsInitData->cacert), NULL);
        mbedtls_ssl_conf_rng(&(tlsInitData->conf), mbedtls_ctr_drbg_random, &(tlsInitData->ctr_drbg));
        mbedtls_ssl_conf_dbg( &(tlsInitData->conf), tls_debug, stdout );
        mbedtls_ssl_set_bio(&(tlsInitData->ssl), &(tlsInitData->server_fd), mbedtls_net_send, NULL, mbedtls_net_recv_timeout);
        if((rc = mbedtls_ssl_setup(&(tlsInitData->ssl), &(tlsInitData->conf))) != 0 )
        {
                LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
                LOG_STR("mbedtls_ssl_setup failed with rc = 0x%x",-rc);
                LOG(logHdr,logStr);
                goto exit;
        }
        while((rc = mbedtls_ssl_handshake(&(tlsInitData->ssl))) != 0 )
        {
           if( rc != MBEDTLS_ERR_SSL_WANT_READ && rc != MBEDTLS_ERR_SSL_WANT_WRITE )
           {
                LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
                LOG_STR("mbedtls_ssl_handshake failed with rc = 0x%x",-rc);
                LOG(logHdr,logStr);
                if(rc == MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED){
                  LOG_STR("ssl_handshake failed with MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED");
                  LOG(logHdr,logStr);
                }
                LOG_STR("ssl state = %d",tlsInitData->ssl.state);
                LOG(logHdr,logStr);
                LOG_STR("ssl version = %s",mbedtls_ssl_get_version(&(tlsInitData->ssl)));
                LOG(logHdr,logStr);
                break;
            }
        }
  exit:
        LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG_STR("rc = %d ",rc);
        LOG(logHdr,logStr);
        LOG(logHdr,"exit::");

        return rc;
 }

 /** Function to read data from the secured tls socket
 * @param - Address of Network Structure
 *        - Buffer to store the read data from the secured socket
 *        - Number of bytes of data to read from secured socket
 *        - Timeout in milliseconds
 * @return - Number of Bytes read on SUCCESS
 *         - -1 on FAILURE
 **/
 int tls_read(Network* n, unsigned char* buffer, int len, int timeout_ms)
 {
        LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

        tls_init_params *tlsInitData = &(n->TLSInitData);
        int rc;
 	int bytes = 0;
 	if (timeout_ms == 0)
 	{
 		timeout_ms = 10;
 	}
 	mbedtls_ssl_conf_read_timeout(&(tlsInitData->conf), timeout_ms);
 	while (bytes < len)
 	{
 		rc = mbedtls_ssl_read(&(tlsInitData->ssl), &buffer[bytes], len - bytes);
 		if (rc < 0)
 		{
 			if ((rc != MBEDTLS_ERR_SSL_WANT_READ) && (rc != MBEDTLS_ERR_SSL_WANT_WRITE) && (rc != MBEDTLS_ERR_SSL_TIMEOUT))
 			{
                                LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
                                LOG_STR("mbedtls_ssl_read failed with rc = %d",rc);
                                LOG(logHdr,logStr);
 				bytes = -1;
 			}
 			break;
 		}
 		else if (rc == 0)
 		{
 			bytes = 0;
 			break;
 		}
 		else
 			bytes += rc;
 	}

        LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG_STR("bytes - %d ",bytes);
        LOG(logHdr,logStr);
        LOG(logHdr,"exit::");

 	return bytes;
 }

 /** Function to write data to the secured tls socket
 * @param - Address of Network Structure
 *        - Buffer storing the data to write to the secured socket
 *        - Number of bytes of data to write to secured socket
 *        - Timeout in milliseconds
 * @return - Number of Bytes written on SUCCESS
 *         - -1 on FAILURE
 **/
 int tls_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
 {
        LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

        Timer timer;
        tls_init_params *tlsInitData = &(n->TLSInitData);
 	int rc = -1;
 	int bytes;
        countdown_ms(&timer, (unsigned int)timeout_ms);
        for (bytes = 0; bytes < len; bytes += rc)
        {
                while (!expired(&timer) &&
                       ((rc = mbedtls_ssl_write(&(tlsInitData->ssl), &buffer[bytes], len - bytes)) <= 0))
                {
                        if((rc != MBEDTLS_ERR_SSL_WANT_READ) && (rc != MBEDTLS_ERR_SSL_WANT_WRITE))
                        {
                                LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
                                LOG_STR("mbedtls_ssl_write failed with rc = %d",rc);
                                LOG(logHdr,logStr);
                                break;
                        }
                }
                if (rc <= 0)
                        break;
        }

        LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG_STR("rc = %d ",rc);
        LOG(logHdr,logStr);
        LOG(logHdr,"exit::");

 	return rc;
 }

 /** Function to clear off the mbedtls structures.
 * @param - Address of tls_init Structure
 *        - Address of tls_connect Structure
 * @return - void
 **/
void teardown_tls(tls_init_params *tlsInitParams,tls_connect_params* tlsConnectData){
        LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

        mbedtls_net_free( &(tlsInitParams->server_fd) );
        mbedtls_ssl_free( &(tlsInitParams->ssl) );
        mbedtls_ssl_config_free( &(tlsInitParams->conf) );
        mbedtls_ctr_drbg_free( &(tlsInitParams->ctr_drbg) );
        mbedtls_entropy_free( &(tlsInitParams->entropy) );
        mbedtls_x509_crt_free( &(tlsInitParams->cacert) );
        mbedtls_x509_crt_free( &(tlsInitParams->clicert) );
        mbedtls_pk_free(&(tlsInitParams->pkey) );

        freeTLSConnectData(tlsConnectData);

        LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"exit::");
 }

 /** Function to clear off the memory allocated for certificates location.
 * @param - Address of tls_connect Structure
 * @return - void
 **/
void freeTLSConnectData(tls_connect_params* tlsConnectData){
        LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

        freePtr(tlsConnectData->pServerCertLocation);
        freePtr(tlsConnectData->pRootCACertLocation);
        freePtr(tlsConnectData->pDeviceCertLocation);
        freePtr(tlsConnectData->pDevicePrivateKeyLocation);
        freePtr(tlsConnectData->pDestinationURL);

        LOG_HDR("%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"exit::");
}
