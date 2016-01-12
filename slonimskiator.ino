#include <SoftwareSerial.h>

SoftwareSerial midi(5, 6); // RX, TX

#define LED 3    // LED pin on Arduino board
#define RESET 2  // Reset button
#define MIDINOTEON 144 //144 = 10010000 in binary, note on command
#define MINNOTE 25
#define MAXNOTE 127
#define MINVELOCITY 90
#define MAXVELOCITY 90
#define P_RANGE 3
#define P_TRANSPOSE 4
#define P_LENGTH 5
#define P_SPEED 6
#define P_STARTNOTE 7

int transpose = 1;
int length = 1;
int range = 0;
int speed = 0;
int startnote = 30;
int velocity = 100;
boolean inReset = false;

void setup() {
  midi.begin(31250);        // Set MIDI baud rate: 31250
  Serial.begin(115200);
  
  randomSeed(analogRead(0));
  
  pinMode(RESET, INPUT);
  attachInterrupt(0, resetSequence, RISING);
}

void resetSequence()
{
  digitalWrite(13, !digitalRead(13));    // Toggle LED on pin 13
  inReset = true;
}

// return a random sequence with a given length of notes
void generateSequence(int notes[], int num_notes, int range) {
  randomSeed(analogRead(0));
  for (int i=0;i<num_notes;i++) {
    notes[i] = random((1-range), (range-1));
  }
}

void loop() {

  int note=startnote;
  int breakout=false;
  char direction = 'A';  // Ascending or Descending  
  updatePots();
  int sequence[length];
  generateSequence(sequence, length, range);

  while (1) {
    
    // play inter/infra/ultra-polated sequence starting on note
    for (int i=0; i<length; i++)  {
      int seq_note = note + sequence[i];
      MIDImessage(MIDINOTEON, seq_note, random(MINVELOCITY, MAXVELOCITY)); //turn note on
      delay(speed); //hold note
      MIDImessage(MIDINOTEON, seq_note, 0); //turn note off
      delay(speed); //wait until triggering next note
      
      // if new pot values, or reset button is pressed, break the while loop
      if (updatePots() || inReset) {
        inReset = false;
        return;
      }
      
      // update speed and startnote without generating a new sequence
      speed = quantizePot(P_SPEED, 500);
      int new_startnote = quantizePot(P_STARTNOTE, 100);
      if (new_startnote != startnote) {
        note = startnote = new_startnote;
      }
    }
    
    // switch direction between ascending and descending
    if ((direction == 'A') && (note + transpose + range) > MAXNOTE) {
      direction = 'D';
    }
    if ((direction == 'D') && (note - transpose - range) < startnote) {
      direction = 'A';
    }
    
    // transpose
    if (direction == 'A') {
      note = note + transpose;
    }
    else note = note - transpose;
  }
}

boolean updatePots() {
  int transpose_new = quantizePot(P_TRANSPOSE, 12);
  int length_new = quantizePot(P_LENGTH, 5);
  int range_new = quantizePot(P_RANGE, 12);
  int speed_new = quantizePot(P_SPEED, 500);
  
  if ((length == length_new) && (transpose == transpose_new) && (range == range_new)) return false;
  else {
    length = length_new;
    transpose = transpose_new;
    range = range_new;
    return true;
  }
}

int quantizePot(int pin, int range) {
  float sensorValue = analogRead(pin);
  
  // assume pot value is 0 - 1024
  // quantize to value between 1 and range
  float value = ((sensorValue / 1024) * (range-1)) + 1;
  return (int)lround(value);
}

//send MIDI message
void MIDImessage(int command, int MIDInote, int MIDIvelocity) {
  midi.write(command);//send note on or note off command 
  midi.write(MIDInote);//send pitch data
  midi.write(MIDIvelocity);//send velocity data
}



