/* See LICENSE file for copyright and license details. */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
 * Maximum number of characters in a line. The rest will be truncated.
 * Change this if you need very long lines.
 */
#define MAXLEN 10000

/*
 * Default date format. Anything that strftime(3) understands works, but 
 * it should determine a date completely up to the minute.
 */
static char *default_format = "%Y-%m-%d %H:%M";

typedef struct Event     Event;
typedef struct EventList EventList;
typedef struct EventNode EventNode;
typedef struct Options   Options;

struct Event{
	struct tm  time;
	char      *text;
};

struct EventList {
	EventNode *first;
	EventNode *last;
	int        count;
};

struct EventNode {
	Event      ev;
	EventNode *next;
};

struct Options {
	struct tm  from;
	struct tm  to;
	char      *format_in;
	char      *format_out;
	char      *separator;
};

static void    add_event(struct tm t, char *text, EventList *evlist);
static int     compare_tm(struct tm *t1, struct tm *t2);
static int     compare_event(Event *event1, Event *event2);
static Options default_op(void);
static int     events_in_range(EventList *evlist, Options op, Event *sel);
static char   *format_line(Event ev, Options op, char *out);
static void    read_input(Options op, EventList *evlist);
static Options read_op(int argc, char *argv[]);
static char   *strtrim(char *t);
static void    write_output(Options op, Event *ev, int n);


static void
add_event(struct tm t, char *text, EventList *evlist)
{
	int l = strlen(text)+1;
	EventNode *next = malloc(sizeof(EventNode));

	next->ev.time = t;
	next->ev.text = malloc(l);
	strncpy(next->ev.text, text, l);
	next->ev.text = strtrim(next->ev.text);
	next->next = NULL;

	if (++evlist->count == 1) {
		evlist->first = next;
		evlist->last  = next;
	} else {
		evlist->last->next = next;
		evlist->last       = next;
	}
}

static int
compare_tm(struct tm *t1, struct tm *t2)
{
	if (t1->tm_year != t2->tm_year)
		return t1->tm_year - t2->tm_year;
	if (t1->tm_mon != t2->tm_mon)
		return t1->tm_mon - t2->tm_mon;
	if (t1->tm_mday != t2->tm_mday)
		return t1->tm_mday - t2->tm_mday;
	if (t1->tm_hour != t2->tm_hour)
		return t1->tm_hour - t2->tm_hour;
	return t1->tm_min - t2->tm_min;
}

static int
compare_event(Event *ev1, Event *ev2)
{
	return compare_tm(&ev1->time, &ev2->time);
}

static Options
default_op(void)
{
	Options op;
	time_t t_now = time(NULL);
	struct tm *now = localtime(&t_now);

	op.format_in  = malloc(MAXLEN);
	op.format_out = malloc(MAXLEN);
	op.separator  = malloc(MAXLEN);
	strcpy(op.format_in,  default_format);
	strcpy(op.format_out, default_format);
	strcpy(op.separator,  "\t");
	op.from = *now;
	op.to   = *now;

	return op;
}

/*
 * Saves the events in ev[] that happen between op->from and op->to in sel[]
 * sorted by date and returns their number.
 */
static int
events_in_range(EventList *evlist, Options op, Event *sel)
{
	EventNode *i;
	int n = 0;


	for (i = evlist->first; i != NULL; i = i->next)
		if (compare_tm(&i->ev.time, &op.from) >= 0 &&
		    compare_tm(&i->ev.time, &op.to) <= 0)
			sel[n++] = i->ev;

	qsort(sel, n, sizeof(Event),
	      (int (*)(const void *, const void *))compare_event);

	return n;
}

static char *
format_line(Event ev, Options op, char *out)
{
	strftime(out, MAXLEN, op.format_out, &ev.time);
	strncat(out, op.separator, MAXLEN - strlen(out));
	strncat(out, ev.text, MAXLEN - strlen(out));

	return out;
}

static void
read_input(Options op, EventList *evlist)
{
	struct tm t;
	char line[MAXLEN], *text_ptr;

	while (fgets(line, MAXLEN, stdin) != NULL)
		if ((text_ptr = strptime(line, op.format_in, &t)) != NULL)
			add_event(t, text_ptr, evlist);
}

static Options
read_op(int argc, char *argv[])
{
	Options op = default_op();
	int i;

	/* Check for format specification.
	 * This changes the way other options are read */
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '+') {
			strncpy(op.format_in,  &argv[i][1], MAXLEN);
			strncpy(op.format_out, &argv[i][1], MAXLEN);
		}
	}

	for (i = 1; i < argc; i++) {
		if        (!strcmp(argv[i], "-v")) {
			puts("sdep-"VERSION);
			exit(0);
		} else if (!strcmp(argv[i], "-d")) {
			puts(default_format);
			exit(0);
		} else if (!strcmp(argv[i], "-s")) {
			strncpy(op.separator, argv[++i], MAXLEN);
		} else if (!strcmp(argv[i], "-f")) {
			if (i+1 >= argc ||
			    strptime(argv[i+1], op.format_in, &op.from) == NULL)
				op.from.tm_year = -1000000000; /* Very large number */
			else
				i++;
		} else if (!strcmp(argv[i], "-t")) {
			if (i+1 >= argc ||
			    strptime(argv[i+1], op.format_in, &op.to) == NULL)
				op.to.tm_year = 1000000000; /* Very small number */
			else
				i++;
		} else if (!strcmp(argv[i], "-w")) {
			op.format_out = argv[++i];
		} else if (argv[i][0] != '+' && strlen(argv[i]) != 0) {
			fprintf(stderr, "usage: sdep [-dv]");
			fprintf(stderr, " [+format] [-f [date]] [-t [date]]");
			fprintf(stderr, " [-w format] [-s string]\n");
			exit(1);
		}
	}

	return op;
}

static char *
strtrim(char *t)
{
	char *s;

	for (s = &t[strlen(t)-1]; s != t && isspace(*s); *s = '\0', s--);
	for (; *t != '\0' && isspace(*t); t++);

	return t;
}

static void
write_output(Options op, Event *ev, int n)
{
	char outline[MAXLEN];
	int i;

	for (i = 0; i < n; i++)
		printf("%s\n", format_line(ev[i], op, outline));
}

int
main(int argc, char *argv[])
{
	Options op;
	EventList evlist = {0};
	Event *selected;

	op = read_op(argc, argv);
	read_input(op, &evlist);
	selected = malloc(sizeof(Event) * evlist.count);
	write_output(op, selected, events_in_range(&evlist, op, selected));

	return 0;
}
