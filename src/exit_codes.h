/**
 * Ryzen SMU Userspace Sensor Monitor
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef EXIT_CODES_H
#define EXIT_CODES_H

// Exit codes, as listed in the help text. When several things fail, the highest code wins.
enum exit_code {
    EXIT_OK = 0,
    EXIT_USAGE = 1,         // invalid option or argument
    EXIT_UNSUPPORTED = 2,   // processor, driver, PM table or setting not supported; not root
    EXIT_DRIVER = 3,        // reading from or commanding the SMU failed
    EXIT_IO = 4,            // dump file or named pipe I/O, out of memory
};

#endif
