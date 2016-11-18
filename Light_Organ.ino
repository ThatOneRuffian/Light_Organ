/*Light Organ\Fun Project written by Marcus Mesta for the MSP4302553 TI microcontroller. */

const unsigned short int analogPin = 6;
const unsigned short int strobePin = 7;
const unsigned short int resetPin = 8;

const unsigned short int ledred = 14;
const unsigned short int ledblue = 13;
const unsigned short int ledgreen = 12;

const unsigned short int switchPin = 2;
const unsigned short int channelWidth = 7;

unsigned int spectrumValue[channelWidth] = {0,0,0,0,0,0,0};
unsigned int offset[channelWidth] = {0,0,0,0,0,0,0};
 
void updateData(void);   //gather data and store it into spectrumValue
void resetDevice(void);  //reset the bandpass device
void filter(void);       //filter out data by comparing to calculated offset
void lightAction(void);  //controls light display
void runMain(void);      //Main program control flow
void normalize(void);    //On boot set threshold limit 
void goodSignal(void);   //Pulse lights to indicate normalization is over
void setAvgThreshold(void); //Set threshold OTG sound must break an average threshold in order to light each channel LED


unsigned int runCount = 0; //keep track of runs, so that averages can be taken every x times.

void setup(){
  
  pinMode(analogPin, INPUT);
  pinMode(strobePin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  pinMode(ledred, OUTPUT);
  pinMode(ledblue, OUTPUT);
  pinMode(ledgreen, OUTPUT);
  
  digitalWrite(resetPin, HIGH);  //device setup
  digitalWrite(strobePin, HIGH);
  
  normalize();  //calibrate device on setup
}

void loop(){
   
  runMain();
}

void updateData(){
  
  for (int i = 0; i < channelWidth; i++){ // C
    digitalWrite(strobePin, LOW);
    delay(1);
    spectrumValue[i] = analogRead(analogPin);
    
    digitalWrite(strobePin, HIGH);
  }
}

void resetDevice(){
  
  digitalWrite(resetPin, HIGH);
  delay(5);     
  digitalWrite(resetPin, LOW);
}

void normalize(){
  
  const int sampleDelay = 60;  //sampleDelay * sample ~= 1 second
  const int samples = 15;      //samples to be taken
  const int maxOffset = 5;     //value added to max value as signal padding
  
  unsigned int dataBuffering[samples][channelWidth]; //store data from samplings
  
  for (int sampleIndex = 0; sampleIndex < samples; sampleIndex++){ //samples channels for however many samples placed above
  
    resetDevice();
    updateData(); //7 ms delay each run.
    
    for (int channel = 0; channel < channelWidth; channel ++){ //store values from sample in data array
      
        dataBuffering[sampleIndex][channel] = spectrumValue[channel];
      }

    delay(sampleDelay);
    }
   
    for(int sampleIndex = 0; sampleIndex < samples; sampleIndex ++){ //Now that the data collection is done, we can do math to find the peak values for each channel within that sample duration.
    
      for(int channel = 0; channel < channelWidth; channel++){
        
        if(dataBuffering[sampleIndex][channel] > offset[channel]){  //Find max values in data samples and set offset as max + BUFFER
          
          offset[channel] = dataBuffering[sampleIndex][channel] + maxOffset;
        }
      }
    }
    goodSignal(); //indicate that normailzation has completed
}


void filter(){    //Compares current values to that of the channel offsets to ensure that they are above threshold

  for( int channel = 0; channel < channelWidth; channel++){
    
    if( spectrumValue[channel] >= offset[channel]){
      
      spectrumValue[channel] = map(spectrumValue[channel], 0,1023,0,255);
    }
    
    else {
      
    spectrumValue[channel] = 0;
    }
  }
}

void goodSignal(){ //Light indicator that calibration is complete and music can now be played. 

  const unsigned short int delayTime = 2; 

  for(int i = 1; i <= 255; i++){ //Fade in
    
    analogWrite(ledred, i);
    analogWrite(ledblue, i);
    analogWrite(ledgreen, i);
    delay(delayTime);
  }
  
  for(int z = 255; z >= 1; z--){  //Fade out
      
    analogWrite(ledred, z);
    analogWrite(ledblue, z);
    analogWrite(ledgreen, z);
    delay(delayTime);
  }  
}

void runMain(){
  
  setAvgThreshold();
  
  resetDevice();
  updateData();
  filter();
  lightAction();
}
void setAvgThreshold()
{
  
}
void lightAction(){
  
analogWrite(ledred, spectrumValue[1]);
analogWrite(ledblue, spectrumValue[3]);
analogWrite(ledgreen, spectrumValue[5]);
}

