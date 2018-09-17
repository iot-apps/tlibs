/*
 * at.h
 *
 * Created: 28/05/2018 20:52:30
 * Author: Jonas
 */ 

#ifndef NATIVE_MQTT 

#include <asf.h>
#include <string.h>

#ifdef GSM_XE910
#include "gsmXE910.h"
#endif
#include "wifiGs2101m.h"
#include "MQTTPacket/MQTTPacket.h"

#define REQUEST_MAX_SIZE 256
#define DATA_RECEIVED_MAX_LEN   64

static QueueHandle_t gs_tcpInQueue = NULL;

bool MQTT_Init(void)
{
	gs_tcpInQueue = xQueueCreate( DATA_RECEIVED_MAX_LEN, sizeof(uint8_t) );

	vQueueAddToRegistry(gs_tcpInQueue, "tcpIn");
}

void MQTT_Deinit(void)
{
	vQueueDelete(gs_tcpInQueue);
}

bool MQTT_IsConnected(void)
{
#ifdef GSM_XE910
	return GSM_getState == GSM_ST_TCP_CONNECTED;
#else
	return WIFI_GetState() == WIFI_ST_TCP_CONNECTED;
#endif //GSM_XE910
}

uint8_t MQTT_Connect(const char* address, uint16_t port, const char* clientId, const char* usr, const char* pwd)
{
	MQTTPacket_connectData connData    = MQTTPacket_connectData_initializer;
	uint8_t  request[REQUEST_MAX_SIZE] = {0};
	uint16_t requestLen                =  0;
	uint8_t  cid                       =  0xff;
	uint8_t  result                    =  0xff;

	connData.clientID.cstring  = clientId;
	connData.keepAliveInterval = 60;
	connData.cleansession      = 1;
	connData.username.cstring  = usr;
	connData.password.cstring  = pwd;

	requestLen = MQTTSerialize_connect(request, sizeof(request), &connData);
	
#ifdef GSM_XE910
	cid = GSM_SocketConnect(address, port);
#else
	cid = WIFI_TcpConnect(address, port);
#endif //GSM_XE910

	//TODO: Criar fila de requests e fazer envio não blocante
	if (MQTT_Write(cid, request, requestLen))
	{
		result = cid;
	}

	return result;
}

bool MQTT_Publish(uint8_t cid, const char* topic, uint32_t len, uint8_t *data, uint8_t qos, bool retain)
{
	MQTTString topicString               = MQTTString_initializer;
	uint8_t    request[REQUEST_MAX_SIZE] = { 0 };
	uint16_t   requestLen                = 0;
	bool       result                    = false;

	if (MQTT_IsConnected())
	{
		topicString.cstring = topic;

		requestLen = MQTTSerialize_publish((void*)request, sizeof(request), 0, 0, 0, 0, topicString, (unsigned char*)data, len);

		if (MQTT_Write(cid, request, requestLen))
		{
			result = true;
		}
	}

	return result;
}

bool MQTT_Subscribe(uint8_t cid, const char* topic, uint8_t qos, uint16_t timeout)
{
	MQTTString topicString               = MQTTString_initializer;
	uint8_t    request[REQUEST_MAX_SIZE] = { 0 };
	uint16_t   requestLen                = 0;
	bool       result                    = false;

	if (MQTT_IsConnected())
	{
		topicString.cstring = topic;

		requestLen = MQTTSerialize_subscribe(request, sizeof(request), 0, 0, 1, &topicString, (int) qos);

		if (MQTT_Write(cid, request, requestLen))
		{
			result = true;
		}
	}

	return result;
}

bool MQTT_Ping(uint8_t cid, uint16_t timeout)
{
	uint8_t    request[REQUEST_MAX_SIZE] = { 0 };
	uint16_t   requestLen                = 0;
	bool       result                    = false;

	if (MQTT_IsConnected())
	{
		requestLen = MQTTSerialize_pingreq(request, sizeof(request));

		if (MQTT_Write(cid, request, requestLen))
		{
			result = true;
		}
	}

	return result;
}

void MQTT_Disconnect(uint8_t cid)
{
	uint8_t    request[REQUEST_MAX_SIZE] = { 0 };
	uint16_t   requestLen                = 0;

	if (MQTT_IsConnected())
	{
		requestLen = MQTTSerialize_disconnect(request, sizeof(request));

		MQTT_Write(cid, request, requestLen);
#ifdef GSM_XE910
		GSM_SocketDisconnect();
#else
		WIFI_TcpDisconnect(cid);
#endif //GSM_XE910
	}
}

bool MQTT_Write(uint8_t cid, uint8_t* data, uint16_t dataLen)
{
	bool result = false;

	if (cid != 0xff)
	{
#ifdef GSM_XE910
		result = GSM_SocketWrite(data, dataLen);
#else
		result = WIFI_TcpWrite(cid, data, dataLen);
#endif
	}

	return result;
}

/**
 * Callbacks
 */
#ifdef GSM_XE910
void GSM_TcpReadCallback(uint8_t data)
{
	xQueueSendFromISR(gs_tcpInQueue, (uint8_t*)&data, NULL);
}
#else
void WIFI_TcpReadCallback(uint8_t data, SerialPort_et port)
{
    xQueueSendFromISR(gs_tcpInQueue, (uint8_t*)&data, NULL);
}
#endif //GSM_XE910

#endif //NATIVE_MQTT