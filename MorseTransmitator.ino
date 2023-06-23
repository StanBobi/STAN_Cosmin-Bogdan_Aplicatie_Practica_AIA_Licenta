#include <PS2Keyboard.h>
#include <LiquidCrystal.h>

#define BUTTON_INPUT 13
#define OUTPUT_PIN 10
#define ARRAY_SIZE 48
#define NUMBER_CHAR 6
#define MORSE_CHARACTERS_NB 36
#define STRING_LENGTH 32//256 

#define EPSILON_K_MEANS 1 // 1ms

#define MARGIN_HIGH 3.75
#define MARGIN_LOW  2.25

#define CLK_PIN 3
#define DATA_PIN 2

#define TIME_MIN_LOWER_THRESHOLD 40U

/* select the pins used on the LCD panel */
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

uint16_t unitTime_u16 = 100;

char lastMessages[STRING_LENGTH * 4] = {'\0'};
uint16_t myMessagesCounter_u16 = 0;

char myString[STRING_LENGTH] = {'\0'};
uint8_t myStringCounter_u8 = 0;

uint16_t btnInputTime_au16[ARRAY_SIZE] = {0};
uint8_t btnInputTime_index = 0;
uint16_t currentInputTime_u16 = 0;
bool btnInputTimeFull_b = false;

/* variable used to check if the init was done */
bool isTheInitDone_b = false;
float centerDot_fl;
float centerLine_fl;

uint8_t lcdButtonIndex_u8 = 0;
uint8_t lcdButtonState_u8 = 0;
uint8_t prevButtonState_u8 = 0;

uint8_t btnLcdOffset = 0;
uint8_t keyboardLcdOffset = 0;

int8_t indexOfDot_i8 = -1;
int8_t indexOfLine_i8 = -1;

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

/* Enum used to display the state of button */
enum ButtonStateEnum
{
  buttonUp           = 0,
  buttonDown         = 1,
  buttonRight        = 2,
  buttonLeft         = 3,
  buttonNotAvailable = 4,
};

enum MenuEnum
{
  startUpMenu         = 0,
  buttonPushHistory   = 1,
  keyboardTextHistory = 2,
  MenuEnumMax         = 3
};

void convertToMorse(char chrToConvert_ch)
{
  for (int i = 0; i < MORSE_CHARACTERS_NB; i++)
  {

    if (chrToConvert_ch == (char)morseChr[i][0][0])
    {
      for (int j = 0; j < strlen(morseChr[i][1]); j++)
      {

        digitalWrite(OUTPUT_PIN, HIGH);
        lcd.setCursor(j + 5, 1);
        
        if (morseChr[i][1][j] == '-')
        {
          delay(3 * unitTime_u16); 
          lcd.print('-');
        }
        else
        {
          delay(unitTime_u16);
          lcd.print('.');
        }
          
        //Serial.print(morseChr[i][1][j]);

        digitalWrite(OUTPUT_PIN, LOW);

        // pause -> part of the same letter
        delay(unitTime_u16);
      }

      return;
    }
  }
}

void printStringOnLcd(char inputString_ach[], uint8_t inputStringCounter_u8)
{
    lcd.clear();
    lcd.setCursor(11, 1);
    lcd.print("/");
    lcd.setCursor(12, 1);
    lcd.print((STRING_LENGTH - 1));
    lcd.setCursor(15, 1);
    lcd.print(")");

    uint8_t maxNbOfChar_u8 = 0;

    if ( 10 > inputStringCounter_u8)
    {
      lcd.setCursor(9, 1);
      lcd.print("(");
      lcd.setCursor(10, 1);
      lcd.print(inputStringCounter_u8);
      maxNbOfChar_u8 = 32 - 7;
    }
    else if( 100 > inputStringCounter_u8)
    {
      lcd.setCursor(8, 1);
      lcd.print("(");
      lcd.setCursor(9, 1);
      lcd.print(inputStringCounter_u8);
      maxNbOfChar_u8 = 32 - 8;
    }
    else
    {
      lcd.setCursor(7, 1);
      lcd.print("(");
      lcd.setCursor(8, 1);
      lcd.print(inputStringCounter_u8);
      maxNbOfChar_u8 = 32 - 9;
    }

    if (maxNbOfChar_u8 >= inputStringCounter_u8)
    {
      for(uint8_t i_u8 = 0; i_u8 < inputStringCounter_u8; i_u8++)
      {
        if (16 > i_u8) lcd.setCursor(i_u8, 0);
        else lcd.setCursor(i_u8 - 16, 1);

        lcd.print(inputString_ach[i_u8]);
      }
    }
    else
    {
      for(uint8_t i_u8 = (inputStringCounter_u8 - 1); i_u8 > (inputStringCounter_u8-1) - maxNbOfChar_u8; i_u8--)
      {
        if ( i_u8 - (inputStringCounter_u8 - maxNbOfChar_u8)  >= 16) 
        {
          lcd.setCursor(i_u8 - (inputStringCounter_u8 - maxNbOfChar_u8) - 16, 1);
        }
        else 
        {
          lcd.setCursor(i_u8 - (inputStringCounter_u8 - maxNbOfChar_u8)  , 0);
        }
        
        lcd.print(inputString_ach[i_u8]);
      }
    }

}

