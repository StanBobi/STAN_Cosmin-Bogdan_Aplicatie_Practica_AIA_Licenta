#include <LiquidCrystal.h>

/* The size of the array which will be used to display the received characters */
#define DISPLAY_ARRAY_SIZE                256

/* The number of LCD pixels / line */
#define PIXELS_ON_LINE                    16

/* Transmiter period 1ms */
#define TR_PERIOD                         1000  

/* use 100us period -> 10KHz frequency */
/* Transmiter is using 1000us = 1ms */
#define TIME_MS                           (TR_PERIOD*1.0f/10)
#define TIME_US                           (TIME_MS*1.0f/1000)

#define BUTTON_INPUT                       13
#define PIN_AUDIO                          12
#define ANALOG_INPUT                       0

#define NUMBER_CHAR                        6
#define MORSE_CHARACTERS_NB                36
#define ARRAY_SIZE                         12//48

#define DISH_DASH_RATIO                    (float)(1.0 / 3)

#define PAUSE_THRESHOLD_FACTOR_ONE_UNIT    0.75f
#define PAUSE_THRESHOLD_FACTOR_THREE_UNITS 2.75f
#define PAUSE_THRESHOLD_FACTOR_SEVEN_UNITS 6.75f

#define DETERMINE_TYPE_LOWER_FACTOR        2.5f
#define DETERMINE_TYPE_HIGHER_FACTOR       6.0f

#define EPSILON_K_MEANS                    2.0f


/* Enum used to display the state of button */
enum ButtonStateEnum
{
  buttonUp           = 0,
  buttonDown         = 1,
  buttonNotAvailable = 2
};

/* select the pins used on the LCD panel */
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

char morseChr[MORSE_CHARACTERS_NB][2][NUMBER_CHAR] = {
    {{'A'}, {'.', '-', 0, 0, 0}},       // A
    {{'B'}, {'-', '.', '.', '.', 0}},   // B
    {{'C'}, {'-', '.', '-', '.', 0}},   // C
    {{'D'}, {'-', '.', '.', 0, 0}},     // D
    {{'E'}, {'.', 0, 0, 0, 0}},         // E
    {{'F'}, {'.', '.', '-', '.', 0}},   // F
    {{'G'}, {'-', '-', '.', 0, 0}},     // G
    {{'H'}, {'.', '.', '.', '.', 0}},   // H
    {{'I'}, {'.', '.', 0, 0, 0}},       // I
    {{'J'}, {'.', '-', '-', '-', 0}},   // J
    {{'K'}, {'-', '.', '-', 0, 0}},     // K
    {{'L'}, {'.', '-', '.', '.', 0}},   // L
    {{'M'}, {'-', '-', 0, 0, 0}},       // M
    {{'N'}, {'-', '.', 0, 0, 0}},       // N
    {{'O'}, {'-', '-', '-', 0, 0}},     // O
    {{'P'}, {'.', '-', '-', '.', 0}},   // P
    {{'Q'}, {'-', '-', '.', '-', 0}},   // Q
    {{'R'}, {'.', '-', '.', 0, 0}},     // R
    {{'S'}, {'.', '.', '.', 0, 0}},     // S
    {{'T'}, {'-', 0, 0, 0, 0}},         // T
    {{'U'}, {'.', '.', '-', 0, 0}},     // U
    {{'V'}, {'.', '.', '.', '-', 0}},   // V
    {{'W'}, {'.', '-', '-', 0, 0}},     // W
    {{'X'}, {'-', '.', '.', '-', 0}},   // X
    {{'Y'}, {'-', '.', '-', '-', 0}},   // Y
    {{'Z'}, {'-', '-', '.', '.', 0}},   // Z
    {{'1'}, {'.', '-', '-', '-', '-'}}, // 1
    {{'2'}, {'.', '.', '-', '-', '-'}}, // 2
    {{'3'}, {'.', '.', '.', '-', '-'}}, // 3
    {{'4'}, {'.', '.', '.', '.', '-'}}, // 4
    {{'5'}, {'.', '.', '.', '.', '.'}}, // 5
    {{'6'}, {'-', '.', '.', '.', '.'}}, // 6
    {{'7'}, {'-', '-', '.', '.', '.'}}, // 7
    {{'8'}, {'-', '-', '-', '.', '.'}}, // 8
    {{'9'}, {'-', '-', '-', '-', '.'}}, // 9
    {{'0'}, {'-', '-', '-', '-', '-'}}  // 0
};

