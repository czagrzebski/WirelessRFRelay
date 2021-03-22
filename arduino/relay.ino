/*
  * Wireless Relay Reciever
  * 
  * Author: Creed Zagrzebski (czagrzebski)
  */

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

int RELAY_PIN;

int relay_pins[4] = {3, 2, 4, 5}; //Translate Relay ID to Arduino Pin

RF24 radio(7, 8); //(CE - Pin 7, CSN - Pin 8)

uint8_t address[][6] = {"1Node", "2Node"};

bool radioNumber = 1; // 0 uses address[0] to transmit, 1 uses address[1] to transmit

typedef struct data
{
    uint32_t relay_id; //ID that represents each relay (e.g: 1,2,3,4)
    uint32_t state; //0 for off, 1 for on
};

data payload;

void setup()
{
    //For serial debugging
    Serial.begin(115200);

    radio.begin();

    // print example's introductory prompt
    Serial.println("Wirelss Relay Receiver");

    char input = '1';
    radioNumber = input == 1;

    //Prevents Power Supply Related Issue (and isn't necessary for testing when they are close together)
    radio.setPALevel(RF24_PA_LOW);

    //Defining Payload Size saves time
    radio.setPayloadSize(8); // Data from RPi is 8 bytes

    // set the RX address of the TX node into a RX pipe
    radio.openReadingPipe(1, address[!radioNumber]); // using pipe 1

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
            RELAY_PIN = relay_pins[payload.relay_id - 1];

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

    delay(200);
}