void removeSentence(char listOfMsg_ach[], uint16_t &nbOfChars_u16)
{
  uint16_t index_u16 = 0;
  
  /* Find the end of the first sentence */ 
  for(uint16_t i_u16 = 0; i_u16 < nbOfChars_u16; i_u16++)
  {
    if ('.' == listOfMsg_ach[i_u16])
    {
      index_u16 = i_u16;
      break;
    }
    else{}
  }

  index_u16++;
  for(uint16_t i_u16 = index_u16; i_u16 < nbOfChars_u16; i_u16++)
  { 
    listOfMsg_ach[i_u16 - index_u16] = listOfMsg_ach[i_u16];
  }
  
  nbOfChars_u16 -= index_u16;

}

void readKeyboard(byte datarecei)
{
  
  if ((65 <= datarecei && 90 >= datarecei) || (48 <= datarecei && 57 >= datarecei) || 32 == datarecei)
  {
    if (STRING_LENGTH > myStringCounter_u8 + 1)
    {
      myString[myStringCounter_u8] = (char)datarecei;
      myString[myStringCounter_u8 + 1] = '\0';
      myStringCounter_u8++;
    }
    printStringOnLcd(myString, myStringCounter_u8);
  }
  else if (97 <= datarecei && 122 >= datarecei)
  {
    if (STRING_LENGTH > myStringCounter_u8 + 1)
    {
      /* convert to upper case */
      myString[myStringCounter_u8] = (char)datarecei - ('a' - 'A');
      myString[myStringCounter_u8 + 1] = '\0';
      myStringCounter_u8++;
    }
    printStringOnLcd(myString, myStringCounter_u8);
  }
  /* DELETE */
  else if ((byte)127 == datarecei)
  {
    if (0 < myStringCounter_u8)
    {
      myStringCounter_u8--;
      myString[myStringCounter_u8] = '\0';
    }
    printStringOnLcd(myString, myStringCounter_u8);
  }
  /* ENTER */
  else if ((byte)13 == datarecei)
  {
   
   if(myStringCounter_u8 > 0) 
   { 
    /* reset the offset for keyboard */
    keyboardLcdOffset = 0;
    uint8_t preMyStringCounter_u8 = myStringCounter_u8;
    
    /* Add spaces after each enter -> spaces will fill the gaps until the next 16 multiplier */
    for (uint16_t i = myStringCounter_u8; (i < myStringCounter_u8 + 16 + 1) && (i < STRING_LENGTH); i++)
    {
      Serial.println(i);
      if (i > 1)
      {
        if ((i + 1)%16 == 0)
        {
          myStringCounter_u8 = i;
          break;   
        }
        else
        {
          myString[i] = ' ';
        }
      }
    }
    
      /* Store the current array in the chat history */
    while((myMessagesCounter_u16 + myStringCounter_u8 + 2) >= (4*STRING_LENGTH))
    {
      /*
      Serial.println("Not enough space :(");
      Serial.print(myMessagesCounter_u16 + myStringCounter_u8 + 2);
      Serial.print(" < ");
      Serial.println(4 * STRING_LENGTH);
      */
      removeSentence(lastMessages, myMessagesCounter_u16);
    }

    if ((myMessagesCounter_u16 + myStringCounter_u8 + 1) < (4*STRING_LENGTH))
    {
      /*
      Serial.println("Insertion done :) ");
      Serial.print(myMessagesCounter_u16 + myStringCounter_u8 + 1);
      Serial.print(" < ");
      Serial.println(4 * STRING_LENGTH);
      */
      for(uint16_t myIndex_u16 = myMessagesCounter_u16; myIndex_u16 < myStringCounter_u8 + myMessagesCounter_u16; myIndex_u16++)
      {
        lastMessages[myIndex_u16] = myString[myIndex_u16 - myMessagesCounter_u16];
      }
      
      myMessagesCounter_u16 += myStringCounter_u8;
      lastMessages[myMessagesCounter_u16] = '.';
      myMessagesCounter_u16 += 1;
      
      for (uint16_t textIndex = 0; textIndex < myMessagesCounter_u16; textIndex += 1) 
      {
        Serial.print(lastMessages[textIndex]);
      }
      Serial.println(" ");
      Serial.print("Lungime: ");
      Serial.println(myMessagesCounter_u16);
      Serial.print("\n");

      // Serial.print("Mesaj: ");
      // Serial.print(lastMessages);
      // Serial.print(" strlen = ");
      // Serial.println(myMessagesCounter_u16);
      // Serial.println("\n\n");
    }
    
    for (int i = 0; i < preMyStringCounter_u8; i++)
    {
      /* SPACE BETWEEN WORDS */
      if (32 == byte(myString[i]))
      {
        /* 4*UNIT + 3*UNIT */
        lcd.clear();
        lcd.setCursor(3,0);
        lcd.print("WORD PAUSE");
        lcd.setCursor(5,1);
        lcd.print("----");
        Serial.print("  ");
        delay(4 * unitTime_u16);
      }
      else if(('A' <= myString[i] && 'Z' >= myString[i]) || ('0' <= myString[i] && myString[i] <= '9'))
      {
        Serial.print(myString[i]);
        lcd.clear();
        lcd.setCursor(5, 0);
        lcd.print(myString[i]);
        lcd.setCursor(1, 0);
        
        /* this is a character from a word */
        convertToMorse(myString[i]);
      }

      /* clear */
      myString[i] = '\0';
      Serial.print(' ');

      /* space between letters */
      delay(3 * unitTime_u16);
    }

    Serial.println(' ');
    
    myStringCounter_u8 = 0;
    lcdButtonIndex_u8 = keyboardTextHistory;
    lcdDisplayUpdate(buttonNotAvailable);
   }
  }
}