uint8_t buttonState_u8 = 0;
uint8_t prevButtonState_u8 = 0;
float counter_fl = 0;

bool bufferFull_b = false;

/* WITH DATASET */

bool kMeansPlusPlusDone_b = true;
float inputValuesBuffer_afl[ARRAY_SIZE] = {10.0f, 10.0f, 30.0f, 30.0f};
float centerDot_fl = 10.0f;
float centerLine_fl = 30.0f;
uint8_t indexBuffer_u8 = 4;
uint8_t cyclicIndexBuffer_u8 = 4;


/* WITHOUT DATASET */
/*
bool kMeansPlusPlusDone_b = false;
float inputValuesBuffer_afl[ARRAY_SIZE] = {0.0f};
float centerDot_fl = 0.0f;
float centerLine_fl = 0.0f;
uint8_t indexBuffer_u8 = 0;
uint8_t cyclicIndexBuffer_u8 = 0;
*/

uint8_t dotCenterIndex_u8 = 0;
uint8_t lineCenterIndex_u8 = 0;

char myCode[NUMBER_CHAR] = "";
uint8_t counterChr_u8 = 0;

float pause_fl = 0;
bool pauseError = false;

bool chrDisp = true;
bool spcDisp = true;

/* --- Variables used by the lcd part --- */

uint16_t dispArrIndex_u16 = 0;
int8_t offsetForScroll_i8 = 0;

uint8_t prebuttonStateLCD_u8 = buttonNotAvailable;
uint8_t buttonStateLCD_u8 = buttonNotAvailable;

char chatHistory_ach[DISPLAY_ARRAY_SIZE] = {'\0'};

/* Check for errors */
uint16_t counterForErrors_u16 = 0;
uint16_t prevSavedTime_u16 = 0;

void shiftValues(char *chatHistory_ach)
{
  uint8_t startIndex;
  for (int i = 0; i < DISPLAY_ARRAY_SIZE; i++)
    if ( ' ' == chatHistory_ach[i])
    {
      startIndex = i;
      break;
    }

  for (int i = 0; i < (DISPLAY_ARRAY_SIZE - startIndex - 1); i++)
  {
    chatHistory_ach[i] = chatHistory_ach[i + startIndex + 1];
  }
  chatHistory_ach[DISPLAY_ARRAY_SIZE - 1] = ' ';

  dispArrIndex_u16 = dispArrIndex_u16 - startIndex;

}

void printOnLcd(char *chatHistory_ach, uint16_t dispArrIndex_u16, int8_t &offsetForScroll_i8)
{
  uint16_t nbOfPixels = 0;

  for (uint16_t i = 2; i < 17; i++)
  {
    if (PIXELS_ON_LINE * i >= dispArrIndex_u16)
    {
      nbOfPixels = PIXELS_ON_LINE * i;
      break;
    }
  }

  lcd.setCursor(0, 0);
  lcd.clear();

  uint16_t lowLimit_u16;
  uint16_t highLimit_u16;


  if (0 == offsetForScroll_i8)
  {
    lowLimit_u16 = nbOfPixels - (2 * PIXELS_ON_LINE);
    highLimit_u16 = dispArrIndex_u16;
  }
  else
  {
    /* check if the value of the offset will result a positive value of low limit */

    /*
    Serial.println("(2 * PIXELS_ON_LINE) - (offsetForScroll_i8 * PIXELS_ON_LINE) > nbOfPixels");
    Serial.print((2 * PIXELS_ON_LINE) - (offsetForScroll_i8 * PIXELS_ON_LINE));
    Serial.print(" > ");
    Serial.println(nbOfPixels);
    */

    if ( (2 * PIXELS_ON_LINE)  - (offsetForScroll_i8 * PIXELS_ON_LINE) > nbOfPixels )
    {
      offsetForScroll_i8 = 2 - (nbOfPixels / PIXELS_ON_LINE);
      lowLimit_u16 = 0;
      if(2 * PIXELS_ON_LINE <= dispArrIndex_u16) highLimit_u16 = 2 * PIXELS_ON_LINE;
      else highLimit_u16 = dispArrIndex_u16;
    }
    else
    {
      lowLimit_u16  = nbOfPixels + (offsetForScroll_i8 * PIXELS_ON_LINE) - (2 * PIXELS_ON_LINE) ;
      highLimit_u16 = nbOfPixels + (offsetForScroll_i8 * PIXELS_ON_LINE);
    }
  }

  for (uint16_t i = lowLimit_u16; i < highLimit_u16; i++)
  {

    if (i < lowLimit_u16 + 16)
    {
      lcd.setCursor(i - lowLimit_u16, 0);
    }
    else
    {
      lcd.setCursor(i - lowLimit_u16 - PIXELS_ON_LINE, 1);
    }
    lcd.print(chatHistory_ach[i]);


  }
}

