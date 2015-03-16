/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: label.h,v $
*	$Revision: 1.9 $
*	$Date: 2006/10/09 16:03:21 $
*	$Author: zapreevis $
*	
*	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
*	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
*	automated verification of properties concerning long-run and instantaneous rewards
*	as well as cumulative rewards.
*	
*	Copyright (C) The University of Twente, 2004-2006.
*	Authors: Maneesh Khattri
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
*	Source description: Definition of storage for labelling.
*/

# include "macro.h"
#ifdef MAC_OS
	# include <stdlib.h>
#else
	# include <malloc.h>
#endif
# include "bitset.h"
# include "string.h"

#ifndef LABEL_H

#define LABEL_H

/*****************************************************************************
			STRUCTURE
name		: labelling
@member n	: The number of labels.
@member temp_n	: The actual number of labels in label (required when reading the .lab file).
@member ns	: The number of states.
@member label	: The ascendingly sorted list of labels.
@member b	: A list of bitsets.
remark		: There is a one-one relation between label and bitset. Each bitset
		  indicates the states in which a certain label is valid. For instance
		  bitset[0] indicates the states in which label[0] is valid.
******************************************************************************/
typedef struct labelling
{
	int n;
	int ns;
	int temp_n;
	char **label;
	bitset **b;
}labelling;

/*****************************************************************************
name		: get_new_label
role       	: get a new labelling structure.
@param     	: int n: the number of labels.
@param     	: int ns: the number of states.
@return		: labelling *: returns a pointer to a new labelling structure.
remark 	        :
******************************************************************************/
extern labelling * get_new_label(int, int);

/*****************************************************************************
name		: add_label
role       	: add a new label in ascending lexicographic order to the given
		  labelling structure.
@param     	: labelling *labellin: the labelling structure.
@param     	: char *label: the label to be added.
@return		: int: 0-fail 1-success. Fails when labellin->n number of labels
		  are already added.
remark 	        : This method also initializes the appropriate bitset with a new
		  bitset.
******************************************************************************/
extern int add_label(labelling *, char *);

/*****************************************************************************
name		: add_label_bitset
role       	: set the bitset of the given labelling structure indexed by the
		  given label to a desired value.
@param     	: labelling *labellin: the labelling structure.
@param     	: char *label: the label whose bitset is to be changed.
@param     	: bitset *b: the new bitset.
@return		: int: 0-fail 1-success.
remark 	        : Fails when the given label cannot be found in the given labelling
		  structure.
******************************************************************************/
extern int add_label_bitset(labelling *, char *, bitset *);

/*****************************************************************************
name		: set_label_bit
role       	: set the bit of the given labelling structure indexed by the given
		  label and position to 1.
@param     	: labelling *labellin: the labelling structure.
@param     	: char *label: the label whose bitset is to be changed.
@param     	: int pos: the position of the bit.
@return		: int: 0-fail 1-success.
remark 	        : Fails when the given label cannot be found in the given labelling
		  structure.
******************************************************************************/
extern int set_label_bit(labelling *, char *, int);

/*****************************************************************************
name		: get_label_bitset
role       	: get the bitset of the given labelling structure indexed by
		  the given label.
@param     	: labelling *labellin: the labelling structure.
@param     	: char *label: the label whose bitset is to be changed.
@return		: bitset *: the desired bitset.
remark 	        : Fails  and return NULL pointer when the given label cannot be
		  found in the given labelling structure.
******************************************************************************/
extern bitset * get_label_bitset(labelling *, char *);

/*****************************************************************************
name		: print_labelling
role       	: prints the given labelling structure.
@param     	: labelling *labellin: the labelling structure to be printed.
@return		: 
remark 	        :
******************************************************************************/
extern void print_labelling(labelling *);

/*****************************************************************************
name		: free_labelling
role       	: frees the given labelling structure.
@param     	: labelling *labellin: the labelling structure to be freed.
@return		: 
remark 	        :
******************************************************************************/
extern void free_labelling(labelling *);

#endif