void initDisplay()
{
  for (uint8_t i = 0; i < 16; i++) 
  {
    lcd.setCursor(i, 0);
    lcd.print("-");

    lcd.setCursor(15-i, 1);
    lcd.print("-");
   
    delay(50);
  }
  
  delay(500);
  lcd.clear();
  lcd.setCursor(0, 0);

  lcd.print("- CODUL  MORSE -");
  delay(500);
  
  lcd.setCursor(0, 1);
  lcd.print("- TRANSMITATOR -");
  delay(1500);

  
  for (uint8_t i = 0; i < 16; i++) 
  {
    lcd.setCursor(i, 0);
    lcd.print("-");

    lcd.setCursor(15-i, 1);
    lcd.print("-");
   
    delay(50);
  }
  
  lcd.clear();
  /* setup message */
  lcd.setCursor(0, 0);
  lcd.print("Salut! Hai sa ne ");
  lcd.setCursor(0, 1);
  lcd.print(" imprietenim :)");
  delay(2000);
  

  
  
  for (uint8_t i = 0; i < 16; i++) 
  {
    lcd.setCursor(i, 0);
    lcd.print("-");

    lcd.setCursor(15-i, 1);
    lcd.print("-");
   
    delay(50);
  }
  
  lcd.clear();
  /* setup message */
  lcd.setCursor(0, 0);
  lcd.print("In codul Morse  ");
  lcd.setCursor(0, 1);
  lcd.print(" exista . si -");
  delay(2000);

  
  for (uint8_t i = 0; i < 16; i++) 
  {
    lcd.setCursor(i, 0);
    lcd.print("-");

    lcd.setCursor(15-i, 1);
    lcd.print("-");
   
    delay(50);
  }
  
  lcd.clear();
  /* setup message */
  lcd.setCursor(0, 0);
  lcd.print("Un dash este cat");
  lcd.setCursor(0, 1);
  lcd.print(" 3 dot-uri ");
  delay(1500);
  
  for (uint8_t i = 0; i < 16; i++) 
  {
    lcd.setCursor(i, 0);
    lcd.print("-");

    lcd.setCursor(15-i, 1);
    lcd.print("-");
   
    delay(50);
  }
  
  lcd.clear();
  /* setup message */
  lcd.setCursor(0, 0);
  lcd.print("In stanga-sus es");
  lcd.setCursor(0, 1);
  lcd.print("te un buton rosu");
  delay(1500);
}