uint8_t readbuttonState_u8()
{
  uint16_t value = analogRead(0);
  /* no press */
  if (1500 < value) return (uint8_t)buttonNotAvailable;
  
  /* right */
  if (50  > value)  return (uint8_t)buttonNotAvailable;
  /* up */
  if (195 > value)  return (uint8_t)buttonUp;
  /* down */
  if (380 > value)  return (uint8_t)buttonDown;
  /* left */
  if (500 > value)  return (uint8_t)buttonNotAvailable;
  /* Select */
  if (700 > value)  return (uint8_t)buttonNotAvailable;
  
  return (uint8_t)buttonNotAvailable;
}

void onChrReceived(char recChr)
{
  //Serial.print("\nonChrRec: ");
  //Serial.println(recChr);
  //Serial.println("");
  if ( (recChr >= 'A' && recChr <= 'Z') || (recChr >= 'a' && recChr <= 'z') || (recChr == ' ') || (recChr <= '9' && recChr >= '0'))
  {
    chatHistory_ach[dispArrIndex_u16] = recChr;
    if (dispArrIndex_u16 < DISPLAY_ARRAY_SIZE - 2)
    {
      dispArrIndex_u16++;
    }
    else
    {
      shiftValues(chatHistory_ach);
    }
    chatHistory_ach[dispArrIndex_u16] = '\0';

/*
    Serial.println(chatHistory_ach);
    Serial.print("Offset: ");
    Serial.println(offsetForScroll_i8);
*/

    printOnLcd(chatHistory_ach, dispArrIndex_u16, offsetForScroll_i8);
  }
}

void determineCharacter(char code_ac[])
{
  
  for (int i = 0; i < MORSE_CHARACTERS_NB; i++)
  {
    if (strlen(morseChr[i][1]) == strlen(code_ac))
    {
      if (strcmp(morseChr[i][1], code_ac) == 0)
      {
        //Serial.print(morseChr[i][0][0]);
        onChrReceived(morseChr[i][0][0]);
        break;
      }
    }
  }
}

void temporaryMessage(float centerDot_fl, float centerLine_fl)
{
  Serial.print("\n Centrele dupa kMeans: ");
  Serial.print(centerDot_fl);
  Serial.print(' ');
  Serial.println(centerLine_fl);
}

// to remove - just for test
void blockProgram()
{
  char c = '.';
  while (c != '\n')
  {
    while (Serial.available() > 0)
    {
      c = Serial.read();
    }
  }
}

char decideType(float centerDot_fl, float centerLine_fl, float value_fl)
{
  // check  which one is the nearest center
  if (abs(centerDot_fl - value_fl) < abs(centerLine_fl - value_fl))
  {
    return '.';
  }
  else
  {
    return '-';
  }

  return NULL;
}

