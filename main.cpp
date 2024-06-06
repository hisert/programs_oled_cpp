#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include "oled.cpp"
#include "war.h"
#include "server.cpp"
using namespace std;
OLED oled;
int animasyon_start_flag = 0;
int animasyon_counter = 0;
int aniasyon_choose = 0;
void INIT_oled()
{
  oled.INIT(128,32,0x3C);
}

void PRINT_WAR()
{
    oled.InvertFont(1);
    if(animasyon_counter < warLen) 
    {
    oled.ClearDisplay();
    oled.Image(warAll[animasyon_counter]);
    oled.Update();   
    animasyon_counter++; 
    }
    else animasyon_counter = 0;
    oled.InvertFont(0);
    this_thread::sleep_for(chrono::milliseconds(10)); 	
}

int findOrder(std::string& mainString, const std::string& searchString) {
    size_t found = mainString.find(searchString);
    if (found != std::string::npos) {
        size_t searchStringLength = searchString.length();
        mainString = mainString.substr(0, found) + mainString.substr(found + searchStringLength);
        return 1;
    } 
    return 0;
}

void handleMessage(const char* message) {
    std::string strMessage = std::string(message);
    if(animasyon_start_flag == 1)oled.ClearDisplay();
    animasyon_start_flag = 0;
    if(findOrder(strMessage,"(TEXT0)")) oled.ClearDisplay();
    else if(findOrder(strMessage,"(TEXT1)")) 
    {
      oled.Write_Text(0,0,"                ");    
      oled.Write_Text(0,0,strMessage);     
    }
    else if(findOrder(strMessage,"(TEXT2)")) 
    {
      oled.Write_Text(0,8,"                ");  
      oled.Write_Text(0,8,strMessage);  
    }
    else if(findOrder(strMessage,"(TEXT3)")) 
    {
      oled.Write_Text(0,16,"                ");  
      oled.Write_Text(0,16,strMessage);
    }
    else if(findOrder(strMessage,"(ANIME)")) 
    {
      animasyon_start_flag = 1;
      animasyon_counter = 0;
      if(findOrder(strMessage,"(WAR)"))  aniasyon_choose = 0;
     
    }  
    oled.Update();
}

void handleDisconnect() {
    std::cout << "All clients disconnected" << std::endl;
}

int main() {
   INIT_oled();
    TCPServer server(8082, handleMessage, handleDisconnect);
    while (true) 
    {
      if(animasyon_start_flag) 
      {
        if(aniasyon_choose == 0) PRINT_WAR();
        
      }
      else sleep(1);
    }
    return 0;
}


