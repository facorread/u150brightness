/*
    brightness: LCD brightness control for Lenovo U150 laptops
    Copyright (C) 2010 Fabio Andres Correa Duran

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

	More information about this program, and contact, at
	http://facorreadtech.blogspot.com/2010/03/gentoo-linux-on-lenovo-u150.html
*/

using namespace std;

#include <iostream>
#include <string>
extern "C" {
//For pci calls
#include <pci/pci.h>
//For sleep
#include <unistd.h>
}

// For a logarithmic brightness control, this defines the change rate
const float multiplier = 1.02;
// Lower brightness limit
const float lowlimit = 1;
// Upper brightness limit
const float uplimit = 255;

void usage() {
	cerr << "usage: brightness up | down\n";
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		usage();
		return 1;
	}
	float factor;
	bool increasing = false;
	if (string("up").compare(argv[1]) == 0) {
		factor = multiplier;
		increasing = true;
	} else if (string("down").compare(argv[1]) == 0) {
		factor = 1 / multiplier;
	} else {
		usage();
		return 1;
	}
	// Gets basic PCI access
	struct pci_access *pciaccess = pci_alloc();
	pci_init(pciaccess);
	// Gets a handle of the backlight device. Change the numbers accordingly to your lspci output.
	struct pci_dev *backlight = pci_get_dev(pciaccess, 0, 0, 2, 0);
	// Brightness register position
	int brightpos = 0xF4;
	// Reads brightness
	const u8 start = pci_read_byte(backlight, brightpos);
	u8 b = start;
	float bright = b;
	for (int t = 0; t < 25; t++) {
		b = pci_read_byte(backlight, brightpos);
		if (b > bright)
			bright = b;
		bright *= factor;
		if ((bright >= lowlimit) && (bright <= uplimit)) {
			// Sets brightness
			b = (u8)bright;
			pci_write_byte(backlight, brightpos, b);
		} else if (bright < lowlimit) {
			// Set brightness to minimum
			pci_write_byte(backlight, brightpos, (u8)lowlimit);
			break;
		} else if (bright > uplimit) {
			// Set brightness to maximum
			pci_write_byte(backlight, brightpos, (u8)uplimit);
			break;
		}
		usleep(10000);
	}
	if (increasing && (start == 1) && (b == start))
		pci_write_byte(backlight, brightpos, (u8)2);
	pci_free_dev(backlight);
	pci_cleanup(pciaccess);
	pci_free_name_list(pciaccess);
	return 0;
}
