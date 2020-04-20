#ifndef SPI8088_H
#define SPI8088_H

#include "Arduino.h" 
#include <SPI.h>


const char MCP23S17_WRITE_00 = 0b01000000;
const char MCP23S17_READ_00 =  0b01000001;
const char MCP23S17_WRITE_01 = 0b01000010;
const char MCP23S17_READ_01 =  0b01000011;
const char MCP23S17_WRITE_10 = 0b01000100;
const char MCP23S17_READ_10 =  0b01000101;
const char IOCON_0A = 0x0A;
const char IODIRA = 0x00;
const char IODIRB = 0x01;
const char GPIOA = 0x12;
const char GPIOB = 0X13;
const char ALL_OUT = 0x00;
const char ALL_IN = 0xFF;

/*
void Write_MCP23S17(byte Address, byte Port, byte Data);
byte Read_MCP23S17(byte Address, byte Port);
*/
void Start_SPI();

void Reset();
void Hold(bool val);

void Write_Memory_Array(unsigned long long int Address, char code_for_8088[], int Length);

/*
void Read_Memory_Array(unsigned long long int Address, char* char_Array, int Length);

char Read_Memory_Byte(unsigned long long int Address);
void Write_Memory_Byte(unsigned long long int Address, char byte_for_8088);

void Write_Memory_Word(unsigned long long int Address, unsigned short int word_for_8088); 

void Read_Video_Memory(unsigned long long int Address, char* char_Array);
*/
#endif
