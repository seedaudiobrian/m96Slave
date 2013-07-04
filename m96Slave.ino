
/* ---------------------------------------------------------------------
    PROGRAM NAME :  m96_slave_v2_i (version j)   
    saved: Thur July 19 2013

    SUMMARY : I pulled this from brian's drop box today.  
    It appears to be very similar to m96 version g (stable) with a few lines changed in 
    part B. 

    FUNCTION: 

    Part A - 
    harvests 48 analog int values from a meyhew mux shield attached to an arduino uno. 
    
    Part B - 
    takes the values stored in the array and sends them over the i2c bus to an arduino
    mega (called master).



    I2C notes:  Code for and i2c based on that of Nick Gammon 
       http://arduino.cc/forum/index.php/topic,104732.0.html
       See the Arduino Bitwise AND Reference: http://www.arduino.cc/en/Reference/BitwiseAnd
       and  the Aruino Bitshift Reference: http://www.arduino.cc/en/Reference/Bitshift
				
    Mayhew Mux Shield notes: Code for the shield based on Mark Mayhew
        http://mayhewlabs.com/products/arduino-mux-shield
        Mayhew mux (hardware version I) uses a TI CD74HC4067
        three separate muxes called 0-15, 16-31, 32-47 and three output analog ports  

 
   ---------------------------------------------------------------------
   -------------------------------- SLAVE ------------------------------ 
   ---------------------------------------------------------------------
*/

// I2C stuff - we use the Wire libraries -------------------------------
#include <Wire.h>

// I2C ADDRESS CONSTANTS
#define MASTER 2
#define SLAVELEFT 3
#define SLAVERIGHT 4

// MUX stuff - arduino analog pin numbers. the mayhew bus is four wide  -------------------------------- 
#define CONTROL0 5    
#define CONTROL1 4
#define CONTROL2 3
#define CONTROL3 2


// Bboard specific ----------------------------------------------------- 


#define ARRAY_SIZE 50
//Create arrays for data from the the MUXs which is all stored in column order 
// never in not in mux pin order


int         sensorArray[ARRAY_SIZE];    // data from the senors
int         minArray[ARRAY_SIZE];       // find min and store here
int         maxArray[ARRAY_SIZE];       // find max and store here
int         tmpArray[ARRAY_SIZE];       // debug 

boolean     stateArray[ARRAY_SIZE];       // boolean data on/off to be sent over the wire
const int buttonPin=12;                  // this is the l/r reset button

#define NOTE_ON 1
#define NOTE_OFF 0
#define LOWTHRESH 30 
//goldata RIGHT int bboardMap[] ={ 0, 8, 18, 28, 38, 1, 9, 19, 34, 44, 5, 13, 23, 33, 43, 42, 29, 39, 2, 10, 20, 30, 40, 3, 7, 15, 25, 35, 45, 6, 14, 24, 11, 21, 31, 41, 4, 12, 22, 32, 17, 27, 37, 47, 16, 26, 36, 46};
 
//goldata LEFT  
int bboardMap[]={10, 20, 30, 40, 1, 11, 21, 31, 16, 6, 45, 35, 25, 15, 5, 44, 41, 2, 12, 22, 32, 42, 3, 13, 47, 37, 27, 17, 7, 46, 36, 26, 23, 33, 43, 4, 14, 24, 34, 0, 39, 29, 19, 9, 38, 28, 18, 8};



// Slave origin byte constant. Comment out as appropriate 
#define SLAVEADDRESSBYTE 1            // right  
//#define SLAVEADDRESSBYTE 0            // left  

// the primary struct for sending I2C.  
// 48 bits fit into three words  

struct 
{
   boolean SlaveOrigin;                     // 0 means left. 1 means right
   unsigned int SwitchOnOffArray[3];        // Three int's which house compressed on/off 
                                            // states of all 48 switches in a bit field                               
} SensorData;


// ------------------------------- SETUP --------------------------------------

