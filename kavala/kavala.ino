/*
  Kavala Institute of Technology
  Copyright (C) 2012-2013 Panagiotis Vafiadis
  http://vafiadis-ultrasonic.org
 */
#include <Ultrasonic.h>

#define TRIGGER_PIN  13
#define ECHO_PIN     12
#define CR 13
#define LF 10
#define SERVER1 "AT+CPBF=\"SERVER1\""
#define TIME_PERIOD "AT+CPBF=\"TIME\""
#define PRODUCT_SERIAL_NUMBER  "1.0"
#define PRODUCT_VERSION_DATE "20-11-2012"

int onModulePin = 2;        // the pin to switch on the module (without press on button)
int logCycle;                // Time Period in minutes
char message[240];           // Message with measurements sent to DataHub
char dist[240];
char phoneNumber1[240];       // Phone Number of DataHub
char timeCycle[240]; 

Ultrasonic ultrasonic(TRIGGER_PIN, ECHO_PIN);
 
// Wait a number of minutes
void wait(int nMinutes)
{
  int i, j;
  
  for(i = 0; i < nMinutes; ++i)
    for(j = 0; j < 60; ++j)
     {
       delay(1000);
     }
}

/*
 Read from gsm module phonebok a specific input entry
 Format AT command:
 +CPBF: 6,"+30694xxxxxxx",145,"SERVER2"
  +CPBF: 6,"1",145,"SERVER2"
*/
bool readPhoneBook(char buffer[])
{
  char c;
  int index = 0;
  int cursor = 0;

  while( Serial.available() > 0 )
    {
      c = Serial.read();

      if(c == CR || c == LF)
	{
       	  buffer[index++] = ','; 
          continue;     
        }
      buffer[index++] = c;      
    }
  
  for(index = 0; buffer[index] != 0; ++index)
    if( buffer[index] == ':' )
      break;
        
  for(; buffer[index] != 0; ++index)
    if( buffer[index] == '\"' )
      break;

  if( buffer[index] == 0 ) return false;
  
  while(true)
    {
      ++index;

      if( buffer[index] == 0 )
         return false;
      else if( buffer[index] == '\"' )
	     break;
      else
         buffer[cursor++] = buffer[index]; 
    }

  buffer[cursor] = '\0';

  return true;
}

// Ignore from serial previous outputs
void ignore_chars()
{
  while( Serial.available() > 0 )
    {
      Serial.read();
    }
}

// Convert float number to string
char *ftoa(char *a, double f, int precision)
{
  long p[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000};
  
  char *ret = a;
  long heiltal = (long)f;
  itoa(heiltal, a, 10);
  while (*a != '\0') a++;
  *a++ = '.';
  long desimal = abs((long)((f - heiltal) * p[precision]));
  itoa(desimal, a, 10);
  return ret;
}
 

// Return specific entry from sim's phonebook entry
void getPhoneBookEntry(char * buffer, const char * entry)
{
  bool flag;
  do {
  ignore_chars();
  buffer[0] = 0;
  Serial.println("");
  delay(200);
  ignore_chars();
  Serial.println(entry);
  Serial.flush();
  delay(1000);
  flag = readPhoneBook(buffer); 
  ignore_chars();
  } while(flag == false);
  
  delay(200);
}


void sendSMS(char * phone, char *msg)
{
    Serial.println("");
    delay(1500);
    Serial.print("AT+CMGS=");               // send the SMS the number
    Serial.write(34);                  // send the " char 
    Serial.print(phone);              // send the phone number
    Serial.write(34);                // send the " char
    Serial.println("");
    delay(20); 
    Serial.print(msg);     // the SMS body
    delay(20);
    Serial.write(0x1A);                // end of message command 1A (hex)
    Serial.println("");
    delay(5000);
}

void switchModule(){
    digitalWrite(onModulePin,HIGH);
    delay(2000);
    digitalWrite(onModulePin,LOW);
}

void setup()
{
  Serial.begin(115200);                
  delay(2000);

  pinMode(onModulePin, OUTPUT);

  switchModule();
    
  for (int i=0;i < 5;i++)
  {
        delay(5000);
  } 
    
    // Set the current phonebook the SIM phonebook                  
  Serial.println(""); delay(500);
  Serial.println("AT+CPBS=\"SM\"");   
  delay(1200);
  ignore_chars();
                       
  // Set the SMS mode to text                      
  Serial.println(""); delay(500);  
  Serial.println("AT+CMGF=1");      
  delay(1200);
  ignore_chars();

 
  // Get server's phone number 
  getPhoneBookEntry(phoneNumber1, SERVER1);
  ignore_chars();

  // Get time period
  getPhoneBookEntry(timeCycle, TIME_PERIOD);
  ignore_chars();
 
   // Convert time period to integer
  logCycle = atoi(timeCycle);
  
  // if time period is invalid set one minute
  if( logCycle <= 0 ) logCycle = 1;
}

void loop()
{
  float cmMsec;
  long microsec = ultrasonic.timing();

   cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);
  ftoa(dist, cmMsec, 1);  
  sprintf(message, "s:%s u:%s", PRODUCT_SERIAL_NUMBER, dist); 
 
  sendSMS(phoneNumber1, message); 
 
  wait(logCycle);
}