void pushedButtonMsg(uint16_t currentInputTime_u16)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("- BUTON  APASAT -");
  lcd.setCursor(0, 1);
  lcd.print("--- ");
  lcd.setCursor(4, 1);
  lcd.print(currentInputTime_u16);
  lcd.setCursor(10, 1);
  lcd.print(" ms ---"); 
}

uint8_t readLcdButtonState_u8()
{
  uint16_t value = analogRead(0);

  /* no press */
  if (1500 < value) return (uint8_t)buttonNotAvailable;
  
  /* right */
  if (50  > value)  return (uint8_t)buttonRight;
  /* up */
  if (195 > value)  return (uint8_t)buttonUp;
  /* down */
  if (380 > value)  return (uint8_t)buttonDown;
  /* left */
  if (500 > value)  return (uint8_t)buttonLeft;
  /* Select */
  if (700 > value)  return (uint8_t)buttonNotAvailable;
  
  return (uint8_t)buttonNotAvailable;
}

void lcdDisplayUpdate(uint8_t lcdBtnState_u8)
{
  switch (lcdBtnState_u8)
  {
    case buttonLeft: 
      if ( ( (uint8_t)startUpMenu + 1)  < lcdButtonIndex_u8) lcdButtonIndex_u8--;
      else lcdButtonIndex_u8 =  MenuEnumMax - 1;
    break;

    case buttonRight: 
      if ( ((uint8_t)MenuEnumMax - 1) > lcdButtonIndex_u8) lcdButtonIndex_u8++;
      else lcdButtonIndex_u8 = startUpMenu + 1;
    break;

    case buttonUp: 
      if (buttonPushHistory == lcdButtonIndex_u8)
      {
        if(!(btnInputTimeFull_b))
        {  
          if(btnLcdOffset + 1 < btnInputTime_index)
          {
            btnLcdOffset++;
          }
          else
          {
            
            if (0 < btnInputTime_index)
            {
              btnLcdOffset = btnInputTime_index - 1;
            }
            else
            {
              btnLcdOffset = 0;
            }
             
          }
          
        }
        else
        {
          if(btnLcdOffset + 1 < ARRAY_SIZE)
          {
            btnLcdOffset++;
          }
          else
          {
            btnLcdOffset = ARRAY_SIZE - 1;
          }
        }
      }
      else if (keyboardTextHistory == lcdButtonIndex_u8)
      {
        uint16_t multiplier_u16 = 0;

          /* cauta cel mai apropiat multiplu de 16 mai mare decat numarul de caractere din text */
          for(uint16_t searchIndex = myMessagesCounter_u16; searchIndex < (myMessagesCounter_u16 * 16 + 1); searchIndex++)
          {
            if (searchIndex % 16 == 0 && 0 != searchIndex)
            {
              multiplier_u16 = (searchIndex / 16) - 1;
              break;
            }
          }

        if (keyboardLcdOffset < (multiplier_u16*16))
        {
          keyboardLcdOffset+=16;
        }
        else 
        {
          keyboardLcdOffset = multiplier_u16 * 16;
        }
      }
    break;

    case buttonDown: 
      if (buttonPushHistory == lcdButtonIndex_u8)
      {
          if(btnLcdOffset - 1 > 0)
          {
            btnLcdOffset--;
          }
          else
          {
            btnLcdOffset = 0;
          }
      }
      else if (keyboardTextHistory == lcdButtonIndex_u8)
      {
        if (keyboardLcdOffset - 16 > 0)
        {
            keyboardLcdOffset-=16;
        }
        else 
        {
          keyboardLcdOffset = 0;
        }
      }
    break;

    default: 
    break;
  
  }

  switch (lcdButtonIndex_u8)
  {
    case buttonPushHistory:

      /* Reset offset for keyboard */
      keyboardLcdOffset = 0;
      
      /* Menu for the Button History */
      lcd.clear();
      
      lcd.setCursor(0, 0);
      lcd.print("Button History");
      lcd.setCursor(0, 1);

      if (0 < btnInputTime_index || (0 == btnInputTime_index && true == btnInputTimeFull_b ))
      {
        lcd.print("i=");
      
        lcd.setCursor(2, 1);
        
        if (btnInputTimeFull_b)
        {
          lcd.print(ARRAY_SIZE - btnLcdOffset - 1);
        }
        else 
        {
          lcd.print(btnInputTime_index - btnLcdOffset - 1);
        }
        
        lcd.setCursor(10, 1);
        
        if (btnInputTimeFull_b) 
        {
          lcd.print(btnInputTime_au16[ARRAY_SIZE - btnLcdOffset - 1]);
        }
        else
        {
          lcd.print(btnInputTime_au16[btnInputTime_index - btnLcdOffset - 1]);
        }
        
        lcd.setCursor(14, 1);
        lcd.print("ms");
      }
      else
      {
        lcd.print("No recent data from button");
      }
      break;
    
    case keyboardTextHistory:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Chat History: ");
      lcd.setCursor(0, 1);

      uint16_t higherBound_u16 = myMessagesCounter_u16;
      uint16_t lowerBound_u16 = myMessagesCounter_u16 - 16;
      
      for(uint16_t i_u16 = (lowerBound_u16 - keyboardLcdOffset) ; i_u16 < (higherBound_u16 - keyboardLcdOffset); i_u16++)
      {
        lcd.setCursor((i_u16 - lowerBound_u16 + keyboardLcdOffset), 1);
        if (('A' <= lastMessages[i_u16] && 'Z' >= lastMessages[i_u16]) || 
            ('0' <= lastMessages[i_u16] && '9' >= lastMessages[i_u16]) || 
             ' ' == lastMessages[i_u16] ||
             '.' == lastMessages[i_u16]) 
        {
          lcd.print(lastMessages[i_u16]);  
        }
      
      }
      
      break;
    
    default:
      break;
  }
}

