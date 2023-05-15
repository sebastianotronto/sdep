# sdep

A simple "date+event" line parser.

sdep follows the UNIX philosphy (do one thing well, use stdin and stdout)
and is heavily inspired by [suckless](https://suckless.org) utilities
such as [dmenu](https://tools.suckless.org/dmenu/).

You can wrap it around shell scripts to turn it into a no-nonsense
calendar system.

## Description

sdep reads lines of the form `date text` from stdin and writes to stdout
those lines such that `date` is between the two dates specified with the
`-f` and `-t` options, both of which default to the current minute.

The format for `date` can be specified with the same syntax as for
date(1).  The dates should correspond to a unique minute in time.

## Installation

Edit the Makefile to match your local configuration and type `make
install`.

## Examples

If `events.txt` contains lines formatted as `date text` then

```
sdep <events.txt
```

will print all lines whose date match the current minute. Instead

```
sdep -f <events.txt
```

will print all the lines whose date is in the past, while

```
sdep -t -w "%A" <events.txt
```

will print all lines whose date is in the future, showing only the day
of the week and the text.

```
sdep -f "1999-01-01 00:00" -t "1999-12-31 23:59" -w "" <events.txt
```

will show only the `text` of all lines with a date in 1999. You can
specify a different format for the dates, for example

```
sdep +"%m/%d/%Y %I:%M%p" -t "12/31/2020 11:59pm" -w "" <events.txt
```

will match all dates from December 31st, 2020, one minute before midnight
(included). Note: this only works if your locale has an am/pm format,
see date(1).

## A stupidly simple calendar app

If you keep your events and reminders in a simple plain text file (say
events.txt), you can run

```
sdep -w "" -s "" | while read text; do notify-send "$text"; done < events.txt
```

every minute, for example using cron(8), to get a notification every
time an events is happens.

You can use `sdep -f -t < events.txt` to list all your events, or 
`sdep -t < events.txt`
to list only the future ones. You can specify any date range. Running

```
temp = $(mktemp)
sdep -t <events.txt >"$temp"
mv "$temp" events.txt
```

will remove all old events from your file.

You can edit your events using any text editor and you can
keep them synced between multiple devices using something like
[rsync](https://rsync.samba.org/).

## Scripts

The `scripts` folder contains the few scripts that I use. They are
basically just a more elaborate version of the calendar system described
above, with support for recurring events (e.g. weekly, daily). You
can install them with `sudo SD=/path/to/scripts/folder make scripts`,
where `SD` specifies the path where the directory where you want your
sdep files to be saved; for example it can be `/home/username/.sdep`.
For example check that the folder SDEPDATA in Makefile suits you.

Most of the scripts rely on the `-d` option of the GNU date utility,
so you should change that too if you are on a BSD system or on MacOS.
