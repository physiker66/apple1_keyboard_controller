#define RIGHT_APPLE 0b00010000
#define LEFT_APPLE  0b00100000
#define CAPS_LOCK   0b01000000
#define SHIFT       0b00001000
#define CTRL        0b10000000


// these are our column input pins. Pin 0 and Pin 1 are not used,
// because they cause issues (presumably because they're TX and RX)
#define pin_x_clock  2
#define pin_x_data   3
#define pin_y_clock  4
#define pin_yz_store 5
#define pin_y_data   6
#define pin_z_clock  7
#define pin_z_data   8
#define RESET       10
#define STROBE      11
#define CLS         12
#define D13         13

#define DEBUG        1
#undef DEBUG
#define RETURN      (char)13

unsigned char InByteY;
unsigned char InByteZ;
static char CommandBuffer[10];
static char output_string[20];
unsigned char CommandPointer;

//ESC     TAB     A       Z   |   /       )       *       ESC
//1!      Q       D       X   |   DOWN    UP      LEFT    RIGHT
//20      W       S       C   |   0       4       8       (
//34      E       H       V   |   1       5       9       -
//4$      R       F       S   |   2       6       .       RETURN
//6"      Y       G       N   |   3       7       +       ,
//5%      T       J       M       \|      `~      RETURN  DELETE
//7&      U       K       ,<      +=       P      UP       DOWN
//8*      I       ;:      .>      0)       [{     SPACE   LEFT
//9(      O       L       /?      -_       ]}      '"      RIGHT

//0     1   2   3   4   5   6   7  8   9  10  11  12  13  14  15   16  17  18  19  20  21  22  23  24 25
//NUL SOH STX ETX EOT ENQ ACK BEL BS  HT  LF  VT  FF  CR  SO  SI  DLE DC1 DC2 DC3 DC4 NAK SYN ETB CAN EM
// 26  27 28  29  30  31     32  33  34  35  36  37  38  39  40  41  42  43  44  45  46  47  48  49  50  51
//SUB ESC FS  GS  RS  US  SPACE   !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /   0   1   2   3
//52  53  54  55  56  57  58  59  60  61  62  63  64  65  66  67  68  69  70  71  72  73  74  75  76  77
// 4   5   6   7   8   9   :   ;   <   =   >   ?   @   A   B   C   D   E   F   G   H   I   J   K   L   M
//78  79  80  81  82  83  84  85  86  87  88  89  90  91  92  93  94  95  96  97  98  99  100 101 102 103
// N   O   P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _   `   a   b   c    d   e   f   g
//104 105 106 107 108 109 110 111 112 113 114 115 116 117 118 119 120 121 122 123 124 125 126 127   
//  h   i   j   k   l   m   n   o   p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~ DEL   

// defined:
// left arrow:  8
// right arrow: 21
// down arrow: 36
// up arrow: 11
// return: 141/8D
//const unsigned char CharTable[] PROGMEM = {
const unsigned char CharTable[] = {
   // normal
  27,  49,  50,  51,  52,  54,  53,  55,  56,  57,
  65,  68,  83,  72,  70,  71,  74,  75,  59,  76,
   9,  81,  87,  69,  82,  89,  84,  85,  73,  79,
  90,  88,  67,  86,  66,  78,  77,  44,  46,  47,
  47,   0,  48,  49,  50,  51,  92,  61,  48,  45,
  41,   0,  52,  53,  54,  55,  96,  80,  91,  93,
  42,   0,  56,  57,  46,  43,  13,  11,  32,  39,
   0,   0,  40,  45,  13,  44, 127,  36,   8,  21,
  // shift
  27,  33,  64,  35,  36,  94,  37,  38,  42,  40,
  97, 100, 115, 104, 102, 103, 106, 107,  58, 108,
   9, 113, 119, 101, 114, 121, 116, 117, 105, 111,
 122, 120,  99, 118,  98, 110, 109,  60,  62,  63,
  47,   0,  48,  49,  50,  51, 124,  43,  41,  95,
  41,   0,  52,  53,  54,  55, 126, 112, 123, 125,
  42,   0,  56,  57,  46,  43,  13,  11,  32,  34,
   0,   0,  40,  45,  13,  44,  10,  36,   8,  21,
  // ctrl
  27,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   1,   4,  19,   8,   6,   7,  10,  11,   0,  12,
   9,  17,  23,   5,  18,  25,  20,  21,   9,  15,
  26,  24,   3,  22,   2,  14,  13,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,  16,   0,   0,
   0,   0,   0,   0,   0,   0,   0,  11,   0,   0,
   0,   0,   0,   0,   0,   0, 127,  36,   8,  21
};

