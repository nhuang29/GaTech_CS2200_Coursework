# Interrupts Project
### Brandonsim is a modified version of a software that simulates processor activity
1 Introduction
We have spent the last few weeks implementing our 32-bit datapath. The simple 32-bit GT-2200 is capable of performing advanced computational tasks and logical decision making. Now it is time for us to move on to something more advanced–the upgraded GT-2200a enables the ability for programs to be interrupted. Your assignment is to fully implement and test interrupts using the provided datapath and Brandonsim. You will hook up the various interrupt and data lines to the new devices, modify the datapath and microcontroller to support interrupt operations, and write interrupt handlers to operate these new devices.

2 Requirements
This project is broken up into two parts:
• Part 1 - Implementing a basic interrupt.
• Part 2 - Implementing priority and and second timer.
Before you begin, please ensure you have done the following:
• Download the proper version of Brandonsim.
• Brandonsim is not perfect and does have small bugs. In certain scenarios, files have been corrupted and students have had to re-do the entire project.

4 Part 1: Implementing a Basic Interrupt
For this part of the assignment, you will add interrupt support to the GT-2200a datapath. Then, you will test your new capabilities to handle interrupts using an external timer device.
Work in the GT-2200a-part1.circ file. You will complete all work for Part 1 in this file. If you wish to use your existing datapath, make a copy with this name, and copy the Timer and Priority Fence subcircuits from the file we provided.

4.1 Initial Interrupt Hardware Support
First, you will need to add the initial hardware support for interrupts.
You must do the following:
1. Our processor needs a way to turn interrupts on and off. Create a new one-bit “Interrupt Enable” (IE) register. You’ll connect this register to your microcontroller in a later step.
2. Create the INT line. The external device you will create in 4.2 will pull this line high (assert a ’1’) when they wish to interrupt the processor. Because in later part of this project, multiple devices can share a single INT line, the devices must use a tri-state circuit, similar to the main bus. When a device does not have an interrupt, it neither pulls the line high or low. To ensure your INT line reads as low (i.e., ’0’) when no devices are requesting an interrupt, add a pull-down resistor (Brandonsim contains a component to do this).
3. When a device receives an IntAck signal, it will drive a 32-bit device ID onto the I/O data bus. To prevent misbehaving devices from interfering with the processor, the I/O data bus is attached to the main bus with a tri-state driver. Create this driver and the bus, and attach the microcontroller’s DrIO signal to the driver.
4. Modify the datapath so that the PC starts at 0x10 when the processor is reset. Normally the PC starts at 0x00, however we need to make space for the interrupt vector table (IVT). Therefore, when you actually load in the test code that you will write, it needs to start at 0x10. Please make sure that your solution ensures that datapath can never execute from below 0x10 - or in other words, force the PC to drive the value 0x10 if the PC is pointing in the range of the vector table.
5. Create hardware to support selecting the register $k0 within the microcode. This is needed by some interrupt related instructions. Because we need to access $k0 outside of regular instructions, we cannot use the Rx / Ry / Rz bits. HINT: Use only the register selection bits that the main ROM already outputs to select $k0.

4.2 Adding First External Timer Device
Hardware timers are an essential device in any CPU design. They allow the CPU to monitor the passing of various time intervals, without dedicating CPU instructions to the cause.
The ability of timers to raise interrupts also enables preemptive multitasking, where the operating system periodically interrupts a running process to let another process take a turn. Timers are also essential to ensuring a single misbehaving program cannot freeze up your entire computer.
You will connect a external timer device to the datapath. And it should have a device ID of 0x1 and a 1000-cycle tick timer interval
The pinout of the timer device is described below. If you like, you may also examine the internals of the device in Brandonsim.
• INT: The device will begin to assert this line when its time interval has elapsed. It will not be lowered until the device receives an INTA signal.
• INTA IN and INTA OUT: When the INTA IN line is asserted while the device has asserted the INT line, it will lower the INT line and drive its device ID to the DATA line in the next clock cycle. If the device receives an INTA signal while it has not asserted INT, it will pass the signal onto the next device through INTA OUT. This functionality can be used to daisy-chain devices.
• ID and DATA: The user may configure the device’s ID through the ID pin. The device ID is passed to the DATA pin when the device receives an INTA signal after asserting the INT line.
• TIME: The user may configure the device’s timer interval through this pin. The interval is specified in number of clock cycles. When the interval has elapsed, the device will raise the INT line.
The INT and ID lines from the timer should be connected to the appropriate buses that you added in the previous section.

