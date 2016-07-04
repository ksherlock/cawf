/*
 *	output-c - output support functions for cawf(1)
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

#include "cawf.h"

_PROTOTYPE(static char *FontHF,(char *cf, char *nf, char *pf, unsigned char **b, int bl, int *n, int t));
_PROTOTYPE(static char *PutHFstr,(unsigned char *buf, int l, char *cf, unsigned char **b, int bl, int *n, int t));


/*
 * FontHF(cf, nf, pf, b, bl, n, t) - change font in a header or footer line
 */

static char *
FontHF(cf, nf, pf, b, bl, n, t)
	char *cf;			/* current font */
	char *nf;			/* new font */
	char *pf;			/* previous font */
	unsigned char **b;		/* optional buffer address pointer */
	int bl;				/* optional limit on buffer */
	int *n;				/* optional buffer character count */
	int t;				/* type: 0 = get interpolated length
					 *	 1 = print */
{
	char f;				/* font */
	int fl;				/* font string length */
	int i;				/* temporary index */
	unsigned char *fp;		/* font string pointer */

	f = (*nf == 'P') ? *pf : *nf;
	if (*cf == f)
		return(NULL);
	switch (f) {
	case 'B':
		fl = Fstr.bl;
		fp = Fstr.b;
		break;
	case 'I':
		fl = Fstr.itl;
		fp = Fstr.it;
		break;
	case 'R':
		fl = Fstr.rl;
		fp = Fstr.r;
		break;
	default:
		return(" unknown font ");
	}
	if (b && (*n + fl) > bl)
		return(" font change makes title too long ");
	if (Fontctl) {
		if (t) {
			for (i = 0; i < fl; i++, fp++) {
				if ( ! b) {
					Charput(fp);
				} else {
					**b = *fp;
					(*b)++;
				}
			}
		}
	}
	*pf = *cf;
	*cf = f;
	*n += fl;
	return(NULL);
}


/*
 * LenprtHF(s, p, t, b, bl) - get length of print header or footer with page
 *			  number, number register, string interpolation, and
 *			  font control
 */

int
LenprtHF(s, p, t, b, bl)
	unsigned char *s;		/* header/footer string */
	int p;				/* page number */
	int t;				/* type: 0 = get interpolated length
					 *	 1 = print */
	unsigned char **b;		/* optional buffer address pointer */
	int bl;				/* optional limit on buffer */
{
	unsigned char buf[MAXLINE];	/* buffer for page number */
	char cf[1], pf[1];		/* fonts */
	unsigned char *cp, *cp1;	/* character pointers */
	char *err;			/* font change error message */
	int i, j, k, n;			/* temporary indexes */
	int len;			/* line length */
	unsigned char nm[4];		/* name buffer */
	
	if (s == NULL)
		return(0);
	if (b)
		n = 0;
	*cf = *pf = 'R';
/*
 * Process the string, interpolating % as page number and \f[BIPR]
 * for font changes.
 */
	for (cp = s, len = 0; *cp; cp++) {
		switch (*cp) {
		case '%':
			(void) sprintf((char *)buf, "%d", p);
			j = strlen((char *)buf);
			if ((err = PutHFstr(buf, j, cf, b, bl, &n, t))
			!= NULL)
			    goto hf_err;
			len += j;
			continue;
		case '\\':
			k = 0;
			cp++;
			switch (*cp) {
		/*
		 * Font change -- "\\fN"
		 */
			case 'f':
			    cp1 = cp + 1;
			    if (*cp1 == 'B' || *cp1 == 'I' || *cp1 == 'P'
			    ||  *cp1 == 'R') {
				if ((err = FontHF(cf, (char *)cp1, pf,
					   b, bl, &n, t))
				!= NULL)
				    goto hf_err;
				cp++;
				k++;
				break;
			    }
			    break;
		/*
	         * Interpolate number - "\\n(NN" or "\\nN"
		 */
			case 'n':
			    cp = Asmcode(&cp, nm);
			    if ((i = Findnum(nm, 0, 0)) < 0) {
				err = " unknown number register ";
				goto hf_err;
			    }
			    (void) sprintf((char *)buf, "%d", Numb[i].val);
			    j = strlen((char *)buf);
			    if ((err = PutHFstr(buf, j, cf, b, bl, &n, t))
			    != NULL)
				goto hf_err;
			    len += j;
			    k++;
			    break;
		/*
		 * Interpolate string - "\\*(NN" or "\\*N"
		 */
			case '*':
			    cp = Asmcode(&cp, nm);
			    cp1 = Findstr(nm, NULL, 0);
			    j = strlen((char *)cp1);
			    if ((err = PutHFstr(cp1, j, cf, b, bl, &n, t))
			    != NULL)
				goto hf_err;
			    len += j;
			    k++;
			    break;
			}
			if (k)
			    break;
			/* fall through */
		default:
			if ((err = PutHFstr(&Trtbl[(int)*cp], 1, cf, b, bl,
				   &n, t))
			!= NULL)
			    goto hf_err;
			len++;
		}
	}
/*
 * Restore the initial font, as required.
 */
	if (Fontctl && *cf != 'R') {
		if ((err = FontHF(cf, "R", pf, b, bl, &n, t)) != NULL)

hf_err:

		{
			Error(WARN, LINE, err, NULL);
			return(0);
		}
	}
	return(len);
}


