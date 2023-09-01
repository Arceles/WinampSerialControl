#include <protothreads.h>
#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>

//Debugging
const bool enableButtonDebugging = false;

//Knob actions
const int buttonTurnLeft = 20; 
const int buttonTurnRight = 21;
bool buttonLeftTurned = false;
bool buttonRightTurned = false;
int counterLeftTurned = 0;
int counterRightTurned = 0;

//Push buttons
const int buttonStop = 32;
int buttonStopState = 0;
bool buttonStopChanged = false;

const int buttonPlay = 33;
int buttonPlayState = 0;
bool buttonPlayChanged = false;

const int buttonREC = 35;
int buttonRECState = 0;
bool buttonRECChanged = false;

const int buttonRW = 36;
int buttonRWState = 0;

const int buttonFF = 37;
int buttonFFState = 0;

const int buttonAMode = 38;
int buttonAModeState = 0;
bool buttonAModeChanged = false;

const int buttonSingle = 39;
int buttonSingleState = 0;
bool buttonSingleChanged = false;

const int buttonEditNo = 40;
int buttonEditNoState = 0;
bool buttonEditNoChanged = false;

const int buttonDisplay = 41;
int buttonDisplayState = 0;
bool buttonDisplayChanged = false;

const int buttonEnterYes = 42;
int buttonEnterYesState = 0;
bool buttonEnterYesChanged = false;

const int buttonKnob = 43; 
int buttonKnobState = 0;
bool buttonKnobChanged = false;

const int buttonRehearsal = 44;
int buttonRehearsalState = 0;
bool buttonRehearsalChanged = false;  

const int buttonEject = 45;
int buttonEjectState = 0;
bool buttonEjectChanged = false;

//Display related
const int maxScreenChars = 36;
String inputString = "";      // a String to hold incoming data
String receivedString = "";      // a String to hold incoming data
bool serialEventComplete = false;  // whether the string is complete
int timeOnCounter = 0;
const int maxDisplayOnTime = 500;
bool turnOffDisplay = false;
bool winampRequestActive = false;
//modes
int currentDeviceMode = 0; //0=playback, 1=playlist
//playback mode
int playbackModeCounter = 0;
String receivedPlaybackStatusString = "";
String receivedPlaytimeString = "";
String receivedCurrentSongString = "";
//playlist mode
String receivedFolderListString = "";

//Serial related
int cyclesBetweenInstructions = 5;
int counterCycles = 0;

/* Display constructor */
//U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI(rotation, cs, dc, reset)
U8G2_SSD1322_NHD_256X64_1_4W_HW_SPI u8g2(U8G2_R0, 53, 48, 46);

