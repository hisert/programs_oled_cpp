#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include "oled.cpp"
using namespace std;
OLED oled;

void INIT_oled()
{
  oled.INIT(128,32,0x3C);
}

int main()
{  
 INIT_oled();
 // while(1)  this_thread::sleep_for(chrono::milliseconds(100));
}
