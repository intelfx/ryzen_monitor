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

/**
 * The --get-* and --set-* options: they read or change SMU settings, print
 * one line each ("get-ppt: 142", "set-ppt: 120 before: 142 after: 120",
 * "NA" if not available, "ERR" on failure) and exit with the highest exit code
 * of the results. Their names and output are those of ryzen_monitor_ng.
 **/

#ifndef CONTROLS_H
#define CONTROLS_H

#include <getopt.h>
#include <stdbool.h>

#include "pm_tables.h"
#include "readinfo.h"

// Append the long options to opts (which has room for them) starting at *n.
void controls_options(struct option *opts, int *n);
// Number of long options controls_options() appends.
int controls_option_count(void);
// Handle an option returned by getopt_long(); false if it is not one of these.
bool controls_handle_option(int code, const char *arg);
// Whether any of these options were given.
bool controls_requested(void);
void controls_print_help(void);

// Run the requested actions in command line order. pmt maps pm_buf, which is
// re-read to show limits after a change.
int controls_run(const pm_table *pmt, unsigned char *pm_buf, const system_info *sysinfo);

#endif
