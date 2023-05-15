/* See LICENSE file for copyright and license details. */

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


typedef struct Event Event;
typedef struct EventList EventList;
typedef struct EventNode EventNode;
typedef struct Options Options;

struct Event{
	struct tm time;
	char *text;
};

struct EventList {
	EventNode *first;
	EventNode *last;
	int count;
};

struct EventNode {
	Event ev;
	EventNode *next;
};

struct Options {
	struct tm from;
	struct tm to;
	char *format_in;
	char *format_out;
	char *separator;
};

static void add_event(struct tm, char *, EventList *);
static int compare_tm(const void *, const void *);
static int compare_event(const void *, const void *);
static Options default_op(void);
static int events_in_range(EventList *, Options, Event *);
static char *format_line(Event, Options, char *);
static int is_space(char);
static void read_input(Options, EventList *);
static Options read_op(int, char *[]);
static void str_copy(char *, char *, int);
static char *str_trim(char *);
static void write_output(Options, Event *, int);


static void
add_event(struct tm t, char *text, EventList *evlist)
{
	size_t l = strlen(text)+1;
	EventNode *next = malloc(sizeof(EventNode));

	next->ev.time = t;
	next->ev.text = malloc(l);
	str_copy(next->ev.text, text, l);
	next->ev.text = str_trim(next->ev.text);
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
compare_tm(const void *v1, const void *v2)
{
	const struct tm *t1 = v1;
	const struct tm *t2 = v2;

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
compare_event(const void *v1, const void *v2)
{
	const Event *ev1 = v1;
	const Event *ev2 = v2;

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

	qsort(sel, n, sizeof(Event), compare_event);

	return n;
}

static char *
format_line(Event ev, Options op, char *out)
{
	size_t l;

	strftime(out, MAXLEN, op.format_out, &ev.time);
	l = strlen(out);

	str_copy(out+l, op.separator, MAXLEN - l);
	l = strlen(out);

	str_copy(out+l, ev.text, MAXLEN - l);

	return out;
}

static int
is_space(char c)
{
	return c == ' '  || c == '\f' || c == '\n' ||
	       c == '\r' || c == '\t' || c == '\v;
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
			str_copy(op.format_in,  &argv[i][1], MAXLEN);
			str_copy(op.format_out, &argv[i][1], MAXLEN);
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
			str_copy(op.separator, argv[++i], MAXLEN);
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

/*
 * Copy up to n characters of the string str to dst and append '\0'.
 * Suggested by NRK.
 */
static void
str_copy(char *dst, char *src, int n)
{
	if (memccpy(dst, src, '\0', n) == NULL)
		dst[n-1] = '\0';
}

static char *
str_trim(char *t)
{
	char *s;

	for (s = &t[strlen(t)-1]; s != t && is_space(*s); *s = '\0', s--);
	for (; *t != '\0' && is_space(*t); t++);

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
