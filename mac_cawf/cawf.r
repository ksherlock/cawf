/*
 *    Copyright (c) 1995 Matthias Neeracher
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

#define SystemSevenOrLater 1

#include "SysTypes.r"		/* To get system types */
#include "Types.r"			/* To get general types */
#include "Cmdo.r"			/* For commando interface */

include "cawf.rsrc";

resource 'vers' (1) {
	0x04, 0x09, release, 0x00, verUS,
	"4.0.9",
	"cawf"
	};

resource 'vers' (2) {
	0x04, 0x09, release, 0x00, verUS,
	"4.0.9",
	"cawf 4.0.9 (21Nov95)"
	};

resource 'cmdo' (128) {
	{
		295,			/* Height of dialog */
		"Cawf -- - C version of the nroff-like, Amazingly Workable (text) Formatter\n"
		"by Vic Abell <abe@vic.cc.purdue.edu>",
		{
			notDependent {}, RadioButtons {
				{
					{ 80,  20,  95, 220}, "none", "", set, 
					"Manual pages.",
					{ 97,  20, 112, 220}, "man", "-man", notset, 
					"Manual pages.",
					{114,  20, 129, 220}, "me", "-me", notset, 
					"Technical papers.",
					{131,  20, 146, 220}, "ms", "-ms", notset, 
					"Articles, reports, books."
				}
			},
			notDependent {}, TextBox {
				gray,
				{ 70,  10, 185, 240},
				"Macro package"
			},
			notDependent {}, MultiFiles {
				"Input File(s)É",
				"Select input files.",
				{ 15, 150, 35, 315},
				"Input files:",
				"",
				MultiInputFiles {
					{TEXT},
					FilterTypes,
					"Only text files",
					"All files",
				}
			},
			notDependent {}, Redirection {
				StandardInput,
				{ 75, 320}
			},
			notDependent {}, Redirection {
				StandardOutput,
				{110, 320}
			},
			notDependent {}, Redirection {
				DiagnosticOutput,
				{145, 320}
			},
			notDependent {}, TextBox {
				gray,
				{ 70, 310, 185, 470},
				"Redirection"
			},
			notDependent {}, VersionDialog {
				VersionString {
					"4.0.9"
				},
				"Cawf by Vic Abell <abe@vic.cc.purdue.edu>\n"
				"MPW port by Matthias Neeracher <neeri@iis.ee.ethz.ch>\n",
				0
			},
		},
	},
};