pt ptDisplay;
int displayThread(struct pt* pt) {
  PT_BEGIN(pt);

  // Loop forever
  for(;;) {
    
    /* draw something on the display with the `firstPage()`/`nextPage()` loop*/
    u8g2.firstPage();
    do 
    {   

      if (turnOffDisplay == false)
      {
        //Playback Mode
        if (currentDeviceMode == 0)
        {
          u8g2.drawStr(0,8,">Playback Mode");
          
          //Display winamp info in screen
          u8g2.drawStr(225,8,receivedPlaybackStatusString.c_str());
          u8g2.drawStr(140,8,receivedPlaytimeString.c_str());

          //String too long, truncate it so it can fit in the screen
          const String tempString = receivedCurrentSongString;
          const int songTitleLength = tempString.length();
          if(songTitleLength > maxScreenChars)
          {
            const int totalLines = songTitleLength/maxScreenChars;
            int i = 0;
            for(i = 0; i < totalLines; i++)
            {
              u8g2.drawStr(0,20+(i*9),tempString.substring((i*maxScreenChars),(i*maxScreenChars + maxScreenChars)).c_str());  
            }
            u8g2.drawStr(0,20+(i*9),tempString.substring((i*maxScreenChars),(songTitleLength-1)).c_str());
          }
          else
          {
            u8g2.drawStr(0,20,tempString.c_str());
          }
        }
        //Playlist Mode
        if (currentDeviceMode == 1)
        {
          u8g2.drawStr(0,8,">Playlist Mode.");
          u8g2.drawStr(0,55,"Press EDIT/NO to Replace.");
          u8g2.drawStr(0,64,"Press ENTER/YES to Enqueue.");

          //String too long, truncate it so it can fit in the screen
          const String tempString = receivedFolderListString;
          const int songTitleLength = tempString.length();
          if(songTitleLength > maxScreenChars)
          {
            const int totalLines = songTitleLength/maxScreenChars;
            int i = 0;
            for(i = 0; i < totalLines; i++)
            {
              u8g2.drawStr(0,20+(i*9),tempString.substring((i*maxScreenChars),(i*maxScreenChars + maxScreenChars)).c_str());  
            }
            u8g2.drawStr(0,20+(i*9),tempString.substring((i*maxScreenChars),(songTitleLength-1)).c_str());
          }
          else
          {
            u8g2.drawStr(0,20,tempString.c_str());
          }
          
        }
      }

      if (enableButtonDebugging == true)
      {
        //Check for actions pressed
        if (buttonLeftTurned == true) {
          u8g2.drawStr(10,30,"inputTurnLeft_ON");
          u8g2.drawStr(10,40,String(counterLeftTurned).c_str());     
        }

        if (buttonRightTurned == true) {
          u8g2.drawStr(10,50,"inputTurnRight_ON");
          u8g2.drawStr(10,60,String(counterRightTurned).c_str());      
        }

        if (buttonEjectState == LOW) {
          u8g2.drawStr(10,30,"inputEject_ON");
        }

        if (buttonKnobState == LOW) {
          u8g2.drawStr(10,30,"inputKnob_ON");
        }
        
        if (buttonRehearsalState == LOW) {
          u8g2.drawStr(10,30,"inputRehearsal_ON");
        }

        if (buttonEnterYesState == LOW) {
          u8g2.drawStr(10,30,"inputEnterYes_ON");
        }

        if (buttonDisplayState == LOW) {
          u8g2.drawStr(10,30,"inputDisplay_ON");
        }

        if (buttonEditNoState == LOW) {
          u8g2.drawStr(10,30,"inputEditNo_ON");
        }

        if (buttonSingleState == LOW) {
          u8g2.drawStr(10,30,"inputSingle_ON");
        }

        if (buttonAModeState == LOW) {
          u8g2.drawStr(10,30,"inputAMode_ON");
        }

        if (buttonRECState == LOW) {
          u8g2.drawStr(10,30,"inputREC_ON");
        }

        if (buttonPlayState == LOW) {
          u8g2.drawStr(10,30,"inputPlay_ON");
        }

        if (buttonStopState == LOW) {
          u8g2.drawStr(10,30,"inputStop_ON");
        }

      }
      
    } while ( u8g2.nextPage() );

    //delay(500);
		PT_YIELD(pt);
  }

  PT_END(pt);
}

pt ptProcessInputs;
int processInputsThread(struct pt* pt) {
  PT_BEGIN(pt);

  // Loop forever
  for(;;) 
  {
    if ((buttonAModeState == LOW) && (buttonAModeChanged == true)) {
      //Switch between playback and playlist modes
      currentDeviceMode++;
      if(currentDeviceMode > 1)
      {
        currentDeviceMode = 0;
      }
      buttonAModeChanged = false;
    }
 
    if ((buttonPlayState == LOW) && (buttonPlayChanged == true)) {
      if (currentDeviceMode == 0)
      {
        //Send play event to winamp
        Serial.write("1\n");
      }

      buttonPlayChanged = false;
      //Reset request in case of delayed startup winamp
      winampRequestActive = false;
      playbackModeCounter = 0;
    }

    if ((buttonStopState == LOW) && (buttonStopChanged == true)) {
      if (currentDeviceMode == 0)
      {
        //Send stop event to winamp
        Serial.write("2\n");
      }
      buttonStopChanged = false;
    }

    if ((buttonRECState == LOW) && (buttonRECChanged == true)) {
      if (currentDeviceMode == 0)
      {
        //Send pause event to winamp
        Serial.write("3\n");
      }
      buttonRECChanged = false;
    }

    if (buttonRightTurned == true)
    {
      if (currentDeviceMode == 0)
      {
        //Send next song event to winamp
        Serial.write("b\n");
      }
      else if (currentDeviceMode == 1)
      {
        //Send next song event to winamp
        Serial.write("n\n");
      }
    }

    if (buttonLeftTurned == true)
    {
      if (currentDeviceMode == 0)
      {
        //Send previous song event to winamp
        Serial.write("c\n");
      }      
      else if (currentDeviceMode == 1)
      {
        //Send next song event to winamp
        Serial.write("o\n");
      }
    }

    if ((buttonEditNoState == LOW) && (buttonEditNoChanged == true)) {
      if (currentDeviceMode == 1)
      {
        //Send Enqueue event to winamp
        Serial.write("p\n");
      }
      buttonEditNoChanged = false;
    }

    if ((buttonEnterYesState == LOW) && (buttonEnterYesChanged == true)) {
      if (currentDeviceMode == 1)
      {
        //Send Replace event to winamp
        Serial.write("q\n");
      }
      buttonEnterYesChanged = false;
    }

    PT_YIELD(pt);
  }

  PT_END(pt);
}

