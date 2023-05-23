#include "msp.h"

//#define


/*********************************
          GLOBAL VARIABLES
 *********************************/

int digit_array[] = {0b11000000, //0
                     0b11111001, //1
                     0b10100100, //2
                     0b10110000, //3
                     0b10011001, //4
                     0b10010010, //5
                     0b10000010, //6
                     0b11111000, //7
                     0b10000000, //8
                     0b10010000, //9
                     0b11111111, //10 "A" Will show blank on display
                     0b11000111, //11 "L"
                     0b11111111, //12 "C" Will show blank on display
                     0b10100001, //13 "d"
                     0b11000000, //14 "O"
                     0b10000110, //15 "E"
                     0b11000110, //16 "C"
                     0b11110111, //17 "_"
                     0b11111111, //18 Blank
                     0b10001100, //19 "p"
                     0b11001000};//20 "n"

int keypad_table[4][4]={{1,2,3,10},
                        {4,5,6,11},
                        {7,8,9,12},
                        {14,0,15,13}};

int port8[]={0b11011111, 0b11101111, 0b11110111, 0b11111011};
//uint8_t display[4]={0,0,0,0};
//uint8_t passcode[4] = {0,0,0,0};

/*************************************
           FUNCTION PROTOTYPES
 ************************************/
void wait(int n);
void show(int numb);
int input_key(int key_press);
int decode(int r, int c);
int openkey_activated(int open);
int lockkey_activated(int lock);
void cleardisp(uint8_t disp[]);
int check_passcode(uint8_t disp[], uint8_t pass[]);
void locdisp(uint8_t disp1[]);
void lddisp(uint8_t disp2[]);
void zerodisp(uint8_t disp3[]);
void opendisp(uint8_t disp[]);
void temp_show(uint8_t disp4[]);
void toggle_led(void);
void load_passcode(uint8_t disp[], uint8_t pass[]);


/**
 * main.c
 */
void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer

/*******************************************
            GPIO INITIALIZATION
 *******************************************/

     /******************************
       P8.x, P4.x, P5.0, P2.5 OUTPUT
     *******************************/
        P8->SEL0 &= ~0x3C;
        P8->SEL1 &= ~0x3C;
        P8->DIR = 0b00111100; //P8.2 to P8.5 is Digit D to A set to OUTPUT

        P4->SEL0 &= ~0xFF;
        P4->SEL1 &= ~0xFF;
        P4->DIR = 0b11111111; //All of P4 is OUTPUT (7-segments)

        P5->SEL0 &= ~0x01;
        P5->SEL1 &= ~0x01;
        P5->DIR = 0b00000001; //P5.0 Red LED OUTPUT

        P2->SEL0 &= ~0x20;
        P2->SEL1 &= ~0x20;
        P2->DIR = 0b00100000; //P2.5 Solenoid Lockbox OUTPUT


     /***********************
             P9.x INPUT
      ***********************/
        P9->SEL0 &= ~0x0F;
        P9->SEL1 &= ~0x0F;
        P9->DIR = 0b11110000;

        P9->OUT = 0b00001111;

