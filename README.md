# A-Digital-Voice-Recorder-Project
Main objective of this project/final is to create a digital voice recorder that can record your voice, playback a selected voice recording, and delete single or all recording data. During this project, you will use various modules such as Timer, PWM, ADC, and External Interrupts.

###	Introduction
The aim of this project is to design a digital voice recorder which contains four states. In start state, (which only happens when the board powers up 7SD) 7SD should show the ID number. In record state, it should show “rcd” and a count down from 5 seconds indicating the recording. In playback state, it should show “PLb” and a count down from 5 seconds indicating the playback. In last state which is IDLE state, 7SD displays IdLE on the screen and does not do anything else. In order to setup the digital voice recorder, 4 digit seven segment display, 4x4 keypad, a speaker, a PWM filter with LM386 audio amplifier, a 24LC512 EEPROM, an electret microphone and stm32g031k8 microprocessor will be used in this project.  

### Schematic Diagram

![resim](https://user-images.githubusercontent.com/44584158/110015916-7edbba00-7d35-11eb-8c8b-0e352d4b3d5c.png)

### Block Diagram

![resim](https://user-images.githubusercontent.com/44584158/110015972-8e5b0300-7d35-11eb-99da-3f3d2ad74d51.png)

### Flowchart

#### Main Flowchart

![resim](https://user-images.githubusercontent.com/44584158/110016086-ac286800-7d35-11eb-91b4-640e5e813eb8.png)

#### TIM1 Handler's Flowchart

![resim](https://user-images.githubusercontent.com/44584158/110016480-20fba200-7d36-11eb-90d6-84cd8d37cede.png)


#### ADC_COMP IRQHandler's Flowchart

![resim](https://user-images.githubusercontent.com/44584158/110016166-c5311900-7d35-11eb-9de4-8e727c905c2d.png)

#### TIM16 Handler's Flowchart

![resim](https://user-images.githubusercontent.com/44584158/110016529-307aeb00-7d36-11eb-9055-bba1e2e5ab65.png)

### Parts List (w/ prices)

- Bunch of resistors and capacitors: 2 TL
- 4 Digit SSD (Seven Segment Display) 7,34 TL
- 4x4 Keypad 8,12 TL
- M-M Jumpers 3,11 TL
-STM32G031K8: 120 TL
- 0.5W Speaker: 8,73 TL
- LM386 Audio Amplifier: 0,87 TL
- 24LC512 EEPROM: 9,82 TL
- Electret Microphone: 15,48 TL
Total: 175,47 TL

### Project Setup (w/ picture)

![resim](https://user-images.githubusercontent.com/44584158/110016685-615b2000-7d36-11eb-99db-3711fcda32bf.png)

It was used four 1k Ω resistor, 1 uF, 47 nF, 220 uF capacitors, 0.5W Speaker, LM386 Audio Amplifier, 24LC512 EEPROM, Electret Microphone, 4 digit SSD (Seven Segment Display), 4x4 Keypad, bunch of M-M Jumpers and STM32G031K8 microprocessor in this project.

#### PWM Filter for the 0.5W Speaker
![resim](https://user-images.githubusercontent.com/44584158/110016651-586a4e80-7d36-11eb-8b1d-592cf3ee4a2c.png)

#### LM386 Audio Amplifier
![resim](https://user-images.githubusercontent.com/44584158/110016749-733cc300-7d36-11eb-8adc-e67439827250.png)

#### 24LC512 EEPROM
![resim](https://user-images.githubusercontent.com/44584158/110016802-851e6600-7d36-11eb-89f6-479e1c9322f3.png)

### Detailed Task List

Task 1. Setup the speaker and microphone circuits. ✓

Task 2. Make analog loopback. ✓

Task 3. Arrange the PWM and Sampling frequency due to the Nyquist. ✓

Task 4. Make digital loopback. ✓

Task 5. Write the audio data to the EEPROM. ✓

Task 6. Read the audio data from the EEPROM. X

Task 7. When the program starts, arrange the 7SD to the START State. ✓ 

Task 8. When the record button is presssed, arrange the 7SD to the RECORD State. ✓

Task 9. When the playback button is presssed, arrange the 7SD to the PLAYBACK State. ✓ 

Task 10. When no button is pressed, arrange the  7SD to the IDLE State. ✓ 

### How The Algorithm Works?

#### State Transition Diagram Of the Project
![resim](https://user-images.githubusercontent.com/44584158/110016916-a5e6bb80-7d36-11eb-96f4-00b5cdf86c49.png)

#### Main Function
Timers, ADC, PWM and I2C are set in main function.

#### ADC_COMP_IRQHandler
This handler comprehends 3 different things as follows:
To record the voice, the record button is pressed from the keypad and record flag opens. After that, the handler takes the data from the electret microphone and save to the data array. When the array’s element reaches a certain number, it writes the array to the EEPROM as a package.
To playback the voice, the playback button is pressed from the keypad and read flag opens. Then, the handler takes the data from the data array and send it to the CCR2 register. When the array’s element reaches a certain number, it reads the data array from EEPROM as a package.
To delete the voice, the playback button is pressed from the keypad and delete flag opens. Then, the handler writes a 0 value to the every element in the data array. When the array’s element reaches a certain number, it writes the data array to EEPROM as a package.

#### TIM1_IRQHandler
This handler displays the current state to the 7SD due to the flags. When the program begins, it shows the start state. When the record flag is opened, it changes the state as record state and displays to the 7SD. After 5 seconds, it changes the state as IDLE state an displays the IDLE. When the read state is opened,  the handler changes the state to playback state for five seconds. After that, it turns back to the IDLE state. 

#### TIM16_IRQHandler
The purpose of this handler is to hold the second. Therefore, the clock speed of the handler is set to 1 hz. After five seconds, it sets the flags as zero and helps to translate the state to IDLE state in TIM1 handler.

Challenges That’s Been Solved:
1) Hold the seconds and display it to the 7SD.
2) Digital Loopback.
3) Brightness difference on the 7SD.

How Was This Challenges Solved:
1) This challenge has been solved using the TIM16 handler. The clock speed of the TIM16 handler was set to 1Hz, and second variable was reduced by one  at each interrupt.
2) This difficulty has been solved by using both the filtering circuit and the appropriate PWM and sampling frequency. The potentiometer used in filtering was brought in the 50% band, and the PWM frequency was set to be 60kHz and the sampling frequency to be 8kHz. The audio data is taken from ADC COMP Handler and given to the TIM2 CCR2 register.
3) To solve this difficulty, dummy delays used in the display function have been removed. Using a different algorithm, a different digit is turned on every interrupt  and when the handler speed is increased, the brightness difference between digits is eliminated.

Unsolved Difficulites:
1) Writing and reading the data from EEPROM properly in five seconds.
2) Understandable recording to the EEPROM.
What is thought to be the root of the problem?
1) It is predicted that the source of this difficulty may be wrongly given priority between interrupt handlers, improper ARR and PSC values or inefficient use of ADC_COMP_IRQ handler.
2) It is anticipated that writing data quickly and reading it slowly may be one of the sources of this problem.

### Conclusion and Comments
In conclusion, a digital voice recorder could not have been designed. In the designing phase, experience has been gained about how to make PWM filter for the 0.5W speaker and using the filter with LM386 audio amplifier. Using this filter, The digital loopback was achieved with proper PWM and sampling frequency. Although the data can be written to the EEPROM and read, an understandable sound could not be produced from the speaker. During the display to 7SD phase, 4 states have been created. (Start, Record, Playback and IDLE state) Due to the flags, the program translates the states.