4.3 Microcontroller Interrupt Support
Before beginning this part, be sure you have read through Appendix A: Microcontroller Unit and Appendix B: GT-2200a Instruction Set Architecture and pay special attention to the new instructions.
In this part of the assignment you will modify the microcontroller and the microcode of the GT-2200a to support interrupts. You will need to do the following:
1. Be sure to read the appendix on the microcontroller before starting this section.
2. Modify the microcontroller to support asserting five new signals (You could ignore the PriClr signal for Part 1):
(a) LdInt & EnInt to control whether interrupts are enabled/disabled. You will use these 2 signals to control the value of your interrupts enabled register.
(b) IntAck to send an interrupt acknowledge to the device. (c) DrIO to drive the value on the I/O bus to the main bus.
(d) PriClr will be used in Part 2. Include this signal as an output from your ROM but do not connect it to anything for now.
3. Extend the size of the ROM accordingly.
4. Add the fourth ROM described in Appendix A: Microcontroller Unit to handle onInt.
5. Modify the FETCH macrostate microcode so that we actively check for interrupts. Normally this is done within the INT macrostate (as described in Chapter 4 of the book and in the lectures) but we are rolling this functionality in the FETCH macrostate for the sake of simplicity. You can accomplish this by doing the following:
(a) First check to see if the CPU should be interrupted. To be interrupted, two conditions must be true: (1) interrupts are enabled (i.e., the IE register must hold a ’1’), and (2), a device must be asserting an interrupt.
(b) If not, continue with FETCH normally. 
(c) If the CPU should be interrupted, then perform the following:
    i. Save the current PC to the register $k0.
    ii. Disable interrupts.
    iii. Assert the interrupt acknowledge signal (IntAck). Next, drive the device ID from the I/O bus and use it to index into the       interrupt vector table to retrieve the new PC value. The should be done in the same clock cycle as the IntAck assertion.
    iv. This new PC value should then be loaded into the PC.
    
Note: onInt works in the same manner that ChkCmp did in Project 1. The processor should branch to the appropriate microstate depending on the value of onInt. onInt should be true when interrupts are enabled AND when there is an interrupt to be acknowledged.
Note: The mode bit mechanism discussed in the textbook has been omitted for simplicity.
6. Implement the microcode for three new instructions for supporting interrupts as described in Chapter 4. These are the EI, DI, and RETI instructions. You need to write the microcode in the main ROM controlling the datapath for these three new instructions. Keep in mind that:(a) EI sets the IE register to 1. (b) DI sets the IE register to 0.(c) RETI loads $k0 into the PC, and enables interrupts.

4.4 Implementing the Timer Interrupt Handler
Our datapath and microcontroller now fully support interrupts from devices, BUT we must now implement the first interrupt handler t1_handler within the prj2.s file to support interrupts from the timer device while also not interfering with the correct operation of any user programs.
In prj2.s, we provide you with a program that runs in the background. For this part of the project, you need to write interrupt handler for only one timer device (device IDs 0x1). You should refer to Chapter 4 of the textbook to see how to write a correct interrupt handler. As detailed in that chapter, your handler will need to do the following:
1. First save the current value of $k0 (the return address to where you came from to the current handler)
2. Enableinterrupts(whichshouldhavebeendisabledimplicitlybytheprocessorwithintheINTmacrostate).
3. Save the state of the interrupted program.
4. Implement the actual work to be done in the handler. In the case of this project, we want you to increment a counter variable in memory, which we have already provided.
5. Restore the state of the original program and return using RETI.

The handler you have written for the timer device should run every time the device’s interrupt is triggered. Make sure to write the handler such that interrupts can be nested. With that in mind, interrupts should be enabled for as long as possible within the handlers.
You will need to do the following:
1. Write the interrupt handler (should follow the above instructions or simply refer to Chapter 4 in your book). In the case of this project, we want the interrupt handler to keep time in memory at the predetermined location: 0xFFFFFD
2. Load the starting address of the first handler you just implemented in prj2.s into the interrupt vector table at the appropriate addresses (the table is indexed using the device ID of the interrupting device).