void afterPause(char myCode[], uint8_t &nr)
{
  /*
    Serial.print(pause);
    Serial.print("    ");
    Serial.print(((sumDot * 1.0 / nbDot) * 3));
    Serial.print(' ');
    Serial.print((3 * ( sumSpc * DISH_DASH_RATIO/ nbSpc)));
    Serial.println(" - asta a fost o litera");
  */

  // Serial.print(myCode);
  // vector de caractere din sir
  nr = 0;

  /*
    Serial.print(" strlen(myCode) = ");
    Serial.println(strlen(myCode));
  */

  determineCharacter(myCode);
  

  for (int i = 0; i < NUMBER_CHAR; i++)
  {
    myCode[i] = '\0';
  }
  // Serial.println("\n----------");
  
}

float computeSum(float *timingsBuffer_pfl, uint8_t nb)
{
  float sum_fl = 0;

  for (uint8_t i = 0; i < nb; i++)
    sum_fl += timingsBuffer_pfl[i];

  return sum_fl;
}

void updateBuffer(float *timingsBuffer_pfl, uint8_t &nb, float val_ui, bool &bufferFull)
{
  timingsBuffer_pfl[nb] = val_ui;
  nb = nb + 1;

  if (nb == ARRAY_SIZE)
    bufferFull = true;

  nb = nb % ARRAY_SIZE;
}

void kMeansPlusPlus(float *inputValuesBuff_pfl,
                    uint8_t numOfValues,
                    float &centerDot_fl,
                    float &centerLine_fl)
{
  float biggestDistance_fl = 0.0;
  float lineCenter_fl = 0.0f;

  // Step 1) Consider the latest value from buffer as one center
  float dotCenter_fl = inputValuesBuff_pfl[numOfValues - 1];

  // Step 2) Select a center - compute the distance from the center to the each value from the buffer.
  //         The farest instance will be the next center
  for (uint8_t i = 0; i < (numOfValues - 1); i++)
  {
    if (inputValuesBuff_pfl[i] - dotCenter_fl > biggestDistance_fl)
    {
      lineCenter_fl = inputValuesBuff_pfl[i];
      biggestDistance_fl = (inputValuesBuff_pfl[i] - dotCenter_fl);
    }
  }

  // Step 3) Repeat the algorithm until there are found n centers - N/A

  // return the values of centers
  if (lineCenter_fl > dotCenter_fl)
  {
    centerLine_fl = lineCenter_fl;
    centerDot_fl = dotCenter_fl;
  }
  else
  {
    centerLine_fl = dotCenter_fl;
    centerDot_fl = lineCenter_fl;
  }
}

void updateIndex(uint8_t &indexBuffer_u8, uint8_t &cyclicIndexBuffer_u8, bool bufferFull_b)
{
  if (true == bufferFull_b)
    indexBuffer_u8 = ARRAY_SIZE;
  else
    indexBuffer_u8 = cyclicIndexBuffer_u8;
}

