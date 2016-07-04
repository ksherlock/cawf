/*
 * macish.c - Macintosh intialization by Matthias Ulrich Neeracher
 *	      <neeri@iis.ee.ethz.ch>
 */

#if	defined(macintosh)
#include <QuickDraw.h>

/*
 * InitToolbox() - initialize the Macintosh tool box
 */

void InitToolbox()
{
	InitGraf(&qd.thePort);
}
#endif	/* defined(macintosh) */