pt ptSerialHandler;
int serialHandlerThread(struct pt* pt) {
  PT_BEGIN(pt);

  // Loop forever
  for(;;) 
  {
      //Process serial string
      if ((serialEventComplete == true) && (inputString.length() > 0)) 
      {
        receivedString = inputString;
        
        if(currentDeviceMode == 0)
        {
          //Handle Playback mode Requests
          if (playbackModeCounter == 0) {receivedPlaybackStatusString = receivedString;     playbackModeCounter++;}
          else if (playbackModeCounter == 1) {receivedPlaytimeString = receivedString;      playbackModeCounter++;}
          else if (playbackModeCounter == 2) {receivedCurrentSongString = receivedString;   playbackModeCounter++;}  
        }
        else if(currentDeviceMode == 1)
        {
          //Handle Playlist mode Requests
          receivedFolderListString = receivedString;
        }
                      
        // clear the incoming string:
        inputString = "";
        receivedString = "";
        winampRequestActive = false;
        serialEventComplete = false;
      }
      
      //Turn off display to avoid burn in
      if(timeOnCounter < maxDisplayOnTime)
      {
        turnOffDisplay = false;
        timeOnCounter++;
      }
      else if (timeOnCounter >= maxDisplayOnTime)
      {
        turnOffDisplay = true;
        winampRequestActive = false; //To be able to retry again whenever the screen has been turned off
      }

      if((currentDeviceMode == 0) && (turnOffDisplay == false))
      {
        //Gater information to display in playback or playlist mode
        if ((playbackModeCounter == 0) && (winampRequestActive == false) && (counterCycles >= cyclesBetweenInstructions)) {
          winampRequestActive = true;
          //Ask for playback status to winamp
          Serial.write("7\n");
          counterCycles = 0;
        }
        else if ((playbackModeCounter == 1) && (winampRequestActive == false) && (counterCycles >= cyclesBetweenInstructions)) {
          winampRequestActive = true;
          //Ask for playtime status to winamp
          Serial.write("g\n");
          counterCycles = 0;
        }
        else if ((playbackModeCounter == 2) && (winampRequestActive == false) && (counterCycles >= cyclesBetweenInstructions)) {
          winampRequestActive = true;
          //Ask for playtime status to winamp
          Serial.write("h\n");
          counterCycles = 0;
        }

        if(playbackModeCounter > 2)
        {
          //Reset counter to 0 to ask for data again
          playbackModeCounter = 0;
        }

        if (counterCycles < cyclesBetweenInstructions)
        {
          counterCycles++;
        }
        
      }
      else if((currentDeviceMode == 1) && (turnOffDisplay == false))
      {
        // Do nothing
      }
    PT_YIELD(pt);
  }

  PT_END(pt);
}

