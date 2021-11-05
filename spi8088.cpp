#include "Arduino.h" 
#include <SPI.h>
#include "spi8088.h"

const int CS0 = 10;

void Write_MCP23S17(byte Address, byte Port, byte Data)
{  
  digitalWrite(CS0, LOW);
  SPI.transfer(Address);
  SPI.transfer(Port);
  SPI.transfer(Data);
  digitalWrite(CS0, HIGH); 
}
byte Read_MCP23S17(byte Address, byte Port)
{  
   byte ret;
  digitalWrite(CS0, LOW);
  SPI.transfer(Address);
  SPI.transfer(Port);
  ret = SPI.transfer(0x00);
  digitalWrite(CS0, HIGH); 
  return ret;
}
//Sets up the MCP23S17 chips
void Start_SPI() 
{
   //CE0-0 mHz-8?   
   SPI.setClockDivider(SPI_CLOCK_DIV2);
   SPI.setBitOrder(MSBFIRST);
   SPI.setDataMode(SPI_MODE0);
   pinMode(CS0, OUTPUT);
   digitalWrite(CS0, HIGH);
   //Serial.begin(9600);   
   SPI.begin();
   
  //Enable address pins mcp23s17
  Write_MCP23S17(MCP23S17_WRITE_10, IOCON_0A, 0b00001000);
   //Port set up
   //8 bit Data
  Write_MCP23S17(MCP23S17_WRITE_00, IODIRA, ALL_IN);  
   //ADDRESS 0-7
  Write_MCP23S17(MCP23S17_WRITE_00, IODIRB, ALL_OUT);
   //ADDRESS 8-15
  Write_MCP23S17(MCP23S17_WRITE_01, IODIRA, ALL_IN); 
   //ADDRESS 16-19
  Write_MCP23S17(MCP23S17_WRITE_01, IODIRB, ALL_IN); 
   //Interrrupt (this is not being used)
  Write_MCP23S17(MCP23S17_WRITE_10, IODIRA, ALL_IN);
   //Control 
  Write_MCP23S17(MCP23S17_WRITE_10, IODIRB, 0b00111111);
}

