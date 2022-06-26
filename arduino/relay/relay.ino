#include <nRF24L01.h>
#include <printf.h>
#include <RF24_config.h>
#include <RF24.h>

/*
  * Wireless nRF24L01 Relay Receiver
  * 
  * Author: Creed Zagrzebski (czagrzebski)
  */

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <EEPROM.h>

// Map Arduino Pins to Relays
int relay_pins[4] = {2, 3, 4, 5}; 

RF24 radio(7, 8); //(CE - Pin 7, CSN - Pin 8)

uint8_t address[][6] = {"1Node"};

bool radioNumber = 1; // 0 uses address[0] to transmit, 1 uses address[1] to transmit

struct data
{
    uint32_t relay_id; //ID that represents each relay (e.g: 1,2,3,4)
    uint32_t state; //0 for off, 1 for on
};

typedef struct data Data;

Data payload;

void setup()
{ 
    //For serial debugging
    Serial.begin(11520);

    radio.begin();

    Serial.println("Wirelss Relay Receiver");

    //Prevents Power Supply Related Issue (and isn't necessary for testing when they are close together)
    radio.setPALevel(RF24_PA_LOW);

    //Defining Payload Size saves processing time
    radio.setPayloadSize(8); // Data from RPi is 8 bytes ( 4-byte ints)

    // set the RX address of the TX node into a RX pipe
    radio.openReadingPipe(1, address[0]); // using pipe 1

    radio.startListening(); // put radio in RX mode

    //Setup All Defined Relay Pins
    for (int i = 0; i < sizeof(relay_pins); i++)
    {
        pinMode(relay_pins[i], OUTPUT);
        digitalWrite(relay_pins[i], HIGH); //start off
    }
}

void loop()
{
    uint8_t pipe;

    if (radio.available(&pipe))
    {
        uint8_t bytes = radio.getPayloadSize(); // get the size of the payload
        radio.read(&payload, bytes);
        Serial.println(); // fetch payload from FIFO queue
        Serial.print("Received ");
        Serial.print(bytes); // size of payload
        Serial.print(F(" bytes on pipe "));
        Serial.print(pipe); // print the pipe number
        Serial.println(payload.state);

        //Relay_ID of -1 represents all relays (Allows the user to Turn on/off all relays at once)
        if (payload.relay_id == -1)
        {
            if (payload.state == 1)
            {
                //Turn all the relays off
                for (int i = 0; i < sizeof(relay_pins); i++)
                {
                    pinMode(relay_pins[i], OUTPUT);
                    digitalWrite(relay_pins[i], HIGH);
                }
            }
            else
            {
                for (int i = 0; i < sizeof(relay_pins); i++)
                {
                    pinMode(relay_pins[i], OUTPUT);
                    digitalWrite(relay_pins[i], LOW);
                }
            }
        }
        else
        {
            //Translate Relay Number to Relay PIN on Arduino
            int RELAY_PIN = relay_pins[payload.relay_id - 1];

            if (payload.state == 1)
            {
                digitalWrite(RELAY_PIN, HIGH);
            }
            else if (payload.state == 0)
            {
                digitalWrite(RELAY_PIN, LOW);
            }
        }
    }

    delay(100);
}

void toggleOff(int relay_id){
    //Translate Relay Number to Relay PIN on Arduino
    int RELAY_PIN = relay_pins[payload.relay_id - 1];
    digitalWrite(RELAY_PIN, LOW);
    saveRelayState(relay_id, 0);
}
  
void toggleOn(int relay_id){
    //Translate Relay Number to Relay PIN on Arduino
    int RELAY_PIN = relay_pins[payload.relay_id - 1];
    digitalWrite(RELAY_PIN, HIGH);
    saveRelayState(relay_id, 1);
}

/**
 * Saves the relay state to EEPROM
 * 
 * @param relay_id The ID of the relay
 * @param state  The state of the relay
 */
void saveRelayState(int relay_id, int state){
    EEPROM.update(relay_id - 1, state);
}