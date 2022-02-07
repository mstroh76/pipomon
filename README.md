# pipomon
Pi Power Monitor  

LED blink dedection from smart meter with optical sensor  
Value output with 4-digit TM1637 display

## compile

```bash
git clone https://github.com/mstroh76/pipomon.git
cd pipomon
wget https://raw.githubusercontent.com/GrazerComputerClub/TM1637Display/master/TM1637Display.cpp
wget https://raw.githubusercontent.com/GrazerComputerClub/TM1637Display/master/TM1637Display.h
g++ -Wall -o pipomon pipomon.c TM1637Display.cpp -lwiringPi
./pipomon
```