//Reset 8088
void Reset()                                                                 
{
	//Write 0 to 8284 reset pin
	Write_MCP23S17(MCP23S17_WRITE_10, GPIOB, 0b00000000);
	//Write 1 to 8284 reset pin    
	Write_MCP23S17(MCP23S17_WRITE_10, GPIOB, 0b10000000);                                          
}
//Hold the 8088 bus, true=hold, false=dont hold
void Hold(bool val)
{
   byte holda;
    if(val == true)
   {
      //Writes 1 to the hold pin 0bx1xxxxxx, keeps reset pin high
      Write_MCP23S17(MCP23S17_WRITE_10, GPIOB, 0b11000111);
      //Reads from the Control port checking for HOLDA pin to go high
      holda = Read_MCP23S17(MCP23S17_READ_10, GPIOB);   
      holda = holda & 0b00100000; 
      //waits for HOLDA
      while(holda != 0b00100000)
      {
	  holda = Read_MCP23S17(MCP23S17_READ_10, GPIOB);   
	  holda = holda & 0b00100000; 
      }    
      //Sets up Control port enables RD, WR, IO/M pins
      Write_MCP23S17(MCP23S17_WRITE_10, IODIRB, 0b00111000);   
      //8 bit Data out (this kind of doesn't mater becasue read and write operations change this port as needed)
      Write_MCP23S17(MCP23S17_WRITE_00, IODIRA, ALL_OUT);   
      //ADDRESS 0-7 port enabled as output 
      Write_MCP23S17(MCP23S17_WRITE_00, IODIRB, ALL_OUT);   
      //ADDRESS 8-15 port enabled as output 
      Write_MCP23S17(MCP23S17_WRITE_01, IODIRA, ALL_OUT);   
      //ADDRESS 16-19 port enabled as output 
      Write_MCP23S17(MCP23S17_WRITE_01, IODIRB, ALL_OUT);   
   }
   else if(val ==false)
   {
      //8 bit Data in this prevents inferance on the 8088 data bus    
       Write_MCP23S17(MCP23S17_WRITE_00, IODIRA, ALL_IN); 
      //ADDRESS 0-7 port in this prevents inferance on the 8088 address bus
       Write_MCP23S17(MCP23S17_WRITE_00, IODIRB, ALL_IN); 
      //ADDRESS 8-15 port in this prevents inferance on the 8088 address bus
       Write_MCP23S17(MCP23S17_WRITE_01, IODIRA, ALL_IN); 
      //ADDRESS 16-19 port in this prevents inferance on the 8088 address bus
       Write_MCP23S17(MCP23S17_WRITE_01, IODIRB, ALL_IN); 
      //Sets up Control port (sets RD, WR, IO/M to inputs)
      Write_MCP23S17(MCP23S17_WRITE_10, IODIRB, 0b00111111);            
      //
      Write_MCP23S17(MCP23S17_WRITE_10, GPIOB, 0b10000111); // or 0b10000000??? 
   }
}
//Writes a block of data to the 8088 (this does not automatically hold the processor 
void Write_Memory_Array(unsigned long long int Address, char code_for_8088[], int Length)
{  
   //8 bit Data out put
   Write_MCP23S17(MCP23S17_WRITE_00, IODIRA, ALL_OUT);  

   for(int i = 0; i < Length; i++) 
   {
      Write_MCP23S17(MCP23S17_WRITE_00, GPIOA, code_for_8088[i]);   

      Write_MCP23S17(MCP23S17_WRITE_00, GPIOB, Address);   
  
      Write_MCP23S17(MCP23S17_WRITE_01, GPIOA, Address >> 8);   

      Write_MCP23S17(MCP23S17_WRITE_01, GPIOB, Address >> 16);   
 
      Write_MCP23S17(MCP23S17_WRITE_10, GPIOB, 0b11000001);   
 
      Write_MCP23S17(MCP23S17_WRITE_10, GPIOB, 0b11000111);   
 
      Address++;
   } 
}
/*
void Read_Memory_Array(unsigned long long int Address, char* char_Array, int Length)
{
   //8 bit Data
   buffer[0] = MCP23S17_WRITE_00; buffer[1] = IODIRA; buffer[2] = ALL_IN;   
   wiringPiSPIDataRW(0, buffer, 3);
    for(int i=0; i < Length; ++i)
    {
         buffer[0] = MCP23S17_WRITE_00; buffer[1] = GPIOB; buffer[2] = Address;   
         wiringPiSPIDataRW(0, buffer, 3);  
         buffer[0] = MCP23S17_WRITE_01; buffer[1] = GPIOA; buffer[2] = Address >> 8;   
         wiringPiSPIDataRW(0, buffer, 3);  
         buffer[0] = MCP23S17_WRITE_01; buffer[1] = GPIOB; buffer[2] = Address >> 16;   
         wiringPiSPIDataRW(0, buffer, 3);  
         buffer[0] = MCP23S17_WRITE_10; buffer[1] = GPIOB; buffer[2] = 0b11000010;   
         wiringPiSPIDataRW(0, buffer, 3);  
         
         buffer[0] = MCP23S17_READ_00; buffer[1] = GPIOA; 0;   
         wiringPiSPIDataRW(0, buffer, 3);         
         char_Array[i] = buffer[2];
         
         buffer[0] = MCP23S17_WRITE_10; buffer[1] = GPIOB; buffer[2] = 0b11000111;   
         wiringPiSPIDataRW(0, buffer, 3);         
         Address++;
      }
}
char Read_Memory_Byte(unsigned long long int Address)
{
   //8 bit Data
   buffer[0] = MCP23S17_WRITE_00; buffer[1] = IODIRA; buffer[2] = ALL_IN;   
   wiringPiSPIDataRW(0, buffer, 3);

   buffer[0] = MCP23S17_WRITE_00; buffer[1] = GPIOB; buffer[2] = Address;   
   wiringPiSPIDataRW(0, buffer, 3);  
   buffer[0] = MCP23S17_WRITE_01; buffer[1] = GPIOA; buffer[2] = Address >> 8;   
   wiringPiSPIDataRW(0, buffer, 3);  
   buffer[0] = MCP23S17_WRITE_01; buffer[1] = GPIOB; buffer[2] = Address >> 16;   
   wiringPiSPIDataRW(0, buffer, 3);  
   buffer[0] = MCP23S17_WRITE_10; buffer[1] = GPIOB; buffer[2] = 0b11000010;   
   wiringPiSPIDataRW(0, buffer, 3);  
   
   buffer[0] = MCP23S17_READ_00; buffer[1] = GPIOA; 0;   
   wiringPiSPIDataRW(0, buffer, 3);         
   char char_byte = buffer[2];
   
   buffer[0] = MCP23S17_WRITE_10; buffer[1] = GPIOB; buffer[2] = 0b11000111;   
   wiringPiSPIDataRW(0, buffer, 3);   
   return char_byte;  
}
void Write_Memory_Byte(unsigned long long int Address, char byte_for_8088) 
{ 
   //Set 8 bit Data port out 
   buffer[0] = MCP23S17_WRITE_00; buffer[1] = IODIRA; buffer[2] = ALL_OUT;   
   wiringPiSPIDataRW(0, buffer, 3);

   buffer[0] = MCP23S17_WRITE_00; buffer[1] = GPIOA; buffer[2] = byte_for_8088;   
   wiringPiSPIDataRW(0, buffer, 3);  
   buffer[0] = MCP23S17_WRITE_00; buffer[1] = GPIOB; buffer[2] = Address;   
   wiringPiSPIDataRW(0, buffer, 3);  
   buffer[0] = MCP23S17_WRITE_01; buffer[1] = GPIOA; buffer[2] = Address >> 8;   
   wiringPiSPIDataRW(0, buffer, 3);  
   buffer[0] = MCP23S17_WRITE_01; buffer[1] = GPIOB; buffer[2] = Address >> 16;   
   wiringPiSPIDataRW(0, buffer, 3);  
   buffer[0] = MCP23S17_WRITE_10; buffer[1] = GPIOB; buffer[2] = 0b11000001;   
   wiringPiSPIDataRW(0, buffer, 3);  
   buffer[0] = MCP23S17_WRITE_10; buffer[1] = GPIOB; buffer[2] = 0b11000111;   
   wiringPiSPIDataRW(0, buffer, 3);  
}
void Write_Memory_Word(unsigned long long int Address, unsigned short int word_for_8088) 
{ 
   //Set 8 bit Data port out 
   buffer[0] = MCP23S17_WRITE_00; buffer[1] = IODIRA; buffer[2] = ALL_OUT;   
   wiringPiSPIDataRW(0, buffer, 3); 
   buffer[0] = MCP23S17_WRITE_00; buffer[1] = GPIOB; buffer[2] = Address;   
   wiringPiSPIDataRW(0, buffer, 3);  
   buffer[0] = MCP23S17_WRITE_01; buffer[1] = GPIOA; buffer[2] = Address >> 8;   
   wiringPiSPIDataRW(0, buffer, 3);  
   buffer[0] = MCP23S17_WRITE_01; buffer[1] = GPIOB; buffer[2] = Address >> 16;   
   wiringPiSPIDataRW(0, buffer, 3);  
   buffer[0] = MCP23S17_WRITE_00; buffer[1] = GPIOA; buffer[2] = word_for_8088;   
   wiringPiSPIDataRW(0, buffer, 3); 
   buffer[0] = MCP23S17_WRITE_10; buffer[1] = GPIOB; buffer[2] = 0b11000001;   
   wiringPiSPIDataRW(0, buffer, 3);  
   buffer[0] = MCP23S17_WRITE_10; buffer[1] = GPIOB; buffer[2] = 0b11000111;   
   wiringPiSPIDataRW(0, buffer, 3);  
   Address++;
   
   buffer[0] = MCP23S17_WRITE_00; buffer[1] = GPIOB; buffer[2] = Address;   
   wiringPiSPIDataRW(0, buffer, 3);  
   buffer[0] = MCP23S17_WRITE_01; buffer[1] = GPIOA; buffer[2] = Address >> 8;   
   wiringPiSPIDataRW(0, buffer, 3);  
   buffer[0] = MCP23S17_WRITE_01; buffer[1] = GPIOB; buffer[2] = Address >> 16;   
   wiringPiSPIDataRW(0, buffer, 3);  
   buffer[0] = MCP23S17_WRITE_00; buffer[1] = GPIOA; buffer[2] = word_for_8088 >> 8;   
   wiringPiSPIDataRW(0, buffer, 3); 
   buffer[0] = MCP23S17_WRITE_10; buffer[1] = GPIOB; buffer[2] = 0b11000001;   
   wiringPiSPIDataRW(0, buffer, 3);  
   buffer[0] = MCP23S17_WRITE_10; buffer[1] = GPIOB; buffer[2] = 0b11000111;   
   wiringPiSPIDataRW(0, buffer, 3);  
}
//This is a fucntion that reads only the char code in video memory and skips the color code.
//It also trys to speed up the rate of raeding 
void Read_Video_Memory(unsigned long long int Address, char* char_Array)
{
   //8 bit Data Port
   buffer[0] = MCP23S17_WRITE_00; buffer[1] = IODIRA; buffer[2] = ALL_IN;   
   wiringPiSPIDataRW(0, buffer, 3);
   
   //Address Segment, lines 16-19
   buffer[0] = MCP23S17_WRITE_01; buffer[1] = GPIOB; buffer[2] = Address >> 16;   
   wiringPiSPIDataRW(0, buffer, 3);  
   //Address Segment, lines 8-15
   char Address8_15 = Address >> 8;
   buffer[0] = MCP23S17_WRITE_01; buffer[1] = GPIOA; buffer[2] = Address8_15;   
   wiringPiSPIDataRW(0, buffer, 3); 
   char Address8_15_Check = Address >> 8;
   
   //reads 2000 bytes
    for(int i=0; i < 0x7D0; ++i)
    {
         buffer[0] = MCP23S17_WRITE_00; buffer[1] = GPIOB; buffer[2] = Address;   
         wiringPiSPIDataRW(0, buffer, 3); 
          
         Address8_15 = Address >> 8;
         
         //this skips rewriting address lines 8-15 if there is no change.
         if(Address8_15 != Address8_15_Check)
         {
            buffer[0] = MCP23S17_WRITE_01; buffer[1] = GPIOA; buffer[2] = Address8_15;   
            wiringPiSPIDataRW(0, buffer, 3);  
            Address8_15_Check = Address >> 8;           
         }         
         buffer[0] = MCP23S17_WRITE_10; buffer[1] = GPIOB; buffer[2] = 0b11000010;   
         wiringPiSPIDataRW(0, buffer, 3);  
         
         buffer[0] = MCP23S17_READ_00; buffer[1] = GPIOA; 0;   
         wiringPiSPIDataRW(0, buffer, 3);         
         char_Array[i] = buffer[2];
         
         buffer[0] = MCP23S17_WRITE_10; buffer[1] = GPIOB; buffer[2] = 0b11000111;   
         wiringPiSPIDataRW(0, buffer, 3);         
         Address++;
         Address++;
      }
}*/