//  loader program
//  TargetAdress = $00F0        ; start address
//  FinalAdress  = $00F2
//  KBDCR        = $D011
//  KBD          = $D010
//                                                     
//  0300 A0 00            ldy     #$00                     ldy     #0      ; set up y counters
//                                                     
//                                                     loop:
//  0302 A5 F0            lda     $F0                      lda TargetAdress
//  0304 C5 F2            cmp     $F2                      cmp FinalAdress
//  0306 D0 09            bne     L0311                    bne firstchar
//  0308 A5 F1            lda     $F1                      lda TargetAdress + 1
//  030A C5 F3            cmp     $F3                      cmp FinalAdress + 1
//  030C D0 03            bne     L0311                    bne firstchar
//  030E 4C 00 FF         jmp     LFF00                    jmp $FF00
//                                                     firstchar:
//  0311 AD 11 D0         lda     $D011                    lda KBDCR
//  0314 10 FB            bpl     L0311                    bpl firstchar
//  0316 AD 10 D0         lda     $D010                    lda KBD      ; get high nibble
//  0319 18               clc                              clc
//  031A 0A               asl     a                        asl
//  031B 0A               asl     a                        asl
//  031C 0A               asl     a                        asl
//  031D 0A               asl     a                        asl
//  031E 91 F0            sta     ($F0),y                  sta (TargetAdress),y ; store into target
//                                                     nextchar:
//  0320 AD 11 D0         lda     $D011                    lda KBDCR
//  0323 10 FB            bpl     L0320                    bpl nextchar
//  0325 AD 10 D0         lda     $D010                    lda KBD      ; get low nibble
//  0328 29 0F            and     #$0F                     and #%00001111
//  032A 18               clc                              clc
//  032B 71 F0            adc     ($F0),y                  adc (TargetAdress),y ; add high nibble
//  032D 91 F0            sta     ($F0),y                  sta (TargetAdress),y ; store into target
//  032F E6 F0            inc     $F0                      inc TargetAdress
//  0331 D0 CF            bne     L0302                    bne loop
//  0333 E6 F1            inc     $F1                      inc TargetAdress + 1
//  0335 4C 02 03         jmp     L0302                    jmp loop



