/*
 *	cawf - a C version of Henry Spencer's awf(1), the Amazingly
 *	       Workable (text) Formatter
 *
 *	V. Abell, Purdue University Computing Center
 */

/*
 *	Copyright (c) 1991 Purdue University Research Foundation,
 *	West Lafayette, Indiana 47907.  All rights reserved.
 *
 *	Written by Victor A. Abell <abe@cc.purdue.edu>,  Purdue	University
 *	Computing Center.  Not derived from licensed software; derived from
 *	awf(1) by Henry Spencer of the University of Toronto.
 *
 *	Permission is granted to anyone to use this software for any
 *	purpose on any computer system, and to alter it and redistribute
 *	it freely, subject to the following restrictions:
 *
 *	1. The author is not responsible for any consequences of use of
 *	   this software, even if they arise from flaws in it.
 *
 *	2. The origin of this software must not be misrepresented, either
 *	   by explicit claim or by omission.  Credits must appear in the
 *	   documentation.
 *
 *	3. Altered versions must be plainly marked as such, and must not
 *	   be misrepresented as being the original software.  Credits must
 *	   appear in the documentation.
 *
 *	4. This notice may not be removed or altered.
 */

static char Version[] = "4.10";

#include "cawf.h"

#include <time.h>

#include <sys/stat.h>

#if	!defined(UNIX)
#include <io.h>
#include <process.h>
#include <string.h>
#include <sys\types.h>
#include <sys\stat.h>
#endif	/* !defined(UNIX) */

#if	defined(macintosh)

# if	!defined(CAWFLIB)
#define	MACINTFCAWF	"Interfaces:cawf:"
# endif	/* !defined(CAWFLIB) */

extern void InitToolbox();
#endif	/* defined(macintosh) */

static char *Month[] =
	{ "January", "February", "March", "April", "May", "June", "July",
	  "August", "September", "October", "November", "December" };


