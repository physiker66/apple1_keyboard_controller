# apple1_keyboard_controller
Convert an Apple IIe keyboard for use with an Apple 1 Clone computer

All files under this project are under the "Attribution-NonCommercial-ShareAlike 4.0" license. Use at your own risk.

Apple 1 clones need a keyboard. Direct support is only available for ASCII and Apple II keyboards - not for Apple IIe. And I was stupid enough to buy such a keyboard and so needed a solution to connect it to my Apple 1 clone. The Apple 2e keyboard interface is defined as:

J16 (Numeric Pad)         J17 (Keyboard)
11      X5                X6      26 25   Y7
10      X6                SHFT*   24 23   Y6
9       X4                Y9      22 21   X4
8       X7                X3      20 19   X5
7       n/c               X1      18 17   X7
6       Y5                X2      16 15   RESET*
5       Y2                XO      14 13   GND
4       Y4                Y8      12 11   CNTL*
3       Y3                Y5      10 9    CAPLOCK*
2       Y1                Y4      8  7    SW0/OAPL
1       Y0                Y3      6  5    SW1/CAPL
                          Y2      4  3    +5V
                          Y1      2  1    Y0

        Main Keyboard   Numeric Keypad
        XO      X1      X2      X3  |   X4      X5      X6      X7
------------------------------------+--------------------------------
YO      ESC     TAB     A       Z   |   /       )       *       ESC
                                    |
Y1      1!      Q       D       X   |   DOWN    UP      LEFT    RIGHT
                                    |
Y2      20      W       S       C   |   0       4       8       (
                                    |
Y3      34      E       H       V   |   1       5       9       -
                                    |
Y4      4$      R       F       S   |   2       6       .       RETURN
                                    |
Y5      6"      Y       G       N   |   3       7       +       ,
                                    +----------------------------------
Y6      5%      T       J       M       \|      `~      RETURN  DELETE

Y7      7&      U       K       ,<      +=       P      UP       DOWN

Y8      8*      I       ;:      .>      0)       [{     SPACE   LEFT

Y9      9(      O       L       /?      -_       ]}      '"      RIGHT

The Apple 1 keyboard has this pinout:

1  Reset   2 +5V
3  B4
5  B3      6 Strobe
7  B2
9  B1     10 ClearScreen
11 B5
13 B6
15 B7     16  GND

The idea is to use an Arduino Nano and some shift registers (there are more input and output pins needed than a nano provides...). The 74..164 serial in/parallel out register provides the necesseray 8 Y[0..7] signals. Two 74..166 parallel in/serial out registers register the output of both X[0..7] lines as well as special lines SHIFT,Control and Caplock.
The Arduino checks all lines, and reads the results. If a normal key is pressed the output lines connected to B1..B7 are set, and Strobe is is set HIGH for a short amount of time.
The Arduino ensures also that on startup both reset and clear screen signals are sent to the Apple 1. That computer needs both signals in correct order upon power up and reset to work. 
The keyboard RESET line (active if both CONTROL AND RESET are pressed) is connected to the RESET input of the Arduino. A reset to the Arduino will trigger a standard reset cycle including a "reset/clearscreen" signal to the Apple 1.
Finally the Arduino notices when the RIGHT APPLE key is held down. Any keys pressed in such a condition are recorded in a buffer and can be used to start special commands. At the moment these commands are implemented:

  RES   resets the Apple 1
  CLS   clear screen
  COD   load Uncle Bernies "codebreaker" program to address 0800
  CHK   load the checksum program to address 0300
  BAS   load the Apple 1 Basic program to address E000

Some notes on program loading. The programs are only loaded into memory. You need to go to the address and press "R+ENTER" to run them. 
The program CHK is directly entered via the WOzMan. That only works for small programs though. It seems there are problems with WozMon to do the same for longer codes. To circumvent this issue I wrote a loader program, that converts ASCII nibbles drectly into byes in memory WITHOUT displaying them on the screen. 
Finally - I do not own the license to distribute either Apple 1 Basic nor Uncle Bernies codebreaker. That's why they are not part of the Arduino code. Even if both are loaded only about half of the Arduino flash memory is used.

Files:
  Pictures of Aple 1 Clone and keyboard converter
    IMG_2471.JPG
    IMG_2473.JPG
    IMG_2474.JPG

  small helper programs running on the Apple 1
    checksum_v01.s    create simple checksums of blocks
    loader_v03.s      loader program to circumvent WozMon

  schematic_v02.pdf   Schematic
  
  keyboard_v06.ino    Arduino code

  license.txt         The license file 