/*****************************************
              LOCAL VARIABLES
******************************************/

        //Keypad & Display Variables
        int x = 0; // referring to the ROW ACTIVE which is P8.5-P8.2
        int digit = 0; //counting which display or Digit in FSM
        int N = 25;
        int row;
        int col = 0;
        int state = 0; //Keypad & Display State
        uint8_t key;
        int counter; //Counter for Debounce

        //Lockbox Variables
        int state2 = 0; //Lockbox State
        int keycount = 0;
        int openkey = 0;
        int lockkey = 0;
        int i = 0;
        int wrong = 0;
        int timer = 0;
        //int p = 0;

        uint8_t display[4]={0,0,0,0};
        uint8_t passcode[4] = {0,0,0,0};

        while(1){
/***********************************************
                 INITIALIZATION
 ***********************************************/

            /****************************
              Enabling xth-row and digit
            *****************************/
                P4->OUT = 0xFF; //BLANK DISPLAY
                P8->OUT = port8[x];
                key = input_key(P9->IN);

            //check if openkey(A) or lockkey(C) is pressed

            /*****************************
                   Setting P4->OUT
             ****************************/
                show(display[x]);

/***********************************************
          KEYPAD & DISPLAY STATE MACHINE
************************************************/
        switch(state){
                       /***************************
                                   IDLE
                        ***************************/
                   case 0:
                       if(state2 == 1){
                           if(i < 90000 && key == 0){
                               i++;
                               if(i == 90000 && key == 0){
                               i = 0;
                               state2 = 2;
                               locdisp(display);
                                    }
                                  }
                           if(key != 0){
                              i = 0;
                              if(lockkey == 1){
                                 lockkey = 0; //reset lockkey
                              }
                              state2 = 0;
                              zerodisp(display);
                              break;
                           }
                       }

                       if(key != 0 && state2 != 1){
                                     col = key;
                                     row = x;
                                     counter = 0;
                                     state = 1;
                                     }
                       break;

                       /***************************
                               PRESS DEBOUNCE
                        ***************************/
                   case 1:
                       if(counter > N){
                           state=2;
                       }
                       if(x == row && key != col){
                           counter = 0;
                           state = 0;
                       }
                       if(x == row && key == col){
                           counter++;
                       }
                       break;

                       /***************************
                               PROCESS KEY
                        ***************************/
                   case 2:
                        if(decode(row,col) < 10){
                            display[digit] = decode(row, col);
                            digit++;
                        }
                        if(decode(row,col) == 10){
                            openkey = 1;
                        }
                        if(decode(row,col) == 12){
                            lockkey = 1;
                        }
                        if(decode(row,col) == 15){
                               zerodisp(display);
                               digit = 0;
                        }
                        if(digit == 4){
                            digit = 0;
                            keycount = 4;
                        }
                        counter = 0;
                        state = 3;
                           break;

                         /***************************
                               RELEASE DEBOUNCE
                         ***************************/
                   case 3:
                       if(counter > N){
                           counter = 0;
                           state = 0;
                       }
                       if(x == row && key != 0){
                           counter = 0;
                       }
                       if(x == row && key == 0){
                           counter++;
                       }
                       break;
        }//End of Keypad FSM


       /****************************
               FOREVER LOOP
       ****************************/
/*
        x++;
        if(x == 4){
           x = 0;
        }
*/
/*******************************************
              LOCKBOX STATE MACHINE
 *******************************************/
        switch(state2){

                /**********************
                         NORMAL
                **********************/
                case 0:
                    P5->OUT |= BIT0; //turn off LED
                    P2->OUT |= BIT5; //solenoid off

                    if(openkey == 1){
                        openkey = 0;
                        opendisp(display);
                        state2 = 4; //Move to Solenoid On state
                    }
                    if(keycount != 4 && lockkey == 1){
                        lockkey = 0;
                    }
                    if(keycount == 4 && lockkey == 1){
                        keycount = 0;
                        lockkey = 0;
                        load_passcode(display, passcode);
                        state2 = 1; //Move to Pre-lock state
                    }
                    break;

                /*********************
                        PRE-LOCK
                **********************/
                case 1:
                        //reset number of key inputs
                        //toggle the red LED
                       if(timer < 5000){
                           P5->OUT &= ~BIT0; //ON
                       }
                       if(timer >= 5000 && timer < 10000 ){
                           P5->OUT |= BIT0; //OFF
                       }
                       if(timer > 15000){
                           timer = 0;
                       }
                       lockkey = 0; //reset lockkey
                       break;


                /*********************
                        LOCK
                **********************/
                case 2:
                    P5->OUT |= BIT0; //turn off LED
                    if(check_passcode(display, passcode) == 0 && openkey == 1){
                             opendisp(display);
                             wrong = 0; //reset # of wrong passcodes
                             keycount = 0; //reset keycount
                             openkey = 0;
                             timer = 0;
                             state2 = 4; //Solenoid ON
                       }
                    if(check_passcode(display, passcode) == 1 && openkey == 1){
                              cleardisp(display);
                              openkey = 0; //reset openkey
                              keycount = 0;
                              wrong++; //increment wrong
                        }
                    if(wrong == 5){
                                   lddisp(display);
                                   wrong = 0;
                                   state2 = 3; //Exceeded number of attempt, move to lockdown state
                                    }
                    break;

                    /*********************
                         LOCKDOWN
                    **********************/
                case 3:
                    P9->DIR = 0xFF; //turn off keypad
                    i = 0;
                    //Turn on display to show "_Ld_"
                    while(i++ < 2500){
                             P8->OUT = port8[x];
                             show(display[x]);
                             x++;
                             if(x == 4){
                             x = 0;
                             }
                             wait(1000);
                    }
                    P9->DIR = 0b11110000;
                    locdisp(display);
                    state2 = 2;
                    break;

                    /*********************
                         SOLENOID ON
                   **********************/
                case 4:
                    //energize solenoid for 5 seconds
                    if(timer < 90000){
                        P2->OUT &= ~BIT5; //turn ON solenoid
                             }
                    //After 5 seconds
                    if(timer > 90000){
                          P2->OUT |= (1 << 5); //turn off solenoid after 5 seconds
                          zerodisp(display); //zero display after correct passcode
                          digit = 0; //reset digit being entered
                          keycount = 0; //reset keycount;
                          lockkey = 0; //reset lockkey;
                          openkey = 0; //reset openkey;
                          state2 = 0; //move to normal state
                      }
                      break;

        }//End of Lock State Machine

        //Timer for Pre-Lock and Solenoid ON State
                if( state2 == 1 || state2 == 4){
                        timer++;

                }
                else{
                    timer = 0;
                }

        //Turning on the digits 1 by 1
                x++;
               if(x == 4){
                   x = 0;

               }
       }//End of While Loop
}//End of Main


        /*********************************
                     FUNCTIONS
         *********************************/
        int check_passcode(uint8_t disp[], uint8_t pass[]){
            int b;
            for(b=0;b<4;b++){
                    if(disp[b] != pass[b])
                        return 1;
                }
                return 0;
        }

        void cleardisp(uint8_t disp[]){
            int a;
            for(a = 0; a < 4; a++){
                disp[a]=18;
            }
        }


        void load_passcode(uint8_t disp[], uint8_t pass[]){
            int a;
            for(a = 0; a < 4; a++){
                 pass[a]=disp[a];
               }
        }

        void locdisp(uint8_t disp1[]){
            disp1[0] = 17; //_
            disp1[1] = 11; //L
            disp1[2] = 14; //O
            disp1[3] = 16; //C
        }

        void lddisp(uint8_t disp2[]){
            disp2[0] = 18; //blank
            disp2[1] = 11; //L
            disp2[2] = 13; //d
            disp2[3] = 18; //blank
        }

        void zerodisp(uint8_t disp3[]){
            disp3[0] = 0; //0
            disp3[1] = 0; //0
            disp3[2] = 0; //0
            disp3[3] = 0; //0
                }

        void opendisp(uint8_t disp[]){
            disp[0] = 0; //O
            disp[1] = 19;//p
            disp[2] = 15;//E
            disp[3] = 20;//n
        }

        void wait(int n){
                    int i;
                    for (i = 0; i < n; i++){
                        //busy wait
                    }
                }

        void show(int numb){
                P4->OUT = digit_array[numb];
        }


        int input_key(int key_press){
            int ret = 0;

            if(key_press == 8){
                ret = 4;
            }
            if(key_press == 4){
                ret = 3;
            }
            if(key_press == 2){
                ret = 2;
            }
            if(key_press == 1){
                ret = 1;
            }

            return ret;
        }

        int decode(int r, int c){
            int output;
                    if(r == 0){
                            if(c ==  4){
                                output = keypad_table[0][0]; //1
                            }
                            if( c == 3){
                                output = keypad_table[0][1]; //2
                            }
                            if( c == 2){
                                output = keypad_table[0][2]; //3
                            }
                            if( c == 1){
                                output = keypad_table[0][3]; //10
                            }

                        }

                    if(r == 1){
                            if(c == 4){
                                output = keypad_table[1][0]; //4
                            }
                            if( c == 3){
                                output = keypad_table[1][1]; //5
                            }
                            if( c == 2){
                                output = keypad_table[1][2]; //6
                            }
                            if( c == 1){
                                output = keypad_table[1][3]; //11
                            }

                        }

                    if(r == 2){
                            if(c == 4){
                                output = keypad_table[2][0]; //7
                            }
                            if( c == 3){
                                output = keypad_table[2][1]; //8
                            }
                            if( c == 2){
                                output = keypad_table[2][2]; //9
                            }
                            if( c == 1){
                                output = keypad_table[2][3]; //12
                            }
                        }

                    if(r == 3){
                            if(c == 4){
                                output = keypad_table[3][0]; //14
                            }
                            if( c == 3){
                                output = keypad_table[3][1]; //0
                            }
                            if( c == 2){
                                output = keypad_table[3][2]; //15
                            }
                            if( c == 1){
                                output = keypad_table[3][3]; //13
                            }
                        }
            return output;
        }

