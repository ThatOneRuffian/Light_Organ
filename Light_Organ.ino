/*Light Organ\Fun Project written by Marcus Mesta for the MSP4302553 TI microcontroller in conjunction with MSGEQ7 bandpass IC. */

void updateData(void);      //gather data and store it into spectrumValue
void resetDevice(void);     //reset the bandpass device
void filter(void);          //filter out data by comparing to calculated offset. Zeros values not greater than initial threshold calculation on boot.
void lightAction(void);     //controls light display
void runMain(void);         //Main program control flow
void normalize(void);       //On boot set threshold limit 
void goodSignal(void);      //Pulse lights to indicate normalization is over
void setupDevices(void);    //setup bandpass device. Reboot, normalize device
void findThreshold(void);   //Set threshold OTG sound must break an average threshold in order to light each channel LED

const unsigned short int analogPin = 6;
const unsigned short int strobePin = 7;
const unsigned short int resetPin = 8;

const unsigned short int switchPin = 2;
const unsigned short int channelWidth = 7;
const unsigned short int lightArray[7] = {4,9,10,12,13,14,19}; //Pins supporting PWM
const unsigned short int minTime = 150; // min wait time for bandpass transitions

unsigned int spectrumValue[channelWidth] = {0,0,0,0,0,0,0};  //array that gets written to LEDs 
unsigned int offset[channelWidth] = {0,0,0,0,0,0,0};         //stores values of first boot noise levels
unsigned int movingThresholdLevel[channelWidth] = {0,0,0,0,0,0,0};       //stores average channel values. Is updated every x cycles
unsigned int runCount = 0; //keep track of runs, so that averages can be taken every x times.
unsigned int runTimes = 5; //How many samples till re-averaging (1 sample is 700us - update every ~250 ms) 

void setup(){
  /******** Setup Pin Modes*************/
  
  pinMode(analogPin, INPUT);
  pinMode(strobePin, OUTPUT);
  pinMode(resetPin, OUTPUT);

  for(unsigned int i = 0; i < channelWidth; i++){//setup led light outputs
    pinMode(lightArray[i], OUTPUT);
  }
  
  setupDevices(); //setup Bandpass IC
}

void loop(){
   
  runMain();
}

void updateData(){
  
  for (unsigned int i = 0; i < channelWidth; i++){ 
    
    digitalWrite(strobePin, LOW);
    delayMicroseconds(minTime);
    spectrumValue[i] = analogRead(analogPin);
    digitalWrite(strobePin, HIGH);
  }
}

void resetDevice(){
  
  digitalWrite(resetPin, HIGH);
  delayMicroseconds(minTime);    //min time required is 100 us added 50 us - datasheet
  digitalWrite(resetPin, LOW);
}

void setupDevices(){
  
  digitalWrite(resetPin, HIGH);  //Bandpass device setup
  delayMicroseconds(minTime);        ////min time required is 100 us added 50 us - datasheet
  digitalWrite(strobePin, HIGH);
  
  normalize();  //calibrate device on setup
}

void normalize(){
  
  const int sampleDelay = 20;  //sampleDelay * sample ~= 0.5 second
  const int samples = 25;      //samples to be taken
  const int buffer = 20;     //value added to max value as signal padding
  
  unsigned int dataBuffering[samples][channelWidth]; //store data from samplings
  
  for (int sampleIndex = 0; sampleIndex < samples; sampleIndex++){ //samples channels for however many samples placed above
  
    resetDevice();
    updateData(); 
    
    for (unsigned int channel = 0; channel < channelWidth; channel ++){ //store values from sample in data array
      
        dataBuffering[sampleIndex][channel] = spectrumValue[channel];
      }

    delay(sampleDelay);    //sample rate delay
    }
   
    for(int sampleIndex = 0; sampleIndex < samples; sampleIndex ++){ //Now that the data collection is done, we can do math to find the peak values for each channel within that sample duration.
    
      for(unsigned int channel = 0; channel < channelWidth; channel++){
        
        if(dataBuffering[sampleIndex][channel] > offset[channel]){  //Find max values in data samples and set offset as max + BUFFER
          
          offset[channel] = dataBuffering[sampleIndex][channel] + buffer;
        }
      }
    }
    goodSignal(); //indicate that normailzation has completed
}


void filter(){    //Compares current values to that of the channel offsets to ensure that they are above threshold

  for( unsigned int channel = 0; channel < channelWidth; channel++){
    
    if( spectrumValue[channel] >= offset[channel] && spectrumValue[channel] >= movingThresholdLevel[channel] ){
      
      spectrumValue[channel] = map(spectrumValue[channel], 0,1023,0,255);
    }
    
    else {
      
    spectrumValue[channel] = 0;
    }
  }
}

void goodSignal(){ //Light indicator that calibration is complete and music can now be played. Fade in Fade out.

  const unsigned short int delayTime = 2; 

  for(int i = 1; i <= 255; i++){ //Fade in
    
      for(unsigned int z = 0; z < channelWidth; z++){
    analogWrite(lightArray[z], i);
  }
    delay(delayTime);
  }
  
  for(int i = 255; i >= 1; i--){  //Fade out
      
      for(unsigned int z = 0; z < channelWidth; z++){
    analogWrite(lightArray[z], i);
    }
    delay(delayTime);
  }  
}

void runMain(){
  
  resetDevice();
  updateData();
  filter();   
  
  if(runCount >= runTimes){
    
    findThreshold();
    runCount = 0;
  }
  
  lightAction();
}

void findThreshold()
{
    
  const int samples = 16;      //samples to be taken 
  double dataBuffer[samples][channelWidth]; //store data from samplings to be averaged
    
  for (int sampleIndex = 0; sampleIndex < samples; sampleIndex++){ //samples channels for however many samples placed above
  
    resetDevice();
    updateData(); 
    
    for (unsigned int channel = 0; channel < channelWidth; channel ++){ //store values from sample in data array
      
        dataBuffer[sampleIndex][channel] = spectrumValue[channel];
      }
    }
   
    for(int sampleIndex = 0; sampleIndex < samples; sampleIndex ++){ //Now that the data collection is done- Find max
    
      for(unsigned int channel = 0; channel < channelWidth; channel++){
        
        if(dataBuffer[sampleIndex][channel] > movingThresholdLevel[channel]){
        movingThresholdLevel[channel] = dataBuffer[sampleIndex][channel];
        }
      }
    }
}

void lightAction(){
  
   for(unsigned int i = 0; i < channelWidth; i++)
    {
      analogWrite(lightArray[i], spectrumValue[i]);
    }
}

