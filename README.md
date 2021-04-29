# Embedded

1.System Design
To design our Sine Wave Generator we have divided the functionalities
specified on the project document to the tasks. We have divided the functionalities
into tasks as follows:
Display:
● Responsible for LCD output,
● Should display the current amplitude and frequency on its screen,
● Should not occupy much CPU time
Keypad:
● Responsible for Keypad input,
● Should set frequency when [1-3 digit]* entered.
● Should set amplitude when [1-3 digit]# entered.
● Any other input must be ignored.
Dac:
● Responsible for generating sinewave & DAC output
● Sine wave must be constructed from a lookup table.
● Sine wave frequency and amplitude should be adjustable.
● Dac should output period
Serial:
● Responsible for processing serial commands
● 4 Single byte commands that correspond to get and set methods of
frequency and amplitude accepted, others are ignored.
● Any invalid characters entered while waiting for a set input cause
cancellation of a set operation.
Round Robin with ISR’s architecture allows us to divide the CPU time as
required between the tasks.