Test your design. If it works correctly, you should see a location in memory increment as the program runs.

5 Part 2: Implementing Priority and and Second Timer
Our datapath and microcontroller currently can support multiple devices through daisy chaining. However, this is often an inefficient approach, especially when one device connected to the processor is considered more important than another.
In this part of the project you will implement a priority mechanism in hardware such that only interrupts of higher priority than what is currently running will be processed. You will then connect a second timer device with a higher priority level.
For Part 2, make a copy of your Part 1 circuit file and name it GT-2200a-part2.circ. You must submit both files when you turn in the assignment. The microcode and assembly files can be shared between both parts.

5.1 About the Priority Encoder and Priority Fence
In order to implement the priority mechanism, you will make use of two devices–a priority encoder and the priority fence.
5.1.1 Priority Encoder
A priority encoder takes in multiple priority input lines. Each line corresponds to a numeric “priority”. The encoder then selects the highest priority number and outputs this value. You may use Brandonsim’s built-in priority encoder for this purpose, or you may make your own.
In following the typical operating system convention, we designate lower numbers as higher priority. Thus priority 0 is the highest priority. The GT-2200a supports only two priority levels, 0 and 1.
5.1.2 Priority Fence
When the CPU is handling an interrupt of priority n, it should only respond to an interrupt of higher priority, i.e. x < n. The priority fence device implements this logic using a hardware stack. It sits between the priority encoder and the CPU.
When the priority fence sees an interrupt acknowledge at level n, it locks the level n into the “fence” register. It will then only output an incoming interrupt to the CPU if its priority is less than n, i.e. it is of higher priority than the currently running interrupt.
When the CPU acknowledges a higher priority interrupt, it must remember the previous fence value for when the higher-priority interrupt returns. When the CPU returns from an interrupt, it signals the priority fence with the PriClr signal to indicate that it may return to its previous level.
We have designed this device and provided it to you.

5.2 Adding the Priority Encoder and Priority Fence
You must do the following:
1. Add a second timer device to your circuit, with reset time 600 and device ID 0x2. This device should use the same I/O bus as the first timer, but will have separate INT and INTA lines. Do not daisy-chain this timer to the first timer.
2. Add a priority encoder to your datapath circuit. This can be found in Brandonsim under the Plexers tab. The encoder should have two priority levels: level 0 (high priority) and level 1 (low priority). You should attach the second timer’s INT line to level 0 and the first timer’s INT line to level 1. Because higher-resolution timers (those with shorter intervals) should always take precedence over lower-resolution timers. The priority encoder should output two things:
• Whether an interrupt is currently asserted.
• The priority level of the asserted interrupt. If both lines are asserted, this will be the higher
priority of the two.
NOTE: Although the priority encoder in Brandonsim considers 1 to be the higher priority, in real- world operating system development, lower numbers normally correspond to higher priority. Therefore, your priority encoder should consider level 0 to be high priority and level 1 to be low priority. Make sure that you account for this in your design.
3. Add the priority fence to your datapath circuit. This is a device provided to you as part of the template. Examine the internals of this circuit and make sure you understand its purpose, as this may be a demo question. Attach the outputs of the priority encoder, as well as the IntAck line from the microcontroller, to the appropriate inputs on the fence.
4. The IntAck signal should only be passed to devices of the currently asserted priority level. Otherwise, a device of a lower priority could see the same signal and also attempt to respond to the CPU. You must ensure through hardware that this signal is passed to the correct device.
5. Finally, when we finish handling an interrupt, we must tell the priority fence to resume allowing interrupts at that priority level. To accomplish this, attach the PriClr line from the microcontroller to the priority fence, and update your RETI instruction to assert this line.

5.3 Implement the Second Timer Interrupt Handler
Now that our priority mechanism is working, we are ready to process interrupts from the second timer.
Building off of your work for Part 1 in prj2.s, write a separate handler for the second timer device that increments a separate variable at memory address 0xFFFFFE.
You may be able to copy most of your handler code from the first handler to the second. Also, remember to update the interrupt vector table to contain the address of your second timer interrupt handler.
