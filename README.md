# ca0132-tools
A collection of tools for debugging the ca0132 HDA codec. Also included is an assembler/disassembler for the onboard "Quartet" DSP.

## ca0132-dump-state
Create a simulator state by dumping the contents of a running ca0132.
Then, you can enter the main loop and simulate from there. Uses an unused
ChipIO ParamID handler to run the dumping program.

## ca0132-8051-command-line

Allows use of the onboard 8051's serial command console by storing commands
in the buffer and updating the write pointer.

Commands are:
* ver   - Prints out device version information. Same data for all ca0132
        codecs it seems.
* echo  - On or off. Turns off command echo in console.
* sfr   - Takes an address, returns the value of the 8051 SFR at that address.
* dmem  - Takes an address, returns the value of the 8051 iram at that address.
* xmem  - Takes an address, returns the value of the 8051 xram at that address.
* cmem  - Takes an address, returns the value of the 8051 pmem at that address.
* hci   - Takes an address, returns the value of the HCI bus at that address.
        The HCI bus is the chipio data interface. For some reason, in the
        driver, it's referred to as both HCI and HIC.
* apb   - Takes an address, returns the value of the APB bus at that address.
        Not sure what this address bus is used for, if it's in use at all.
        Range is only 8-bits, returns a 32-bit value.
* hda   - Takes an address, returns data at that address. This seems to be
        the actual HDA link interface control, uses SFR 0xdb for the
        address, and 0xdc for the data. Things written here are streamID's
        and stream format values. Range is 8-bits.
* pmu   - Takes an address, returns the value of the 8051 PLL PMU at that address.
* eepr  - Takes an address, returns data. Not sure if the eeprom actually
        exists, or if this is just left over from development.
* i2c   - Has multiple forms:
        i2c target <addr> - sets the master target address.
        i2c mode <op_mode> <7/10> - Sets the op and address modes. op modes
                                    are: slave, std, fast, fast+.
        i2c rd <cnt> - Reads 1-8 bytes from i2c.
        i2c wr <<b0> ... <b7>> - Writes 1-8 bytes to i2c.
        None of these seem functional, reads/writes always return I/O
        error.
* flag  - With no arguments, prints all flag values. With a value argument,
        prints out the flag value, string, and if it's set.
* ctrl  - Same as flag command, either prints all chipio control param values
        or a specific one specified in an argument.
* log   - Prints a log of some sort, not sure from where.
* pin   - Prints out HDA node pin values. Not sure what they represent.
* codec - Takes a codec number value, seems to print out the codecs power state.
* q     - Prints out the high water mark of the verb buffer.