pt ptButton;
int buttonThread(struct pt* pt) {
  PT_BEGIN(pt);

  // Loop forever
  for(;;) 
  {
    // read the state of the pushbutton value:
    buttonEjectState = digitalRead(buttonEject);

    if(buttonKnobState != digitalRead(buttonKnob))
    {
      buttonKnobChanged = true;
      buttonKnobState = digitalRead(buttonKnob);
      //Turn on screen
      timeOnCounter = 0;
    }

    buttonRehearsalState = digitalRead(buttonRehearsal);

    if(buttonEnterYesState != digitalRead(buttonEnterYes))
    {
      buttonEnterYesChanged = true;
      buttonEnterYesState = digitalRead(buttonEnterYes);
      //Turn on screen
      timeOnCounter = 0;
    }

    if(buttonDisplayState != digitalRead(buttonDisplay))
    {
      buttonDisplayChanged = true;
      buttonDisplayState = digitalRead(buttonDisplay);
      //Turn on screen
      timeOnCounter = 0;
    }

    if(buttonEditNoState != digitalRead(buttonEditNo))
    {
      buttonEditNoChanged = true;
      buttonEditNoState = digitalRead(buttonEditNo);
      //Turn on screen
      timeOnCounter = 0;
    } 

    buttonSingleState = digitalRead(buttonSingle);

    if(buttonAModeState != digitalRead(buttonAMode))
    {
      buttonAModeChanged = true;
      buttonAModeState = digitalRead(buttonAMode);
      //Turn on screen
      timeOnCounter = 0;
    } 

    if(buttonRECState != digitalRead(buttonREC))
    {
      buttonRECChanged = true;
      buttonRECState = digitalRead(buttonREC);
      //Turn on screen
      timeOnCounter = 0;
    }

    if(buttonPlayState != digitalRead(buttonPlay))
    {
      buttonPlayChanged = true;
      buttonPlayState = digitalRead(buttonPlay);
      //Turn on screen
      timeOnCounter = 0;
    }

    if(buttonStopState != digitalRead(buttonStop))
    {
      buttonStopChanged = true;
      buttonStopState = digitalRead(buttonStop);
      //Turn on screen
      timeOnCounter = 0;
    }

    PT_YIELD(pt);
  }

  PT_END(pt);
}

pt ptKnob;
int knobThread(struct pt* pt) {
  PT_BEGIN(pt);

  // Loop forever
  for(;;) 
  {
    if((counterLeftTurned == 1) )
    {
      counterLeftTurned++;
      buttonLeftTurned = true;
      //Turn on screen
      timeOnCounter = 0;
    }
    
    if((counterRightTurned == 1) )
    {
      counterRightTurned++;
      buttonRightTurned = true;
      //Turn on screen
      timeOnCounter = 0;
    }

    if(counterLeftTurned == 2)
    {
      counterLeftTurned++;
      counterRightTurned = 0;
    }    
    else if((counterLeftTurned >= 3) || (counterLeftTurned == 0))
    {
      buttonLeftTurned = false;
    }

    if(counterRightTurned == 2)
    {
      counterRightTurned++;
      counterLeftTurned = 0;
    }    
    else if((counterRightTurned >= 3) || (counterRightTurned == 0))
    {
      buttonRightTurned = false;
    }

    PT_YIELD(pt);
  }

  PT_END(pt);
}

void setup(void) 
{
  // initialize butons
  pinMode(buttonTurnLeft, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonTurnLeft), knobTurnLeft, CHANGE);
  pinMode(buttonTurnRight, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonTurnRight), knobTurnRight, CHANGE);
  pinMode(buttonEject, INPUT_PULLUP);
  pinMode(buttonKnob, INPUT_PULLUP);
  pinMode(buttonRehearsal, INPUT_PULLUP);
  pinMode(buttonEnterYes, INPUT_PULLUP);
  pinMode(buttonDisplay, INPUT_PULLUP);
  pinMode(buttonEditNo, INPUT_PULLUP);
  pinMode(buttonSingle, INPUT_PULLUP);
  pinMode(buttonAMode, INPUT_PULLUP);
  pinMode(buttonREC, INPUT_PULLUP);
  pinMode(buttonPlay, INPUT_PULLUP);
  pinMode(buttonStop, INPUT_PULLUP);
  /* u8g2.begin() is required and will sent the setup/init sequence to the display */
  u8g2.begin();
  u8g2.setFont(u8g2_font_HelvetiPixel_tr);
  // initialize serial:
  Serial.begin(9600);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
}

void loop(void) 
{
  PT_SCHEDULE(displayThread(&ptDisplay));
  PT_SCHEDULE(serialHandlerThread(&ptSerialHandler));
  PT_SCHEDULE(processInputsThread(&ptProcessInputs));
  PT_SCHEDULE(buttonThread(&ptButton));
  PT_SCHEDULE(knobThread(&ptKnob));
}

void knobTurnLeft() 
{
    if(HIGH == digitalRead(buttonTurnRight) && (counterRightTurned == 0))
    {
      counterLeftTurned = 1;
    }  
    else
    {
      counterLeftTurned = 0;
    }
}

void knobTurnRight() 
{
    if(LOW == digitalRead(buttonTurnLeft) && (counterLeftTurned == 0))
    {
      counterRightTurned = 1;
    }  
    else
    {
      counterRightTurned = 0;
    }
}
/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\v') {    
      serialEventComplete = true;
    }
  }
  //Workaround for not finding end of line char
  serialEventComplete = true; 
}