/*
 * WebSocketClient.ino
 *
 *  Created on: 24.05.2015
 *
 */

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <WebSocketsClient.h>

#include <Hash.h>

#include <SPI.h>
#include <MFRC522.h>

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
String UID, PS, COIN;

#define USE_SERIAL Serial
#define RST_PIN 9				  //Pin 9 para el reset del RC522
#define SS_PIN 10				  //Pin 10 para el SS (SDA) del RC522
MFRC522 mfrc522(SS_PIN, RST_PIN); //Creamos el objeto para el RC522

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{

	switch (type)
	{
	case WStype_DISCONNECTED:
		USE_SERIAL.printf("[WSc] Disconnected!\n");
		break;
	case WStype_CONNECTED:
	{
		USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);

		// send message to server when Connected
		webSocket.sendTXT("Connected");
	}
	break;
	case WStype_TEXT:
		USE_SERIAL.printf("[WSc] get text: %s\n", payload);

		// send message to server
		// webSocket.sendTXT("message here");
		break;
	case WStype_BIN:
		USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
		hexdump(payload, length);

		// send data to server
		// webSocket.sendBIN(payload, length);
		break;
	case WStype_PING:
		// pong will be send automatically
		USE_SERIAL.printf("[WSc] get ping\n");
		break;
	case WStype_PONG:
		// answer to a ping we send
		USE_SERIAL.printf("[WSc] get pong\n");
		break;
	}
}

void setup()
{
	// USE_SERIAL.begin(921600);
	USE_SERIAL.begin(9600);
	SPI.begin();
	mfrc522.PCD_Init(); // Iniciamos  el MFRC522
	USE_SERIAL.println("Lectura del UID");
	//Serial.setDebugOutput(true);
	USE_SERIAL.setDebugOutput(true);

	USE_SERIAL.println();
	USE_SERIAL.println();
	USE_SERIAL.println();

	for (uint8_t t = 4; t > 0; t--)
	{
		USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
		USE_SERIAL.flush();
		delay(1000);
	}

	WiFiMulti.addAP("SSID", "passpasspass");

	//WiFi.disconnect();
	while (WiFiMulti.run() != WL_CONNECTED)
	{
		delay(100);
	}

	// server address, port and URL
	webSocket.begin("192.168.0.123", 81, "/");

	// event handler
	webSocket.onEvent(webSocketEvent);

	// use HTTP Basic Authorization this is optional remove if not needed
	webSocket.setAuthorization("user", "Password");

	// try ever 5000 again if connection has failed
	webSocket.setReconnectInterval(5000);

	// start heartbeat (optional)
	// ping server every 15000 ms
	// expect pong from server within 3000 ms
	// consider connection disconnected if pong is not received 2 times
	webSocket.enableHeartbeat(15000, 3000, 2);
}

void loop()
{
	webSocket.loop();
	// Revisamos si hay nuevas tarjetas  presentes y Seleccionamos una tarjeta
	if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
	{
			// Enviamos serialemente su UID Leer tarjeta.
			USE_SERIAL.print("Card UID:");
			for (byte i = 0; i < mfrc522.uid.size; i++)
			{
				USE_SERIAL.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
				USE_SERIAL.print(mfrc522.uid.uidByte[i], HEX);
			}
			Serial.println();
			// Terminamos la lectura de la tarjeta  actual
			mfrc522.PICC_HaltA();
		
	}
}