# ryzen_monitor
Monitor power information of Ryzen processors via the PM table of the SMU.

This tool is based on the [ryzen_smu](https://gitlab.com/leogx9r/ryzen_smu) kernel module for reading the PM Table from the SMU. It is a continuation of the demo-tool provided with that project.

**ryzen_monitor** features support for multiple PM table versions (i.e. multiple bios versions), adds support for Ryzen 5000, and presents more fields to user. It is especially focused around providing a more realistic image of the actual power draw and hence true thermal output of the CPU package.

## Supported PM tables
What can be shown depends on the PM table version that ships with your BIOS and whether ryzen_monitor knows how to read it:

| Version | Processors | Source |
|---|---|---|
| 0x100100 | Zen+ (Pinnacle Ridge, made-up version, experimental) | linnaea/ryzen_monitor |
| 0x1E0004 | Picasso | mann1x/ryzen_monitor_ng |
| 0x240803 | Ryzen 9 3950X | |
| 0x240903 | Ryzen 5 3600 to Ryzen 7 3800XT | kvic-z/ryzen_monitor_ng |
| 0x370003, 0x370005 | Renoir, Lucienne | mann1x/ryzen_monitor_ng |
| 0x380804, 0x380805 | Ryzen 9 5900X, 5950X | |
| 0x380904, 0x380905 | Ryzen 5 5600X, Ryzen 7 5800X | |
| 0x400005 | Ryzen 5 5600G, Ryzen 7 5700G | |
| 0x540004 | Raphael with 2 CCDs (experimental) | ChrisDeadman/ryzen_monitor_ng |
| 0x540104 | Raphael with 1 CCD (experimental) | ChrisDeadman/ryzen_monitor_ng |
| 0x620205 | Granite Ridge with 2 CCDs | amkillam/ryzen_smu #67 |

For an unknown version, ryzen_monitor names a known revision of the same table, if there is one, which can be tried with `-f`.

## Screenshot
![ryzen monitor](https://user-images.githubusercontent.com/50338637/107124627-4e733e00-68a5-11eb-9596-bb3d35120a73.png)

## Building
First install the [ryzen_smu](https://github.com/amkillam/ryzen_smu) kernel module, version 0.1.7: the bundled
library only works with that version of the driver. A good indication of whether the module works for you is
whether `/sys/kernel/ryzen_smu_drv/pm_table` is available.

Then pull and make **ryzen_monitor**:
```bash
git clone https://github.com/hattedsquirrel/ryzen_monitor.git
cd ryzen_monitor/
make

sudo ./src/ryzen_monitor
```
To build against the library of a ryzen_smu checkout instead of the bundled copy, use
`make LIBSMU_DIR=/path/to/ryzen_smu/lib`.

## Usage
`ryzen_monitor -h` lists all options.

### Views
The output is made of views: `info`, `cores`, `stats`, `co` (Curve Optimizer counts), `electrical`, `memory`,
`gfx`, `power` and `summary`. `--view=a,b,...` selects which are shown; all but `summary` are by default.
`-c` leaves out details. In the interactive monitor, keys toggle the views
(`i`, `k`, `s`, `o`, `e`, `m`, `g`, `p`, `y`) and compact mode (`c`); `q` quits.

### Output formats
`-o` selects the output format: `boxdrawing` (the default), `text` (one `Label: value` line per row), `json`,
`ndjson`, `influx`, `prometheus` or `netdata`. `-1` prints a single sample and exits; `-u` sets the update
interval in seconds.

* **telegraf**: `-e /path/to/pipe` writes InfluxDB line protocol to a named pipe, with the measurement and field
  names of ryzen_monitor_ng, for telegraf's `tail` input with `pipe = true`.
* **Prometheus**: `ryzen_monitor -o prometheus -1 > ryzen.prom` produces a file for node_exporter's textfile
  collector.
* **Netdata**: as an external plugin, a `ryzen_monitor.plugin` in plugins.d only has to run
  `exec ryzen_monitor -o netdata -u "$1"`.

### SMU settings
The `--get-*` and `--set-*` options read and change PPT, TDC, EDC, STAPM and temperature limits, the PBO scalar,
OC mode, the APU power mode and Curve Optimizer counts, print the result and exit. **Changing them can make
the system unstable or damage it. Use them at your own risk.**

### Working out new tables
`scripts/dump_pm_table.py` saves PM table dumps at idle and under load. `-w <file>` saves a single dump.
`-t <file> -f <version>` reads a dump instead of the live table; with `--dumptable` it also prints every
element of the table.

## About the quality of the provided information
Don't rely on the information given by this tool.

To my knowledge there is no official documentation on how to do this. Everything was created by starring at numbers, a lot of guesswork and finding fragments somewhere on the web. It is possible that some assignments or calculations are incorrect.

This program is provided as is. If anything, it is a toy for the curious. Nothing more. Use it at your own risk.
