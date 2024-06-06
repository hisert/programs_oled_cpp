#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include "oled.cpp"
#include "war.h"
#include "server.cpp"
using namespace std;
OLED oled;

void INIT_oled()
{
  oled.INIT(128,32,0x3C);
}

void PRINT_WAR()
{
    oled.InvertFont(1);
    for(unsigned char x=0;x<warLen ;x++)
    {
    oled.ClearDisplay();
    oled.Image(warAll[x]);
    oled.Update();   
    this_thread::sleep_for(chrono::milliseconds(10)); 	    
    }
    oled.InvertFont(0);
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
    if(findOrder(strMessage,"(TEXT1)")) oled.Write_Text(0,0,strMessage);
    else if(findOrder(strMessage,"(TEXT2)")) oled.Write_Text(8,0,strMessage);
    else if(findOrder(strMessage,"(TEXT3)")) oled.Write_Text(16,0,strMessage);
    else if(findOrder(strMessage,"(WAR)")) PRINT_WAR();        
    else std::cout << strMessage << std::endl;
    oled.Update();
}

void handleDisconnect() {
    std::cout << "All clients disconnected" << std::endl;
}

int main() {
   INIT_oled();
    TCPServer server(8082, handleMessage, handleDisconnect);
    while (true) sleep(1);  
    return 0;
}


