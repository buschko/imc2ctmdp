/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: macro.h,v $
*	$Revision: 1.11 $
*	$Date: 2006/07/27 11:20:16 $
*	$Author: zapreevis $
*	
*	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
*	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
*	automated verification of properties concerning long-run and instantaneous rewards
*	as well as cumulative rewards.
*	
*	Copyright (C) The University of Twente, 2004-2006.
*	Authors: Ivan Zapreev
*
*	This program is free software; you can redistribute it and/or
*	modify it under the terms of the GNU General Public License
*	as published by the Free Software Foundation; either version 2
*	of the License, or (at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program; if not, write to the Free Software
*	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
*	Our contacts:
*		Formal Methods and Tools Group, University of Twente,
*		P.O. Box 217, 7500 AE Enschede, The Netherlands,
*		Phone: +31 53 4893767, Fax: +31 53 4893247,
*		E-mail: mrmc@cs.utwente.nl
*
*	Source description: This file contains main macro definitions.
*/

#include <stdlib.h>

#ifndef MACRO_H

#define MACRO_H

	//Set it to zerro to disable printing the resulting states and probabilities
	#define TOPRINT 1

	#ifndef BOOL
		#define BOOL short
	#endif
	
	#ifndef TRUE
		#define TRUE 1
	#endif
	
	#ifndef FALSE
		#define FALSE 0
	#endif
	
	#ifndef NULL
		#define NULL 0
	#endif
	
	#ifndef MIN
		#define MIN(a,b) (a<b?a:b)
	#endif

	//#define MAC_OS 0
	
	//This macro definitions are used for working with bit flags
	#define set_flag(flags,bit) (flags = flags | bit)
	#define clear_flag(flags,bit) (flags = flags & ~bit)
	#define test_flag(flags,bit) (flags & bit)

#endif