main(argc, argv)
	int     argc;
	char    *argv[];
{
	char dt[32];			/* date for td and DY strings */
	char *ep;               	/* environment pointer */
	int fff = 0;			/* final form feed status */
	char *files[MAXFILES];  	/* file names */
	int help = 0;			/* help status */
	int i;	               		/* temporary index */
	size_t l;                       /* length */
	char *lib = CAWFLIB;		/* library path */
	int libl;			/* library path length */
	struct tm *lt;			/* local time structure pointer */
	int mac = 0;			/* macro specification status */
	int nf = 0;             	/* number of files */
	char *np;               	/* name pointer */
	int pc;                 	/* prolog count */
	struct stat sbuf;               /* stat buffer */
	time_t tm;			/* system time of day */
/*
 * Save program name.
 */
	if ((Pname = strrchr(argv[0], '\\')) != NULL)
		Pname++;
	else if ((Pname = strrchr(argv[0], '/')) != NULL)
		Pname++;
	else
		Pname = argv[0];

#if	defined(macintosh)
/*
 * Perform Macintosh initialization.
 */
	InitToolbox();

# if	!defined(CAWFLIB)
	if ((ep = getenv("MPW")) == NULL)
		ep = "";
	if ((CAWFLIB = (char *)malloc(strlen(ep) + strlen(MACINTFCAWF) + 1))
	== NULL) {
		(void) fprintf(stderr,
			"%s: no CAWFLIB buffer space for \"%s%s\"\n",
			Pname, ep, MACINTFCAWF);
		exit(1);
	}
	(void) sprintf(CAWFLIB, "%s%s", ep, MACINTFCAWF);
	lib = CAWFLIB;
# endif	/* !defined(CAWFLIB) */
#endif	/* defined(macintosh) */


#if	defined(__EMX__)
/*
 * Perform OS2 initialization:
 *
 *	expand response files;
 *	do file name glob'bing
 */
	_response(&argc, &argv);
	_wildcard(&argc, &argv);
#endif	/* defined(__EMX__) */

/*
 * Set error file stream pointer.
 */
	Efs = stderr;
/*
 * Get library name.
 */
	if ((np = getenv("CAWFLIB")) != NULL)
		lib = np;
	libl = strlen(lib);
/*
 * Get device file name.
 */
	for (ep = getenv("TERM");; ep = NULL) {
		if (ep == NULL || *ep == '\0')
			ep = "dumb";
		l = libl + 1 + strlen(ep) + strlen(".dev") + 1;
		if ((np = malloc(l)) == NULL)
			Error(FATAL, NOLINE,
				" no string space for device file: ", ep);
		(void) sprintf(np, "%s%s%s.dev", lib,

#if	defined(macintosh)
			"",
#else	/* !defined(macintosh) */
			"/",
#endif	/* defined(macintosh) */

			ep);
		if (stat(np, &sbuf) == 0)
			break;
		if (strcmp(ep, "dumb") == 0)
			Error(FATAL, NOLINE, " no dumb.dev file in ", lib);
		(void) free(np);
	}
	files[nf++] = np;
/*
 * Get common text file name.
 */
	l = libl + 1 + strlen("common") + 1;
	if ((np = malloc(l)) == NULL)
		Error(FATAL, NOLINE, " no string space for common file name",
			NULL);
	(void) sprintf(np, "%s%scommon", lib,

#if	defined(macintosh)
		""
#else	/* !defined(macintosh) */
		"/"
#endif	/* defined(macintosh) */

		);

	files[nf++] = np;
/*
 * Process options.
 */
	while ((i = getopt(argc, argv, "c:d:ef:hm:n:o:")) != EOF) {
		switch (i) {
	/*
	 * -c c
	 *
	 * Set the device configuration file path to c.
	 */
		case 'c':
			Devconf = optarg;
			break;
	/*
	 * -d d
	 *
	 * Set the output device name to d.
	 *
	 * The default output device name is NORMAL -- i.e., a device that
	 * does bold face with backspace and overprinting and italic face with
	 * underscore.  NORMAL is usually a terminal device.
	 *
	 * There is a built-in device, named ANSI, that does bold face with
	 * the ANSI shadow mode and italic face with the ANSI underscore mode.
	 * ANSI is normally a terminal device that supports the ANSI shadow
	 * and underscore modes.
	 *
	 * There is a built-in output device, named NONE, that does nothing
	 * at all for the bold or italic faces.  This is usually a terminal
	 * device.
	 *
	 * All other device names must match a stanza in the device
	 * configuration file.
	 */
		case 'd':
			Device = optarg;
			break;
	/*
	 * -e -- eject: issue final form feed
	 */
		case 'e':
			fff = 1;
			break;
	/*
	 * -f f
	 *
	 * Set f as the font name for the output device (from the device
	 * configuration file).
	 */
		case 'f':
			Devfont = optarg;
			break;
	/*
	 * -h -- display help (usage)
	 */
		case 'h':
			help = 1;
			break;
	/*
	 * -m m
	 *
	 * Set the macro file name to m.
	 *
	 *  Special support is provided for -man, -me and -ms.
	 */
		case 'm':
			if (mac) {
				Error(WARN, NOLINE,
					"multiple macro file declaration",
					NULL);
				break;
			}
			l = libl + 2 + strlen(optarg) + strlen(".mac") + 1;
			if ((np = malloc(l)) == NULL)
				Error(FATAL, NOLINE, " no string space for ",
					argv[1]);
			(void) sprintf(np, "%s%sm%s.mac", lib,

#if	defined(macintosh)
				"",
#else	/* !defined(macintosh) */
				"/",
#endif	/* defined(macintosh) */

				optarg);

			files[nf++] = np;
			if (strcmp(optarg, "an") == 0)
				Marg = MANMACROS;
			else if (strcmp(optarg, "e") == 0)
				Marg = MEMACROS;
			else if (strcmp(optarg, "s") == 0)
				Marg = MSMACROS;
			mac++;
			break;
	/*
	 * -n n
	 *
	 * Set the starting page number to n.
	 */
		case 'n':
			Thispg = atoi(optarg);
			break;
	/*
	 * -o o
	 *
	 * Set the pages to print list from the string o.
	 *
	 * Entries are separated by commas.  A range is specified as n-m.
	 * A starting range of the form -m has one for a starting value,
	 * and a range of the form n- has an ending value equivalent to
	 * the end of the document.  For example:
	 *
	 *	-5,8,10-
	 *
	 * specifies the printing of pages 1 through 5, 8, and 10 through
	 * the end of the document.
	 */
		case 'o':
			if (!AsmPgRange(optarg))
				Error(WARN, NOLINE, "invalid page list: ",
					optarg);
			break;
	/*
	 * Option not recognized by getopt().
	 */
		case '?':
			Err = 1;
		}
	}
	if (Defdev())
		Err++;
	if (help || Err) {
	  (void) fprintf(stderr,
	    "%s %s usage: [-c c] [-d d] [-e] [-f f] [-h] [-m m]\n",
		Pname, Version);
	  (void) fprintf(stderr, "\t[-n n] [-o o] file...\n");
	  (void) fprintf(stderr,
	    "\t-c c      c is the device configuration file path\n");
	  (void) fprintf(stderr,
	    "\t          (default = %s%s%s)\n", CAWFLIB,

#if	defined(macintosh)
	    "",
#else	/* !defined(macintosh) */
	    "/",
#endif	/* defined(macintosh) */

  	    DEVCONFIG);
	  (void) fprintf(stderr,
	    "\t-d d      d is the output device name\n");
	  (void) fprintf(stderr,
	    "\t          (default = NORMAL, using \\b for bold and italic)\n");
	  (void) fprintf(stderr,
	    "\t          (built-ins = ANSI, NONE and NORMAL)\n");
	  (void) fprintf(stderr,
	    "\t-e        issue eject after last page\n");
	  (void) fprintf(stderr,
	    "\t-f f      f is the output device font name\n");
	  (void) fprintf(stderr,
	    "\t-h        display help (this output)\n");
	  (void) fprintf(stderr,
	    "\t-m m      m is the macro file name\n");
	  (void) fprintf(stderr,
	    "\t-n n      n is the starting page number\n");
	  (void) fprintf(stderr,
	    "\t-o o      o specifies output pages and page ranges\n");
	  (void) fprintf(stderr,
	    "\tfile ...  source file names\n");
	  exit(Err);
	}
	if (mac == 0) {

	    /*
	     * No macros - enable Bold, Italic and Roman fonts.
	     */
		for (i = 0; Fcode[i].nm; i++) {
			switch (Fcode[i].nm) {
			case 'B':
			case 'I':
			case 'R':
				Fcode[i].status = '1';
			}
		}
	}
/*
 * Add user-supplied file names.
 */
	pc = nf;
	if (optind >= argc) {
		files[nf++] = NULL;       /* STDIN */
	} else {
		while (optind < argc) {
			if (nf >= MAXFILES)
				Error(WARN, NOLINE, " too many files at ",
					argv[optind]);
			files[nf++] = argv[optind++];
		}
	}
/*
 * Make sure all input files are accessible.
 */
	for (i = 0; i < nf; i++) {
		if (files[i] != NULL) {
			if (stat(files[i], &sbuf) != 0)
				Error(WARN, NOLINE, " can't find ", files[i]);
		}
	}
	if (Err)
		exit(1);
/*
 * Miscellaneous initialization.
 */

	for (i = 0; ; i++) {
		if (Pat[i].re == NULL)
			break;
		if ((Pat[i].pat = regcomp(Pat[i].re)) == NULL)
			Error(WARN, NOLINE, Pat[i].re, " regcomp failure");
	}
	if ((i = Findscale((int)'n', 0.0, 0)) < 0)
		Error(WARN, NOLINE, " can't find Scale['n']", NULL);
	Scalen = Scale[i].val;
	if ((i = Findscale((int)'u', 0.0, 0)) < 0)
		Error(WARN, NOLINE, " can't find Scale['u']", NULL);
	Scaleu = Scale[i].val;
	if ((i = Findscale((int)'v', 0.0, 0)) < 0)
		Error(WARN, NOLINE, " can't find Scale['v']", NULL);
	Scalev = Scale[i].val;
	(void) Findstr((unsigned char *)"CH", (unsigned char *)"= % -", 1);
	Cont = Newstr((unsigned char *)" ");
	Contlen = 1;
	if ((Trtbl = (unsigned char *)malloc(256)) == NULL)
		Error(WARN, NOLINE, " can't allocate translate table space",
			NULL);
	else {
		*Trtbl = ' ';
		for (i = 1; i < 256; i++)
			Trtbl[i] = (unsigned char) i;
	}
	if (Err)
		exit(1);
/*
 * Here begins pass1 of awf - reading input lines and expanding macros.
 */

/*
 * Output prolog.
 */
	if (Fstr.i)
		Stringput(Fstr.i, Fstr.il);
	Macro((unsigned char *)".^x");
	Macro((unsigned char *)".^b");
	Macro((unsigned char *)".^# 1 <prolog>");
/*
 * Put today's date in the -me td string or the -ms DY string.
 * Put the month name in the -ms MO string.
 */
	if (Marg == MEMACROS || Marg == MSMACROS) {
		tm = time(NULL);
		lt = localtime(&tm);
		(void) sprintf(dt, "%s %d, %d",
			Month[lt->tm_mon], lt->tm_mday, lt->tm_year + 1900);
		Findstr(
			(Marg == MEMACROS) ? (unsigned char *)"td"
					   : (unsigned char *)"DY",
			(unsigned char *)dt, 1);
		if (Marg == MSMACROS)
			Findstr((unsigned char *)"MO",
				(unsigned char *)Month[lt->tm_mon], 1);
	}
/*
 * Read input files.
 */
	for (i = 0; i < nf; i++) {
		if (files[i] == NULL) {
			np = "STDIN";
			Ifs = stdin;
		} else {

#if	defined(UNIX)
			if ((Ifs = fopen(files[i], "r")) == NULL)
#else	/* !defined(UNIX) */
			if ((Ifs = fopen(files[i], "rt")) == NULL)
#endif	/* defined(UNIX) */

				Error(FATAL, NOLINE, " can't open ", files[i]);
			np = files[i];
		}
		Findstr((unsigned char *)"IF", (unsigned char *)np, 1);
		if (i >= pc) {
			(void) sprintf((char *)Line, ".^# 1 %s", np);
			Macro(Line);
			NR = 0;
		}
		Fsp = 0;
		do {
			while (fgets((char *)Line, MAXLINE, Ifs) != NULL) {
				NR++;
				if ((np = strrchr((char *)Line, '\n')) != NULL)
					*np = '\0';
				else
					Line[MAXLINE-1] = '\0';
				Macro(Line);
			}
			if (i >= pc)
				Macro((unsigned char *)".^e");
			if (Ifs != stdin)
				(void) fclose(Ifs);
			if (Fsp > 0) {
				Free(&Inname);
				Inname = Inn_stk[Fsp-1];
				NR = NR_stk[Fsp-1];
				Ifs = Ifs_stk[Fsp-1];
			}
		} while (Fsp-- > 0);
	}
	Macro(NULL);
	if (fff)
		Charput("\f");
	return(Err);
}


