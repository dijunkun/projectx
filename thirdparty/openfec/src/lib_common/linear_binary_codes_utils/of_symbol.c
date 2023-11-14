/* $Id: of_symbol.c 206 2014-12-10 04:47:09Z roca $ */
/*
 * OpenFEC.org AL-FEC Library.
 * (c) Copyright 2009 - 2012 INRIA - All rights reserved
 * Contact: vincent.roca@inria.fr
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software.  You can  use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and,  more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#include "of_linear_binary_code.h"


#ifdef OF_USE_LINEAR_BINARY_CODES_UTILS


#ifndef OF_DEBUG
void	of_add_from_multiple_symbols	(void		*to,
					 const void	**from,
					 _UINT32		from_size,
					 _UINT32		symbol_size)
#else
void	of_add_from_multiple_symbols	(void		*to,
					 const void	**from,
					 _UINT32		from_size,
					 _UINT32		symbol_size,
					 _UINT32		*op)
#endif
{
	OF_ENTER_FUNCTION
#ifdef OF_DEBUG
	if (op != NULL)
		(*op)+=from_size;
#endif
	_UINT32	i;
//#ifndef ASSEMBLY_SSE_OPT /* { */
	_UINT32		symbolSize32;
	_UINT32		symbolSize32rem;
	//printf("%i\n",from_size);
	
	symbolSize32	= symbol_size >> 2;
	symbolSize32rem = symbol_size % 4;	// Remaining bytes when the symbol
	// size is not multiple of 32 bits.
	