void updateCentersWithKMeans(float *inputValuesBuff_pfl,
                             uint8_t numOfValues_u8,
                             float &centerDot_fl,
                             float &centerLine_fl,
                             int8_t &indexOfDot_i8,
                             int8_t &indexOfLine_i8)
{

  float epsilon_f = EPSILON_K_MEANS;
  float oldCenterDot_fl;
  float oldCenterLine_fl;

  // 1) First step of the k-means is to select the centers - already done

  // 2) Second step is to alloc the instances to the nearest center
  uint8_t nbOfDots_u8 = 0;
  uint8_t nbOfLines_u8 = 0;
  float sumOfDots_u32 = 0.0;
  float sumOfLines_u32 = 0.0;

  do
  {
    oldCenterDot_fl = centerDot_fl;
    oldCenterLine_fl = centerLine_fl;
    
    for (uint8_t i = 0; i < numOfValues_u8; i++)
    {
      
      // verify which is the nearest center for the instance
      if (abs((inputValuesBuff_pfl[i] - centerDot_fl)) < abs((inputValuesBuff_pfl[i] - centerLine_fl)))
      {
        /*  ERROR CHEK: 
         *  Do not replace the current value from buffer (where the index is) 
         *  if the number of values on which the center is based is equal to 1.
         *  Otherwise the value will cause the removal of the center -> next cycle 
         *  will compute the centers close to eachother.
         */
        sumOfDots_u32 += inputValuesBuff_pfl[i];
        nbOfDots_u8++;
        indexOfDot_i8 = i;
      }
      else
      {
        sumOfLines_u32 += inputValuesBuff_pfl[i];
        nbOfLines_u8++;
        indexOfLine_i8 = i;
      }
      
    }

    // Step 3 - Compute the new center values
    if (nbOfDots_u8 > 0) 
    {
      centerDot_fl = sumOfDots_u32 * 1.0f / nbOfDots_u8;
    }
    else {}
    if (nbOfLines_u8 > 0)
    {
      centerLine_fl = sumOfLines_u32 * 1.0f / nbOfLines_u8;
    }
    else {}
    
    /*
        Serial.print(" Old dot: ");
        Serial.print(oldCenterDot_fl);
        Serial.print(" Dot:  ");
        Serial.print(centerDot_fl);
        Serial.print(" Diff: ");
        Serial.println(abs(centerDot_fl - oldCenterDot_fl));

        Serial.print(" Old line: ");
        Serial.print(oldCenterLine_fl);
        Serial.print(" Line:  ");
        Serial.print(oldCenterLine_fl);
        Serial.print(" Diff: ");
        Serial.println(abs(centerDot_fl - oldCenterDot_fl));
        Serial.print("\n");
    */
  } while (((abs(centerDot_fl - oldCenterDot_fl) > epsilon_f) || (abs(centerLine_fl - oldCenterLine_fl) > epsilon_f)));

    /*  If the number of dots or of the lines is higher than 1
     *  Then, the value which will be removed will not cause the 
     *  removal of the center.
     */
    if (1 != nbOfDots_u8)
    {
      indexOfDot_i8 = -1; 
    }
    if (1 != nbOfLines_u8)
    {
      indexOfLine_i8 = -1;
    }
    
        
    temporaryMessage(centerDot_fl, centerLine_fl);
}

