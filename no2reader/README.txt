=== Build ===

Install systemd and systemd-devel

make

=== Clean ===

make clean

=== Run ===

1) Copy 22-microchip-hid.rules to /etc/udev/rules.d. Set the permission and owner of the file: -rw-r--r--. root root

2) Add user to the dialout group.

3) Connect the board to the computer using the micro-b USB cable.

4) Find /dev/hidraw0

=== Read the card ===

Connect the board to the computer using the micro-b USB cable

Run the command "./hidr" to read the raw data.