void kMeans(uint16_t *btnInputTime_pau16, 
            bool btnInputTimeFull_b, 
            uint8_t btnInputTime_index, 
            float &centerDot_fl, 
            float &centerLine_fl,
            int8_t &indexOfDot_i8,
            int8_t &indexOfLine_i8,
            uint16_t &unitTime_u16)
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

  uint8_t numOfValues_u8 = btnInputTime_index;
  if (true == btnInputTimeFull_b)
  {
    numOfValues_u8 = ARRAY_SIZE;
  }

  do
  {
    oldCenterDot_fl = centerDot_fl;
    oldCenterLine_fl = centerLine_fl;
    
    for (uint8_t i = 0; i < numOfValues_u8; i++)
    {
      
      // verify which is the nearest center for the instance
      if (abs((btnInputTime_pau16[i] - centerDot_fl)) < abs((btnInputTime_pau16[i] - centerLine_fl)))
      {
        /*  ERROR CHEK: 
         *  Do not replace the current value from buffer (where the index is) 
         *  if the number of values on which the center is based is equal to 1.
         *  Otherwise the value will cause the removal of the center -> next cycle 
         *  will compute the centers close to eachother.
         */
        sumOfDots_u32 += btnInputTime_pau16[i];
        nbOfDots_u8++;
        indexOfDot_i8 = i;
      }
      else
      {
        sumOfLines_u32 += btnInputTime_pau16[i];
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

    unitTime_u16 = centerDot_fl;
     
}

uint16_t askForData(uint16_t lowLimit_u16, uint16_t highLimit_u16, String mesaj)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(mesaj);

  uint16_t readValue = 0;
  bool isRead = false;

  do{
    readValue = 0;
    isRead = false;

    while(digitalRead(BUTTON_INPUT))
    {
      readValue++;
      isRead = true;
      delay(1);
    }

    if (isRead)
    {
       /* FOR DEBUG */
      Serial.print(highLimit_u16);
      Serial.print(" > ");
      Serial.println(readValue);
      Serial.print(lowLimit_u16);
      Serial.print(" < ");
      Serial.println(readValue);
      Serial.println(" ");
      delay(1);
    }
    delay(1);
    
  }while( highLimit_u16 <= readValue || lowLimit_u16 >= readValue);
  
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Perfect!");
  lcd.setCursor(4,1);
  lcd.print(String(readValue));
  lcd.setCursor(8,1);
  lcd.print("ms");
  
  delay(1000);
  lcd.clear();
  
  return readValue;
}

void myFirstSetup(uint16_t *btnInputTime_pau16, uint8_t &btnInputTime_index, bool &btnInputTimeFull_b)
{
  uint16_t myTime = 1500;
  btnInputTime_pau16[0] = askForData(0, 1000, "Trimite un DOT");

  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("(DASH=3*DOT)");
  delay(myTime);
  btnInputTime_pau16[1] = askForData(btnInputTime_pau16[0] * MARGIN_LOW, btnInputTime_pau16[0] * MARGIN_HIGH, "Trimite un DASH");

  
  btnInputTime_pau16[2] = askForData(0.5 *  btnInputTime_pau16[0], 2 *  btnInputTime_pau16[0], "Trimite un DOT");

  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("(DASH=3*DOT)");
  delay(myTime/3);
  btnInputTime_pau16[3] = askForData((btnInputTime_pau16[0] + btnInputTime_pau16[2]) / 2.0 * MARGIN_LOW, 
                                (btnInputTime_pau16[0]+ btnInputTime_pau16[2]) / 2.0 * MARGIN_HIGH, 
                                "Trimite un DASH");


  
  btnInputTime_pau16[4] = askForData(0.5 * (btnInputTime_pau16[0] +  btnInputTime_pau16[2]) / 2.0, 
                                    btnInputTime_pau16[0] +  btnInputTime_pau16[2], "Trimite un DOT");

  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("(DASH=3*DOT)");
  delay(myTime/3);
  btnInputTime_pau16[5] = askForData((btnInputTime_pau16[0] + btnInputTime_pau16[2] + btnInputTime_pau16[4]) / 3.0 * MARGIN_LOW, 
                                (btnInputTime_pau16[0]+ btnInputTime_pau16[2] + btnInputTime_pau16[4]) / 3.0 * MARGIN_HIGH, 
                                "Trimite un DASH");

  btnInputTime_index = 6;
  if (btnInputTime_index >= ARRAY_SIZE)
  {
    btnInputTime_index = 0;
    btnInputTimeFull_b = true;                              
  }

  int8_t indexOfDot_i8 = -1;
  int8_t indexOfLine_i8 = -1;

  centerDot_fl = (btnInputTime_pau16[0] + btnInputTime_pau16[2] + btnInputTime_pau16[4]) / 3.0;
  centerLine_fl = (btnInputTime_pau16[1] + btnInputTime_pau16[3] + btnInputTime_pau16[5]) / 3.0;

  kMeans(btnInputTime_pau16, 
         btnInputTimeFull_b, 
         btnInputTime_index, 
         centerDot_fl, 
         centerLine_fl,
         indexOfDot_i8,
         indexOfLine_i8,
         unitTime_u16);

  /* FOR DEBUG */ 
  Serial.println("Dupa kmeans centrele sunt: ");
  Serial.println(centerDot_fl);Serial.println(centerLine_fl);
  Serial.println(centerLine_fl / centerDot_fl);
  Serial.println("\n");

  uint16_t retVal_u16 = 0;
  
  /* CHECK THE RATIO BETWEEN CENTERS */
  while(centerLine_fl / centerDot_fl < 2.75)
  {
    /* Jump over this index -> do not remove this value from the array */
    if (btnInputTime_index == indexOfDot_i8 || btnInputTime_index == indexOfLine_i8)
    {
      btnInputTime_index++;
      if (btnInputTime_index >= ARRAY_SIZE)
      {
        btnInputTime_index = 0;
        btnInputTimeFull_b = true;                              
      }
    }

    retVal_u16 = askForData(centerDot_fl * 0.5, centerDot_fl * 2, "Trimite un DOT");
    
    if (!((centerDot_fl * 2.75 > centerLine_fl) && (centerLine_fl > retVal_u16 && centerDot_fl < retVal_u16))) 
    {
      btnInputTime_pau16[btnInputTime_index] = retVal_u16;
    
      btnInputTime_index++;
      if (btnInputTime_index >= ARRAY_SIZE)
      {
        btnInputTime_index = 0;
        btnInputTimeFull_b = true;                              
      }
    }
    else
    {
      Serial.print("Valoare ne va incurca -> o ignoram: ");
      Serial.print(centerDot_fl);
      Serial.print(" < " );
      Serial.print(retVal_u16);
      Serial.print(" < " );
      Serial.println(centerLine_fl);
    }
    
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("(DASH=3*DOT)");
    delay(myTime/3);
    
    
    /* Jump over this index -> do not remove this value from the array */
    if (btnInputTime_index == indexOfDot_i8 || btnInputTime_index == indexOfLine_i8)
    {
      btnInputTime_index++;
      if (btnInputTime_index >= ARRAY_SIZE)
      {
        btnInputTime_index = 0;
        btnInputTimeFull_b = true;                              
      }
    }

    if (!((centerDot_fl * 2.75 > centerLine_fl) && (centerLine_fl > retVal_u16 && centerDot_fl < retVal_u16))) 
    {
      btnInputTime_pau16[btnInputTime_index] = askForData(centerDot_fl * MARGIN_LOW, centerDot_fl * MARGIN_HIGH, "Trimite un DASH");
  
      btnInputTime_index++;
      if (btnInputTime_index >= ARRAY_SIZE)
      {
        btnInputTime_index = 0;
        btnInputTimeFull_b = true;                              
      }
    }
    else
    {
      Serial.print("Valoare ne va incurca -> o ignoram: ");
      Serial.print(centerDot_fl);
      Serial.print(" < " );
      Serial.print(retVal_u16);
      Serial.print(" < " );
      Serial.println(centerLine_fl);
    }

    /* Update kMeans and check the center values */ 
    kMeans(btnInputTime_pau16, 
         btnInputTimeFull_b, 
         btnInputTime_index, 
         centerDot_fl, 
         centerLine_fl,
         indexOfDot_i8,
         indexOfLine_i8,
         unitTime_u16);

         
    /* FOR DEBUG */
    if (btnInputTimeFull_b) 
    {
      for(int ix = 0; ix < ARRAY_SIZE; ix++) {Serial.print(btnInputTime_pau16[ix]);Serial.print(' ');}
    }
    else 
    {
       for(int ix = 0; ix < btnInputTime_index; ix++) {Serial.print(btnInputTime_pau16[ix]);Serial.print(' ');}
    }

    Serial.print("unitTime_u16: ");
    Serial.println(unitTime_u16);
    Serial.println("Dupa kmeans centrele sunt: ");
    Serial.println(centerDot_fl);Serial.println(centerLine_fl);
    Serial.println(centerLine_fl / centerDot_fl);
    Serial.println("\n"); 
    
    
  }

}

void syncTxRx(uint16_t centerDot_u16, uint16_t centerLine_u16)
{
  digitalWrite(OUTPUT_PIN, LOW);
  delay(centerDot_u16);
  
  /* . */
  digitalWrite(OUTPUT_PIN, HIGH);
  delay(centerDot_u16);
  digitalWrite(OUTPUT_PIN, LOW);
  delay(centerDot_u16);
  /* . */
  digitalWrite(OUTPUT_PIN, HIGH);
  delay(centerDot_u16);
  digitalWrite(OUTPUT_PIN, LOW);
  delay(centerDot_u16);
  /* . */
  digitalWrite(OUTPUT_PIN, HIGH);
  delay(centerDot_u16);
  digitalWrite(OUTPUT_PIN, LOW);
  delay(centerLine_u16); /* eof ch */
  
  /* - */
  digitalWrite(OUTPUT_PIN, HIGH);
  delay(centerLine_u16);
  digitalWrite(OUTPUT_PIN, LOW);
  delay(centerLine_u16); /* eof ch */

  
  /* . */
  digitalWrite(OUTPUT_PIN, HIGH);
  delay(centerDot_u16);
  digitalWrite(OUTPUT_PIN, LOW);
  delay(centerDot_u16);
  /* - */ 
  digitalWrite(OUTPUT_PIN, HIGH);
  delay(centerLine_u16);
  digitalWrite(OUTPUT_PIN, LOW);
  delay(centerLine_u16); /* eof ch */

  /* - */
  digitalWrite(OUTPUT_PIN, HIGH);
  delay(centerLine_u16);
  digitalWrite(OUTPUT_PIN, LOW);
  delay(centerDot_u16);
  /* . */
  digitalWrite(OUTPUT_PIN, HIGH);
  delay(centerDot_u16);
  digitalWrite(OUTPUT_PIN, LOW);
  delay(centerLine_u16);

  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Super! Transmite");
  lcd.setCursor(3, 1);
  lcd.print("ce doresti");
  delay(1500);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("folosind butonul");
  lcd.setCursor(1, 1);
  lcd.print("sau tastatura");
  delay(1500);
  
}

int main(void)
{
  init();
  
  lcd.begin(16, 2);              /* start the library */
  lcd.setCursor(0, 0);          

  PS2Keyboard mykb;
  mykb.begin(DATA_PIN, CLK_PIN);

  pinMode(BUTTON_INPUT, INPUT);
  pinMode(OUTPUT_PIN, OUTPUT);

  Serial.begin(9600);

  initDisplay();
  myFirstSetup(btnInputTime_au16, btnInputTime_index, btnInputTimeFull_b);

  /* Init the receiver */
  syncTxRx(centerDot_fl, centerLine_fl);
 
  while (1)
  {
    if (mykb.available())
    {
      byte datarecei = mykb.read();
      /* //Serial.print(datarecei); */
      //Serial.print("o sa folosesc ca unitate de timp pt un dot: ");
      //Serial.println(unitTime_u16);

       readKeyboard(datarecei);
    }    
    else
    {
      currentInputTime_u16 = 0;
      uint8_t btnState_u8 = digitalRead(BUTTON_INPUT);
      digitalWrite(OUTPUT_PIN, btnState_u8);
      
      while(btnState_u8)
      {
        currentInputTime_u16++;
        delay(1);
        btnState_u8 = digitalRead(BUTTON_INPUT);
        digitalWrite(OUTPUT_PIN, btnState_u8);
      }
      /* ignore the values which are to short to represent an input time */
      if ( TIME_MIN_LOWER_THRESHOLD < currentInputTime_u16 ) 
      {
        if (!((centerDot_fl * 2.75 > centerLine_fl) && (centerLine_fl > currentInputTime_u16 && centerDot_fl < currentInputTime_u16))) 
        {

          /* Jump over this index -> do not remove this value from the array */
          if (btnInputTime_index == indexOfDot_i8 || btnInputTime_index == indexOfLine_i8)
          {
            btnInputTime_index++;
            if (btnInputTime_index >= ARRAY_SIZE)
            {
              btnInputTime_index = 0;
              btnInputTimeFull_b = true;                              
            }
          }

          btnInputTime_au16[btnInputTime_index] = currentInputTime_u16;
        
          Serial.println(currentInputTime_u16);
          btnInputTime_index++;
          
          if (ARRAY_SIZE <= btnInputTime_index)
          {
            btnInputTimeFull_b = true;
            btnInputTime_index = 0;
          }
        }
        else
        {
          Serial.print("OutOfINIT - Valoare ne va incurca -> o ignoram: ");
          Serial.print(centerDot_fl);
          Serial.print(" < " );
          Serial.print(currentInputTime_u16);
          Serial.print(" < " );
          Serial.println(centerLine_fl);
        }
        
        pushedButtonMsg(currentInputTime_u16);
        kMeans(btnInputTime_au16, btnInputTimeFull_b, btnInputTime_index, centerDot_fl, centerLine_fl, indexOfDot_i8, indexOfLine_i8, unitTime_u16);
      }

      lcdButtonState_u8 = readLcdButtonState_u8();
      if (prevButtonState_u8 != lcdButtonState_u8) lcdDisplayUpdate(lcdButtonState_u8);
      prevButtonState_u8 = lcdButtonState_u8;
      
    }
    delay(1);
  }
  
}