#if defined (__LP64__) || (__WORDSIZE == 64) // {
	/*
	 * 64-bit machines
	 */
	
	_UINT32		symbolSize64;		// Size of symbols in 64-bit unit
	// symbol_size is not necessarily a multiple of 8, but >> 3 will divide
	// it by 8 and keep the integral part automatically.
	symbolSize64	= symbol_size >> 3;
	while (from_size >=8)
	{
		 _UINT64* t = (_UINT64*) to;	// to pointer to 64-bit integers
		 _UINT64* ps1 = (_UINT64*)from[0];
		 _UINT64* ps2 = (_UINT64*)from[1];
		 _UINT64* ps3 = (_UINT64*)from[2];
		 _UINT64* ps4 = (_UINT64*)from[3];
		 _UINT64* ps5 = (_UINT64*)from[4];
		 _UINT64* ps6 = (_UINT64*)from[5];
		 _UINT64* ps7 = (_UINT64*)from[6];
		 _UINT64* ps8 = (_UINT64*)from[7];
		from_size-=8;
		from+=8;
		for (i = symbolSize64; i > 0; i--)
		{
			*t ^= (*ps1 ^ *ps2 ^ *ps3 ^ *ps4 ^ *ps5 ^ *ps6 ^ *ps7 ^ *ps8);
			t++;
			ps1++;	
			ps2++;
			ps3++;
			ps4++;
			ps5++;
			ps6++;
			ps7++;
			ps8++;		
		}
		_UINT32* t32 = (_UINT32*) t;	// to pointer to 32-bit integers
		_UINT32* ps1_32 = (_UINT32*) ps1;	// from pointer to 32-bit integers
		_UINT32* ps2_32 = (_UINT32*) ps2;	// from pointer to 32-bit integers
		_UINT32* ps3_32 = (_UINT32*) ps3;	// from pointer to 32-bit integers
		_UINT32* ps4_32 = (_UINT32*) ps4;	// from pointer to 32-bit integers
		_UINT32* ps5_32 = (_UINT32*) ps5;	// from pointer to 32-bit integers
		_UINT32* ps6_32 = (_UINT32*) ps6;	// from pointer to 32-bit integers
		_UINT32* ps7_32 = (_UINT32*) ps7;	// from pointer to 32-bit integers
		_UINT32* ps8_32 = (_UINT32*) ps8;	// from pointer to 32-bit integers
		
		/* then perform a 32-bit XOR if needed... */
		if ( (symbolSize64 << 1) < symbolSize32)
		{
			* (_UINT32*) t32 ^= (* (_UINT32*) ps1_32 ^ * (_UINT32*) ps2_32 ^ * (_UINT32*) ps3_32 ^ * (_UINT32*) ps4_32 ^ 
								* (_UINT32*) ps5_32 ^ * (_UINT32*) ps6_32 ^ * (_UINT32*) ps7_32 ^ * (_UINT32*) ps8_32);
			ps1_32++;
			ps2_32++;
			ps3_32++;
			ps4_32++;
			ps5_32++;
			ps6_32++;
			ps7_32++;
			ps8_32++;
			t32++;
		}
		if (symbolSize32rem > 0)
		{
			for (i = 0; i < symbolSize32rem; i++)
			{
				* (_UINT8*) ( (_UINT8*) t32 + i) ^= ((* (_UINT8*) ( (_UINT8*) ps1_32 + i)) ^ (* (_UINT8*) ( (_UINT8*) ps2_32 + i) ) ^ 
								   (* (_UINT8*) ( (_UINT8*) ps3_32 + i) ) ^ (* (_UINT8*) ( (_UINT8*) ps4_32 + i) ) ^ 
								   (* (_UINT8*) ( (_UINT8*) ps5_32 + i) ) ^ (* (_UINT8*) ( (_UINT8*) ps6_32 + i) ) ^ 
								   (* (_UINT8*) ( (_UINT8*) ps7_32 + i) ) ^ (* (_UINT8*) ( (_UINT8*) ps8_32 + i) ) );
			}
		}
	}
	while (from_size >=4)
	{
		_UINT64* t = (_UINT64*) to;	// to pointer to 64-bit integers
		_UINT64* ps1 = (_UINT64*)from[0];
		_UINT64* ps2 = (_UINT64*)from[1];
		_UINT64* ps3 = (_UINT64*)from[2];
		_UINT64* ps4 = (_UINT64*)from[3];
		from_size-=4;
		from+=4;
		for (i = symbolSize64; i > 0; i--)
		{
			*t ^= (*ps1 ^ *ps2 ^ *ps3 ^ *ps4);
			t++;
			ps1++;	
			ps2++;
			ps3++;
			ps4++;		
		}
		_UINT32* t32 = (_UINT32*) t;	// to pointer to 32-bit integers
		_UINT32* ps1_32 = (_UINT32*) ps1;	// from pointer to 32-bit integers
		_UINT32* ps2_32 = (_UINT32*) ps2;	// from pointer to 32-bit integers
		_UINT32* ps3_32 = (_UINT32*) ps3;	// from pointer to 32-bit integers
		_UINT32* ps4_32 = (_UINT32*) ps4;	// from pointer to 32-bit integers
		
		/* then perform a 32-bit XOR if needed... */
		if ( (symbolSize64 << 1) < symbolSize32)
		{
			* (_UINT32*) t32 ^= (* (_UINT32*) ps1_32 ^ * (_UINT32*) ps2_32 ^ * (_UINT32*) ps3_32 ^ * (_UINT32*) ps4_32 );
			ps1_32++;
			ps2_32++;
			ps3_32++;
			ps4_32++;
			t32++;
		}
		if (symbolSize32rem > 0)
		{
			for (i = 0; i < symbolSize32rem; i++)
			{
				* (_UINT8*) ( (_UINT8*) t32 + i) ^= ((* (_UINT8*) ( (_UINT8*) ps1_32 + i)) ^ (* (_UINT8*) ( (_UINT8*) ps2_32 + i) ) ^ 
								   (* (_UINT8*) ( (_UINT8*) ps3_32 + i) ) ^ (* (_UINT8*) ( (_UINT8*) ps4_32 + i) ) );
			}
		}
	}
	while (from_size >=2)
	{
		_UINT64* t = (_UINT64*) to;	// to pointer to 64-bit integers
		_UINT64* ps1 = (_UINT64*)from[0];
		_UINT64* ps2 = (_UINT64*)from[1];
		from+=2;
		from_size-=2;
		for (i = symbolSize64; i > 0; i--)
		{
			*t ^= (*ps1 ^ *ps2);
			t++;
			ps1++;	
			ps2++;
		}
		_UINT32* t32 = (_UINT32*) t;	// to pointer to 32-bit integers
		_UINT32* ps1_32 = (_UINT32*) ps1;	// from pointer to 32-bit integers
		_UINT32* ps2_32 = (_UINT32*) ps2;	// from pointer to 32-bit integers
		/* then perform a 32-bit XOR if needed... */
		if ( (symbolSize64 << 1) < symbolSize32)
		{
			* (_UINT32*) t32 ^= (* (_UINT32*) ps1_32 ^ * (_UINT32*) ps2_32);
			ps1_32++;
			ps2_32++;
			t32++;
		}
		if (symbolSize32rem > 0)
		{
			for (i = 0; i < symbolSize32rem; i++)
			{
				* (_UINT8*) ( (_UINT8*) t32 + i) ^= ((* (_UINT8*) ( (_UINT8*) ps1_32 + i)) ^ (* (_UINT8*) ( (_UINT8*) ps2_32 + i) ));
			}
		}
	}
	
	if (from_size == 0) return;
	
   	_UINT64	*t = (_UINT64*) to;	// to pointer to 64-bit integers
	_UINT64	*f = (_UINT64*) from[0];	// from pointer to 64-bit integers
	for (i = symbolSize64; i > 0; i--)
	{
		*t ^= *f;
		t++;
		f++;
	}
	_UINT32	*t32 = (_UINT32*) t;	// to pointer to 32-bit integers
	_UINT32	*f32 = (_UINT32*) f;	// from pointer to 32-bit integers
	/* then perform a 32-bit XOR if needed... */
	if ( (symbolSize64 << 1) < symbolSize32)
	{
		* (_UINT32*) t32 ^= * (_UINT32*) f32;
		t32++;
		f32++;
	}
	/* finally perform as many 8-bit XORs as needed if symbol size is not
	* multiple of 32 bits... */
	if (symbolSize32rem > 0)
	{
		for (i = 0; i < symbolSize32rem; i++)
		{
			* (_UINT8*) ( (_UINT8*) t32 + i) ^= * (_UINT8*) ( (_UINT8*) f32 + i);
		}
	}
												   