/*
 * AsmPg(pa, pb) - assemble an ASCII page number
 */

char *
AsmPg(pa, pb)
	char *pa;			/* page number in ASCII */
	int *pb;			/* page number in binary */
{
	for (*pb = 0; *pa; pa++) {
		if (*pa < '0' || *pa > '9')
			return(pa);
		*pb = (*pb * 10) + (int)(*pa - '0');
	}
	return(pa);
}


/*
 * AsmPgRange(s) - assemble a page range
 */

int
AsmPgRange(s)
	char *s;			/* page range string */
{
	char *cp, *tbuf;
	int pg, pl, pu;
	struct pgrange *pr;

	if (s == NULL || (pl = strlen(s)) == 0)
		return(1);
/*
 * Remove spaces from the range string.
 */
	if ((tbuf = (char *)malloc(pl + 1)) == NULL)
		Error(FATAL, NOLINE, " no string space for page range", NULL);
	for (cp = tbuf; *s; *s++) {
		if (*s != ' ')
			*cp++ = *s;
	}
	*cp = '\0';
/*
 * Parse the range string.
 */
	for (cp = tbuf; *cp;) {

	/*
	 * Get the starting value.
	 * A missing initial range value defaults to one.
	 */
		if (*cp == '-')
			pg = 1;
		else
			cp = AsmPg(cp, &pg);
	/*
	 * Base further processing on the range terminator character.
	 */
		switch (*cp) {
		case ',':		/* single page number */
		case '\0':
			pl = pu = pg;
			break;
		case '-':		/* page number range */
			cp++;
			pl = pg;
		/*
		 * Assemble the terminating page number.  A missing
		 * number defaults to page 1,000,000 -- the end of
		 * the document.
		 */
			if (!*cp) {
				pu = 10000;
				break;
			}
			cp = AsmPg(cp, &pg);
			pu = pg;
			break;
		default:
			return(0);
		}
	/*
	 * Check the page numbers.
	 */
		if (pl < 0 || pu < 0 || pl > pu)
			return(0);
		if (*cp) {
			if (*cp != ',')
				return(0);
			cp++;
		}
	/*
	 * Allocate, fill in, and connect a page range structure.
	 */
		if ((pr = (struct pgrange *)malloc(sizeof(struct pgrange)))
		== NULL) {
			Error(FATAL, NOLINE, " no space for page range entry",
				NULL);
		}
		pr->l = pl;
		pr->u = pu;
		pr->next = PgRange;
		PgRange = pr;
	}
	return(1);
}


