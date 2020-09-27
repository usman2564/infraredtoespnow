#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>


//CODE FOR RECIEVER ESP -- BY USMAN QURESHI


int binarycode = 0;

//the esp-reciever must be attached to an IR LED to ultimately control the TV;
//create the IRLED class
class IRLED
{
public:

//IRLED pin as the input for the class
IRLED(int p)
{
ledPin = p;
}

void initialize()
{
//in intialization, the ledpin must be an output
pinMode(ledPin, OUTPUT);
}

//carrier frequency of the reciever is 38khz or approximately a period of 26 microseconds. So one whole
//blink for the led should happen in 26microseconds (6 microseconds on + 16 microseconds off + small time to start command)
//Sony protocol has a carrier duty cycle of 1/4 to 1/3. 6/26 is close to 1/4.
void carrier(int bitTime)
{
  for(int i = 0; i < (bitTime/26); i++)
  {
  digitalWrite(ledPin, HIGH);
  delayMicroseconds(6);
  digitalWrite(ledPin, LOW);
  delayMicroseconds(16);
  }
}

//inputted code for the LED to blink to. Each code corresponds to a unique remote button
void scanCode(int bincode)
{

//start bit, carrier should be on for 2400 microseconds, and all dead-times are 600 microseconds.
  carrier(2400);
  delayMicroseconds(600);

//compare the LSB and check if it is a 1 or 0. If its 1, carrier for 1200 microseconds (according to sony's protocol) else a 0
//means carrier for 600 microseconds. Iterate through the 12 bits, shifting right through each iteration.
  for(int i = 0; i < 12; i++) 
  {
    if(bincode>>i & 0b000000000001)
    {
      carrier(1200);
    } else {
      carrier(600);
    }
    delayMicroseconds(600);
  }

//small delay
  delay(45); 
}

private:
int ledPin;
unsigned long code;
};



//ir-led pin attached to pin 21
IRLED irled(21);


//function calling interupts to recieve codes. Once codes are inputted, save as pointer? which is saved to variable.
void onDataRecieve(const uint8_t *mac, const uint8_t *data, int len) {

int * codePointer = (int*)data;

binarycode = *codePointer;   


}

//initialize esp-reciever, check if it starts properly
//initialize the irled
void setup() {
  // put your setup code here, to run once:
irled.initialize();
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  Serial.println("Reciver Mac Adress:");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;

}
}


//check if a code is recieved through esp now. If it is, use the code with irled. After using the code, set the code to 0
//so we dont repeat the same command.
void loop() {
  // put your main code here, to run repeatedly:

   esp_now_register_recv_cb(onDataRecieve);

  if(binarycode != (0)) {
  irled.scanCode(binarycode);
}

binarycode = 0;
}