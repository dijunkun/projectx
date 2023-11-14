/* $Id: of_types.h 72 2012-04-13 13:27:26Z detchart $ */
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


#ifndef OF_TYPES
#define OF_TYPES


#ifndef __cplusplus
#ifndef bool
#define		bool		_UINT32
#define		true		1
#define		false		0
#endif
#endif /* !__cplusplus */

#ifndef _UINT32
#define		_INT8		char
#define		_INT16		short
#define		_UINT8		unsigned char
#define		_UINT16		unsigned short

#if defined(__LP64__) || (__WORDSIZE == 64) /* 64 bit architectures */
#define		_INT32		int		/* yes, it's also true in LP64! */
#define		_UINT32		unsigned int	/* yes, it's also true in LP64! */

#else  /* 32 bit architectures */

#define		_INT32		int		/* int creates fewer compilations pbs than long */
#define		_UINT32		unsigned int	/* int creates fewer compilations pbs than long */
#endif /* 32/64 architectures */

#endif /* !_UINT32 */

#ifndef _UINT64
// #ifdef _WIN32
// #define		_INT64		__int64
// #define		_UINT64		__uint64
// #else  /* UNIX */
#define		_INT64		long long
#define		_UINT64		unsigned long long
// #endif /* OS */
#endif /* !_UINT64 */


/**
 * gf type is used for Reed-Solomon codecs (over 2^8 and 2^m with m <=8).
 * It reprensents a Galois field element.
 */
typedef unsigned char gf;

/* VR: added for WIN CE support */
#ifdef _WIN32_WCE
#define bzero(to,sz)	memset((to), 0, (sz))

#define bcmp(a,b,sz)	memcmp((a), (b), (sz))
#endif /* WIN32_WCE */

/*
 * compatibility stuff
 */
#if defined(_WIN32)	/* but also for others, e.g. sun... */
#define NEED_BCOPY
#define bcmp(a,b,n) memcmp(a,b,n)
#endif

#ifdef NEED_BCOPY
#define bcopy(s, d, siz)        memcpy((d), (s), (siz))
#define bzero(d, siz)   memset((d), '\0', (siz))
#endif

#ifndef _UINT32
#define _UINT32 unsigned long
#endif

/*
 * stuff used for testing purposes only
 */

#ifdef TICK		/* VR: avoid a warning under Solaris */
#undef TICK
#endif

//#define TEST
#ifdef	TEST /* { */

#define DEB(x) x
#define DDB(x) x
#define	OF_RS_DEBUG	4	/* minimal debugging */
#if defined(_WIN32)
#include <time.h>
struct timeval
{
	unsigned long ticks;
};
#define gettimeofday(x, dummy) { (x)->ticks = clock() ; }
#define DIFF_T(a,b) (1+ 1000000*(a.ticks - b.ticks) / CLOCKS_PER_SEC )
typedef unsigned long _UINT32 ;
typedef unsigned short u_short ;
#else /* typically, unix systems */
#include <sys/time.h>
#define DIFF_T(a,b) \
	(1+ 1000000*(a.tv_sec - b.tv_sec) + (a.tv_usec - b.tv_usec) )
#endif

#define TICK(t) \
	{struct timeval x ; \
	gettimeofday(&x, NULL) ; \
	t = x.tv_usec + 1000000* (x.tv_sec & 0xff ) ; \
	}
#define TOCK(t) \
	{ _UINT32 t1 ; TICK(t1) ; \
	  if (t1 < t) t = 256000000 + t1 - t ; \
	  else t = t1 - t ; \
	  if (t == 0) t = 1 ;}

_UINT32 ticks[10];	/* vars for timekeeping */

#else  /* } { */

#define DEB(x)
#define DDB(x)
#define TICK(x)
#define TOCK(x)

#endif /* } TEST */


/*
 * You should not need to change anything beyond this point.
 * The first part of the file implements linear algebra in GF.
 *
 * gf is the type used to store an element of the Galois Field.
 * Must constain at least GF_BITS bits.
 *
 * Note: unsigned char will work up to GF(256) but int seems to run
 * faster on the Pentium. We use int whenever have to deal with an
 * index, since they are generally faster.
 */
#if (GF_BITS < 2  && GF_BITS >16)
#error "GF_BITS must be 2 .. 16"
#endif
/*#if (GF_BITS <= 8)
typedef unsigned char gf;
#else
typedef unsigned short gf;
#endif*/

#define	GF_SIZE ((1 << GF_BITS) - 1)	/* powers of \alpha */

#endif //OF_TYPES
