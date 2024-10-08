# Aegir - GPS/LTE/Iridium Tracker

*One off challenge to build a custom GPS tracker that sends data to cloud over LTE and Iridium networks. Tested successfully on the expedition yacht S/Y Southern Star while crossing Barents Sea from Norway to Svalbard in the Arctics. Built with RockBLOCK 9603N(Iridium), SIMCOM7600G (LTE/GSM) and STM32 microcontroller (ARM,FreeRTOS).*

![Aegir GPS Tracker](/photos/1.jpg)

## Background

I travel quite a lot by motorcycle and 4x4, both on-road and off-road. In the case of the latter, navigation is done using topographic maps, and we record and ride along so-called "tracks," which are routes saved using navigation devices and applications like Garmin. These tracks can be saved and shared with others in GPX format.

I am the author of an application for drawing such tracks [GPXLAB](https://gpxlab.net). Since we record routes while traveling, I use GPS trackers installed in vehicles that send location data to a server. This data can be exported to GPX.

The most interesting journeys are to remote places, where there is often no mobile network coverage, and civilization can be hundreds of kilometers away: oceans, deserts, exotic countries. In such places, the only real way to communicate with the world is through Iridium, which is used by devices like the Garmin InReach. This is an important topic because these devices not only allow recording GPX tracks but also enable sending messages or calling for help in case of serious problems.

The next step, as an amateur electronics enthusiast, is to build my own tracker. While building a GSM-based tracker is relatively simple (controller + GPS + sim800l), a project with LTE and satellite communication requires the use of more expensive and better components.

The ultimate motivator for building this project was the opportunity to join the crew of the S/Y Southern Star yacht, which sails in the Arctic.

## Device

![Aegir GPS Tracker](/photos/2.jpg)

### Requirements
- **must work everywhere**!
- while in range of "cilization" send data every 10s over LTE
- when in remote location send data every 1h over Iridium
- save datapoints to the SD card
- show live position, speed, course and other parameters
- easy to extend further with exposed IO pins
- allow connection of extra sensors with I2C
- powered by 12V DC (car, ship, etc)
- backup power (~2hrs)
- case protecting from the elements

### Hardware
- STM32F401 on the BlackPill board as the primary controller
- NEO 8M GPS receiver
- RockBLOCK 9603N Iridium transceiver
- SIM7600G LTE module equipped with 1NCE SIM card
- OLED 128x64 display
- SD module
- power supply module
- backup power module
- custom PCB to glue all part together

### Firmware
I wrote the firmware in C, using the FreeRTOS and CMSISv1 implementation provided by STM (CubeMX). The real-time system made it easy to divide tasks among different parts of the program, such as a separate thread for display or collecting data from the GPS module. I used the BlackPill module with STM32F401 due to its increased RAM capacity, which allowed me to fit the program's data without the need for extreme optimization. The STM32F401 offers three hardware UARTs, to which the GPS, LTE, and Iridium modules are connected. However, to make debugging easier, I used a trick with hardware UART emulation, which allows a separate UART->USB converter to be connected to a single pin to monitor the device's operation. The microcontroller is programmed using an STLink dongle.

Obviously the firmware could have been done better, it's not perfect but the time constraints were strict and afterall it is a hobby project. Yes, the code sucks, deal with it :)

### How it works?
- every 10s the controller add a report to the queue with the current position, course, speed and altitude
- when LTE is available the reports are being sent instantly
- when LTE ir unavailable the device attempts to send the latest report over Iridium satellite network
- to be able to send using Iridium, the network availability is monitored, the pin signals that a satellite is being seen by the transceiver
- every 10s reports are being dumped to the SD card
- a separate thread controls the display and presented parameters

### GPS
The GNSS NEO 8M module sends data every 1 second via UART in NMEA format. The module is connected to an external, inexpensive GPS antenna. Importantly, although the module supports Baidou and Glonass, the antenna itself has a filter for the GPS frequency, so the other GNSS systems were likely not used for determining the position in this case. The device acquires a fix in about a minute and is able to determine the position with 4 or more satellites available. For NMEA parsing, there is a very good library Minmea.

### Iridium
The Iridium module is a RockBlock board with an Iridium 9603N transceiver. Communication is done via UART using dedicated AT commands. Additionally, there is a pin that indicates satellite network visibility, which is important because the module can sometimes only send data every few minutes, when a satellite happens to pass over the device. RockBlock acts as an intermediary between us and Iridium, so it is necessary to pay for network availability and purchase a data package.

### LTE
After various issues with LTE, I used the SIM7600G module along with a SIM card from 1nce, ensuring that the device should have coverage in about 170 countries worldwide. The LTE module is connected to an external antenna for the LTE band, and interestingly, during tests in the Arctic, the device picked up a signal much earlier than iPhones and Android mobile phones. Communication is done via UART using AT commands. Why LTE and not GSM (GPRS/2G)? I had previously tested the 2G GPS Trackers in northern Finland/Sweden/Norway and had some issues with network availability while LTE worked pretty much all the time, thus decided to go with LTE.

### Display
A cheap OLED 128x64 blue display bought from China.

### Power
The project has two power supply components. The first is a standard step-down converter with a capacity of up to 3A, set to 5V, allowing the device to be powered with voltages from 7V to around 32V. It works without issues with a 12-14V battery voltage. Optionally, the board supports a UPS power module with three batteries, providing continuous operation of the device for about 2 hours. Additionally, there are several electrolytic capacitors, as both the LTE and Iridium modules have sudden power demands when communicating with the network.


### Cloud
The device was initially intended to send data to a system I had created earlier. SkySpy is an application that collects data from GPS trackers, displays it on a map, and allows exporting the data to GPX.

### Case
I bought it before designing the PCB board, which I designed to fit inside. The enclosure is made of polycarbonate G 214C, and the transparent lid made it easier to mount the display.

## Circuit Schematic
![Circuit Schematic](/photos/circuit.png)

## PCB Board
![PCB Board](/photos/pcb.png)

I quickly designed the project in EasyEDA because there were ready-made public components available in the cloud, which significantly sped up the work. I sent the PCB design to JLCPCB for production. Of course, there were some errors, but a few trace cuts and jumpers solved the problem. The layout of the components was arranged to fit everything, which also caused some issues, such as the absence of a power switch or the SD card slot being hard to access.

## Prototype
![Prototype](/photos/3.jpg)

Being a fan of bread boards I assembled the first prototype before having the final PCB to test all components. Next designed the PCB and assembled the final device. There were some stupid mistakes but easy to fix by cutting traces and adding some wires.


## Expedition

The motivation to complete the project was the plan for a sailing expedition to Svalbard.

I installed the device above the yacht's entrance, where I had access to 12V power. The three external antennas for LTE, Iridium, and GPS were simply placed nearby. On board, we also had a Garmin InReach satellite phone and an Iridium Edge terminal, so we had something to compare the project's performance against.

We set sail from Tromsø in northern Norway and, due to a storm in the Barents Sea, we navigated through the fjords to the northeast. When a weather window appeared, we headed north toward Bear Island and further to Svalbard. To the south, the first base in Svalbard is the Polish Polar Station in Hornsund, which we visited thanks to the kindness of the researchers. Continuing north, we reached Prince Charles Land and the highest point of the expedition at 78°44'N, then we sailed down to Isfjord and into the port of Longyearbyen.

![Map](/photos/4.jpg)

[Full Track on the Map](https://app.gpxlab.net/share/5b85c2e5-82e0-42d7-afcc-8595f449a423)

As expected, the tracker sent data to the server every 10 seconds in the coastal waters, meaning that the entire Norwegian fjords had LTE coverage. The signal was lost about 20 nautical miles (37 km) from the Norwegian coast. In the ocean in polar areas, the only means of communication is Iridium, and here we had satellites almost constantly overhead, so the device sent positions without major problems every hour. LTE coverage returned around Isfjord, where Longyearbyen, the capital of Svalbard and the largest settlement on the island of Svalbard, is located.

The project essentially concludes with this expedition. Further development of this device doesn't make much sense due to the exorbitant costs of Iridium transmission through RockBlock, namely £13 for a monthly line rental and £14 for the cheapest package of 100 credits, which equals 5 kB (!?). In comparison, a subscription for Garmin InReach is cheaper, as it sends points every 10 minutes. For me, this was the most challenging electronic project I have completed so far. It forced me to learn both STM and FreeRTOS, so in that regard, it was worth it, and there's the eternal satisfaction that it actually worked in the Arctic :)

## Photos
![Photo](/photos/5.jpg)
![Photo](/photos/6.jpg)
![Photo](/photos/7.jpg)
![Photo](/photos/8.jpg)
![Photo](/photos/9.jpg)
![Photo](/photos/10.jpg)
![Photo](/photos/11.jpg)


## Media
One of our mates is a youtuber and he had managed to capture some footage while sailing the arctic waters.
- [Ep1. Rejs na Arktykę](https://www.youtube.com/watch?v=Z5egoxSh6OI)
- [Ep2. Miejsca bez Powrotu w Arktyce](https://www.youtube.com/watch?v=qfbvTMd-tyc)
- [Ep3. Sztorm na Krańcu Świata](https://www.youtube.com/watch?v=2DE-GZrynNQ)

I had a pleasure to talk about the project for [Elektroda.pl](https://elektroda.pl) podcast.
- [GPS Iridium Arktyka - podcast #29](https://www.youtube.com/watch?v=6I0lGnK7-Hg)

## Yacht & Crew
- [S/Y Southern Star](https://www.sysouthernstar.com/)
- [Navigare Yacht Club](https://navigareyc.pl/)

## Resources
- [RockBLOCK 9603N](https://www.groundcontrol.com/product/rockblock-9603-compact-plug-and-play-satellite-transmitter/)
- [Minmea NMEA parser](https://github.com/kosma/minmea)
- [OLED Driver](https://github.com/afiskon/stm32-ssd1306)
- [Soft Uart Driver](https://github.com/liyanboy74/soft-uart)