#else //defined (__LP64__) || (__WORDSIZE == 64) } {
	
	/*
	 * 32-bit machines
	 */
	_UINT32		*t32 = (_UINT32*) to;	// to pointer to 32-bit integers
	_UINT32		*f32 = (_UINT32*)  from;	// from pointer	to 32-bit integers
	/* First perform as many 32-bit XORs as needed... */
	for (i = symbolSize32; i > 0; i--)
	{
		*t32 ^= *f32;
		t32++;
		f32++;
	}
	/* finally perform as many 8-bit XORs as needed if symbol size is not
	 * multiple of 32 bits... */
	if (symbolSize32rem > 0)
	{
		for (i = 0; i < symbolSize32rem; i++)
		{
			* (_UINT8*) ( (_UINT8*) t32 + i) ^= * (_UINT8*) ( (_UINT8*) f32 + i);
		}
	}
#endif //defined (__LP64__) || (__WORDSIZE == 64) }
	
//#else  /* } ASSEMBLY_SSE_OPT { */

//#endif
}


#ifndef OF_DEBUG
void	of_add_to_multiple_symbols	(void		**to,
					 const void	*from,
					 _UINT32		to_size,
					 _UINT32		symbol_size)
#else
void	of_add_to_multiple_symbols	(void		**to,
					 const void	*from,
					 _UINT32		to_size,
					 _UINT32		symbol_size,
					 _UINT32		*op)
