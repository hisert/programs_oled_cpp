
#!/bin/bash

rm -r programs_oled_cpp
pkill oled.cpp
git clone https://github.com/hisert/programs_oled_cpp.git
cd  programs_oled_cpp
g++ -o oled main.cpp
/root/programs_oled_cpp/oled

//---------------------------------------------------------------------------//


sudo sed -i '$e echo "python3 /root/programs_oled/oled &"' /etc/rc.local
