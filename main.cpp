#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include "oled.cpp"
#include "war.h"
using namespace std;
OLED oled;

void INIT_oled()
{
  oled.INIT(128,32,0x3C);
}

void PRINT_WAR()
{
    oled.InvertFont(1);
    for(unsigned char x=0;x<warallArray_LEN ;x++)
    {
    oled.ClearDisplay();
    oled.Image(warallArray[x]);
    oled.Update();   
    this_thread::sleep_for(chrono::milliseconds(10)); 	    
    }
    oled.InvertFont(0);
}
int main()
{  
 INIT_oled();
  PRINT_WAR();
//  oled.Write_Text(0,0,"hello");  
//  oled.Update();
 // while(1)  this_thread::sleep_for(chrono::milliseconds(100));
}