#endif
{
	OF_ENTER_FUNCTION
#ifdef OF_DEBUG
	if (op != NULL)
		(*op) += to_size;
#endif
	_UINT32	i;
	//#ifndef ASSEMBLY_SSE_OPT /* { */
	_UINT32		symbolSize32;
	_UINT32		symbolSize32rem;
	
	symbolSize32	= symbol_size >> 2;
	symbolSize32rem = symbol_size % 4;	// Remaining bytes when the symbol
	// size is not multiple of 32 bits.
#if defined (__LP64__) || (__WORDSIZE == 64) // {
	/*
	 * 64-bit machines
	 */
	_UINT32		symbolSize64;		// Size of symbols in 64-bit unit
	// symbol_size is not necessarily a multiple of 8, but >> 3 will divide
	// it by 8 and keep the integral part automatically.
	symbolSize64	= symbol_size >> 3;
	_UINT64		*pt1, *pt2, *pt3, *pt4, *pt5, *pt6, *pt7, *pt8, *from_s, from_value;

	while (to_size >= 8)
	{
		from_s = (_UINT64*) from;	// to pointer to 64-bit integers
		pt1 = (_UINT64*)to[0];
		pt2 = (_UINT64*)to[1];
		pt3 = (_UINT64*)to[2];
		pt4 = (_UINT64*)to[3];
		pt5 = (_UINT64*)to[4];
		pt6 = (_UINT64*)to[5];
		pt7 = (_UINT64*)to[6];
		pt8 = (_UINT64*)to[7];
		to += 8;
		to_size -= 8;
		for (i = symbolSize64; i > 0; i--)
		{
			from_value = *from_s;
			*pt1 ^= from_value;
			*pt2 ^= from_value ;
			*pt3 ^= from_value ;
			*pt4 ^= from_value ;
			*pt5 ^= from_value ;
			*pt6 ^= from_value ;
			*pt7 ^= from_value ;
			*pt8 ^= from_value ;
			from_s++;
			pt1++;	
			pt2++;
			pt3++;
			pt4++;
			pt5++;	
			pt6++;
			pt7++;
			pt8++;			
		}
		_UINT32* from_s32 = (_UINT32*) from_s;	// to pointer to 32-bit integers
                /* then perform a 32-bit XOR if needed... */
		from_s32 = (_UINT32*) from_s;    // pointer to 32-bit integers
		if ( (symbolSize64 << 1) < symbolSize32)
		{               
			(* (_UINT32*) pt1++) ^= *from_s32;
			(* (_UINT32*) pt2++) ^= *from_s32;
			(* (_UINT32*) pt3++) ^= *from_s32;
			(* (_UINT32*) pt4++) ^= *from_s32;
			(* (_UINT32*) pt5++) ^= *from_s32;
			(* (_UINT32*) pt6++) ^= *from_s32;
			(* (_UINT32*) pt7++) ^= *from_s32;
			(* (_UINT32*) pt8++) ^= *from_s32;
			from_s32++;
		}
		if (symbolSize32rem > 0)
		{
			_UINT8* s8;
			for (i = 0; i < symbolSize32rem; i++)
			{
				s8  = ( (_UINT8*) from_s32 + i);
				(* (_UINT8*) ( (_UINT8*) pt1 + i)) ^= * (_UINT8*) s8;
				(* (_UINT8*) ( (_UINT8*) pt2 + i)) ^= * (_UINT8*) s8;
				(* (_UINT8*) ( (_UINT8*) pt3 + i)) ^= * (_UINT8*) s8;
				(* (_UINT8*) ( (_UINT8*) pt4 + i)) ^= * (_UINT8*) s8;
				(* (_UINT8*) ( (_UINT8*) pt5 + i)) ^= * (_UINT8*) s8;
				(* (_UINT8*) ( (_UINT8*) pt6 + i)) ^= * (_UINT8*) s8;
				(* (_UINT8*) ( (_UINT8*) pt7 + i)) ^= * (_UINT8*) s8;
				(* (_UINT8*) ( (_UINT8*) pt8 + i)) ^= * (_UINT8*) s8;	 
			}
		}
	}
	while (to_size >= 4)
	{
		_UINT32	*from_s32;		// to pointer to 32-bit integers

		from_s = (_UINT64*) from;	// to pointer to 64-bit integers
		pt1 = (_UINT64*)to[0];
		pt2 = (_UINT64*)to[1];
		pt3 = (_UINT64*)to[2];
		pt4 = (_UINT64*)to[3];
		to += 4;
		to_size -= 4;
		for (i = symbolSize64; i > 0; i--)
		{
			from_value = *from_s;
			*pt1 ^= from_value;
			*pt2 ^= from_value;
			*pt3 ^= from_value;
			*pt4 ^= from_value;
			from_s++;
			pt1++;	
			pt2++;
			pt3++;
			pt4++;		
		}
		/* then perform a 32-bit XOR if needed... */ 
		from_s32 = (_UINT32*) from_s;
		if ( (symbolSize64 << 1) < symbolSize32) 
		{ 
			(* (_UINT32*) pt1++) ^= *from_s32; 
			(* (_UINT32*) pt2++) ^= *from_s32; 
			(* (_UINT32*) pt3++) ^= *from_s32; 
			(* (_UINT32*) pt4++) ^= *from_s32;                        
			from_s32++; 
		} 
		if (symbolSize32rem > 0)
		{
			_UINT8	*s8;
			for (i = 0; i < symbolSize32rem; i++)
			{
				s8  = ( (_UINT8*) from_s32 + i);
				(* (_UINT8*) ( (_UINT8*) pt1 + i)) ^= * (_UINT8*) s8;
				(* (_UINT8*) ( (_UINT8*) pt2 + i)) ^= * (_UINT8*) s8;
				(* (_UINT8*) ( (_UINT8*) pt3 + i)) ^= * (_UINT8*) s8;
				(* (_UINT8*) ( (_UINT8*) pt4 + i)) ^= * (_UINT8*) s8;
			}
		}
	}
	while (to_size >= 2)
	{
		_UINT32	*from_s32;			// to pointer to 32-bit integers

		from_s = (_UINT64*) from;	// to pointer to 64-bit integers
		pt1 = (_UINT64*)to[0];
		pt2 = (_UINT64*)to[1];

		to += 2;
		to_size -= 2;
		for (i = symbolSize64; i > 0; i--)
		{
			*pt1 ^= *from_s ;
			*pt2 ^= *from_s ;
			from_s++;
			pt1++;	
			pt2++;
		}
                /* then perform a 32-bit XOR if needed... */
                from_s32 = (_UINT32*) from_s;    // pointer to 32-bit integers
                if ( (symbolSize64 << 1) < symbolSize32) 
                { 
                        (* (_UINT32*) pt1++) ^= *from_s32; 
                        (* (_UINT32*) pt2++) ^= *from_s32; 
                        from_s32++; 
                }
		if (symbolSize32rem > 0)
		{
			_UINT8	*s8;
			for (i = 0; i < symbolSize32rem; i++)
			{
				s8  = ( (_UINT8*) from_s32 + i);
				(* (_UINT8*) ( (_UINT8*) pt1 + i)) ^= * (_UINT8*) s8;
				(* (_UINT8*) ( (_UINT8*) pt2 + i)) ^= * (_UINT8*) s8;
			}
		}
	}
	
	if (to_size == 0) return;
	
   	_UINT64	*t = (_UINT64*) to[0];	// to pointer to 64-bit integers
	_UINT64	*f = (_UINT64*) from;	// from pointer to 64-bit integers
	for (i = symbolSize64; i > 0; i--)
	{
		*t ^= *f;
		t++;
		f++;
	}
	_UINT32* t32 = (_UINT32*) t;	// to pointer to 32-bit integers
	_UINT32		*f32 = (_UINT32*) f;	// from pointer to 32-bit integers
	/* then perform a 32-bit XOR if needed... */
	if ( (symbolSize64 << 1) < symbolSize32)
	{
		* (_UINT32*) t32 ^= * (_UINT32*) f32;
		t32++;
		f32++;
	}
	/* finally perform as many 8-bit XORs as needed if symbol size is not
	 * multiple of 32 bits... */
	if (symbolSize32rem > 0)
	{
		for (i = 0; i < symbolSize32rem; i++)
		{
			* (_UINT8*) ( (_UINT8*) t32 + i) ^= * (_UINT8*) ( (_UINT8*) f32 + i);
		}
	}

#else //defined (__LP64__) || (__WORDSIZE == 64) } {
	
	/*
	 * 32-bit machines
	 */
	_UINT32		*t32 = (_UINT32*) to;	// to pointer to 32-bit integers
	_UINT32		*f32 = (_UINT32*)  from;	// from pointer	to 32-bit integers
	/* First perform as many 32-bit XORs as needed... */
	for (i = symbolSize32; i > 0; i--)
	{
		*t32 ^= *f32;
		t32++;
		f32++;
	}
	/* finally perform as many 8-bit XORs as needed if symbol size is not
	 * multiple of 32 bits... */
	if (symbolSize32rem > 0)
	{
		for (i = 0; i < symbolSize32rem; i++)
		{
			* (_UINT8*) ( (_UINT8*) t32 + i) ^= * (_UINT8*) ( (_UINT8*) f32 + i);
		}
	}
#endif //defined (__LP64__) || (__WORDSIZE == 64) }
	
	//#else  /* } ASSEMBLY_SSE_OPT { */
	
	//#endif /* } ASSEMBLY_SSE_OPT */
}
	
	

