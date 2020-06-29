# Using OpenSTM32 IDE to Compile OSTC4 Firmware #

The free and Open [STM32 IDE](http://www.openstm32.org/System%2BWorkbench%2Bfor%2BSTM32) is an integrated development environment based on _Eclipse_ that will allows you to compile and link code for the OSTC4 dive computer.
You will then be able to generate `.bin` firmware files to upload to your computer using your favourite tools, eg. [OSTC Companion](https://ostc-planner.net/companion). 

1. Register on http://www.openstm32.org/ : that will allows you to download the IDE.
2. Once registered and logged in, you can go to _Documentation_ and _Installation Manual_. Follow the instructions to install the IDE (available for Windows 7, MacOS and Linux).
3. Use the ready made project ... in ... 

Or you can create a brand new project by following the _How To_'s:

- [HowTo compile CPU1-Discovery main firmware](Detailed CPU1-Discovery Project.md)
- [HowTo compile CPU1-Discovery protected area firmware](Detailed CPU1-Upper Project.md)
- [HowTo compile CPU2-RTE real-time firmware](Detailed CPU2-RTE Project.md)
- [HowTo generate uploadable firmware file](Make Firmware File.md)