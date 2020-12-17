# How to Create From Scratch a Project for _CPU1-Discovery_ Code #

The OpenSTM32 wiki have a [page](http://www.openstm32.org/Getting%2Bstarted%2Bwith%2BSystem%2BWorkbench%2Bfor%2BSTM32) to start a project from scratch. With the latest version of every support libraries and sources from ST. But here I prefer to have a project with the support files validated (and compatible) with the _OSTC4_ code.

## Start with an Empty Project ##

**Special Notes**:

- _Linux_ users should read about this **well known _Eclipse_ bug** : http://www.openstm32.org/faq1#q22
- We do have a _STM32F427IIT6_ chip as CPU1. But the support file provided by _ST_ lake for _LCD TFT Display Controller_ (aka LTDC), even in the latest available version 1.7.2 from 2017-11-06... So we use the STM32F429IITx support file instead, that seems to be similar enough. 

In the following description, I suppose you used _mercurial_ to clone the sources into a **D:\Dev\OSTC4** directory. Make the changes for your own settings...

0. Start the IDE. On most machine it will be the `eclipse` in the _Ac6_ folder you installed.
1. Create an empty project: `File` > `New` > `C Project`
2. Choose `AC6 STM32 MCU Project` and `AC6 STM32 MCU GCC`. Click `Next`  
Note: **make sure NOT TO CLICK FINISH... as it will skip all the rest of the configuration wizard !**  
3. Keep `Debug` and `Release` configurations. Click `Next`
4. Create a new board, that you can name **OSTC4-CPU1-F429**, by choosing:  
>   Series: `STM32F4`  
>   Mcu: `STM32F429IITx`   
>   Debug: `JTAG`    
Click `Next`.    
5. Keep `No firmware` and select `Don't generate startup files`. Then click `Finish`

## Configure Various Settings ##

1. In the _C/C++ Projects_ explorer, select your project top-level folder
2. Right on it, select `Properties`, `Resource` and set `Text file encoding` to **UTF-8**
3. Right on it, select `Properties`, `Resource` then `Linked Resources` and click on `New...` to define a new variable **OSTC4** that points to your source folder **D:\Dev\OSTC4** (or whatever path you used on your system).    
This should allow to change the project's origin with a single variable change.

# Add Sources Folders #

1. In the _C/C++ Projects_ explorer, select your project top-level folder
2. Right-click to add a `New` > `Folder`. In `Advanced settings >` click `Link to alternate location` and then the `Variables...` button. Click on the **OSTC4** variable, then click `Extend...` and select the **Common\\** sub-directory. Click `Finish`
3. Do the same to import the linked folder  **OSTC4\Discovery**

## Make _Eclipse_ aware you just added sources, not junk ##

(_Note: this step is only useful on specific versions of Eclipse. And already done by default on others..._)

1. In the _C/C++ Projects_ explorer, open your project top-level project.
2. Right click on the `Common` folder, select `Resource Configurations...`, and `Exclude from build`. Make sure to UNCHECK both `Debug` and `Release` check-boxes.
3. Do the same for the `Discovery` folder.

## Make _Eclipse_ aware not to compile templates ##

Some sources or directories in the _ST_ distribution contains source you shall not compile. So you should tell _OSE_.

1. In the _Project Explorer_, open item `Common/Drivers/STM32F4xx_HAL_DRIVER_v120/Src`. Right click on `stm32f4xx_hal_msp_template.c`source file, select `Resource Configurations...`, and `Exclude from build`. Make sure both `Debug` and `Release` check-boxes are CHECKED.
2. Do the same on the `Common/Drivers/STM32F4xx_v220/Source/Templates` directory.

# Compile #

1. Select your top-level project, right-click to edit `properties`. In `C/C++ Build` choose `Settings`, then `MCU GCC Compiler` and `Optimization`. Choose `Optimization Level`: -Os (To reduce flash time)

Ok, sources are there. You can read them. But you need a few more efforts to let the IDE find everything he requires to compile _CPU1 Discovery_ firmware.

## Add include paths ##

1. Select your top-level project, right-click to edit `properties`. In `C/C++ Build` choose `Settings`, then `MCU GCC Compiler` and `Includes`. **Make sure to select `Configuration: [All Configurations]` **. Click `Add...` choose `Workspace...` and select the `Common/Drivers/CMIS/Include` directory.
2. Do the same for `Common/Drivers/STM32F4xx/Include` directory.
3. Do the same for `Common/Drivers/STM32F4xx_HAL_DRIVER/Inc` directory.
4. Do the same for `Common/Inc` directory.
5. Do the same for `Discovery/Inc` directory.
6. Use the _Move Up_ icon to make sure `Discovery/Inc` is first.
7. Use the `Configuration:` pop-up to check you have all the includes in both `Debug` and `Release` configuration.

Once done, If you go to the _C/C++ Projects explorer_, open your top-level project, and open the `Includes` item, you should see:

- a few lines added by the system `.../Ac6/Workbench/plugins/.../compiler/...` with include paths required by the GCC compiler.
- then your 5 include directories (shown in alphabetic order, not priority order... too bad). Make sure to open each of them and to double-click on a .h file to check it opens correctly in the IDE.

## Add Link Options ##

1. Select your top-level project, right-click to edit `properties`. In `C/C++ Build` choose `Settings`, then `MCU G++ Linker` and `General`.
And select _linker Script_: `OSTC4/Common/CPU1-F429.ld`
2. In `C/C++ Build` choose `Settings`, then `MCU GCC Linker` and `Miscellaneous`. Add option -u _printf_float in Linker flags. This is necessary to prevent IDE warning "The float formatting support (-u _printf_float) is not enabled from linker flags".

## Prepare generation of bin file ##
1. Select your top-level project, right-click to edit `properties`. In `C/C++ Build` choose `Settings`, then tab `Build Steps` and `Post build step command`.
2. Add option -R .upper\* and -R.font_firmware_data to the command to removed the upper memory section, which is provided by the font library =>arm-none-eabi-objcopy -R .upper\* -R.font_firmware_data -O binary "${BuildArtifactFileBaseName}.elf" "${BuildArtifactFileBaseName}.bin"; arm-none-eabi-size "${BuildArtifactFileName}"

## Build ELF firmware ##

1. Click the `Build` button (hammer icon) on the top bar.