/*
 * PageInRange(pg) - is page number in printable range?
 */

int
PageInRange(pg)				/* page number */
	int pg;
{
	struct pgrange *pr;

	if (!PgRange)
		return(1);
	for (pr = PgRange; pr; pr = pr->next) {
		if (pg >= pr->l && pg <= pr->u)
			return(1);
	}
	return(0);
}


/*
 * PutHFstr(buf, l, cf, b, bl, n, t) - put header/footer string
 */

static char *
PutHFstr(buf, l, cf, b, bl, n, t)
	unsigned char *buf;		/* string to put */
	int l;				/* length of string */
	char *cf;			/* current font */
	unsigned char **b;		/* optional buffer address pointer */
	int bl;				/* optional limit on buffer */
	int *n;				/* optional buffer character count */
	int t;				/* type: 0 = get interpolated length
					 *	 1 = print */
{
	unsigned char cb[4];
	int i;

	if (b) {
		if (Fontctl == 0) {
			if (*cf == 'B')
				i = 5;
			else if (*cf == 'I')
				i = 3;
			else
				i = 1;
		} else
			i = 1;
		if ((*n + i) > bl)
			return(" title too long ");
		*n += i;
	}
	if (t) {
		for (i = 0; i < l; i++) {
			if ( ! b) {
			    if (Fontctl == 0) {
				if (*cf == 'B') {
					cb[1] = cb[3] = (unsigned char)'\b';
					cb[0] = cb[2] = buf[i];
					Stringput(cb, 4);
				} else if (*cf == 'I')
					Stringput((unsigned char *)"_\b", 2);
			    }
			    Charput(&buf[i]);
			} else {
			    if (Fontctl == 0) {
				if (*cf == 'B') {
					**b = buf[i];
					(*b)++;
					**b = (unsigned char)'\b';
					(*b)++;
					**b = buf[i];
					(*b)++;
					**b = (unsigned char)'\b';
					(*b)++;
				} else if (*cf == 'I') {
					**b = (unsigned char)'_';
					(*b)++;
					**b = (unsigned char)'\b';
					(*b)++;
				}
			    }
			    **b = buf[i];
			    (*b)++;
			}
		}
	}
	return(NULL);
}


/*
 * Stringput(s) - put a string to output, subject to diversion
 */

void
Stringput(s, l)
	unsigned char *s;	/* string to put */
	int l;			/* length */
{
	if (!Divert && Pageprt)
		(void) fwrite(s, l, 1, stdout);
}