void MorseCodeProcess()
{
  char c = NULL;
  buttonState_u8 = digitalRead(BUTTON_INPUT)^1;
  digitalWrite(PIN_AUDIO, buttonState_u8);
  //Serial.println(buttonState_u8);

  // state = "1"
  if (1 == buttonState_u8 && 1 == prevButtonState_u8)
  {
    counter_fl += TIME_US;

    /* error check */
    if (counterForErrors_u16 < (0.75f * centerDot_fl))
    {
      counterForErrors_u16 += TIME_US; 
    }
  }
  // falling edge
  else if (1 == prevButtonState_u8 && 0 == buttonState_u8)
  {
    counter_fl += TIME_US;

    // Serial.print("counter: ");
    // Serial.print(counter_fl);

    if (!kMeansPlusPlusDone_b)
    {
      float sum_fl = computeSum(inputValuesBuffer_afl, indexBuffer_u8);
      float average_f = sum_fl * 1.0f / indexBuffer_u8;

      updateBuffer(inputValuesBuffer_afl, cyclicIndexBuffer_u8, counter_fl, bufferFull_b);
      updateIndex(indexBuffer_u8, cyclicIndexBuffer_u8, bufferFull_b);

      if ((counter_fl >= (DETERMINE_TYPE_LOWER_FACTOR* average_f) && counter_fl <= (DETERMINE_TYPE_HIGHER_FACTOR * average_f)) ||
          (counter_fl >= (average_f / DETERMINE_TYPE_HIGHER_FACTOR) && counter_fl <= (average_f / DETERMINE_TYPE_LOWER_FACTOR)))
      {

        // start k-means++
        kMeansPlusPlus(inputValuesBuffer_afl, indexBuffer_u8, centerDot_fl, centerLine_fl);
        kMeansPlusPlusDone_b = true;
      }
      else
      {
        // nothing to do
      }
    }
    else
    {
      int8_t indexOfDots_i8;
      int8_t indexOfLines_i8;
      
      updateCentersWithKMeans(inputValuesBuffer_afl, indexBuffer_u8, centerDot_fl, centerLine_fl, indexOfDots_i8, indexOfLines_i8);

      if (true == bufferFull_b)
      {
        if ((-1 != indexOfDots_i8 || -1 != indexOfLines_i8) && (indexOfDots_i8 == cyclicIndexBuffer_u8 || indexOfLines_i8 == cyclicIndexBuffer_u8))
        {
          /* DO NOT REMOVE THE CURRENT VALUE FROM THE CYCLIC BUFFER */
          Serial.println("CAZ SPECIAL:\nDO NOT REMOVE:");
          Serial.println(inputValuesBuffer_afl[cyclicIndexBuffer_u8]);
          cyclicIndexBuffer_u8++;
          cyclicIndexBuffer_u8 = cyclicIndexBuffer_u8 % ARRAY_SIZE;
          //for(int xi = 0; xi < ARRAY_SIZE; xi++) Serial.println(inputValuesBuffer_afl[xi]);
        }
        else
        {
          /* nothing to do */
        }
      }
      else
      {
        /* nothing to do */
      }
      /* check if the difference between centers is higher than 2 */
      if (centerDot_fl * 2 < centerLine_fl)
      {
        /* update the buffer with the newest input value */
        if (counter_fl > 0) 
        {
          updateBuffer(inputValuesBuffer_afl, cyclicIndexBuffer_u8, counter_fl, bufferFull_b);
        //Serial.print("Value: ");Serial.println(counter_fl);
        //Serial.println("");
        updateIndex(indexBuffer_u8, cyclicIndexBuffer_u8, bufferFull_b);
        }
      }
      else
      {
        /* 2*centerDot > centerLine => centers are too close to each other
           The value which will be added shouldn't be between centers, to avoid
           the proximity of the centers.
        */
        if (((centerDot_fl > counter_fl) || (counter_fl > centerLine_fl)) && counter_fl > 0)
        {
          updateBuffer(inputValuesBuffer_afl, cyclicIndexBuffer_u8, counter_fl, bufferFull_b);
          Serial.print("Value: ");Serial.println(counter_fl);
          Serial.println("");
          updateIndex(indexBuffer_u8, cyclicIndexBuffer_u8, bufferFull_b);
        }
        else 
        {
          /* the value is between centers */
          /* centerDot < counter && counter < centerLine */
          Serial.println("Eroare la adaugare value: ");
          Serial.print(counter_fl);
          Serial.print(centerDot_fl);
          Serial.println(centerLine_fl);
         }
      }
      /* determine which is the group for the newest value */
      char inputChr = decideType(centerDot_fl, centerLine_fl, counter_fl);
      //Serial.println(inputChr);

      /* based on the input values -> check if is a line or a dot */
      if (NULL != inputChr && NUMBER_CHAR > (counterChr_u8 + 1))
      {
        myCode[counterChr_u8] = inputChr;
        counterChr_u8++;
      }
    }

    if ( counterForErrors_u16 > 0 )
    {
      prevSavedTime_u16 = counter_fl;
      counterForErrors_u16 -= TIME_US;
    }
    else
    {
      counterForErrors_u16 = 0;
      prevSavedTime_u16 = 0;
    }
    
      
    /* Reset the counter */
    counter_fl = 0;
  }

  
  /* state = "0" */
  else if (0 == prevButtonState_u8 && 0 == buttonState_u8)
  {
    pause_fl += TIME_US;

    /*
      1. The length of a dot is one unit.
      2. A dash is three units.
      3. The space between parts of the same letter is one unit.
      4. The space between letters is three units.
      5. The space between words is seven units.
    */

    float avgDot_fl = centerDot_fl;
    float avgSpc_fl = centerLine_fl;

    /* if the pause is lower than 0.75 of a point maybe this was a line, but was disturbed */
    if ( (pause_fl < (PAUSE_THRESHOLD_FACTOR_ONE_UNIT * avgDot_fl)) &&
         (pause_fl < (PAUSE_THRESHOLD_FACTOR_ONE_UNIT * (avgSpc_fl * DISH_DASH_RATIO) ) )
        )
    {

      /*
        pauseError = true;
        counter += pause;
        Serial.println("Eroare de pauza... Actualizez counter-ul");
        Serial.println("Press something ... ");
        blockProgram();
      */
    }
    else
    {
      if ((pause_fl >= PAUSE_THRESHOLD_FACTOR_ONE_UNIT * avgDot_fl) && 
          (pause_fl >= PAUSE_THRESHOLD_FACTOR_ONE_UNIT * (avgSpc_fl * DISH_DASH_RATIO) )
          )
      {
        /* case 3 -> the space between parts of the same letter is one unit */
        /* nothing to do */
      }
      if (false == chrDisp && 
         (pause_fl >= PAUSE_THRESHOLD_FACTOR_THREE_UNITS * avgDot_fl) && 
         (pause_fl >= PAUSE_THRESHOLD_FACTOR_THREE_UNITS * (avgSpc_fl * DISH_DASH_RATIO))
         )
      {
        chrDisp = true;
        /* case 4 -> determine the character */
        if (kMeansPlusPlusDone_b)
          afterPause(myCode, counterChr_u8);
      }
      if (false == spcDisp && 
         (pause_fl >= PAUSE_THRESHOLD_FACTOR_SEVEN_UNITS * avgDot_fl) && 
         (pause_fl >= PAUSE_THRESHOLD_FACTOR_SEVEN_UNITS * (avgSpc_fl * DISH_DASH_RATIO) )
         )
      {
        spcDisp = true;
        /* case 5 -> this was the word */
        // Serial.println("SPATIU");
        // temporaryMessage(centerDot_fl, centerLine_fl);

        //Serial.print(' ');
        onChrReceived(' ');
        c = ' ';
      }
    }

    /* check for errors */
    if (counterForErrors_u16 >= 0) 
    {
      counterForErrors_u16 -= TIME_US;
    }
    else 
    {
      counterForErrors_u16 = 0;
      prevSavedTime_u16 = 0;
    }
  }
  /* rising edge */
  else if (0 == prevButtonState_u8 && 1 == buttonState_u8)
  {
    chrDisp = false;
    spcDisp = false;

    /* check for errors */
    if (counterForErrors_u16 > 0)
    {
      counter_fl = pause_fl + prevSavedTime_u16;
      Serial.println("Am corectat timpul, a fost o eroare :)");
      if (counterForErrors_u16 < (0.75 * centerDot_fl)) counterForErrors_u16 += TIME_US;
    }
    else
    {
      counter_fl = 0;
      prevSavedTime_u16 = 0; 
    }

    /* reset the pause */
    pause_fl = 0;
  }
  else
  {
    /* nothing to do... */
   
  }

  prevButtonState_u8 = buttonState_u8;
  delay(1);

  /*
    Serial.print("counter: ");
    Serial.println((int)counter_u32);
    Serial.println(" ");
  */
}

