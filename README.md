# whack-a-mole-Game-on-STM32
A Whack-A-Mole Game on STM32F402RCTx

The game features a RESET function, WIN and LOSS logic and VARIABLE DIFFICULTY using a potientometer.

**Hardware Required**
1-Any Number of LEDs
2-Any Number of Buttons
3-A Potentiometer for variable difficulty.
4-MTM or MTF Wires
5-Breadboard

**Working Principle, A breakdown of the Code**

Three STRUCTS define LED, BUTTON and GAMELOGIC, the STRUCT contain information needed to define each button, LED and the games internal Logic.

**GameLogic Function, the brains**
The GameLogic function is the brains of the whole project, it takes in 3 structs passed as pointers, and an unsigned integer which is the time in ms that will be passed by the function HAL_GetTick().

**Debounce and CheckButton**
Debounce() uses non-blocking function to define the logic for a debounce, while CheckButton() called Debounc() internally to check if a button is pressed or not, basically Register a button.

**readPot and scaleTime**
readPot() simply does the basics to read and call the potientiometer, while scaleTime scales the read values of the potientiometer into something that can be actually used for difficulty reading (linear interpolation).

The RNG() function uses linear feedback shift register to make RNG numbers, since Im too lazy to figure out how to use STMS own RNG system.

Rest of the functions are self-explanatory.
