# Aegir - GPS/LTE/Iridium Tracker

*One off challenge to build a custom GPS tracker that sends data to cloud over LTE and Iridium networks. Tested successfully on the expedition yacht S/Y Southern Star while crossing Barents Sea from Norway to Svalbard in the Arctics. Built with RockBLOCK 9603N(Iridium), SIMCOM7600G (LTE/GSM) and STM32 microcontroller (ARM,FreeRTOS).*

![Aegir GPS Tracker](/photos/1.jpg)

## Backgorund

I travel quite a lot by motorcycle and 4x4, both on-road and off-road. In the case of the latter, navigation is done using topographic maps, and we record and ride along so-called "tracks," which are routes saved using navigation devices and applications like Garmin. These tracks can be saved and shared with others in GPX format.

I am the author of an application for drawing such tracks [GPXLAB](https://gpxlab.net). Since we record routes while traveling, I use GPS trackers installed in vehicles that send location data to a server. This data can be exported to GPX.

The most interesting journeys are to remote places, where there is often no mobile network coverage, and civilization can be hundreds of kilometers away: oceans, deserts, exotic countries. In such places, the only real way to communicate with the world is through Iridium, which is used by devices like the Garmin InReach. This is an important topic because these devices not only allow recording GPX tracks but also enable sending messages or calling for help in case of serious problems.

The next step, as an amateur electronics enthusiast, is to build my own tracker. While building a GSM-based tracker is relatively simple (controller + GPS + sim800l), a project with LTE and satellite communication requires the use of more expensive and better components.

The ultimate motivator for building this project was the opportunity to join the crew of the S/Y Southern Star yacht, which sails in the Arctic.

## Youtube
One of our mates is a youtuber and he had managed to capture some footage while sailing the arctic waters.
- [Ep1. Rejs na Arktykę](https://www.youtube.com/watch?v=Z5egoxSh6OI)
- [Ep2. Miejsca bez Powrotu w Arktyce](https://www.youtube.com/watch?v=qfbvTMd-tyc)
- [Ep3. Sztorm na Krańcu Świata](https://www.youtube.com/watch?v=2DE-GZrynNQ)

## Yacht & Crew
- [S/Y Southern Star](https://www.sysouthernstar.com/)
- [Navigare Yacht Club](https://navigareyc.pl/)