int main()
{
  init();

  Serial.begin(9600);
  pinMode(BUTTON_INPUT, INPUT);
  pinMode(PIN_AUDIO, OUTPUT);

  lcd.begin(16, 2);              /* start the library */
  lcd.setCursor(0, 0);          

  unsigned long prevTime_ul = micros();
  
  while (1)
  {
    if(micros() - prevTime_ul > TIME_US * 1000)
    {
      MorseCodeProcess();
      
      buttonStateLCD_u8 = readbuttonState_u8();

    if (buttonDown == (ButtonStateEnum)buttonStateLCD_u8 && buttonNotAvailable == (ButtonStateEnum)prebuttonStateLCD_u8)
    {
      if (offsetForScroll_i8 < 0)
      {
        offsetForScroll_i8++;
        printOnLcd(chatHistory_ach, dispArrIndex_u16, offsetForScroll_i8);
      }
      Serial.print("Offset: ");
      Serial.println(offsetForScroll_i8);
    }
    else if (buttonUp == (ButtonStateEnum)buttonStateLCD_u8 && buttonNotAvailable == (ButtonStateEnum)prebuttonStateLCD_u8)
    {
      offsetForScroll_i8--;
      printOnLcd(chatHistory_ach, dispArrIndex_u16, offsetForScroll_i8);
      //Serial.print("Offset: ");
      //Serial.println(offsetForScroll_i8);
    }
    else
    {
      /* nothing to do */
    }

  prebuttonStateLCD_u8 = buttonStateLCD_u8;


      prevTime_ul = micros();
    }
  }
}
