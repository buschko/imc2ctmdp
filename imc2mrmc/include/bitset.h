/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: bitset.h,v $
*	$Revision: 1.20 $
*	$Date: 2006/10/11 14:28:43 $
*	$Author: zapreevis $
*	
*	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
*	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
*	automated verification of properties concerning long-run and instantaneous rewards
*	as well as cumulative rewards.
*	
*	Copyright (C) The University of Twente, 2004-2006.
*	Authors: Maneesh Khattri, Ivan Zapreev
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
*	Source description: Define a Bitset-For the satisfaction relation in MCC.
*	Uses: Definition of bitset - bitset.h
*/

# include "macro.h"
#ifdef MAC_OS
	# include <stdlib.h>
#else
	# include <malloc.h>
#endif
# include <math.h>
# include "macro.h"

#ifndef BITSET_H

#define BITSET_H

//This is the size of the bitset block
#define BITSET_BLOCK_SIZE 32

/**
* The simple bit structure
* @member b: One Bit.
*/
typedef struct bit
{
	unsigned b: 1;
}bit;

/**
*This are the constant bits
*/
static const bit BIT_ON = {1}, BIT_OFF = {0};

/**
* The bitset is an array of bytes, this is the structure that defines one byte
* NOTE:
* 1. This has no relation to the structure 'bit'.
* 2. This structure has BITSET_BLOCK_SIZE bits although it is named bytes.
* @member bits	: A Unit of BITSET_BLOCK_SIZE bits.
*/
typedef struct bytes
{
	unsigned bits : BITSET_BLOCK_SIZE;
}bytes;

/**
* The bitset structure
* @member n			: The number of valid bits in this bitset.
* @member blocks_num	: The number of elements (bytes) int the bytesp array
* @member bytesp		: A pointer to an array of structure 'bytes'.
*/
typedef struct bitset
{
	int n;
	int blocks_num;
	struct bytes *bytesp;
}bitset;

/*****************************************************************************
name		: get_new_bitset
role		: get a new bitset.
@param		: int n: indicates the size of the required bitset.
@return	: bitset *: returns a pointer to a new bitset.
remark		: A bitset is composed of an array of BITSET_BLOCK_SIZE-bit unsigned integers.
******************************************************************************/
extern bitset * get_new_bitset(int);

/*****************************************************************************
name		: or_result
role       	: obtain the disjunction of two bitsets and put result in arg2.
@param     	: bitset *a: One of the operands to the disjunction operation.
@param		: bitset *b: The second operand to the disjunction operation.
******************************************************************************/
extern void or_result(bitset *a, bitset *result);

/*****************************************************************************
name		: copy_bitset
role       	: This method just copies elements from one bitset into enother.
		: The two bitsets must have the same size.
@param     	: bitset *from_bitset: the bitset to be copied .
@param		: bitset *to_bitset: the bitset to copy to.
******************************************************************************/
void copy_bitset(bitset *from_bitset, bitset *to_bitset);

/*****************************************************************************
name		: xor_result
role       	: obtain the xor of two bitsets and put result in arg2.
@param     	: bitset *a: One of the operands to the x-or operation.
@param		: bitset *b: The second operand to the x-or operation.
******************************************************************************/
extern void xor_result(bitset *a, bitset *result);

/*****************************************************************************
name		: and_result
role       	: obtain the conjunction of two bitsets and put result in arg2.
@param     	: bitset *a: One of the operands to the conjunction operation.
@param		: bitset *b: The second operand to the conjunction operation.
******************************************************************************/
extern void and_result(bitset *a, bitset *result);

/*****************************************************************************
name		: not_result
role       	: obtain the conjunction of two bitsets and put result in arg2.
@param     	: bitset *result: One of the operands to the complement operation.
@return	: the result pointer, for convenience
******************************************************************************/
extern bitset *not_result(bitset *result);

/*****************************************************************************
name		: get_bit_val
role       	: obtain the value of a specified bit of a bitset.
@param     	: bitset *a: One of the bits of this bitset is of interest.
@param		: int i: The position of the bit of interest in the bitset.
@param		: bit * (result by reference): The bit of interest is returned
		    	in this parameter.
@return		: int: indicates the success/failure of the get operation.
			 -1: invalid i, 1: success
remark 	        :
******************************************************************************/
extern int get_bit_val(const bitset *, const int i, bit *);

/*****************************************************************************
name		: set_bit_val
role       	: set the value of a specified bit of a bitset.
@param     	: bitset *a: One of the bits of this bitset is to be changed.
@param		: int i: The position of the bit of interest in the bitset.
@param		: bit * val: The new value for the bit of interest.
remark 	        :
******************************************************************************/
extern void set_bit_val(bitset *, int i, const bit *);

/*****************************************************************************
name		: print_bitset_states
role       	: used to print the position of the bits
		 in the given bitset whose value is 1.
@param     	: bitset *a: The bitset to be printed.
@return		: 
remark 	        :
******************************************************************************/
extern void print_bitset_states(bitset *);

/*****************************************************************************
name		: free_bitset
role       	: used to free the given bitset.
@param     	: bitset *a: The bitset to be freed.
@return		: 
remark 	        :
******************************************************************************/
extern void free_bitset(bitset *);

/*****************************************************************************
name		: fill_bitset_one
role       	: fills the given bitset with one.
@param     	: bitset *a: The bitset to be filled.
@return		: bitset *: returns a pointer to the result of the filling operation.
remark 	        :
******************************************************************************/
extern void fill_bitset_one(bitset *);

/*****************************************************************************
name		: is_bitset_zero
role       	: checks if the bitset contains only zeros.
@param     	: bitset *a: The bitset to be checked.
@return		: 0: bitset contains a non-zero, 1: bitset contains only zeros.
******************************************************************************/
extern int is_bitset_zero(bitset *);

/**
* Get the Index of the next non-zero element.
* @param	: bitset *a: The bitset to be checked.
* @param	: int idx: The Present index.
*		NOTE: If set to -1 then it is the first time call
*		for this method on the given bitset i.e. we have
*		to start searching from the very beginning
* @return	: next index.
*/
extern int get_idx_next_non_zero( const bitset *, const int);

/*****************************************************************************
name		: count_non_zero
role       	: count the number of non-zero elements in the given bitset.
@param     	: bitset *: The bitset to be checked.
@return		: the count.
******************************************************************************/
extern int count_non_zero(const bitset *);

/**
* Count the number of valid bits in a bitset.
* put all the valid-bit ids in an int-array.
* the first element of the array is the count.
* the rest are the bit ids of valid (set) bits.
* @param	: bitset *toCount: the bitset in question
* @return	: the count of bits that are set and their ids.
*/
extern int * count_set(bitset *toCount);

#endif