void setup()
{
    Serial.begin(38400);
    Serial.println ("console set to 38400 -----------");    
  // Low values need to be filled with a high value to start:
  for (int i; i < ARRAY_SIZE; i++ ) 
  {
  minArray[i]=500;      //the sensor should never be higher than 1220 - the Analog input max
  }

 pinMode(buttonPin, INPUT);  // uno pin 12 used for 

  // This is a boolean value in the struct that acts as header information to the master. This byte tells which slave the data packet is coming from.
 SensorData.SlaveOrigin = SLAVEADDRESSBYTE;
  
  //Set MUX control pins to output
  pinMode(CONTROL0, OUTPUT);
  pinMode(CONTROL1, OUTPUT);
  pinMode(CONTROL2, OUTPUT);
  pinMode(CONTROL3, OUTPUT);
  Wire.begin(SLAVERIGHT);                // join i2c bus with address #2  
}


void loop()
{
  

//  ---------------------------------------------------------------------
//   -------------- PART A - READ ALL OF THE MUX VALUES  -----------------
//   ---------------------------------------------------------------------



int newValue0 = 0;
int newValue1 = 0;
int newValue2 = 0;
int buttonLocation;
int sensorTmp;
int minTmp;
int maxTmp;
 

// check for pin 12 on uno 
if ( digitalRead(buttonPin) == HIGH) 
{
  Serial.println ("resetting minmax ");
    delay (100);
  for (int r=0; r< 48; r++)
  {
    Serial.print (" .");
    minArray[r]=500;
    maxArray[r]=0;
   // resetMinMax=0;
  }
  Serial.println (" ");
}  
     

  for (int i=0; i< 16 ; i++)
   {    
     
    // step one send bit mask
    // explain this.........     
    digitalWrite(CONTROL0, (i&15)>>3); 
    digitalWrite(CONTROL1, (i&7)>>2);  
    digitalWrite(CONTROL2, (i&3)>>1);  
    digitalWrite(CONTROL3, (i&1)); 
    
    // step two read 0-15
    newValue0=analogRead(0);
    buttonLocation=bboardMap[i];
    sensorArray[buttonLocation]=newValue0;
          
          minTmp= min(newValue0, minArray[buttonLocation]);
          minArray[buttonLocation]=minTmp;
          maxTmp= max(newValue0, maxArray[buttonLocation]);
          maxArray[buttonLocation]=maxTmp;
          
    
    // step three read 16-31
    newValue1=analogRead(1);
    buttonLocation=bboardMap[i+16];
    sensorArray[buttonLocation]=newValue1;
          
          minTmp= min(newValue1, minArray[buttonLocation]);
          minArray[buttonLocation]=minTmp;
          maxTmp= max(newValue1, maxArray[buttonLocation]);
          maxArray[buttonLocation]=maxTmp;
         
   
    // step four read 32-47
    newValue2=analogRead(2);
    buttonLocation=bboardMap[i+32];
    sensorArray[buttonLocation]=newValue2;  
        
          minTmp= min(newValue2, minArray[buttonLocation]);
          minArray[buttonLocation]=minTmp;
          maxTmp= max(newValue2, maxArray[buttonLocation]);
          maxArray[buttonLocation]=maxTmp;
         
    

    }// i loop


for (int p=0; p<48; p++) {
  
    sensorTmp=sensorArray[p];
    minTmp=minArray[p];
    maxTmp=maxArray[p];

 int setPoint=( minTmp*1.3); 
 
 
 
 stateArray[2] = HIGH;
 //stateArray[2] = LOW;
 //stateArray[3] = HIGH;
 //stateArray[4] = LOW;
 //stateArray[6] = HIGH;
 /*
 // still need some debounce
  if (sensorTmp > (setPoint) )
          {
            stateArray[p]=NOTE_ON;
          }
          else
          { 
           stateArray[p]=NOTE_OFF;
          }
          
          /*
} // end p       

// ------------------------------------ REPORTING -----------------

#define Begin 0
#define End 48

delay(500);
// reporting 
for (int j=Begin; j<End; j++)
{
  
    sensorTmp=sensorArray[j];
    minTmp=minArray[j];
    maxTmp=maxArray[j];

  Serial.print (j); Serial.print (" "); Serial.print (stateArray[j]); 
  Serial.print (" low "); Serial.print (minTmp);
    Serial.print ("    sensor "); Serial.print (sensorTmp); Serial.print ("     max "); Serial.println (maxTmp);
 
 
 /*
 
  if (stateArray[j] == NOTE_ON)
  {
    Serial.print (j); Serial.print (" ON"); Serial.print (" low "); Serial.print (minTmp);
    Serial.print (" sensor "); Serial.print (sensorTmp); Serial.print (" max "); Serial.print (maxTmp);
  Serial.println (" - ");
  }
*/

} // end j 
Serial.println ("-");



//   ---------------------------------------------------------------------
//   -------------- PART B - TRANSMIT DATA TO MASTER --------------------- 
//   ---------------------------------------------------------------------

//    using two long ints - 
//        pack first 32 into array[0] 
//        pack second 16 into array[1] 
//        and put flag into remainder of array[1]  ?? or not


  // traverse SensorArray and based on that value, 
  // pack the appropriate bit into Sensor.Data.SwitchOnOffArray  
  // to populate our two long variables with compressed binary sensor data:

  
  // pack the first word (array[0]) 
  // we are traversing in column order and not by mux address - 
  for(int indexOne=0;indexOne<16;indexOne++)
  {
    if(stateArray[indexOne] == NOTE_OFF )                      
    {                                           // set array to off
      SensorData.SwitchOnOffArray[0] = SensorData.SwitchOnOffArray[0] << 1;
    }
    else 
    {                                                // set array to on
      SensorData.SwitchOnOffArray[0] = (SensorData.SwitchOnOffArray[0] << 1) + 1;
    }
  } // first for
  
  for(int indexTwo=0;indexTwo<16;indexTwo++)
  {
    if(stateArray[indexTwo+16] == NOTE_OFF ) 
    {                                               // set array to off 
      SensorData.SwitchOnOffArray[1] = SensorData.SwitchOnOffArray[1] << 1;
    }
    else
    {                                              // set array to on 
      SensorData.SwitchOnOffArray[1] = (SensorData.SwitchOnOffArray[1] << 1) + 1;  
    }
  } // second for
  
  //populate our two long variables with compressed binary sensor data:
  for(int indexThree=0;indexThree<16;indexThree++)
  {
    if(stateArray[indexThree+32] == NOTE_OFF ) 
    {                                              // set array to off 
        SensorData.SwitchOnOffArray[2] = SensorData.SwitchOnOffArray[2] << 1;
    }
    else 
    {                                              // set array to on 
      SensorData.SwitchOnOffArray[2] = (SensorData.SwitchOnOffArray[2] << 1) + 1;
    }
  } // third for 
  
  
  // after packing all of the data - send it across the wire
  // drc : here i am wondering if we should for request from master first
  
     
  Wire.beginTransmission (MASTER);
  Wire.write ((byte *) &SensorData, sizeof SensorData);
  Wire.endTransmission ();
  delay(10);
  
  
}// main for loop




// version 7     
 //goldata RIGHT int bboardMap[] ={ 0, 8, 18, 28, 38, 1, 9, 19, 34, 44, 5, 13, 23, 33, 43, 42, 29, 39, 2, 10, 20, 30, 40, 3, 7, 15, 25, 35, 45, 6, 14, 24, 11, 21, 31, 41, 4, 12, 22, 32, 17, 27, 37, 47, 16, 26, 36, 46};

//goldata LEFT int bboardMap[]={10, 20, 30, 40, 1, 11, 21, 31, 16, 6, 45, 35, 25, 15, 5, 44, 41, 2, 12, 22, 32, 42, 3, 13, 47, 37, 27, 17, 7, 46, 36, 26, 23, 33, 43, 4, 14, 24, 34, 0, 39, 29, 19, 9, 38, 28, 18, 8};

  
 

