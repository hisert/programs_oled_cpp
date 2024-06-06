#include <iostream>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <ctime>

#include <thread>
#include <chrono>
#include "oled.cpp"
#include "war.h"
#include "server.cpp"
using namespace std;

OLED oled;
int animasyon_start_flag = 0;
int animasyon_counter = 0;
int animasyon = 0;

//----------------------------------------------------------------------------------------------------------------------------------------FUNCTS

string getSystemIPAddress() {
    struct ifaddrs* ifAddrStruct = nullptr;
    struct ifaddrs* ifa = nullptr;
    void* tmpAddrPtr = nullptr;
    string ipAddress;
    if (getifaddrs(&ifAddrStruct) == -1) {
        cerr << "Ağ arabirimleri alınamadı." << endl;
        return ipAddress;
    }
    for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) { // IPv4 adresi
            tmpAddrPtr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            string interfaceName(ifa->ifa_name);
            string address(addressBuffer);
            
            // localhost IP adresini atla
            if (address != "127.0.0.1") {
                ipAddress = address;
                break;
            }
        }
    }
    if (ifAddrStruct != nullptr) {
        freeifaddrs(ifAddrStruct);
    }
    return ipAddress;
}
string getCPUtemperature() {
    ifstream file("/sys/class/thermal/thermal_zone0/temp");
    if (!file) {
        cerr << "İşlemci sıcaklık dosyası açılamadı." << endl;
        return "";
    }
    string temperature;
    getline(file, temperature);
    file.close();
    int tempValue = stoi(temperature);
    float cpuTemp = tempValue / 1000.0;

    return to_string(cpuTemp);
}

string GetCurrentDateTime()
{
    time_t currentTime = time(nullptr);
    string dateTime = ctime(&currentTime);
    dateTime.pop_back(); // Son karakter olan newline karakterini kaldır
    dateTime = dateTime.substr(4); //Günü kaldırır
    dateTime = dateTime.substr(0, dateTime.length() - 5); //yılı kaldırır.

    return dateTime;
}
string getElapsedTimeInSeconds()
{
    static const auto startTime = chrono::steady_clock::now();
    const auto currentTime = chrono::steady_clock::now();
    const auto elapsedTime = chrono::duration_cast<chrono::seconds>(currentTime - startTime).count();
    return to_string(elapsedTime);
}

void PRINT_INFO()
{
    oled.ClearDisplay();
    oled.Write_Text((0 + 15),(0 + 4),getSystemIPAddress());
    oled.Write_Text((0 + 4),(16 + 4),"TEMP->");
    oled.Write_Text((0 + 4 +50),(16 + 4),getCPUtemperature());   
    oled.Update(); 
    this_thread::sleep_for(chrono::milliseconds(100)); 	
}

//----------------------------------------------------------------------------------------------------------------------------------------  
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
    if(animasyon_start_flag == 1) 
    {
      animasyon_start_flag = 0;
      oled.ClearDisplay();
    }
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
      if(findOrder(strMessage,"(WAR)"))  animasyon = 0;   
      else if(findOrder(strMessage,"(INFO)"))  animasyon = 1;   
    }  
    oled.Update();
}

void handleDisconnect() {
    handleMessage("(ANIME)(INFO)");
}

int main() {
    oled.INIT(128,32,0x3C);
    handleMessage("(ANIME)(INFO)");
    TCPServer server(8082, handleMessage, handleDisconnect);
    while (true) 
    {
      if(animasyon_start_flag) 
      {
        if(animasyon == 0) PRINT_WAR();
        else if(animasyon == 1) PRINT_INFO();
      }
      else sleep(1);
    }
    return 0;
}