#ifdef OF_DEBUG
void	of_add_to_symbol (void		*to,
			  const void	*from,
			  _UINT32	symbol_size,
			  _UINT32	*op)
#else
void	of_add_to_symbol (void		*to,
			  const void	*from,
			  _UINT32	symbol_size)
#endif
{
	//OF_ENTER_FUNCTION
	_UINT32		i;
#ifdef OF_DEBUG
	if (op != NULL)
		(*op)++;
#endif
#ifndef ASSEMBLY_SSE_OPT /* { */
	_UINT32		symbolSize32;
	_UINT32		symbolSize32rem;

	symbolSize32	= symbol_size >> 2;
	symbolSize32rem = symbol_size % 4;	// Remaining bytes when the symbol
						// size is not multiple of 32 bits.

#if defined (__LP64__) || (__WORDSIZE == 64) // {
	/*
	 * 64-bit machines
	 */

	_UINT32		symbolSize64;		// Size of symbols in 64-bit unit
	// symbol_size is not necessarily a multiple of 8, but >> 3 will divide
	// it by 8 and keep the integral part automatically.
	symbolSize64	= symbol_size >> 3;
	/* First perform as many 64-bit XORs as needed... */
	_UINT64		*t = (_UINT64*) to;	// to pointer to 64-bit integers
	_UINT64		*f = (_UINT64*) from;	// from pointer to 64-bit integers
	for (i = symbolSize64; i > 0; i--)
	{
		*t ^= *f;
		t++;
		f++;
	}
	_UINT32		*t32 = (_UINT32*) t;	// to pointer to 32-bit integers
	_UINT32		*f32 = (_UINT32*) f;	// from pointer to 32-bit integers
	/* then perform a 32-bit XOR if needed... */
	if ( (symbolSize64 << 1) < symbolSize32)
	{
		* (_UINT32*) t32 ^= * (_UINT32*) f32;
		t32++;
		f32++;
	}
	/* finally perform as many 8-bit XORs as needed if symbol size is not
	 * multiple of 32 bits... */
	if (symbolSize32rem > 0)
	{
		for (i = 0; i < symbolSize32rem; i++)
		{
			* (_UINT8*) ( (_UINT8*) t32 + i) ^= * (_UINT8*) ( (_UINT8*) f32 + i);
		}
	}

#else //defined (__LP64__) || (__WORDSIZE == 64) } {

	/*
	 * 32-bit machines
	 */
	_UINT32		*t32 = (_UINT32*) to;	// to pointer to 32-bit integers
	_UINT32		*f32 = (_UINT32*) from;	// from pointer	to 32-bit integers
	/* First perform as many 32-bit XORs as needed... */
	for (i = symbolSize32; i > 0; i--)
	{
		*t32 ^= *f32;
		t32++;
		f32++;
	}
	/* finally perform as many 8-bit XORs as needed if symbol size is not
	* multiple of 32 bits... */
	if (symbolSize32rem > 0)
	{
		for (i = 0; i < symbolSize32rem; i++)
		{
			* (_UINT8*) ( (_UINT8*) t32 + i) ^= * (_UINT8*) ( (_UINT8*) f32 + i);
		}
	}
#endif //defined (__LP64__) || (__WORDSIZE == 64) }

#else  /* } ASSEMBLY_SSE_OPT { */

	/*
	 * machine with SSE capable CPU
	 * Use assembly language to XOR two 128 bit registers at a time.
	 */
	_UINT32 symbolSize32rem = symbol_size % 16;	// Remaining bytes when the symbol
						// size is not multiple of 32 bits.
	_UINT32 symbolSize128;
	symbolSize128	= symbol_size >> 4;
	float *dst128;
	float *src128;
	dst128 = (float *) to;
	src128 = (float *) from;
	__m128 a, b;

	for (i = symbolSize128  ; i--  ;)
	{
		a = _mm_load_ps (dst128);
		b = _mm_load_ps (src128);
		a = _mm_xor_ps (a, b);
		_mm_store_ps (dst128, a);
		dst128 += 4;
		src128 += 4;
	}
	if (symbolSize32rem > 0)
	{
		for (i = 0; i < symbolSize32rem; i++)
		{
			* (_UINT8*) ( (_UINT8*) dst128 + i) ^= * (_UINT8*) ( (_UINT8*) src128 + i);
		}
	}
#endif /* } ASSEMBLY_SSE_OPT */
	OF_EXIT_FUNCTION
}


void	of_print_xor_symbols_statistics (of_symbol_stats_op_t	*xor)
{
	if (xor != NULL)
	{
		OF_TRACE_LVL(0, ("XOR stats:\n\tnb_xor_for_IT=%u  nb_xor_for_ML=%u\n",
						 xor->nb_xor_for_IT, xor->nb_xor_for_ML))
	}
}

#endif //OF_USE_LINEAR_BINARY_CODES_UTILS
