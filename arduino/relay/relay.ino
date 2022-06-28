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
int relay_pins[4] = {5, 4, 3, 2}; 

RF24 radio(7, 8); //(CE - Pin 7, CSN - Pin 8)

uint8_t address[][6] = {"1Node"};

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
    Serial.begin(115200);

    radio.begin();

    Serial.println("Wireless Relay Receiver v1.0.0");

    //Prevents Power Supply Related Issue (and isn't necessary for testing when they are close together)
    radio.setPALevel(RF24_PA_LOW);

    //Defining Payload Size saves processing time
    radio.setPayloadSize(8); // Data from RPi is 8 bytes ( 4-byte ints)

    // set the RX address of the TX node into a RX pipe
    radio.openReadingPipe(0, address[0]); // using pipe 1

    // put radio in RX mode
    radio.startListening(); 

    //Setup All Defined Relay Pins and restore last state from EEPROM
    for (int i = 0; i < sizeof(relay_pins); i++)
    {
        pinMode(relay_pins[i], OUTPUT);
        if(EEPROM.read(i) == 1){
            digitalWrite(relay_pins[i], HIGH);
        } else {
            digitalWrite(relay_pins[i], LOW);
        }
      
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
            if (payload.state == 1)
            {
                toggleOn(payload.relay_id);
            }
            else 
            {
                toggleOff(payload.relay_id);
            }
        }
    }
}

/**
 * Changes relay state to LOW
 * 
 * @param relay_id Relay ID
 */
void toggleOff(int relay_id){
    //Translate Relay Number to Relay PIN on Arduino
    int RELAY_PIN = relay_pins[relay_id - 1];
    digitalWrite(RELAY_PIN, LOW);
    saveRelayState(relay_id, 0);
}
  
/**
 * Changes relay state to HIGH
 * 
 * @param relay_id Relay ID 
 */
void toggleOn(int relay_id){
    //Translate Relay Number to Relay PIN on Arduino
    int RELAY_PIN = relay_pins[relay_id - 1];
    digitalWrite(RELAY_PIN, HIGH);
    saveRelayState(relay_id, 1);
}

/**
 * Saves the relay state to EEPROM. Safety feature for
 * unexpected power outages. 
 * 
 * @param relay_id The ID of the relay
 * @param state  The state of the relay
 */
void saveRelayState(int relay_id, int state){
    Serial.println(relay_id);
    Serial.println(state);
    EEPROM.update(relay_id - 1, state);
}