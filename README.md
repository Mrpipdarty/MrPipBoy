# MrPipBoy
Open source DIY "smert" watch based on ESP32S3 with touchscreen. Similar to a KiisuV4b meets a basic smartwatch.

Please note: I am barely versed in any coding language, and am using this project to take a more "Make a problem, and spend months agonizing over the solution" approach to learning how to work with microcontrollers. I can barely code a website, and spend most of my nerding out time working with advanced networking and basic homelab tomfoolery. 

TLDR: I suck at coding, and have no peers to assist in code review or development. I embarrassingly write what code I think I can to make my ideas work, feed it to claude (please dont hate me, I am not proud of using Ai but try to take an ethical approach in its application and use it more for initial error correction and in depth debugging when I cannot find a solution). No code will ever be committed without my own personal testing, review, and in most cases modification from what claude has provided from my initial code. 

I will attempt to only commit functional code to this github once developed, and happily ask others interested to jump in, comment your ideas, help make me make something that probably nobody needs but is pretty neat anyways! Who doesnt want a sleek and semi practical daily driver "Pip Boy" like from fallout? Or just a general purpose swiss army knife platform that can be as basic as a smartwatch with a beefy screen, to a powerful swiss army knife tool for tinkerers like you and me! 


||    Basic build    ||
I am using an ESP32S3 development board with touchscreen, also referred to as a "cyd" but come from many vendors in many configurations. My own personal development is for the hoysnod brand of touchscreen development board, but you may have to test and modify my code to work with your particular board. 

From my understanding, the majority of things to check and consider BEFORE uploading to your own display:
        -Touch display driver
        -Pinout of touch screen, sd card, and any sensors or accessories
        -Library versions, as I needed to downgrade my library packages several times before I could get code to run on my display

  If anyone can potetially help create other firmwares for other boards or a more universal firmware, please let me know!!!! 

|| BUT WHAT DOES IT DO?!? ||
Great question! 
    Baseline firmware is developed to act as a basic smartwatch/clock, connecting to a wifi network (currently hardcoded in firmware, but i will be making a settings page to configure the wifi) and obtaining time and date via a pre defined NTP server. May implement a setting to change ntp server / time zone on device. 
    Integrated applications: The home screen will have options for the devices on board settings, several USB HID devices that can use the touchscreen to behave as input devices on a USB connected pc. These applications will include a touchpad to act as a mouse, a tiny touchscreen keyboard, a number pad, and possibly a small midi controller. Without a connection to a pc, I will include some on board applications that are of use to my daily life such as: 
    -Calculator 
    -Dice roller
    -Coin flip
    -Notepad (notes saved to sd card)
    -Weather forecast
    -Control over onboard neopixel LED (your board may not have a neopixel, or any rgb led at all) to function as a flashlight and         mood lighting
    -digital business card (qr code display to bring people to my website, as I am a content creator and will be wearing this devivce      to conventions)
The list of burned-in functionality will change as I learn more about the capability of the ESP32 board I am working with, but the goal is to have a handful of useful tools once flashed to tinker with, and allowing users to implement their own apps flipper zero style to add or remove specialized tools for everyones day to day task. Maybe we can lean into the smart flipper esque tools and allow the device to perform wifi and bt analysis, run ducky scripts, and talk to accessories made by us to expand the board in unlimited ways. 

My end goal is to make the above device functional, add in actual smartwatch companion functionality IE connecting to a smartphone via a companion app to allow remote configuration of the device, and to obtain information like notifications/weather forecasts/ time / etc. I also am an avid fan of meshtastic, and am looking to eventually add a module to the board that will allow the device to act as a wrist mounted meshtastic radio. 