// length of loader: 64 byte
const unsigned char Loader[] PROGMEM = {
0xA0,0x00,0xA5,0xF0,0xC5,0xF2,0xD0,0x09,
0xA5,0xF1,0xC5,0xF3,0xD0,0x03,0x4C,0x00,
0xFF,0xAD,0x11,0xD0,0x10,0xFB,0xAD,0x10,
0xD0,0x18,0x0A,0x0A,0x0A,0x0A,0x91,0xF0,
0xAD,0x11,0xD0,0x10,0xFB,0xAD,0x10,0xD0,
0x29,0x0F,0x18,0x71,0xF0,0x91,0xF0,0xE6,
0xF0,0xD0,0xCF,0xE6,0xF1,0x4C,0x02,0x03,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

//                                                     ; checksum program for apple 1 replica
//                                                     ; entered via WozMon
//                                                     ; on execution reads out keyboard interface and stores data into RAM
//                                                     ; keyboard interface only uses 7bits, so data is transferred as 2 nibbles
//                                                     ; high nibble comes first
//                                                     ; program expects start address as pointer in F0/F1
//                                                     ; program expects end address as pointer in F2/F3
//  
//  
//  
//                                                     .segment "ZEROPAGE"
//  
//  
//                                                     .segment "VARS"
//                                                     StartAddress = $00F0
//                                                     StopAddress  = $00F2
//                                                     KBDCR        = $D011
//                                                     KBD          = $D010
//                                                     DSP          = $D012
//                                                     PRINTBYTE    = $FFDC
//                                                     ECHO         = $FFEF
//  
//                                                     .segment "STARTUP"
//  
//                                                     ; ##########################################################################################
//                                                     start:
//                                                     ;brk
//                                                     ;nop
//                                                     ;nop
//  0300 A0 00            ldy     #$00                     ldy #0      ; set up y counters
//  0302 A9 00            lda     #$00                     lda #0
//  
//                                                     @loop:
//  0304 AA               tax                              tax
//  0305 A5 F0            lda     $F0                      lda StartAddress
//  0307 C5 F2            cmp     $F2                      cmp StopAddress
//  0309 D0 09            bne     L0314                    bne @loop1
//  030B A5 F1            lda     $F1                      lda StartAddress + 1
//  030D C5 F3            cmp     $F3                      cmp StopAddress + 1
//  030F D0 03            bne     L0314                    bne @loop1
//  0311 4C 00 FF         jmp     LFF00                    jmp $FF00
//  
//                                                     @loop1:
//  0314 8A               txa                              txa
//  0315 18               clc                              clc
//  0316 71 F0            adc     ($F0),y                  adc (StartAddress),y     ; add next byte
//  0318 E6 F0            inc     $F0                      inc StartAddress
//  031A D0 E8            bne     L0304                    bne @loop
//  031C 20 DC FF         jsr     LFFDC                    jsr PRINTBYTE
//  031F A9 A0            lda     #$A0                     lda #$A0
//  0321 20 EF FF         jsr     LFFEF                    jsr ECHO
//  0324 A9 00            lda     #$00                     lda #0
//  0326 E6 F1            inc     $F1                      inc StartAddress + 1
//  0328 4C 04 03         jmp     L0304                    jmp @loop




// length of Checksum: 48 byte
const unsigned char Checksum[] PROGMEM = {
0xA0,0x00,0xA9,0x00,0xAA,0xA5,0xF0,0xC5,
0xF2,0xD0,0x09,0xA5,0xF1,0xC5,0xF3,0xD0,
0x03,0x4C,0x00,0xFF,0x8A,0x18,0x71,0xF0,
0xE6,0xF0,0xD0,0xE8,0x20,0xDC,0xFF,0xA9,
0xA0,0x20,0xEF,0xFF,0xA9,0x00,0xE6,0xF1,
0x4C,0x04,0x03,0x00,0x00,0x00,0x00,0x00};

const unsigned char BasicData[] PROGMEM = {
   // due to possible license issues code data is missing
};

const unsigned char CodeBreakerData[] PROGMEM = {
  // due to possible license issues code data is missing
};

// send the value to the display
void Display(char val)
{  
    PORTC=0;
    delay(1);
    digitalWrite(STROBE, LOW);
    PORTC = val;  // writes to analog pins
    if(val >= 64)
      digitalWrite(D13,    HIGH);
    else
      digitalWrite(D13,    LOW);
    
    delay(1);
    digitalWrite(STROBE, HIGH);
    delayMicroseconds(50);
    digitalWrite(STROBE, LOW);
    delay(30);
}

void DoTransfer(const int address, const int size,const unsigned char *data)
{
  int i,j;
  const int blocking = 8;
  unsigned char charBuffer;
  
  for(i=0;i<size;i=i+blocking)
  {
    // generate string "Address: "
    sprintf(output_string,"%04x: ",address + i);
    // output address to display
    for(j=0;j<6;j++)
    {
      Display(output_string[j]);
    }

    // output one line of code 
    for(j=0;j<blocking;j++)
    {
      charBuffer = pgm_read_byte_near(data + (i+j) );
      
      sprintf(output_string,"%02X ",charBuffer);
      Display(output_string[0]);
      Display(output_string[1]);
      Display(output_string[2]);
    }
    Display(RETURN);
    delay(200);
  }

  // print out transferred programm
  sprintf(output_string,"%04X.%04X",address,address+size-1);
  for(j=0;j<9;j++)
  {
    Display(output_string[j]);
  }
  Display(RETURN);
  delay(5000);
}

void SendData(int chars,int address,const unsigned char *data)
{
  int i;
  unsigned char charBuffer;

  const int loader_address = 0x0300;
  const int loader_size = 64;

  DoTransfer(loader_address, loader_size,Loader);

  // set values for
  //  TargetAdress = $00F0
  //  FinaleAdress = $00F2 
  sprintf(output_string,"00F0: %02X %02X %02X %02X",lowByte(address),highByte(address),lowByte(address+chars),highByte(address+chars));
  for(i=0;i<17;i++)
  {
    Display(output_string[i]);
  }
  delay(100);
  Display(RETURN);
  delay(300);
  // check values
  for(i=0;i<9;i++)
  {
    Display("00F0.00F3"[i]);
  }
  Display(RETURN);
  delay(300);
  // set to start address
  for(i=0;i<5;i++)
  {
    Display("0300:"[i]);
  }
  Display(RETURN);
  delay(100);

  // run loader
  for(i=0;i<1;i++)
  {
    Display("R"[i]);
  }
  delay(100);
  Display(RETURN);
  delay(1000);

  Serial.println(" start send");
  // now send real data
  char sum;
  for(i=0;i<chars;i++)
  {
    if((i % 256) == 0)
    {
      Serial.println(" ");
      Serial.print(" sum:");
      Serial.println((unsigned char) sum,HEX);
      sum=0;
    }
    charBuffer = pgm_read_byte_near(data + i);
    sum = sum + charBuffer;
    Display((charBuffer >>4) & 15);
    delay(5);
    Display(charBuffer & 15);
    if(i % 50 == 0)
    {
      Serial.print(i);
      Serial.write(" ");
    }
    delay(5);
  }
  Serial.println(" ");
  Serial.print(" sum:");
  Serial.println((unsigned char) sum,HEX);
  delay(100);
  Serial.println(" end send");
}

void runCodeBreaker()
{
  int chars, address;

#ifdef DEBUG
  Serial.println(" running codebreaker");
#endif //DEBUG 

  chars  = 256 * 8; 
  address = 0x0800;
  SendData(chars,address,CodeBreakerData); 
}


void runCheckSum()
{
  const int address = 0x0300;
  const int size = 48;

  DoTransfer(address, size,Checksum);
  
}

void runBasic()
{
  int chars, address;

#ifdef DEBUG
  Serial.println(" running basic");
#endif //DEBUG

  chars  = 512 * 8; 
  address = 0xe000;
  SendData(chars,address,BasicData); 
}

void clearScreen()
{
#ifdef DEBUG
  Serial.println(" doing clear screen");
#endif //DEBUG   
  PORTC = B00000000;  // writes to analog pins
  digitalWrite(D13,     LOW);
  pinMode(CLS,          OUTPUT);
  digitalWrite(STROBE,  HIGH);
  digitalWrite(CLS,     LOW);
  delay(1000);
  digitalWrite(CLS,     HIGH);
  delay(1000);
  digitalWrite(CLS,     LOW);
  delay(10);
  pinMode(CLS,          INPUT);
}

void doReset()
{
#ifdef DEBUG
  Serial.println(" doing reset");
#endif //DEBUG 
  PORTC = B00000000;  // writes to analog pins
  digitalWrite(D13,     LOW);
  pinMode(CLS,          OUTPUT);
  digitalWrite(RESET,   HIGH);
  digitalWrite(STROBE,  HIGH);
  digitalWrite(CLS,     LOW);
  delay(1000);
  digitalWrite(RESET,   LOW);
  digitalWrite(CLS,     HIGH);
  delay(1000);
  digitalWrite(CLS,     LOW);
  delay(100);
  digitalWrite(RESET,   HIGH);
  pinMode(CLS,          INPUT);

//  runCodeBreaker();
}

void ClearCommandBuffer()
{
#ifdef DEBUG
  Serial.println(" doing ClearCommandBuffer");
#endif //DEBUG 
  CommandBuffer[0] = (char) NULL;
  CommandPointer   = 0;
}

void SerialBinaryBytOut(int var)
{
  for (unsigned int test = 0x0200; test; test >>= 1)
  {
    Serial.write(var  & test ? '1' : '0');
  }
}

void WriteBitX(int data)
{
  digitalWrite(pin_x_data, data);
  digitalWrite(pin_x_clock, LOW);
  delayMicroseconds(1);
  digitalWrite(pin_x_clock, HIGH);
  delayMicroseconds(1);
  digitalWrite(pin_x_clock, LOW);
  digitalWrite(pin_x_data, LOW);
}

void yzStore()
{
  digitalWrite(pin_yz_store, LOW);
  ClockY();
  ClockZ();
  digitalWrite(pin_yz_store, HIGH);
}

void ClockY()
{
  digitalWrite(pin_y_clock, LOW);
  delayMicroseconds(1);
  digitalWrite(pin_y_clock, HIGH);
  delayMicroseconds(1);
  digitalWrite(pin_y_clock, LOW);
}

unsigned char DecodeCharacter(unsigned int charRead, int x,unsigned char shift,unsigned char ctrl)
{
  unsigned int colY;
  int Pointer;
  
  for(colY=0; colY<10; colY++)
  {
    if((charRead & (1<<colY)) > 0)
    {
      break;
    }      
  }

  Pointer = colY + x*10 + shift * 80 + ctrl * 160;

#ifdef DEBUG
  Serial.print(" colY:");
  Serial.print(colY);
  Serial.print(" pointer:");
  Serial.print(Pointer);
  Serial.print(" char:");
  Serial.print((char)CharTable[Pointer]);
#endif //DEBUG

  return CharTable[Pointer];
}

char readByteY()
{
  char together=0;
  for (int i = 0; i < 8; i++){
     char in = digitalRead(pin_y_data);
     together += ( in << (7-i));
     ClockY();
  }
  return together;
}

void ClockZ()
{
  digitalWrite(pin_z_clock, LOW);
  delayMicroseconds(1);
  digitalWrite(pin_z_clock, HIGH);
  delayMicroseconds(1);
  digitalWrite(pin_z_clock, LOW);
}

char readByteZ()
{
  char together=0;
  for (int i = 0; i < 8; i++){
     char in = digitalRead(pin_z_data);
     together += ( in << (7-i));
     ClockZ();
  }
  return together;
}

int ReadInput(int x)
{
  // store current data in shift register
  static unsigned int LastChar  = 0;
  static unsigned int LastCount = 0;
  static unsigned int DidOutput = 1;
  static unsigned char rightApple;
  static unsigned char leftApple;
  static unsigned char capsLock;
  static unsigned char shift;
  static unsigned char ctrl;
  
  static unsigned int LastX;

  unsigned int charRead = 0;
  unsigned char val;

  if(x >=8) // if x==8 no key is currently pressed
  {
    InByteY    = 0;
    InByteZ    = 0;
    LastChar   = -1;
    LastCount  = -1;
    LastX      = -1;
    rightApple = 0;
    leftApple  = 0;
    capsLock   = 0;
    shift      = 0;
    ctrl       = 0;
    DidOutput  = 0;
    PORTC      = 0;
    digitalWrite(D13,    LOW);
    digitalWrite(STROBE, LOW);
    return -3;
  }
  else
  {
    yzStore();
    InByteY = readByteY();
    InByteZ = readByteZ();
    rightApple = (InByteZ & RIGHT_APPLE) ? 1 : 0 ;
    leftApple  = (InByteZ & LEFT_APPLE)  ? 1 : 0 ;
    // the next 3 keys are default high
    capsLock   = (InByteZ & CAPS_LOCK)   ? 0 : 1 ;
    shift      = (InByteZ & SHIFT)       ? 0 : 1 ;
    ctrl       = (InByteZ & CTRL)        ? 0 : 1 ;
  }

  charRead = InByteY + ((InByteZ &1) + (InByteZ &2)) * 256;
  charRead = charRead & 0b00001111111111;

  if(charRead == 0)
  {
    return -2;   
  }

  if(LastX != x)
  {
    LastChar  = 0;
    LastCount = 0;
    PORTC      = 0;
    digitalWrite(D13,    LOW);
    digitalWrite(STROBE, LOW);
  }

//#ifdef DEBUG
//    Serial.print(" rightApple:");
//    Serial.print(rightApple);
//    Serial.print(" charRead:");
//    Serial.print(charRead);
//    Serial.print(" LastChar:");
//    Serial.print(LastChar);
//#endif //DEBUG 

  if(rightApple)
  {
    if(charRead == LastChar)
      return charRead;
      
    CommandBuffer[CommandPointer] = DecodeCharacter(charRead,x,shift,ctrl);
    CommandPointer ++;
    CommandBuffer[CommandPointer] = (char)NULL;
    LastChar   = charRead;
    LastX      = x;
    DidOutput  = 1;
    
#ifdef DEBUG
    Serial.print(" CommandPointer:");
    Serial.print(CommandPointer);
    Serial.print(" CommandBuffer:");
    Serial.write(CommandBuffer[0]);
    Serial.write(CommandBuffer[1]);
    Serial.write(CommandBuffer[2]);
    Serial.print(" rightApple:");
    Serial.print(rightApple);
    Serial.print(" charRead:");
    Serial.print(charRead);
    Serial.print(" LastChar:");
    Serial.print(LastChar);
    Serial.println("");
#endif //DEBUG 

    // prevent overflows
    if( CommandPointer > 10)
    {
      ClearCommandBuffer();
    }
    else if(strncmp(CommandBuffer,"BAS",3)==0)
    {
      runBasic();
      ClearCommandBuffer();
    }
    else if(strncmp(CommandBuffer,"COD",3)==0)
    {
      runCodeBreaker();
      ClearCommandBuffer();
    }
    else if(strncmp(CommandBuffer,"CHK",3)==0)
    {
      runCheckSum();
      ClearCommandBuffer();
    }
    else if(strncmp(CommandBuffer,"CLS",3)==0)
    {
      clearScreen();
      ClearCommandBuffer();
    }
    else if(strncmp(CommandBuffer,"RES",3)==0)
    {
      doReset();  
      ClearCommandBuffer();
    }
    return charRead;
  }
  else if( charRead != LastChar )
  {
    // rightApple is not longer held, clear command buffer
    ClearCommandBuffer();

    // get new character 
    LastChar   = charRead;
    LastX      = x;
    DidOutput  = 0;
    LastCount  = 1;

#ifdef DEBUG
    Serial.print(" InByteY:");
    SerialBinaryBytOut(InByteY);
    Serial.print(" InByteZ:");
    SerialBinaryBytOut(InByteZ);
#endif //DEBUG

    // override shift if ctrl is pressed
    if(ctrl) shift = 0;

    delayMicroseconds(1);
    return LastChar;
  }

  // we want to have at least 5 consecutive reads to remove bouncing
  if(LastCount < 5)
  {
    LastCount++;
    delayMicroseconds(1);
    return LastChar;
  }
  else if(LastCount > 5)
  {
    delayMicroseconds(1);
    return LastChar;
  }
  else
  {
    LastCount++;
  }

  // we want output only once
  if(DidOutput > 0)
  {
    return -1;
  }
  else
  {

    val = DecodeCharacter(charRead,x,shift,ctrl);
    
#ifdef DEBUG
    Serial.print(" x:");
    Serial.print(x);
    Serial.print(" val:");
    Serial.print((int)val,DEC);
    
    if(rightApple) Serial.print(" RIGHT_APPLE");
    if(leftApple)  Serial.print(" LEFT_APPLE");
    if(capsLock)   Serial.print(" CAPS_LOCK");
    if(shift)      Serial.print(" SHIFT");
    if(ctrl)       Serial.print(" CTRL");
  
    Serial.println("");
#endif //DEBUG

    Display(val); // send the value to the display

    DidOutput = 1;
    return(LastChar);
  }
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(57600);
  Serial.print("");
  Serial.println(F("Starting"));

  ClearCommandBuffer();
  
  pinMode(pin_x_clock,  OUTPUT);
  pinMode(pin_x_data,   OUTPUT);
  pinMode(pin_y_clock,  OUTPUT);
  pinMode(pin_yz_store, OUTPUT);
  pinMode(pin_y_data,   INPUT);
  pinMode(pin_z_clock,  OUTPUT);
  pinMode(pin_z_data,   INPUT);

  pinMode(RESET,        OUTPUT);
  pinMode(STROBE,       OUTPUT);
  pinMode(CLS,          OUTPUT);

  // DDRC - The Port C Data Direction Register - read/write
  // PORTC - The Port C Data Register - read/write 
  // PORTC = B10101010;  // writes to analog pins

  DDRC  = B11111111;  // sets Arduino pins 0 to 7 as outputs
  pinMode(D13, OUTPUT);
  doReset();
  
  digitalWrite(pin_yz_store, HIGH);

  // clock out any old setting
  for(int i=0;i<8;i++)
  {
    WriteBitX(LOW);
  }

}

void loop()
{
  // put your main code here, to run repeatedly:
  for(int x=0;x<9;x++) // if x == 8 there is NO key pressed
  {
    if(x == 0)
    {
      WriteBitX(HIGH);
    }
    else
    {
      WriteBitX(LOW);
    }
    if(ReadInput(x)>=0)
    {
      break;
    }
  }
  delay(1);
}