/*
 * Macro(inp) - process a possible macro statement
 *		pass non-macros and macros alike to pass 2
 */

void
Macro(inp)
	unsigned char *inp;		/* possible macro statement pointer */
{
	unsigned char c[2];		/* characters */
	int endm;			/* end of macro status */
	FILE *fs;			/* temporary file stream */
	int req;			/* request character status */
	unsigned char *s1, *s2;		/* temporary string pointers */

	if (inp == NULL) {
		Pass2(NULL);
		return;
	}
	req = (*inp == '.' || *inp == '\'') ? 1 : 0;
/*
 * Check for file name designator.
 */
	if (req && inp[1] == '^' && inp[2] == '#') {
		Free(&Inname);
		Inname = Field(3, inp, 1);
		F = NULL;
		Pass2(inp);
		return;
	}
/*
 * Check for source command - "^[.']so".
 */
	if (req && inp[1] == 's' && inp[2] == 'o') {
		if ((s1 = Field(2, inp, 1)) == NULL) {
			Error(WARN, LINE, " no file specified", NULL);
			return;
		}
		if ((fs = fopen((char *)s1, "r")) == NULL) {
			Error(WARN, LINE, " can't open", NULL);
			return;
		}
		if (Fsp >= MAXFSTK) {
			(void) fclose(fs);
			Error(WARN, LINE, " nesting too deep", NULL);
			return;
		}
		Ifs_stk[Fsp] = Ifs;
		Ifs = fs;
		Inn_stk[Fsp] = Inname;
		Inname = F;
		F = NULL;
		NR_stk[Fsp++] = NR;
		NR = 0;
		return;
	}
 /*
  * Check for start of macro definition.
  */
	if (req && inp[1] == 'd' && inp[2] == 'e') {
		if (inp[3] != ' ' || inp[4] == '\0') {
			Error(WARN, LINE, " illegal macro definition", NULL);
			return;
		}
		c[0] = inp[4];
		c[1] = inp[5];
		Curmx = Findmacro(c, 1);
		return;
	}
/*
 * Check for macro text.  Remove double backslashes.
 */
	if (req && (inp[1] == '\0' || (inp[2] == '\0' && inp[0] == inp[1])))
		endm = 1;
	else
		endm = 0;
	if (Curmx >= 0 && !endm) {
		if (Mtx >= MAXMTXT)
			Error(FATAL, LINE, " out of macro text space", NULL);
		if ((s1 = (unsigned char *)strchr((char *)inp, '\\')) == NULL)
			Macrotxt[Mtx] = Newstr(inp);
		else {
			for (s1 = Pass1ln, s2 = inp;; s1++) {
				if ((*s1 = *s2++) == '\0')
					break;
				if (*s1 == '\\' && *s2 == '\\')
					s2++;
			}
			Macrotxt[Mtx] = Newstr(Pass1ln);
		}
		if (Macrotab[Curmx].bx == -1)
			Macrotab[Curmx].bx = Mtx;
		Mtx++;
		Macrotab[Curmx].ct++;
		return;
	}
/*
 * Check for end of macro.
 */
	if (Curmx >= 0 && endm) {
		Curmx = -1;
		(void) sprintf((char *)Pass1ln, ".^# %d %s", NR, Inname);
		Pass2(Pass1ln);
		return;
	}
 /*
  * Check for conditionals and macro expansions.
  */
	if (req
	&&  (Findmacro(inp+1, 0) != -1 || regexec(Pat[0].pat, inp))) {
		Expand(inp);
		return;
	}
/*
 * None of the above: forward the line.
 */
	Pass2(inp);
}
