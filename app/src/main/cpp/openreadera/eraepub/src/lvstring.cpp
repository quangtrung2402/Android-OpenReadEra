/*******************************************************

   CoolReader Engine

   lvstring.cpp:  string classes implementation

   (c) Vadim Lopatin, 2000-2006
   Copyright (C) 2013-2020 READERA LLC

   This source code is distributed under the terms of
   GNU General Public License
   See LICENSE file for details

*******************************************************/

#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <assert.h>
#include <zlib.h>
#include "include/lvstring.h"
#include "include/lvref.h"
#include "include/arabic_tables.h"
#include "include/charProps.h"

ref_count_rec_t ref_count_rec_t::null_ref(NULL);

#define LS_DEBUG_CHECK

// set to 1 to enable debugging
#define DEBUG_STATIC_STRING_ALLOC 0


static lChar8 empty_str_8[] = {0};
static lstring8_chunk_t empty_chunk_8(empty_str_8);
lstring8_chunk_t * lString8::EMPTY_STR_8 = &empty_chunk_8;

static lChar16 empty_str_16[] = {0};
static lstring16_chunk_t empty_chunk_16(empty_str_16);
lstring16_chunk_t * lString16::EMPTY_STR_16 = &empty_chunk_16;

//================================================================================
// atomic string storages for string literals
//================================================================================

static const void * const_ptrs_8[CONST_STRING_BUFFER_SIZE] = {NULL};
static lString8 values_8[CONST_STRING_BUFFER_SIZE];
static int size_8 = 0;

/// get reference to atomic constant string for string literal e.g. cs8("abc") -- fast and memory effective
const lString8 & cs8(const char * str) {
    int index = (((int)((ptrdiff_t)str)) * CONST_STRING_BUFFER_HASH_MULT) & CONST_STRING_BUFFER_MASK;
    for (;;) {
        const void * p = const_ptrs_8[index];
        if (p == str) {
            return values_8[index];
        } else if (p == NULL) {
#if DEBUG_STATIC_STRING_ALLOC == 1
            CRLog::trace("allocating static string8 %s", str);
#endif
            const_ptrs_8[index] = str;
            size_8++;
            values_8[index] = lString8(str);
            values_8[index].addref();
            return values_8[index];
        }
        if (size_8 > CONST_STRING_BUFFER_SIZE / 4) {
            crFatalError(-1, "out of memory for const string8");
        }
        index = (index + 1) & CONST_STRING_BUFFER_MASK;
    }
    return lString8::empty_str;
}

static const void * const_ptrs_16[CONST_STRING_BUFFER_SIZE] = {NULL};
static lString16 values_16[CONST_STRING_BUFFER_SIZE];
static int size_16 = 0;

/// get reference to atomic constant wide string for string literal e.g. cs16("abc") -- fast and memory effective
const lString16 & cs16(const char * str) {
    int index = (((int)((ptrdiff_t)str)) * CONST_STRING_BUFFER_HASH_MULT) & CONST_STRING_BUFFER_MASK;
    for (;;) {
        const void * p = const_ptrs_16[index];
        if (p == str) {
            return values_16[index];
        } else if (p == NULL) {
#if DEBUG_STATIC_STRING_ALLOC == 1
            CRLog::trace("allocating static string16 %s", str);
#endif
            const_ptrs_16[index] = str;
            size_16++;
            values_16[index] = lString16(str);
            values_16[index].addref();
            return values_16[index];
        }
        if (size_16 > CONST_STRING_BUFFER_SIZE / 4) {
            crFatalError(-1, "out of memory for const string8");
        }
        index = (index + 1) & CONST_STRING_BUFFER_MASK;
    }
    return lString16::empty_str;
}

/// get reference to atomic constant wide string for string literal e.g. cs16(L"abc") -- fast and memory effective
const lString16 & cs16(const lChar16 * str) {
    int index = (((int)((ptrdiff_t)str)) * CONST_STRING_BUFFER_HASH_MULT) & CONST_STRING_BUFFER_MASK;
    for (;;) {
        const void * p = const_ptrs_16[index];
        if (p == str) {
            return values_16[index];
        } else if (p == NULL) {
#if DEBUG_STATIC_STRING_ALLOC == 1
            CRLog::trace("allocating static string16 %s", LCSTR(str));
#endif
            const_ptrs_16[index] = str;
            size_16++;
            values_16[index] = lString16(str);
            values_16[index].addref();
            return values_16[index];
        }
        if (size_16 > CONST_STRING_BUFFER_SIZE / 4) {
            crFatalError(-1, "out of memory for const string8");
        }
        index = (index + 1) & CONST_STRING_BUFFER_MASK;
    }
    return lString16::empty_str;
}



//================================================================================
// memory allocation slice
//================================================================================
struct lstring_chunk_slice_t {
    lstring8_chunk_t * pChunks; // first chunk
    lstring8_chunk_t * pEnd;    // first free byte after last chunk
    lstring8_chunk_t * pFree;   // first free chunk
    int used;
    lstring_chunk_slice_t( int size )
    {
        pChunks = (lstring8_chunk_t *) malloc(sizeof(lstring8_chunk_t) * size);
        pEnd = pChunks + size;
        pFree = pChunks;
        for (lstring8_chunk_t * p = pChunks; p<pEnd; ++p)
        {
            p->buf8 = (char*)(p+1);
            p->size = 0;
        }
        (pEnd-1)->buf8 = NULL;
    }
    ~lstring_chunk_slice_t()
    {
        free( pChunks );
    }
    inline lstring8_chunk_t * alloc_chunk()
    {
        lstring8_chunk_t * res = pFree;
        pFree = (lstring8_chunk_t *)res->buf8;
        return res;
    }
    inline lstring16_chunk_t * alloc_chunk16()
    {
        lstring16_chunk_t * res = (lstring16_chunk_t *)pFree;
        pFree = (lstring8_chunk_t *)res->buf16;
        return res;
    }
    inline bool free_chunk( lstring8_chunk_t * pChunk )
    {
        if (pChunk < pChunks || pChunk >= pEnd)
            return false; // chunk does not belong to this slice
/*
#ifdef LS_DEBUG_CHECK
        if (!pChunk->size)
        {
            crFatalError(); // already freed!!!
        }
        pChunk->size = 0;
#endif
*/
        pChunk->buf8 = (char *)pFree;
        pFree = pChunk;
        return true;
    }
    inline bool free_chunk16(lstring16_chunk_t * pChunk)
    {
        if ((lstring8_chunk_t *)pChunk < pChunks || (lstring8_chunk_t *)pChunk >= pEnd)
            return false; // chunk does not belong to this slice
/*
#ifdef LS_DEBUG_CHECK
        if (!pChunk->size)
        {
            crFatalError(); // already freed!!!
        }
        pChunk->size = 0;
#endif
*/
        pChunk->buf16 = (lChar16 *)pFree;
        pFree = (lstring8_chunk_t *)pChunk;
        return true;
    }
};

//#define FIRST_SLICE_SIZE 256
//#define MAX_SLICE_COUNT  20

////////////////////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////////////////////

inline int _lStr_len(const lChar16 * str)
{
    int len;
    for (len=0; *str; str++)
        len++;
    return len;
}

inline int _lStr_len(const lChar8 * str)
{
    int len;
    for (len=0; *str; str++)
        len++;
    return len;
}

inline int _lStr_nlen(const lChar16 * str, int maxcount)
{
    int len;
    for (len=0; len<maxcount && *str; str++)
        len++;
    return len;
}

inline int _lStr_nlen(const lChar8 * str, int maxcount)
{
    int len;
    for (len=0; len<maxcount && *str; str++)
        len++;
    return len;
}

inline int _lStr_cpy(lChar16 * dst, const lChar16 * src)
{
    int count;
    for ( count=0; (*dst++ = *src++); count++ )
        ;
    return count;
}

inline int _lStr_cpy(lChar8 * dst, const lChar8 * src)
{
    int count;
    for ( count=0; (*dst++ = *src++); count++ )
        ;
    return count;
}

inline int _lStr_cpy(lChar16 * dst, const lChar8 * src)
{
    int count;
    for ( count=0; (*dst++ = *src++); count++ )
        ;
    return count;
}

inline int _lStr_cpy(lChar8 * dst, const lChar16 * src)
{
    int count;
    for ( count=0; (*dst++ = (lChar8)*src++); count++ )
        ;
    return count;
}

inline int _lStr_ncpy(lChar16 * dst, const lChar16 * src, int maxcount)
{
    int count = 0;
    do
    {
        if (++count > maxcount)
        {
            *dst = 0;
            return count;
        }
    } while ((*dst++ = *src++));
    return count;
}

inline int _lStr_ncpy(lChar16 * dst, const lChar8 * src, int maxcount)
{
    int count = 0;
    do
    {
        if (++count > maxcount)
        {
            *dst = 0;
            return count;
        }
    } while ((*dst++ = (unsigned char)*src++));
    return count;
}

inline int _lStr_ncpy(lChar8 * dst, const lChar8 * src, int maxcount)
{
    int count = 0;
    do
    {
        if (++count > maxcount)
        {
            *dst = 0;
            return count;
        }
    } while ((*dst++ = *src++));
    return count;
}

inline void _lStr_memcpy(lChar16 * dst, const lChar16 * src, int count)
{
    while ( count-- > 0)
        (*dst++ = *src++);
}

inline void _lStr_memcpy(lChar8 * dst, const lChar8 * src, int count)
{
    while ( count-- > 0)
        (*dst++ = *src++);
}

inline void _lStr_memset(lChar16 * dst, lChar16 value, int count)
{
    while ( count-- > 0)
        *dst++ = value;
}

inline void _lStr_memset(lChar8 * dst, lChar8 value, int count)
{
    while ( count-- > 0)
        *dst++ = value;
}

int lStr_len(const lChar16 * str)
{
    return _lStr_len(str);
}

int lStr_len(const lChar8 * str)
{
    return _lStr_len(str);
}

int lStr_nlen(const lChar16 * str, int maxcount)
{
    return _lStr_nlen(str, maxcount);
}

int lStr_nlen(const lChar8 * str, int maxcount)
{
    return _lStr_nlen(str, maxcount);
}

int lStr_cpy(lChar16 * dst, const lChar16 * src)
{
    return _lStr_cpy(dst, src);
}

int lStr_cpy(lChar8 * dst, const lChar8 * src)
{
    return _lStr_cpy(dst, src);
}

int lStr_cpy(lChar16 * dst, const lChar8 * src)
{
    return _lStr_cpy(dst, src);
}

int lStr_ncpy(lChar16 * dst, const lChar16 * src, int maxcount)
{
    return _lStr_ncpy(dst, src, maxcount);
}

int lStr_ncpy(lChar8 * dst, const lChar8 * src, int maxcount)
{
    return _lStr_ncpy(dst, src, maxcount);
}

void lStr_memcpy(lChar16 * dst, const lChar16 * src, int count)
{
    _lStr_memcpy(dst, src, count);
}

void lStr_memcpy(lChar8 * dst, const lChar8 * src, int count)
{
    _lStr_memcpy(dst, src, count);
}

void lStr_memset(lChar16 * dst, lChar16 value, int count)
{
    _lStr_memset(dst, value, count);
}

void lStr_memset(lChar8 * dst, lChar8 value, int count)
{
    _lStr_memset(dst, value, count);
}

int lStr_cmp(const lChar16 * dst, const lChar16 * src)
{
    while ( *dst == *src)
    {
        if (! *dst )
            return 0;
        ++dst;
        ++src;
    }
    if ( *dst > *src )
        return 1;
    else
        return -1;
}

int lStr_cmp(const lChar8 * dst, const lChar8 * src)
{
    while ( *dst == *src)
    {
        if (! *dst )
            return 0;
        ++dst;
        ++src;
    }
    if ( *dst > *src )
        return 1;
    else
        return -1;
}

int lStr_cmp(const lChar16* dst, const lChar8* src)
{
    while (*dst == (lChar16)* src)
    {
        if (! *dst )
            return 0;
        ++dst;
        ++src;
    }
    if (*dst > (lChar16) * src)
        return 1;
    else
        return -1;
}

int lStr_cmp(const lChar8 * dst, const lChar16 * src)
{
    while ( (lChar16)*dst == *src)
    {
        if (! *dst )
            return 0;
        ++dst;
        ++src;
    }
    if ( (lChar16)*dst > *src )
        return 1;
    else
        return -1;
}

////////////////////////////////////////////////////////////////////////////
// lString16
////////////////////////////////////////////////////////////////////////////

void lString16::free()
{
    if (pchunk==EMPTY_STR_16)
        return;
    //assert(pchunk->buf16[pchunk->len]==0);
    ::free(pchunk->buf16);
    ::free(pchunk);
}

void lString16::alloc(int sz)
{
    pchunk = (lstring_chunk_t*) ::malloc(sizeof(lstring_chunk_t));
    pchunk->buf16 = (lChar16*) ::malloc(sizeof(lChar16) * (sz + 1));
    assert(pchunk->buf16 != NULL);
    pchunk->size = sz;
    pchunk->nref = 1;
}

lString16::lString16(const lChar16 * str)
{
    if (!str || !(*str))
    {
        pchunk = EMPTY_STR_16;
        addref();
        return;
    }
    size_type len = _lStr_len(str);
    alloc( len );
    pchunk->len = len;
    _lStr_cpy( pchunk->buf16, str );
}

lString16::lString16(const lChar8 * str)
{
    if (!str || !(*str))
    {
        pchunk = EMPTY_STR_16;
        addref();
        return;
    }
    pchunk = EMPTY_STR_16;
    addref();
	*this = Utf8ToUnicode(str);
}

/// constructor from utf8 character array fragment
lString16::lString16(const lChar8 * str, size_type count)
{
    if (!str || !(*str))
    {
        pchunk = EMPTY_STR_16;
        addref();
        return;
    }
    pchunk = EMPTY_STR_16;
    addref();
	*this = Utf8ToUnicode( str, count );
}


lString16::lString16(const value_type * str, size_type count)
{
    if ( !str || !(*str) || count<=0 )
    {
        pchunk = EMPTY_STR_16; addref();
    }
    else
    {
        size_type len = _lStr_nlen(str, count);
        alloc(len);
        _lStr_ncpy( pchunk->buf16, str, len );
        pchunk->len = len;
    }
}

lString16::lString16(const lString16 & str, size_type offset, size_type count)
{
    if ( count > str.length() - offset )
        count = str.length() - offset;
    if (count<=0)
    {
        pchunk = EMPTY_STR_16; addref();
    }
    else
    {
        alloc(count);
        _lStr_memcpy( pchunk->buf16, str.pchunk->buf16+offset, count );
        pchunk->buf16[count]=0;
        pchunk->len = count;
    }
}

lString16 & lString16::assign(const lChar16 * str)
{
    if (!str || !(*str))
    {
        clear();
    }
    else
    {
        size_type len = _lStr_len(str);
        if (pchunk->nref==1)
        {
            if (pchunk->size<=len)
            {
                // resize is necessary
                pchunk->buf16 = (lChar16*) ::realloc( pchunk->buf16, sizeof(lChar16)*(len+1) );
                pchunk->size = len+1;
            }
        }
        else
        {
            release();
            alloc(len);
        }
        _lStr_cpy( pchunk->buf16, str );
        pchunk->len = len;
    }
    return *this;
}

lString16 & lString16::assign(const lChar8 * str)
{
    if (!str || !(*str))
    {
        clear();
    }
    else
    {
        size_type len = _lStr_len(str);
        if (pchunk->nref==1)
        {
            if (pchunk->size<=len)
            {
                // resize is necessary
                pchunk->buf16 = (lChar16*) ::realloc( pchunk->buf16, sizeof(lChar16)*(len+1) );
                pchunk->size = len+1;
            }
        }
        else
        {
            release();
            alloc(len);
        }
        _lStr_cpy( pchunk->buf16, str );
        pchunk->len = len;
    }
    return *this;
}

lString16 & lString16::assign(const lChar16 * str, size_type count)
{
    if ( !str || !(*str) || count<=0 )
    {
        clear();
    }
    else
    {
        size_type len = _lStr_nlen(str, count);
        if (pchunk->nref==1)
        {
            if (pchunk->size<=len)
            {
                // resize is necessary
                pchunk->buf16 = (lChar16*) ::realloc( pchunk->buf16, sizeof(lChar16)*(len+1) );
                pchunk->size = len+1;
            }
        }
        else
        {
            release();
            alloc(len);
        }
        _lStr_ncpy( pchunk->buf16, str, count );
        pchunk->len = len;
    }
    return *this;
}

lString16 & lString16::assign(const lChar8 * str, size_type count)
{
    if ( !str || !(*str) || count<=0 )
    {
        clear();
    }
    else
    {
        size_type len = _lStr_nlen(str, count);
        if (pchunk->nref==1)
        {
            if (pchunk->size<=len)
            {
                // resize is necessary
                pchunk->buf16 = (lChar16*) ::realloc( pchunk->buf16, sizeof(lChar16)*(len+1) );
                pchunk->size = len+1;
            }
        }
        else
        {
            release();
            alloc(len);
        }
        _lStr_ncpy( pchunk->buf16, str, count );
        pchunk->len = len;
    }
    return *this;
}

lString16 & lString16::assign(const lString16 & str, size_type offset, size_type count)
{
    if ( count > str.length() - offset )
        count = str.length() - offset;
    if (count<=0)
    {
        clear();
    }
    else
    {
        if (pchunk==str.pchunk)
        {
            if (&str != this)
            {
                release();
                alloc(count);
            }
            if (offset>0)
            {
                _lStr_memcpy( pchunk->buf16, str.pchunk->buf16+offset, count );
            }
            pchunk->buf16[count]=0;
        }
        else
        {
            if (pchunk->nref==1)
            {
                if (pchunk->size<=count)
                {
                    // resize is necessary
                    pchunk->buf16 = (lChar16*) ::realloc( pchunk->buf16, sizeof(lChar16)*(count+1) );
                    pchunk->size = count+1;
                }
            }
            else
            {
                release();
                alloc(count);
            }
            _lStr_memcpy( pchunk->buf16, str.pchunk->buf16+offset, count );
            pchunk->buf16[count]=0;
        }
        pchunk->len = count;
    }
    return *this;
}

lString16 & lString16::erase(size_type offset, size_type count)
{
    if ( count > length() - offset )
        count = length() - offset;
    if (count<=0)
    {
        clear();
    }
    else
    {
        size_type newlen = length()-count;
        if (pchunk->nref==1)
        {
            _lStr_memcpy( pchunk->buf16+offset, pchunk->buf16+offset+count, newlen-offset+1 );
        }
        else
        {
            lstring_chunk_t * poldchunk = pchunk;
            release();
            alloc( newlen );
            _lStr_memcpy( pchunk->buf16, poldchunk->buf16, offset );
            _lStr_memcpy( pchunk->buf16+offset, poldchunk->buf16+offset+count, newlen-offset+1 );
        }
        pchunk->len = newlen;
        pchunk->buf16[newlen]=0;
    }
    return *this;
}

void lString16::reserve(size_type n)
{
    if (pchunk->nref==1)
    {
        if (pchunk->size < n)
        {
            pchunk->buf16 = (lChar16*) ::realloc( pchunk->buf16, sizeof(lChar16)*(n+1) );
            pchunk->size = n;
        }
    }
    else
    {
        lstring_chunk_t * poldchunk = pchunk;
        release();
        alloc( n );
        _lStr_memcpy( pchunk->buf16, poldchunk->buf16, poldchunk->len+1 );
        pchunk->len = poldchunk->len;
    }
}

void lString16::lock( size_type newsize )
{
    if (pchunk->nref>1)
    {
        lstring_chunk_t * poldchunk = pchunk;
        release();
        alloc( newsize );
        size_type len = newsize;
        if (len>poldchunk->len)
            len = poldchunk->len;
        _lStr_memcpy( pchunk->buf16, poldchunk->buf16, len );
        pchunk->buf16[len]=0;
        pchunk->len = len;
    }
}

// lock string, allocate buffer and reset length to 0
void lString16::reset( size_type size )
{
    if (pchunk->nref>1 || pchunk->size<size)
    {
        release();
        alloc( size );
    }
    pchunk->buf16[0] = 0;
    pchunk->len = 0;
}

void lString16::resize(size_type n, lChar16 e)
{
    lock( n );
    if (n>=pchunk->size)
    {
        pchunk->buf16 = (lChar16*) ::realloc( pchunk->buf16, sizeof(lChar16)*(n+1) );
        pchunk->size = n;
    }
    // fill with data if expanded
    for (size_type i=pchunk->len; i<n; i++)
        pchunk->buf16[i] = e;
    pchunk->buf16[pchunk->len] = 0;
}

lString16 & lString16::append(int val)
{
    const lString16 valstr = lString16::itoa(val);
    return this->append(valstr);
}

lString16 & lString16::append(const lChar16 * str)
{
    size_type len = _lStr_len(str);
    reserve( pchunk->len+len );
    _lStr_memcpy(pchunk->buf16+pchunk->len, str, len+1);
    pchunk->len += len;
    return *this;
}

lString16 & lString16::append(const lChar16 * str, size_type count)
{
    reserve(pchunk->len + count);
    _lStr_ncpy(pchunk->buf16 + pchunk->len, str, count);
    pchunk->len += count;
    return *this;
}

lString16 & lString16::append(const lChar8 * str)
{
    size_type len = _lStr_len(str);
    reserve( pchunk->len+len );
    _lStr_ncpy(pchunk->buf16+pchunk->len, str, len+1);
    pchunk->len += len;
    return *this;
}

lString16 & lString16::append(const lChar8 * str, size_type count)
{
    reserve(pchunk->len + count);
    _lStr_ncpy(pchunk->buf16+pchunk->len, str, count);
    pchunk->len += count;
    return *this;
}

lString16 & lString16::append(const lString16 & str)
{
    size_type len2 = pchunk->len + str.pchunk->len;
    reserve( len2 );
    _lStr_memcpy( pchunk->buf16+pchunk->len, str.pchunk->buf16, str.pchunk->len+1 );
    pchunk->len = len2;
    return *this;
}

lString16 & lString16::append(const lString16 & str, size_type offset, size_type count)
{
    if ( str.pchunk->len>offset )
    {
        if ( offset + count > str.pchunk->len )
            count = str.pchunk->len - offset;
        reserve( pchunk->len+count );
        _lStr_ncpy(pchunk->buf16 + pchunk->len, str.pchunk->buf16 + offset, count);
        pchunk->len += count;
        pchunk->buf16[pchunk->len] = 0;
    }
    return *this;
}

lString16 & lString16::append(size_type count, lChar16 ch)
{
    reserve( pchunk->len+count );
    _lStr_memset(pchunk->buf16+pchunk->len, ch, count);
    pchunk->len += count;
    pchunk->buf16[pchunk->len] = 0;
    return *this;
}

lString16 & lString16::insert(size_type p0, size_type count, lChar16 ch)
{
    if (p0>pchunk->len)
        p0 = pchunk->len;
    reserve( pchunk->len+count );
    for (size_type i=pchunk->len+count; i>p0; i--)
        pchunk->buf16[i] = pchunk->buf16[i-1];
    _lStr_memset(pchunk->buf16+p0, ch, count);
    pchunk->len += count;
    pchunk->buf16[pchunk->len] = 0;
    return *this;
}

lString16 & lString16::insert(size_type p0, const lString16 & str)
{
    if (p0>pchunk->len)
        p0 = pchunk->len;
    int count = str.length();
    reserve( pchunk->len+count );
    for (size_type i=pchunk->len+count; i>p0; i--)
        pchunk->buf16[i] = pchunk->buf16[i-1];
    _lStr_memcpy(pchunk->buf16 + p0, str.c_str(), count);
    pchunk->len += count;
    pchunk->buf16[pchunk->len] = 0;
    return *this;
}

lString16 lString16::substr(size_type pos, size_type n) const
{
    if (pos>=length())
        return lString16::empty_str;
    if (pos+n>length())
        n = length() - pos;
    return lString16( pchunk->buf16+pos, n );
}

lString16 & lString16::pack()
{
    if (pchunk->len + 4 < pchunk->size )
    {
        if (pchunk->nref>1)
        {
            lock(pchunk->len);
        }
        else
        {
            pchunk->buf16 = (lChar16 *) realloc( pchunk->buf16, sizeof(lChar16)*(pchunk->len+1) );
            pchunk->size = pchunk->len;
        }
    }
    return *this;
}
bool isAlNum(lChar16 ch) {
    lUInt16 props = lGetCharProps(ch);
    return (props & (CH_PROP_ALPHA | CH_PROP_DIGIT)) != 0;
}

/// trims non alpha at beginning and end of string
lString16 & lString16::trimNonAlpha()
{
    int firstns;
    for (firstns = 0; firstns<pchunk->len && !isAlNum(pchunk->buf16[firstns]); ++firstns)
        ;
    if (firstns >= pchunk->len)
    {
        clear();
        return *this;
    }
    int lastns;
    for (lastns = pchunk->len-1; lastns>0 && !isAlNum(pchunk->buf16[lastns]); --lastns)
        ;
    int newlen = lastns-firstns+1;
    if (newlen == pchunk->len)
        return *this;
    if (pchunk->nref == 1)
    {
        if (firstns>0)
            lStr_memcpy( pchunk->buf16, pchunk->buf16+firstns, newlen );
        pchunk->buf16[newlen] = 0;
        pchunk->len = newlen;
    }
    else
    {
        lstring_chunk_t * poldchunk = pchunk;
        release();
        alloc( newlen );
        _lStr_memcpy( pchunk->buf16, poldchunk->buf16+firstns, newlen );
        pchunk->buf16[newlen] = 0;
        pchunk->len = newlen;
    }
    return *this;
}

lString16 & lString16::trim()
{
    //
    int firstns;
    for (firstns = 0; firstns<pchunk->len &&
        (pchunk->buf16[firstns]==' ' || pchunk->buf16[firstns]=='\t'); ++firstns)
        ;
    if (firstns >= pchunk->len)
    {
        clear();
        return *this;
    }
    int lastns;
    for (lastns = pchunk->len-1; lastns>0 &&
        (pchunk->buf16[lastns]==' ' || pchunk->buf16[lastns]=='\t'); --lastns)
        ;
    int newlen = lastns-firstns+1;
    if (newlen == pchunk->len)
        return *this;
    if (pchunk->nref == 1)
    {
        if (firstns>0)
            lStr_memcpy( pchunk->buf16, pchunk->buf16+firstns, newlen );
        pchunk->buf16[newlen] = 0;
        pchunk->len = newlen;
    }
    else
    {
        lstring_chunk_t * poldchunk = pchunk;
        release();
        alloc( newlen );
        _lStr_memcpy( pchunk->buf16, poldchunk->buf16+firstns, newlen );
        pchunk->buf16[newlen] = 0;
        pchunk->len = newlen;
    }
    return *this;
}

int lString16::atoi() const
{
    int n = 0;
    atoi(n);
    return n;
}

static const char * hex_digits = "0123456789abcdef";
// converts 0..15 to 0..f
char toHexDigit( int c )
{
    return hex_digits[c&0xf];
}

// returns 0..15 if c is hex digit, -1 otherwise
int hexDigit( int c )
{
    if ( c>='0' && c<='9')
        return c-'0';
    if ( c>='a' && c<='f')
        return c-'a'+10;
    if ( c>='A' && c<='F')
        return c-'A'+10;
    return -1;
}

// decode LEN hex digits, return decoded number, -1 if invalid
int decodeHex( const lChar16 * str, int len ) {
    int n = 0;
    for ( int i=0; i<len; i++ ) {
        if ( !str[i] )
            return -1;
        int d = hexDigit(str[i]);
        if ( d==-1 )
            return -1;
        n = (n<<4) | d;
    }
    return n;
}

// decode LEN decimal digits, return decoded number, -1 if invalid
int decodeDecimal( const lChar16 * str, int len ) {
    int n = 0;
    for ( int i=0; i<len; i++ ) {
        if ( !str[i] )
            return -1;
        int d = str[i] - '0';
        if ( d<0 || d>9 )
            return -1;
        n = n*10 + d;
    }
    return n;
}

bool lString16::atoi( int &n ) const
{
	n = 0;
    int sgn = 1;
    const lChar16 * s = c_str();
    while (*s == ' ' || *s == '\t')
        s++;
    if ( s[0]=='0' && s[1]=='x') {
        s+=2;
        for (;*s;) {
            int d = hexDigit(*s++);
            if ( d>=0 )
                n = (n<<4) | d;
        }
        return true;
    }
    if (*s == '-')
    {
        sgn = -1;
        s++;
    }
    else if (*s == '+')
    {
        s++;
    }
    if ( !(*s>='0' && *s<='9') )
        return false;
    while (*s>='0' && *s<='9')
    {
        n = n * 10 + ( (*s++)-'0' );
    }
    if ( sgn<0 )
        n = -n;
    return *s=='\0' || *s==' ' || *s=='\t';
}

bool lString16::atoi( lInt64 &n ) const
{
    int sgn = 1;
    const lChar16 * s = c_str();
    while (*s == ' ' || *s == '\t')
        s++;
    if (*s == '-')
    {
        sgn = -1;
        s++;
    }
    else if (*s == '+')
    {
        s++;
    }
    if ( !(*s>='0' && *s<='9') )
        return false;
    while (*s>='0' && *s<='9')
    {
        n = n * 10 + ( (*s++)-'0' );
    }
    if ( sgn<0 )
        n = -n;
    return *s=='\0' || *s==' ' || *s=='\t';
}

#define STRING_HASH_MULT 31
lUInt32 lString16::getHash() const
{
    lUInt32 res = 0;
    for (lInt32 i=0; i<pchunk->len; i++)
        res = res * STRING_HASH_MULT + pchunk->buf16[i];
    return res;
}



void lString16Collection::reserve(int space)
{
    if (count + space > size)
    {
        size = count + space + 64;
        chunks = (lstring16_chunk_t**) realloc(chunks, sizeof(lstring16_chunk_t*) * size);
    }
}

static int (str16_comparator)(const void * n1, const void * n2)
{
    lstring16_chunk_t ** s1 = (lstring16_chunk_t **)n1;
    lstring16_chunk_t ** s2 = (lstring16_chunk_t **)n2;
    return lStr_cmp( (*s1)->data16(), (*s2)->data16() );
}

static int(*custom_lstr16_comparator_ptr)(lString16 & s1, lString16 & s2);
static int (str16_custom_comparator)(const void * n1, const void * n2)
{
    lString16 s1(*((lstring16_chunk_t **)n1));
    lString16 s2(*((lstring16_chunk_t **)n2));
    return custom_lstr16_comparator_ptr(s1, s2);
}

void lString16Collection::sort(int(comparator)(lString16 & s1, lString16 & s2))
{
    custom_lstr16_comparator_ptr = comparator;
    qsort(chunks,count,sizeof(lstring16_chunk_t*), str16_custom_comparator);
}

void lString16Collection::sort()
{
    qsort(chunks,count,sizeof(lstring16_chunk_t*), str16_comparator);
}

int lString16Collection::add( const lString16 & str )
{
    reserve(1);
    chunks[count] = str.pchunk;
    str.addref();
    return count++;
}
void lString16Collection::clear()
{
    for (int i=0; i<count; i++)
    {
        ((lString16 *)chunks)[i].release();
    }
    if (chunks)
        free(chunks);
    chunks = NULL;
    count = 0;
    size = 0;
}

void lString16Collection::erase(int offset, int cnt)
{
    if (count<=0)
        return;
    if (offset < 0 || offset + cnt >= count)
        return;
    int i;
    for (i = offset; i < offset + cnt; i++)
    {
        ((lString16 *)chunks)[i].release();
    }
    for (i = offset + cnt; i < count; i++)
    {
        chunks[i-cnt] = chunks[i];
    }
    count -= cnt;
    if (!count)
        clear();
}

void lString8Collection::split( const lString8 & str, const lString8 & delimiter )
{
    if (str.empty())
        return;
    for (int startpos = 0; startpos < str.length(); ) {
        int pos = str.pos(delimiter, startpos);
        if (pos < 0)
            pos = str.length();
        add(str.substr(startpos, pos - startpos));
        startpos = pos + delimiter.length();
    }
}

void lString16Collection::split( const lString16 & str, const lString16 & delimiter )
{
    if (str.empty())
        return;
    for (int startpos = 0; startpos < str.length(); ) {
        int pos = str.pos(delimiter, startpos);
        if (pos < 0)
            pos = str.length();
        add(str.substr(startpos, pos - startpos));
        startpos = pos + delimiter.length();
    }
}

void lString8Collection::erase(int offset, int cnt)
{
    if (count <= 0)
        return;
    if (offset < 0 || offset + cnt > count)
        return;
    int i;
    for (i = offset; i < offset + cnt; i++)
    {
        ((lString8 *)chunks)[i].release();
    }
    for (i = offset + cnt; i < count; i++)
    {
        chunks[i-cnt] = chunks[i];
    }
    count -= cnt;
    if (!count)
        clear();
}

void lString8Collection::reserve(int space)
{
    if ( count + space > size )
    {
        size = count + space + 64;
        chunks = (lstring8_chunk_t * *)realloc( chunks, sizeof(lstring8_chunk_t *) * size );
    }
}

int lString8Collection::add( const lString8 & str )
{
    reserve( 1 );
    chunks[count] = str.pchunk;
    str.addref();
    return count++;
}
void lString8Collection::clear()
{
    for (int i=0; i<count; i++)
    {
        ((lString8 *)chunks)[i].release();
    }
    if (chunks)
        free(chunks);
    chunks = NULL;
    count = 0;
    size = 0;
}

lUInt32 calcStringHash( const lChar16 * s )
{
    lUInt32 a = 2166136261u;
    while (*s)
    {
        a = a * 16777619 ^ (*s++);
    }
    return a;
}

static const char * str_hash_magic="STRS";

/// serialize to byte array (pointer will be incremented by number of bytes written)
void lString16HashedCollection::serialize( SerialBuf & buf )
{
    if ( buf.error() )
        return;
    int start = buf.pos();
    buf.putMagic( str_hash_magic );
    lUInt32 count = length();
    buf << count;
    for ( int i=0; i<length(); i++ )
    {
        buf << at(i);
    }
    buf.putCRC( buf.pos() - start );
}

/// calculates CRC32 for buffer contents
lUInt32 lStr_crc32(lUInt32 prevValue, const void * buf, int size )
{
    return crc32( prevValue, (const lUInt8 *)buf, size );
}

/// add CRC32 for last N bytes
void SerialBuf::putCRC( int size )
{
    if ( error() )
        return;
    if ( size>_pos ) {
        *this << (lUInt32)0;
        seterror();
    }
    lUInt32 n = 0;
    n = lStr_crc32( n, _buf + _pos-size, size );
    *this << n;
}

/// get CRC32 for the whole buffer
lUInt32 SerialBuf::getCRC()
{
    if (error())
        return 0;
    lUInt32 n = 0;
    n = lStr_crc32( n, _buf, _pos );
    return n;
}

/// read crc32 code, comapare with CRC32 for last N bytes
bool SerialBuf::checkCRC( int size )
{
    if ( error() )
        return false;
    if ( size>_pos ) {
        seterror();
        return false;
    }
    lUInt32 n0 = 0;
    n0 = lStr_crc32(n0, _buf + _pos-size, size);
    lUInt32 n = 0;
    *this >> n;
    if ( error() )
        return false;
    if ( n!=n0 )
        seterror();
    return !error();
}

/// deserialize from byte array (pointer will be incremented by number of bytes read)
bool lString16HashedCollection::deserialize( SerialBuf & buf )
{
    if ( buf.error() )
        return false;
    clear();
    int start = buf.pos();
    buf.putMagic( str_hash_magic );
    lInt32 count = 0;
    buf >> count;
    for ( int i=0; i<count; i++ ) {
        lString16 s;
        buf >> s;
        if ( buf.error() )
            break;
        add( s.c_str() );
    }
    buf.checkCRC( buf.pos() - start );
    return !buf.error();
}

lString16HashedCollection::lString16HashedCollection( lString16HashedCollection & v )
: lString16Collection( v )
, hashSize( v.hashSize )
, hash( NULL )
{
    hash = (HashPair *)malloc( sizeof(HashPair) * hashSize );
    for ( int i=0; i<hashSize; i++ ) {
        hash[i].clear();
        hash[i].index = v.hash[i].index;
        HashPair * next = v.hash[i].next;
        while ( next ) {
            addHashItem( i, next->index );
            next = next->next;
        }
    }
}

void lString16HashedCollection::addHashItem( int hashIndex, int storageIndex )
{
    if ( hash[ hashIndex ].index == -1 ) {
        hash[hashIndex].index = storageIndex;
    } else {
        HashPair * np = (HashPair *)malloc(sizeof(HashPair));
        np->index = storageIndex;
        np->next = hash[hashIndex].next;
        hash[hashIndex].next = np;
    }
}

void lString16HashedCollection::clearHash()
{
    if ( hash ) {
        for ( int i=0; i<hashSize; i++) {
            HashPair * p = hash[i].next;
            while ( p ) {
                HashPair * tmp = p->next;
                free( p );
                p = tmp;
            }
        }
        free( hash );
    }
    hash = NULL;
}

lString16HashedCollection::lString16HashedCollection( lUInt32 hash_size )
: hashSize(hash_size), hash(NULL)
{

    hash = (HashPair *)malloc( sizeof(HashPair) * hashSize );
    for ( int i=0; i<hashSize; i++ )
        hash[i].clear();
}

lString16HashedCollection::~lString16HashedCollection()
{
    clearHash();
}

int lString16HashedCollection::find( const lChar16 * s )
{
    if ( !hash || !length() )
        return -1;
    lUInt32 h = calcStringHash( s );
    lUInt32 n = h % hashSize;
    if ( hash[n].index!=-1 )
    {
        const lString16 & str = at( hash[n].index );
        if ( str == s )
            return hash[n].index;
        HashPair * p = hash[n].next;
        for ( ;p ;p = p->next ) {
            const lString16 & str = at( p->index );
            if ( str==s )
                return p->index;
        }
    }
    return -1;
}

void lString16HashedCollection::reHash( int newSize )
{
    if (hashSize == newSize)
        return;
    clearHash();
    hashSize = newSize;
    if (hashSize > 0) {
        hash = (HashPair *)malloc( sizeof(HashPair) * hashSize );
        for ( int i=0; i<hashSize; i++ )
            hash[i].clear();
    }
    for ( int i=0; i<length(); i++ ) {
        lUInt32 h = calcStringHash( at(i).c_str() );
        lUInt32 n = h % hashSize;
        addHashItem( n, i );
    }
}

int lString16HashedCollection::add( const lChar16 * s )
{
    if ( !hash || hashSize < length()*2 ) {
        int sz = 16;
        while ( sz<length() )
            sz <<= 1;
        sz <<= 1;
        reHash( sz );
    }
    lUInt32 h = calcStringHash( s );
    lUInt32 n = h % hashSize;
    if ( hash[n].index!=-1 )
    {
        const lString16 & str = at( hash[n].index );
        if ( str == s )
            return hash[n].index;
        HashPair * p = hash[n].next;
        for ( ;p ;p = p->next ) {
            const lString16 & str = at( p->index );
            if ( str==s )
                return p->index;
        }
    }
    lUInt32 i = lString16Collection::add( lString16(s) );
    addHashItem( n, i );
    return i;
}

const lString16 lString16::empty_str;


////////////////////////////////////////////////////////////////////////////
// lString8
////////////////////////////////////////////////////////////////////////////

void lString8::free()
{
    if ( pchunk==EMPTY_STR_8 )
        return;
    ::free(pchunk->buf8);
    ::free(pchunk);
}

void lString8::alloc(int sz)
{
    pchunk = (lstring_chunk_t*)::malloc(sizeof(lstring_chunk_t));
    pchunk->buf8 = (lChar8*) ::malloc( sizeof(lChar8) * (sz+1) );
    assert( pchunk->buf8!=NULL );
    pchunk->size = sz;
    pchunk->nref = 1;
}

lString8::lString8(const lChar8 * str)
{
    if (!str || !(*str))
    {
        pchunk = EMPTY_STR_8;
        addref();
        return;
    }
    size_type len = _lStr_len(str);
    alloc( len );
    pchunk->len = len;
    _lStr_cpy( pchunk->buf8, str );
}

lString8::lString8(const lChar16 * str)
{
    if (!str || !(*str))
    {
        pchunk = EMPTY_STR_8;
        addref();
        return;
    }
    size_type len = _lStr_len(str);
    alloc( len );
    pchunk->len = len;
    _lStr_cpy( pchunk->buf8, str );
}

lString8::lString8(const value_type * str, size_type count)
{
    if ( !str || !(*str) || count<=0 )
    {
        pchunk = EMPTY_STR_8; addref();
    }
    else
    {
        size_type len = _lStr_nlen(str, count);
        alloc(len);
        _lStr_ncpy( pchunk->buf8, str, len );
        pchunk->len = len;
    }
}

lString8::lString8(const lString8 & str, size_type offset, size_type count)
{
    if ( count > str.length() - offset )
        count = str.length() - offset;
    if (count<=0)
    {
        pchunk = EMPTY_STR_8; addref();
    }
    else
    {
        alloc(count);
        _lStr_memcpy( pchunk->buf8, str.pchunk->buf8+offset, count );
        pchunk->buf8[count]=0;
        pchunk->len = count;
    }
}

lString8 & lString8::assign(const lChar8 * str)
{
    if (!str || !(*str))
    {
        clear();
    }
    else
    {
        size_type len = _lStr_len(str);
        if (pchunk->nref==1)
        {
            if (pchunk->size<=len)
            {
                // resize is necessary
                pchunk->buf8 = (lChar8*) ::realloc( pchunk->buf8, sizeof(lChar8)*(len+1) );
                pchunk->size = len+1;
            }
        }
        else
        {
            release();
            alloc(len);
        }
        _lStr_cpy( pchunk->buf8, str );
        pchunk->len = len;
    }
    return *this;
}

lString8 & lString8::assign(const lChar8 * str, size_type count)
{
    if ( !str || !(*str) || count<=0 )
    {
        clear();
    }
    else
    {
        size_type len = _lStr_nlen(str, count);
        if (pchunk->nref==1)
        {
            if (pchunk->size<=len)
            {
                // resize is necessary
                pchunk->buf8 = (lChar8*) ::realloc( pchunk->buf8, sizeof(lChar8)*(len+1) );
                pchunk->size = len+1;
            }
        }
        else
        {
            release();
            alloc(len);
        }
        _lStr_ncpy( pchunk->buf8, str, count );
        pchunk->len = len;
    }
    return *this;
}

lString8 & lString8::assign(const lString8 & str, size_type offset, size_type count)
{
    if ( count > str.length() - offset )
        count = str.length() - offset;
    if (count<=0)
    {
        clear();
    }
    else
    {
        if (pchunk==str.pchunk)
        {
            if (&str != this)
            {
                release();
                alloc(count);
            }
            if (offset>0)
            {
                _lStr_memcpy( pchunk->buf8, str.pchunk->buf8+offset, count );
            }
            pchunk->buf8[count]=0;
        }
        else
        {
            if (pchunk->nref==1)
            {
                if (pchunk->size<=count)
                {
                    // resize is necessary
                    pchunk->buf8 = (lChar8*) ::realloc( pchunk->buf8, sizeof(lChar8)*(count+1) );
                    pchunk->size = count+1;
                }
            }
            else
            {
                release();
                alloc(count);
            }
            _lStr_memcpy( pchunk->buf8, str.pchunk->buf8+offset, count );
            pchunk->buf8[count]=0;
        }
        pchunk->len = count;
    }
    return *this;
}

lString8 & lString8::erase(size_type offset, size_type count)
{
    if ( count > length() - offset )
        count = length() - offset;
    if (count<=0)
    {
        clear();
    }
    else
    {
        size_type newlen = length()-count;
        if (pchunk->nref==1)
        {
            _lStr_memcpy( pchunk->buf8+offset, pchunk->buf8+offset+count, newlen-offset+1 );
        }
        else
        {
            lstring_chunk_t * poldchunk = pchunk;
            release();
            alloc( newlen );
            _lStr_memcpy( pchunk->buf8, poldchunk->buf8, offset );
            _lStr_memcpy( pchunk->buf8+offset, poldchunk->buf8+offset+count, newlen-offset+1 );
        }
        pchunk->len = newlen;
        pchunk->buf8[newlen]=0;
    }
    return *this;
}

void lString8::reserve(size_type n)
{
    if (pchunk->nref==1)
    {
        if (pchunk->size < n)
        {
            pchunk->buf8 = (lChar8*) ::realloc( pchunk->buf8, sizeof(lChar8)*(n+1) );
            pchunk->size = n;
        }
    }
    else
    {
        lstring_chunk_t * poldchunk = pchunk;
        release();
        alloc( n );
        _lStr_memcpy( pchunk->buf8, poldchunk->buf8, poldchunk->len+1 );
        pchunk->len = poldchunk->len;
    }
}

void lString8::lock( size_type newsize )
{
    if (pchunk->nref>1)
    {
        lstring_chunk_t * poldchunk = pchunk;
        release();
        alloc( newsize );
        size_type len = newsize;
        if (len>poldchunk->len)
            len = poldchunk->len;
        _lStr_memcpy( pchunk->buf8, poldchunk->buf8, len );
        pchunk->buf8[len]=0;
        pchunk->len = len;
    }
}

// lock string, allocate buffer and reset length to 0
void lString8::reset( size_type size )
{
    if (pchunk->nref>1 || pchunk->size<size)
    {
        release();
        alloc( size );
    }
    pchunk->buf8[0] = 0;
    pchunk->len = 0;
}

void lString8::resize(size_type n, lChar8 e)
{
    lock( n );
    if (n>=pchunk->size)
    {
        pchunk->buf8 = (lChar8*) ::realloc( pchunk->buf8, sizeof(lChar8)*(n+1) );
        pchunk->size = n;
    }
    // fill with data if expanded
    for (size_type i=pchunk->len; i<n; i++)
        pchunk->buf8[i] = e;
    pchunk->buf8[pchunk->len] = 0;
}

lString8 & lString8::append(const lChar8 * str)
{
    size_type len = _lStr_len(str);
    reserve( pchunk->len+len );
    _lStr_memcpy(pchunk->buf8+pchunk->len, str, len+1);
    pchunk->len += len;
    return *this;
}

lString8 & lString8::appendDecimal(lInt64 n)
{
    lChar8 buf[24];
    int i=0;
    int negative = 0;
    if (n==0)
        return append(1, '0');
    else if (n<0)
    {
        negative = 1;
        n = -n;
    }
    for ( ; n; n/=10 )
    {
        buf[i++] = '0' + (n % 10);
    }
    reserve(length() + i + negative);
    if (negative)
        append(1, '-');
    for (int j=i-1; j>=0; j--)
        append(1, buf[j]);
    return *this;
}

lString8 & lString8::appendHex(lUInt64 n)
{
    if (n == 0)
        return append(1, '0');
    reserve(length() + 16);
    bool foundNz = false;
    for (int i=0; i<16; i++) {
        int digit = (n >> 60) & 0x0F;
        if (digit)
            foundNz = true;
        if (foundNz)
            append(1, (lChar8)toHexDigit(digit));
        n >>= 4;
    }
    return *this;
}

lString16 & lString16::appendDecimal(lInt64 n)
{
    lChar16 buf[24];
    int i=0;
    int negative = 0;
    if (n==0)
        return append(1, '0');
    else if (n<0)
    {
        negative = 1;
        n = -n;
    }
    for ( ; n; n/=10 )
    {
        buf[i++] = '0' + (n % 10);
    }
    reserve(length() + i + negative);
    if (negative)
        append(1, '-');
    for (int j=i-1; j>=0; j--)
        append(1, buf[j]);
    return *this;
}

lString16 & lString16::appendHex(lUInt64 n)
{
    if (n == 0)
        return append(1, '0');
    reserve(length() + 16);
    bool foundNz = false;
    for (int i=0; i<16; i++) {
        int digit = (n >> 60) & 0x0F;
        if (digit)
            foundNz = true;
        if (foundNz)
            append(1, toHexDigit(digit));
        n >>= 4;
    }
    return *this;
}

lString8 & lString8::append(const lChar8 * str, size_type count)
{
    size_type len = _lStr_nlen(str, count);
    reserve( pchunk->len+len );
    _lStr_ncpy(pchunk->buf8+pchunk->len, str, len);
    pchunk->len += len;
    return *this;
}

lString8 & lString8::append(const lString8 & str)
{
    size_type len2 = pchunk->len + str.pchunk->len;
    reserve( len2 );
    _lStr_memcpy( pchunk->buf8+pchunk->len, str.pchunk->buf8, str.pchunk->len+1 );
    pchunk->len = len2;
    return *this;
}

lString8 & lString8::append(const lString8 & str, size_type offset, size_type count)
{
    if ( str.pchunk->len>offset )
    {
        if ( offset + count > str.pchunk->len )
            count = str.pchunk->len - offset;
        reserve( pchunk->len+count );
        _lStr_ncpy(pchunk->buf8 + pchunk->len, str.pchunk->buf8 + offset, count);
        pchunk->len += count;
        pchunk->buf8[pchunk->len] = 0;
    }
    return *this;
}

lString8 & lString8::append(size_type count, lChar8 ch)
{
    reserve( pchunk->len+count );
    memset( pchunk->buf8+pchunk->len, ch, count );
    //_lStr_memset(pchunk->buf8+pchunk->len, ch, count);
    pchunk->len += count;
    pchunk->buf8[pchunk->len] = 0;
    return *this;
}

lString8 & lString8::insert(size_type p0, size_type count, lChar8 ch)
{
    if (p0>pchunk->len)
        p0 = pchunk->len;
    reserve( pchunk->len+count );
    for (size_type i=pchunk->len+count; i>p0; i--)
        pchunk->buf8[i] = pchunk->buf8[i-1];
    //_lStr_memset(pchunk->buf8+p0, ch, count);
    memset(pchunk->buf8+p0, ch, count);
    pchunk->len += count;
    pchunk->buf8[pchunk->len] = 0;
    return *this;
}

lString8 lString8::substr(size_type pos, size_type n) const
{
    if (pos>=length())
        return lString8::empty_str;
    if (pos+n>length())
        n = length() - pos;
    return lString8( pchunk->buf8+pos, n );
}

int lString8::pos(const lString8 & subStr) const
{
    if (subStr.length()>length())
        return -1;
    int l = subStr.length();
    int dl = length() - l;
    for (int i=0; i<=dl; i++)
    {
        int flg = 1;
        for (int j=0; j<l; j++)
            if (pchunk->buf8[i+j]!=subStr.pchunk->buf8[j])
            {
                flg = 0;
                break;
            }
        if (flg)
            return i;
    }
    return -1;
}

/// find position of substring inside string starting from right, -1 if not found
int lString8::rpos(const char * subStr) const
{
    if (!subStr || !subStr[0])
        return -1;
    int l = lStr_len(subStr);
    if (l > length())
        return -1;
    int dl = length() - l;
    for (int i=dl; i>=0; i--)
    {
        int flg = 1;
        for (int j=0; j<l; j++)
            if (pchunk->buf8[i+j] != subStr[j])
            {
                flg = 0;
                break;
            }
        if (flg)
            return i;
    }
    return -1;
}

/// find position of substring inside string, -1 if not found
int lString8::pos(const char * subStr) const
{
    if (!subStr || !subStr[0])
        return -1;
    int l = lStr_len(subStr);
    if (l > length())
        return -1;
    int dl = length() - l;
    for (int i=0; i<=dl; i++)
    {
        int flg = 1;
        for (int j=0; j<l; j++)
            if (pchunk->buf8[i+j] != subStr[j])
            {
                flg = 0;
                break;
            }
        if (flg)
            return i;
    }
    return -1;
}

int lString8::pos(const lString8 & subStr, int startPos) const
{
    if (subStr.length() > length() - startPos)
        return -1;
    int l = subStr.length();
    int dl = length() - l;
    for (int i = startPos; i <= dl; i++) {
        int flg = 1;
        for (int j=0; j<l; j++)
            if (pchunk->buf8[i+j]!=subStr.pchunk->buf8[j])
            {
                flg = 0;
                break;
            }
        if (flg)
            return i;
    }
    return -1;
}

int lString16::pos(const lString16 & subStr, int startPos) const
{
    if (subStr.length() > length() - startPos)
        return -1;
    int l = subStr.length();
    int dl = length() - l;
    for (int i = startPos; i <= dl; i++) {
        int flg = 1;
        for (int j=0; j<l; j++)
            if (pchunk->buf16[i+j]!=subStr.pchunk->buf16[j])
            {
                flg = 0;
                break;
            }
        if (flg)
            return i;
    }
    return -1;
}

/// find position of substring inside string, -1 if not found
int lString8::pos(const char * subStr, int startPos) const
{
    if (!subStr || !subStr[0])
        return -1;
    int l = lStr_len(subStr);
    if (l > length() - startPos)
        return -1;
    int dl = length() - l;
    for (int i = startPos; i <= dl; i++) {
        int flg = 1;
        for (int j=0; j<l; j++)
            if (pchunk->buf8[i+j] != subStr[j])
            {
                flg = 0;
                break;
            }
        if (flg)
            return i;
    }
    return -1;
}

/// find position of substring inside string, -1 if not found
int lString16::pos(const lChar16 * subStr, int startPos) const
{
    if (!subStr || !subStr[0])
        return -1;
    int l = lStr_len(subStr);
    if (l > length() - startPos)
        return -1;
    int dl = length() - l;
    for (int i = startPos; i <= dl; i++) {
        int flg = 1;
        for (int j=0; j<l; j++)
            if (pchunk->buf16[i+j] != subStr[j])
            {
                flg = 0;
                break;
            }
        if (flg)
            return i;
    }
    return -1;
}

/// find position of substring inside string, right to left, return -1 if not found
int lString16::rpos(lString16 subStr) const
{
    if (subStr.length()>length())
        return -1;
    int l = subStr.length();
    int dl = length() - l;
    for (int i=dl; i>=0; i++)
    {
        int flg = 1;
        for (int j=0; j<l; j++)
            if (pchunk->buf16[i+j]!=subStr.pchunk->buf16[j])
            {
                flg = 0;
                break;
            }
        if (flg)
            return i;
    }
    return -1;
}

int lString16::rpos(const lChar8 * subStr) const
{
    std::string src(LCSTR(lString16(*this)));
    int pos = src.rfind(subStr);
    if (pos != std::string::npos)
    {
        return pos;
    }
    return -1;
}

/// find position of substring inside string, -1 if not found
int lString16::pos(const lChar16 * subStr) const
{
    if (!subStr)
        return -1;
    int l = lStr_len(subStr);
    if (l > length())
        return -1;
    int dl = length() - l;
    for (int i=0; i <= dl; i++)
    {
        int flg = 1;
        for (int j=0; j<l; j++)
            if (pchunk->buf16[i+j] != subStr[j])
            {
                flg = 0;
                break;
            }
        if (flg)
            return i;
    }
    return -1;
}

/// find position of substring inside string, -1 if not found
int lString16::pos(const lChar8 * subStr) const
{
    if (!subStr)
        return -1;
    int l = lStr_len(subStr);
    if (l > length())
        return -1;
    int dl = length() - l;
    for (int i=0; i <= dl; i++)
    {
        int flg = 1;
        for (int j=0; j<l; j++)
            if (pchunk->buf16[i+j] != subStr[j])
            {
                flg = 0;
                break;
            }
        if (flg)
            return i;
    }
    return -1;
}

/// find position of substring inside string, -1 if not found
int lString16::pos(const lChar8 * subStr, int start) const
{
    if (!subStr)
        return -1;
    int l = lStr_len(subStr);
    if (l > length() - start)
        return -1;
    int dl = length() - l;
    for (int i = start; i <= dl; i++)
    {
        int flg = 1;
        for (int j=0; j<l; j++)
            if (pchunk->buf16[i+j] != subStr[j])
            {
                flg = 0;
                break;
            }
        if (flg)
            return i;
    }
    return -1;
}

int lString16::pos(lString16 subStr) const
{
    if (subStr.length()>length())
        return -1;
    int l = subStr.length();
    int dl = length() - l;
    for (int i=0; i<=dl; i++)
    {
        int flg = 1;
        for (int j=0; j<l; j++)
            if (pchunk->buf16[i+j]!=subStr.pchunk->buf16[j])
            {
                flg = 0;
                break;
            }
        if (flg)
            return i;
    }
    return -1;
}

lString8 & lString8::pack()
{
    if (pchunk->len + 4 < pchunk->size )
    {
        if (pchunk->nref>1)
        {
            lock(pchunk->len);
        }
        else
        {
            pchunk->buf8 = (lChar8 *) realloc( pchunk->buf8, sizeof(lChar8)*(pchunk->len+1) );
            pchunk->size = pchunk->len;
        }
    }
    return *this;
}

lString8 & lString8::trim()
{
    //
    int firstns;
    for (firstns = 0;
            firstns < pchunk->len &&
            (pchunk->buf8[firstns] == ' ' ||
            pchunk->buf8[firstns] == '\t');
            ++firstns)
        ;
    if (firstns >= pchunk->len)
    {
        clear();
        return *this;
    }
    size_t lastns;
    for (lastns = pchunk->len-1;
            lastns>0 &&
            (pchunk->buf8[lastns]==' ' || pchunk->buf8[lastns]=='\t');
            --lastns)
        ;
    int newlen = (int)(lastns - firstns + 1);
    if (newlen == pchunk->len)
        return *this;
    if (pchunk->nref == 1)
    {
        if (firstns>0)
            lStr_memcpy( pchunk->buf8, pchunk->buf8+firstns, newlen );
        pchunk->buf8[newlen] = 0;
        pchunk->len = newlen;
    }
    else
    {
        lstring_chunk_t * poldchunk = pchunk;
        release();
        alloc( newlen );
        _lStr_memcpy( pchunk->buf8, poldchunk->buf8+firstns, newlen );
        pchunk->buf8[newlen] = 0;
        pchunk->len = newlen;
    }
    return *this;
}

int lString8::atoi() const
{
    int sgn = 1;
    int n = 0;
    const lChar8 * s = c_str();
    while (*s == ' ' || *s == '\t')
        s++;
    if (*s == '-')
    {
        sgn = -1;
        s++;
    }
    else if (*s == '+')
    {
        s++;
    }
    while (*s>='0' && *s<='9')
    {
        n = n * 10 + ( (*s)-'0' );
        s++;
    }
    return (sgn>0)?n:-n;
}

lInt64 lString8::atoi64() const
{
    int sgn = 1;
    lInt64 n = 0;
    const lChar8 * s = c_str();
    while (*s == ' ' || *s == '\t')
        s++;
    if (*s == '-')
    {
        sgn = -1;
        s++;
    }
    else if (*s == '+')
    {
        s++;
    }
    while (*s>='0' && *s<='9')
    {
        n = n * 10 + ( (*s)-'0' );
    }
    return (sgn>0) ? n : -n;
}

// constructs string representation of integer
lString8 lString8::itoa( int n )
{
    lChar8 buf[16];
    int i=0;
    int negative = 0;
    if (n==0)
        return cs8("0");
    else if (n<0)
    {
        negative = 1;
        n = -n;
    }
    for ( ; n; n/=10 )
    {
        buf[i++] = '0' + (n%10);
    }
    lString8 res;
    res.reserve(i+negative);
    if (negative)
        res.append(1, '-');
    for (int j=i-1; j>=0; j--)
        res.append(1, buf[j]);
    return res;
}

// constructs string representation of integer
lString8 lString8::itoa( unsigned int n )
{
    lChar8 buf[16];
    int i=0;
    if (n==0)
        return cs8("0");
    for ( ; n; n/=10 )
    {
        buf[i++] = '0' + (n%10);
    }
    lString8 res;
    res.reserve(i);
    for (int j=i-1; j>=0; j--)
        res.append(1, buf[j]);
    return res;
}

// constructs string representation of integer
lString8 lString8::itoa( lInt64 n )
{
    lChar8 buf[32];
    int i=0;
    int negative = 0;
    if (n==0)
        return cs8("0");
    else if (n<0)
    {
        negative = 1;
        n = -n;
    }
    for ( ; n; n/=10 )
    {
        buf[i++] = '0' + (n%10);
    }
    lString8 res;
    res.reserve(i+negative);
    if (negative)
        res.append(1, '-');
    for (int j=i-1; j>=0; j--)
        res.append(1, buf[j]);
    return res;
}

// constructs string representation of integer
lString16 lString16::itoa( int n )
{
    return itoa( (lInt64)n );
}

// constructs string representation of integer
lString16 lString16::itoa( lInt64 n )
{
    lChar16 buf[32];
    int i=0;
    int negative = 0;
    if (n==0)
        return cs16("0");
    else if (n<0)
    {
        negative = 1;
        n = -n;
    }
    for ( ; n && i<30; n/=10 )
    {
        buf[i++] = (lChar16)('0' + (n%10));
    }
    lString16 res;
    res.reserve(i+negative);
    if (negative)
        res.append(1, L'-');
    for (int j=i-1; j>=0; j--)
        res.append(1, buf[j]);
    return res;
}

bool lvUnicodeIsAlpha( lChar16 ch )
{
    if ( ch<128 ) {
        if ( (ch>='a' && ch<='z') || (ch>='A' && ch<='Z') )
            return true;
    } else if ( ch>=0xC0 && ch<=0x1ef9) {
        return true;
    }
    return false;
}


lString16 & lString16::uppercase()
{
    lStr_uppercase( modify(), length() );
    return *this;
}

lString16 & lString16::lowercase()
{
    lStr_lowercase( modify(), length() );
    return *this;
}

void lStr_uppercase( lChar16 * str, int len )
{
    for ( int i=0; i<len; i++ ) {
        lChar16 ch = str[i];
        if ( ch>='a' && ch<='z' ) {
            str[i] = ch - 0x20;
        } else if ( ch>=0xE0 && ch<=0xFF ) {
            str[i] = ch - 0x20;
        } else if ( ch>=0x430 && ch<=0x44F ) {// cyrillic
            str[i] = ch - 0x20;
        } else if( ch == 0x451) { //cyrillic "Ё"
            str[i] = 0x401;
        } else if ( ch>=0x3b0 && ch<=0x3cF ) {
            str[i] = ch - 0x20;
        } else if ( (ch >> 8)==0x1F ) { // greek
            lChar16 n = ch & 255;
            if (n<0x70) {
                str[i] = ch | 8;
            } else if (n<0x80) {

            } else if (n<0xF0) {
                str[i] = ch | 8;
            }
        } else if(ch >= 0x561 && ch <= 0x586) { // armenian
            str[i] = ch - 0x30;
        } else if ((ch >= 0x10D0 && ch <= 0x10F5)|| ch == 0x10F7 || ch == 0x10FD ) {  // georgian
            str[i] = ch - 0x30;
        }
    }
}

void lStr_lowercase( lChar16 * str, int len )
{
    for ( int i=0; i<len; i++ ) {
        lChar16 ch = str[i];
        if ( ch>='A' && ch<='Z' ) {
            str[i] = ch + 0x20;
        } else if ( ch>=0xC0 && ch<=0xDF ) {
            str[i] = ch + 0x20;
        } else if ( ch>=0x410 && ch<=0x42F ) {  //cyrillic
            str[i] = ch + 0x20;
        } else if( ch == 0x401) { //cyrillic "Ё"
            str[i] = 0x451;
        } else if ( ch>=0x390 && ch<=0x3aF ) { // Greek
            str[i] = ch + 0x20;
        } else if ( (ch >> 8)==0x1F ) { // greek
            lChar16 n = ch & 255;
            if (n<0x70) {
                str[i] = ch & (~8);
            } else if (n<0x80) {

            } else if (n<0xF0) {
                str[i] = ch & (~8);
            }
        }
        else if(ch >= 0x531 && ch <= 0x556) {  // armenian
            str[i] = ch + 0x30;
        } else if ((ch >= 0x10A0 && ch <= 0x10C5)|| ch == 0x10C7 || ch == 0x10CD ) {  // georgian
            str[i] = ch + 0x30;
        }
    }
}

void lString16Collection::parse( lString16 string, lChar16 delimiter, bool flgTrim )
{
    int wstart=0;
    for ( int i=0; i<=string.length(); i++ ) {
        if ( i==string.length() || string[i]==delimiter ) {
            lString16 s( string.substr( wstart, i-wstart) );
            if ( flgTrim )
                s.trimDoubleSpaces(false, false, false);
            if ( !flgTrim || !s.empty() )
                add( s );
            wstart = i+1;
        }
    }
}

void lString16Collection::parse( lString16 string, lString16 delimiter, bool flgTrim )
{
    if ( delimiter.empty() || string.pos(delimiter)<0 ) {
        lString16 s( string );
        if ( flgTrim )
            s.trimDoubleSpaces(false, false, false);
        add(s);
        return;
    }
    int wstart=0;
    for ( int i=0; i<=string.length(); i++ ) {
        bool matched = true;
        for ( int j=0; j<delimiter.length() && i+j<string.length(); j++ ) {
            if ( string[i+j]!=delimiter[j] ) {
                matched = false;
                break;
            }
        }
        if ( matched ) {
            lString16 s( string.substr( wstart, i-wstart) );
            if ( flgTrim )
                s.trimDoubleSpaces(false, false, false);
            if ( !flgTrim || !s.empty() )
                add( s );
            wstart = i+delimiter.length();
            i+= delimiter.length()-1;
        }
    }
}

int TrimDoubleSpaces(lChar16 * buf, int len,  bool allowStartSpace, bool allowEndSpace, bool removeEolHyphens)
{
    lChar16 * psrc = buf;
    lChar16 * pdst = buf;
    int state = 0; // 0=beginning, 1=after space, 2=after non-space
    while ((len--) > 0) {
        lChar16 ch = *psrc++;
        if (ch == ' ' || ch == '\t') {
            if ( state==2 ) {
                if ( *psrc || allowEndSpace ) // if not last
                    *pdst++ = ' ';
            } else if ( state==0 && allowStartSpace ) {
                *pdst++ = ' ';
            }
            state = 1;
        } else if ( ch=='\r' || ch=='\n' ) {
            if ( state==2 ) {
                if ( removeEolHyphens && pdst>(buf+1) && *(pdst-1)=='-' && lvUnicodeIsAlpha(*(pdst-2)) )
                    pdst--; // remove hyphen at end of line
                if ( *psrc || allowEndSpace ) // if not last
                    *pdst++ = ' ';
            } else if ( state==0 && allowStartSpace ) {
                *pdst++ = ' ';
            }
            state = 1;
        } else {
            *pdst++ = ch;
            state = 2;
        }
    }
    return (int) (pdst - buf);
}

lString16 & lString16::trimDoubleSpaces( bool allowStartSpace, bool allowEndSpace, bool removeEolHyphens )
{
    if ( empty() )
        return *this;
    lChar16 * buf = modify();
    int len = length();
    int nlen = TrimDoubleSpaces(buf, len,  allowStartSpace, allowEndSpace, removeEolHyphens);
    if (nlen < len)
        limit(nlen);
    return *this;
}

// constructs string representation of integer
lString16 lString16::itoa(unsigned int n)
{
    return itoa( (lUInt64) n );
}

// constructs string representation of integer
lString16 lString16::itoa( lUInt64 n )
{
    lChar16 buf[24];
    int i=0;
    if (n==0)
        return cs16("0");
    for ( ; n; n/=10 )
    {
        buf[i++] = (lChar16)('0' + (n%10));
    }
    lString16 res;
    res.reserve(i);
    for (int j=i-1; j>=0; j--)
        res.append(1, buf[j]);
    return res;
}


lUInt32 lString8::getHash() const
{
    lUInt32 res = 0;
    for (int i=0; i < pchunk->len; i++)
        res = res * STRING_HASH_MULT + pchunk->buf8[i];
    return res;
}

const lString8 lString8::empty_str;

int Utf8CharCount(const lChar8 * str)
{
    int count = 0;
    lUInt8 ch;
    while ((ch=*str++)) {
        if ((ch & 0x80) == 0) {

        } else if ((ch & 0xE0) == 0xC0) {
            if (!(*str++) )
                break;
        } else if ((ch & 0xF0) == 0xE0) {
            if (!(*str++))
                break;
            if (!(*str++))
                break;
        } else if ((ch & 0xF8) == 0xF0) {
            if (!(*str++))
                break;
            if (!(*str++))
                break;
            if (!(*str++))
                break;
        } else if ((ch & 0xFC) == 0xF8) {
            if (!(*str++))
                break;
            if (!(*str++))
                break;
            if (!(*str++))
                break;
            if (!(*str++))
                break;
        } else {
            if (!(*str++))
                break;
            if (!(*str++))
                break;
            if (!(*str++))
                break;
            if (!(*str++))
                break;
            if (!(*str++))
                break;
        }
        count++;
    }
    return count;
}

int Utf8CharCount(const lChar8 * str, int len)
{
    if (len == 0)
        return 0;
    int count = 0;
    lUInt8 ch;
    const lChar8 * endp = str + len;
    while ((ch=*str++)) {
        if ( (ch & 0x80) == 0 ) {
        } else if ( (ch & 0xE0) == 0xC0 ) {
            str++;
        } else if ( (ch & 0xF0) == 0xE0 ) {
            str+=2;
        } else if ( (ch & 0xF8) == 0xF0 ) {
            str+=3;
        } else if ( (ch & 0xFC) == 0xF8 ) {
            str+=4;
        } else {
            str+=5;
        }
        if (str > endp)
            break;
        count++;
    }
    return count;
}

inline int charUtf8ByteCount(int ch) {
    if (!(ch & ~0x7F))
        return 1;
    if (!(ch & ~0x7FF))
        return 2;
    if (!(ch & ~0xFFFF))
        return 3;
    if (!(ch & ~0x1FFFFF))
        return 4;
    if (!(ch & ~0x3FFFFFF))
        return 5;
    return 6;
}

int Utf8ByteCount(const lChar16* str)
{
    int count = 0;
    lUInt32 ch;
    while ((ch=*str++)) {
        count += charUtf8ByteCount(ch);
    }
    return count;
}

int Utf8ByteCount(const lChar16* str, int len)
{
    int count = 0;
    lUInt32 ch;
    while ((len--) > 0) {
        ch = *str++;
        count += charUtf8ByteCount(ch);
    }
    return count;
}

lString16 Utf8ToUnicode(const lString8 & str)
{
	return Utf8ToUnicode(str.c_str());
}

#define CONT_BYTE(index,shift) (((lChar16)(s[index]) & 0x3F) << shift)

static void DecodeUtf8(const char* s,  lChar16* p, int len)
{
    lChar16 * endp = p + len;
    lUInt32 ch;
    while (p < endp) {
        ch = *s++;
        if ( (ch & 0x80) == 0 ) {
        	*p++ = (char)ch;
        } else if ( (ch & 0xE0) == 0xC0 ) {
            *p++ = ((ch & 0x1F) << 6)
                    | CONT_BYTE(0,0);
            s++;
        } else if ( (ch & 0xF0) == 0xE0 ) {
            *p++ = ((ch & 0x0F) << 12)
                | CONT_BYTE(0,6)
                | CONT_BYTE(1,0);
            s += 2;
        } else if ( (ch & 0xF8) == 0xF0 ) {
            *p++ = ((ch & 0x07) << 18)
                | CONT_BYTE(0,12)
                | CONT_BYTE(1,6)
                | CONT_BYTE(2,0);
            s += 3;
        } else if ( (ch & 0xFC) == 0xF8 ) {
            *p++ = ((ch & 0x03) << 24)
                | CONT_BYTE(0,18)
                | CONT_BYTE(1,12)
                | CONT_BYTE(2,6)
                | CONT_BYTE(3,0);
            s += 4;
        } else {
            *p++ = ((ch & 0x01) << 30)
                | CONT_BYTE(0,24)
                | CONT_BYTE(1,18)
                | CONT_BYTE(2,12)
                | CONT_BYTE(3,6)
                | CONT_BYTE(4,0);
            s += 5;
        }
    }
}

void Utf8ToUnicode(const lUInt8 * src,  int &srclen, lChar16 * dst, int &dstlen)
{
    const lUInt8 * s = src;
    const lUInt8 * ends = s + srclen;
    lChar16 * p = dst;
    lChar16 * endp = p + dstlen;
    lUInt32 ch;
    while (p < endp && s < ends) {
        ch = *s;
        if ( (ch & 0x80) == 0 ) {
            *p++ = (char)ch;
            s++;
        } else if ( (ch & 0xE0) == 0xC0 ) {
            if (s + 2 > ends)
                break;
            *p++ = ((ch & 0x1F) << 6)
                    | CONT_BYTE(1,0);
            s += 2;
        } else if ( (ch & 0xF0) == 0xE0 ) {
            if (s + 3 > ends)
                break;
            *p++ = ((ch & 0x0F) << 12)
                | CONT_BYTE(1,6)
                | CONT_BYTE(2,0);
            s += 3;
        } else if ( (ch & 0xF8) == 0xF0 ) {
            if (s + 4 > ends)
                break;
            *p++ = ((ch & 0x07) << 18)
                | CONT_BYTE(1,12)
                | CONT_BYTE(2,6)
                | CONT_BYTE(3,0);
            s += 4;
        } else if ( (ch & 0xFC) == 0xF8 ) {
            if (s + 5 > ends)
                break;
            *p++ = ((ch & 0x03) << 24)
                | CONT_BYTE(1,18)
                | CONT_BYTE(2,12)
                | CONT_BYTE(3,6)
                | CONT_BYTE(4,0);
            s += 5;
        } else {
            if (s + 6 > ends)
                break;
            *p++ = ((ch & 0x01) << 30)
                | CONT_BYTE(1,24)
                | CONT_BYTE(2,18)
                | CONT_BYTE(3,12)
                | CONT_BYTE(4,6)
                | CONT_BYTE(5,0);
            s += 6;
        }
    }
    srclen = (int)(s - src);
    dstlen = (int)(p - dst);
}

lString16 Utf8ToUnicode(const char* s) {
    if (!s || !s[0])
      return lString16::empty_str;
    int len = Utf8CharCount(s);
    if (!len)
      return lString16::empty_str;
    lString16 dst;
    dst.append(len, (lChar16)0);
    lChar16 * p = dst.modify();
    DecodeUtf8(s, p, len);
    return dst;
}

lString16 Utf8ToUnicode(const char* s, int sz)
{
    if (!s || !s[0] || sz <= 0)
      return lString16::empty_str;
    int len = Utf8CharCount( s, sz );
    if (!len)
      return lString16::empty_str;
    lString16 dst;
    dst.append(len, 0);
    lChar16 * p = dst.modify();
    DecodeUtf8(s, p, len);
    return dst;
}


lString8 UnicodeToUtf8(const lChar16* s, int count)
{
    if (count <= 0)
      return lString8::empty_str;
    lString8 dst;
    int len = Utf8ByteCount(s, count);
    if (len <= 0)
      return lString8::empty_str;
    dst.append(len, ' ');
    lChar8 * buf = dst.modify(); {
        lUInt32 ch;
        while ((count--) > 0) {
            ch = *s++;
            if (!(ch & ~0x7F)) {
                *buf++ = ((lUInt8)ch);
            } else if (!(ch & ~0x7FF)) {
                *buf++ = ( (lUInt8) ( ((ch >> 6) & 0x1F) | 0xC0 ) );
                *buf++ = ( (lUInt8) ( ((ch ) & 0x3F) | 0x80 ) );
            } else if (!(ch & ~0xFFFF)) {
                *buf++ = ( (lUInt8) ( ((ch >> 12) & 0x0F) | 0xE0 ) );
                *buf++ = ( (lUInt8) ( ((ch >> 6) & 0x3F) | 0x80 ) );
                *buf++ = ( (lUInt8) ( ((ch ) & 0x3F) | 0x80 ) );
            } else if (!(ch & ~0x1FFFFF)) {
                *buf++ = ( (lUInt8) ( ((ch >> 18) & 0x07) | 0xF0 ) );
                *buf++ = ( (lUInt8) ( ((ch >> 12) & 0x3F) | 0x80 ) );
                *buf++ = ( (lUInt8) ( ((ch >> 6) & 0x3F) | 0x80 ) );
                *buf++ = ( (lUInt8) ( ((ch ) & 0x3F) | 0x80 ) );
            } else if (!(ch & ~0x3FFFFFF)) {
                *buf++ = ( (lUInt8) ( ((ch >> 24) & 0x03) | 0xF8 ) );
                *buf++ = ( (lUInt8) ( ((ch >> 18) & 0x3F) | 0x80 ) );
                *buf++ = ( (lUInt8) ( ((ch >> 12) & 0x3F) | 0x80 ) );
                *buf++ = ( (lUInt8) ( ((ch >> 6) & 0x3F) | 0x80 ) );
                *buf++ = ( (lUInt8) ( ((ch ) & 0x3F) | 0x80 ) );
            } else {
                *buf++ = ( (lUInt8) ( ((ch >> 30) & 0x01) | 0xFC ) );
                *buf++ = ( (lUInt8) ( ((ch >> 24) & 0x3F) | 0x80 ) );
                *buf++ = ( (lUInt8) ( ((ch >> 18) & 0x3F) | 0x80 ) );
                *buf++ = ( (lUInt8) ( ((ch >> 12) & 0x3F) | 0x80 ) );
                *buf++ = ( (lUInt8) ( ((ch >> 6) & 0x3F) | 0x80 ) );
                *buf++ = ( (lUInt8) ( ((ch ) & 0x3F) | 0x80 ) );
            }
        }
    }
    return dst;
}

lString8 UnicodeToUtf8(const lString16& str)
{
    return UnicodeToUtf8(str.c_str(), str.length());
}

lString8 UnicodeTo8Bit(const lString16& str, const lChar8** table)
{
    lString8 buf;
    buf.reserve( str.length() );
    for (int i=0; i < str.length(); i++) {
        lChar16 ch = str[i];
        const lChar8 * p = table[ (ch>>8) & 255 ];
        if ( p ) {
            buf += p[ ch&255 ];
        } else {
            buf += '?';
        }
    }
    return buf;
}

lString16 ByteToUnicode(const lString8& str, const lChar16* table)
{
    lString16 buf;
    buf.reserve( str.length() );
    for (int i=0; i < str.length(); i++) {
        lChar16 ch = (unsigned char)str[i];
        lChar16 ch16 = ((ch & 0x80) && table) ? table[ (ch&0x7F) ] : ch;
        buf += ch16;
    }
    return buf;
}

lString8 UnicodeToLocal( const lString16 & str )
{
    return UnicodeToUtf8( str );
}

lString16 LocalToUnicode( const lString8 & str )
{
    return Utf8ToUnicode( str );
}


static const char * getCharTranscript( lChar16 ch )
{
    if ( ch>=0x410 && ch<0x430 )
        return russian_capital[ch-0x410];
    else if (ch>=0x430 && ch<0x450)
        return russian_small[ch-0x430];
    else if (ch>=0xC0 && ch<0xFF)
        return latin_1[ch-0xC0];
    else if (ch==0x450)
        return "E";
    else if ( ch==0x451 )
        return "e";
    return "?";
}


lString8  UnicodeToTranslit( const lString16 & str )
{
    lString8 buf;
	if ( str.empty() )
		return buf;
    buf.reserve( str.length()*5/4 );
    for ( int i=0; i<str.length(); i++ ) {
		lChar16 ch = str[i];
        if ( ch>=32 && ch<=127 ) {
            buf.append( 1, (lChar8)ch );
        } else {
            const char * trans = getCharTranscript(ch);
            buf.append( trans );
        }
	}
    buf.pack();
    return buf;
}

inline lUInt16 getCharProp(lChar16 ch) {
    //static const lChar16 maxchar = sizeof(char_props) / sizeof( lUInt16 );
    static const lChar16 maxchar = char_props_count;
    if (ch<maxchar)
        return char_props[ch];
    else if ((ch>>8) == 0x1F)
        return char_props_1f00[ch & 255];
    else if (ch>=0x2012 && ch<=0x2015)
        return CH_PROP_DASH|CH_PROP_SIGN;
    else if (ch==0x2026) //add symbols here if not caught
        return CH_PROP_SIGN;
    return CH_PROP_HIEROGLYPH;
    //return 0;
}

void lStr_getCharProps( const lChar16 * str, int sz, lUInt16 * props )
{
    for ( int i=0; i<sz; i++ ) {
        lChar16 ch = str[i];
        props[i] = getCharProp(ch);
    }
}

/// find alpha sequence bounds
void lStr_findWordBounds( const lChar16 * str, int sz, int pos, int & start, int & end )
{
    int hwStart, hwEnd;

//    // skip spaces
//    for (hwStart=pos-1; hwStart>0; hwStart--)
//    {
//        lChar16 ch = str[hwStart];
//        if ( ch<(int)maxchar ) {
//            lUInt16 props = char_props[ch];
//            if ( !(props & CH_PROP_SPACE) )
//                break;
//        }
//    }
//    // skip punctuation signs and digits
//    for (; hwStart>0; hwStart--)
//    {
//        lChar16 ch = str[hwStart];
//        if ( ch<(int)maxchar ) {
//            lUInt16 props = char_props[ch];
//            if ( !(props & (CH_PROP_PUNCT|CH_PROP_DIGIT)) )
//                break;
//        }
//    }
    // skip until first alpha
    for (hwStart = pos-1; hwStart > 0; hwStart--)
    {
        lChar16 ch = str[hwStart];
        lUInt16 props = getCharProp(ch);
        if ( props & CH_PROP_ALPHA )
            break;
    }
    if ( hwStart<0 ) {
        // no alphas found
        start = end = pos;
        return;
    }
    hwEnd = hwStart+1;
    // skipping while alpha
    for (; hwStart>0; hwStart--)
    {
        lChar16 ch = str[hwStart];
        //int lastAlpha = -1;
        if (getCharProp(ch) & CH_PROP_ALPHA) {
            //lastAlpha = hwStart;
        } else {
            hwStart++;
            break;
        }
    }
//    if ( lastAlpha<0 ) {
//        // no alphas found
//        start = end = pos;
//        return;
//    }
    for (hwEnd=hwStart+1; hwEnd<sz; hwEnd++) // 20080404
    {
        lChar16 ch = str[hwEnd];
        if (!(getCharProp(ch) & CH_PROP_ALPHA))
            break;
        ch = str[hwEnd-1];
        if ( (ch==' ' || ch==UNICODE_SOFT_HYPHEN_CODE) )
            break;
    }
    start = hwStart;
    end = hwEnd;
    //CRLog::debug("Word bounds: '%s'", LCSTR(lString16(str+start, end-start)));
}

void  lString16::limit( size_type sz )
{
    if ( length() > sz ) {
        modify();
        pchunk->len = sz;
        pchunk->buf16[sz] = 0;
    }
}

lUInt16 lGetCharProps( lChar16 ch )
{
    return getCharProp(ch);
}

/// returns true if string starts with specified substring, case insensitive
bool lString16::startsWithNoCase ( const lString16 & substring ) const
{
    lString16 a = *this;
    lString16 b = substring;
    a.uppercase();
    b.uppercase();
    return a.startsWith( b );
}

/// returns true if string starts with specified substring
bool lString8::startsWith( const char * substring ) const
{
    if (!substring || !substring[0])
        return true;
    int len = (int)strlen(substring);
    if (length() < len)
        return false;
    const lChar8 * s1 = c_str();
    const lChar8 * s2 = substring;
    for (int i=0; i<len; i++ )
        if ( s1[i] != s2[i] )
            return false;
    return true;
}

/// returns true if string starts with specified substring
bool lString8::startsWith( const lString8 & substring ) const
{
    if ( substring.empty() )
        return true;
    int len = substring.length();
    if (length() < len)
        return false;
    const lChar8 * s1 = c_str();
    const lChar8 * s2 = substring.c_str();
    for (int i=0; i<len; i++ )
        if ( s1[i] != s2[i] )
            return false;
    return true;
}

/// returns true if string ends with specified substring
bool lString8::endsWith( const lChar8 * substring ) const
{
	if ( !substring || !*substring )
		return true;
    int len = (int)strlen(substring);
    if ( length() < len )
        return false;
    const lChar8 * s1 = c_str() + (length()-len);
    const lChar8 * s2 = substring;
	return lStr_cmp( s1, s2 )==0;
}

/// returns true if string ends with specified substring
bool lString16::endsWith( const lChar16 * substring ) const
{
	if ( !substring || !*substring )
		return true;
    int len = lStr_len(substring);
    if ( length() < len )
        return false;
    const lChar16 * s1 = c_str() + (length()-len);
    const lChar16 * s2 = substring;
	return lStr_cmp( s1, s2 )==0;
}

/// returns true if string ends with specified substring
bool lString16::endsWith( const lChar8 * substring ) const
{
    if ( !substring || !*substring )
        return true;
    int len = lStr_len(substring);
    if ( length() < len )
        return false;
    const lChar16 * s1 = c_str() + (length()-len);
    const lChar8 * s2 = substring;
    return lStr_cmp( s1, s2 )==0;
}

/// returns true if string ends with specified substring
bool lString16::endsWith ( const lString16 & substring ) const
{
    if ( substring.empty() )
        return true;
    int len = substring.length();
    if ( length() < len )
        return false;
    const lChar16 * s1 = c_str() + (length()-len);
    const lChar16 * s2 = substring.c_str();
	return lStr_cmp( s1, s2 )==0;
}

/// returns true if string starts with specified substring
bool lString16::startsWith( const lString16 & substring ) const
{
    if ( substring.empty() )
        return true;
    int len = substring.length();
    if ( length() < len )
        return false;
    const lChar16 * s1 = c_str();
    const lChar16 * s2 = substring.c_str();
    for ( int i=0; i<len; i++ )
        if ( s1[i]!=s2[i] )
            return false;
    return true;
}

/// returns true if string starts with specified substring
bool lString16::startsWith(const lChar16 * substring) const
{
    if (!substring || !substring[0])
        return true;
    int len = _lStr_len(substring);
    if ( length() < len )
        return false;
    const lChar16 * s1 = c_str();
    const lChar16 * s2 = substring;
    for ( int i=0; i<len; i++ )
        if ( s1[i] != s2[i] )
            return false;
    return true;
}

/// returns true if string starts with specified substring
bool lString16::startsWith(const lChar8 * substring) const
{
    if (!substring || !substring[0])
        return true;
    int len = _lStr_len(substring);
    if ( length() < len )
        return false;
    const lChar16 * s1 = c_str();
    const lChar8 * s2 = substring;
    for ( int i=0; i<len; i++ )
        if (s1[i] != s2[i])
            return false;
    return true;
}

//removes UFFD (unicode questionmark) from the end of the line.
lString16 lString16::TrimEndQuestionChar(lString16 & str){
    lString16 uffd;
    uffd.append(1,L'\ufffd');
        if(str.endsWith(uffd))
    {
        str = lString16(str,0,str.length()-1);
    }
    //CRLog::debug("TrimEndQUestionChar: %s", UnicodeToUtf8(str).c_str());
    return str;
}
//converting all different spaces to one type space
lString16 lString16::ReplaceUnusualSpaces()
{
    lString16 buffer = *this;
    for (int i = 0; i <buffer.length() ; ++i)
    {
        lChar16 ch = buffer.at(i);
        if ((ch == L'\u00A0')
            || (ch == L'\u180E')
            || ((ch >= L'\u2000') && (ch <= L'\u200B'))
            || (ch == L'\u202F')
            || (ch == L'\u205F')
            || (ch == L'\u3000')
            || (ch == L'\uFEFF'))
        {
            buffer.at(i) = L' ';
        }
    }
    return buffer;
}

bool lString16::DigitsOnly() {
    for (int i = 0; i < this->length(); ++i) {
        lChar16 ch = this->at(i);
        if ( ch < 48 && ch != 45 && ch != 32 )
        {
            return false;
        }
        else if(ch > 57)
        {
            return false;
        }
    }
    return true;
}


bool lString16::split2( const lString16 & delim, lString16 & value1, lString16 & value2 )
{
    if ( empty() )
        return false;
    int p = pos(delim);
    if ( p<=0 || p>=length()-delim.length() )
        return false;
    value1 = substr(0, p);
    value2 = substr(p+delim.length());
    return true;
}

bool lString16::split2( const lChar16 * delim, lString16 & value1, lString16 & value2 )
{
    if (empty())
        return false;
    int p = pos(delim);
    int l = lStr_len(delim);
    if (p<=0 || p >= length() - l)
        return false;
    value1 = substr(0, p);
    value2 = substr(p + l);
    return true;
}

bool lString16::split2( const lChar8 * delim, lString16 & value1, lString16 & value2 )
{
    if (empty())
        return false;
    int p = pos(delim);
    int l = lStr_len(delim);
    if (p<=0 || p >= length() - l)
        return false;
    value1 = substr(0, p);
    value2 = substr(p + l);
    return true;
}

bool splitIntegerList( lString16 s, lString16 delim, int &value1, int &value2 )
{
    if ( s.empty() )
        return false;
    lString16 s1, s2;
    if ( !s.split2( delim, s1, s2 ) )
        return false;
    int n1, n2;
    if ( !s1.atoi(n1) )
        return false;
    if ( !s2.atoi(n2) )
        return false;
    value1 = n1;
    value2 = n2;
    return true;
}

lString8 & lString8::replace(size_type p0, size_type n0, const lString8 & str) {
    lString8 s1 = substr( 0, p0 );
    lString8 s2 = length() - p0 - n0 > 0 ? substr( p0+n0, length()-p0-n0 ) : lString8::empty_str;
    *this = s1 + str + s2;
    return *this;
}

lString16 & lString16::replace(size_type p0, size_type n0, const lString16 & str)
{
    lString16 s1 = substr( 0, p0 );
    lString16 s2 = length() - p0 - n0 > 0 ? substr( p0+n0, length()-p0-n0 ) : lString16::empty_str;
    *this = s1 + str + s2;
    return *this;
}

/// replaces part of string, if pattern is found
bool lString16::replace(const lString16 & findStr, const lString16 & replaceStr)
{
    int p = pos(findStr);
    if ( p<0 )
        return false;
    *this = replace( p, findStr.length(), replaceStr );
    return true;
}

lString16 lString16::replaceAllPunctuation(const lString16 & replaceStr)
{
    int TRFLAGS = 0 ;
    TRFLAGS |= CH_PROP_ALPHA;
    TRFLAGS |= CH_PROP_SPACE;
    TRFLAGS |= CH_PROP_HIEROGLYPH;

    for (int i = 0; i < this->length(); i++)
    {
        lChar16 ch = this->at(i);
        int alpha = lGetCharProps(ch) & TRFLAGS; //  words check here
        if (!alpha)
        {
            this->replace(i,1,replaceStr);
        }
        if(ch == L'\n')
        {
            this->replace(i,1,replaceStr);
        }
    }
    return *this;
}

bool lString16::CheckIfHasPunctuation()
{
    int TRFLAGS = 0;
    TRFLAGS |= CH_PROP_ALPHA;
    TRFLAGS |= CH_PROP_SPACE;
    TRFLAGS |= CH_PROP_HIEROGLYPH;

    if (this->length() > 1)
    {
        return false;
    }

    lChar16 ch = this->at(0);
    int alpha = lGetCharProps(ch) & TRFLAGS; //  words check here
    if (!alpha)
    {
        return true;
    }
    return false;
}

lString16 lString16::AddtoAllPunctuation(const lString16 & addStrBefore, const lString16 & addStrAfter)
{
    int TRFLAGS = 0 ;
    TRFLAGS |= CH_PROP_ALPHA;
    TRFLAGS |= CH_PROP_SPACE;
    TRFLAGS |= CH_PROP_HIEROGLYPH;

    for (int i = 0; i < this->length(); i++)
    {
        lChar16 ch = this->at(i);
        int alpha = lGetCharProps(ch) & TRFLAGS; //  words check here
        if (!alpha || ch == L'\n')
        {
            this->insert(i+1,addStrAfter);
            this->insert(i,addStrBefore);
            i+=addStrBefore.length();
            i+=addStrAfter.length();
        }
    }
    return *this;
}

bool lString16::replaceParam(int index, const lString16 & replaceStr)
{
    return replace( cs16("$") + fmt::decimal(index), replaceStr );
}

/// replaces first found occurence of "$N" pattern with itoa of integer, where N=index
bool lString16::replaceIntParam(int index, int replaceNumber)
{
    return replaceParam( index, lString16::itoa(replaceNumber));
}

/*
static int decodeHex( lChar16 ch )
{
    if ( ch>='0' && ch<='9' )
        return ch-'0';
    else if ( ch>='a' && ch<='f' )
        return ch-'a'+10;
    else if ( ch>='A' && ch<='F' )
        return ch-'A'+10;
    return -1;
}

static lChar16 decodeHTMLChar( const lChar16 * s )
{
    if (s[0] == '%') {
        int d1 = decodeHex( s[1] );
        if (d1 >= 0) {
            int d2 = decodeHex( s[2] );
            if (d2 >= 0) {
                return (lChar16)(d1*16 + d2);
            }
        }
    }
    return 0;
}
*/

char from_hex(char ch) {
    return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

std::string url_decode(std::string text) {
    char h;
    std::ostringstream escaped;
    escaped.fill('0');

    for (auto i = text.begin(), n = text.end(); i != n; ++i) {
        std::string::value_type c = (*i);

        if (c == '%') {
            if (i[1] && i[2]) {
                h = from_hex(i[1]) << 4 | from_hex(i[2]);
                escaped << h;
                i += 2;
            }
        } else if (c == '+') {
            escaped << ' ';
        } else {
            escaped << c;
        }
    }

    return escaped.str();
}

/// decodes path like "file%20name" to "file name"
lString16 DecodeHTMLUrlString( lString16 s )
{
    return lString16(url_decode(LCSTR(s)).c_str());
    /*
    const lChar16 * str = s.c_str();
    for ( int i=0; str[i]; i++ ) {
        if ( str[i]=='%'  ) {
            lChar16 ch = decodeHTMLChar( str + i );
            if ( ch==0 ) {
                continue;
            }
            // HTML encoded char found
            lString16 res;
            res.reserve(s.length());
            res.append(str, i);
            res.append(1, ch);
            i+=3;

            // continue conversion
            for ( ; str[i]; i++ ) {
                if ( str[i]=='%'  ) {
                    ch = decodeHTMLChar( str + i );
                    if ( ch==0 ) {
                        res.append(1, str[i]);
                        continue;
                    }
                    res.append(1, ch);
                    i+=2;
                } else {
                    res.append(1, str[i]);
                }
            }
            return res;
        }
    }
    return s;
     */
}

void limitStringSize(lString16 & str, int maxSize) {
    if (str.length() < maxSize)
		return;
	int lastSpace = -1;
	for (int i = str.length() - 1; i > 0; i--)
		if (str[i] == ' ') {
			while (i > 0 && str[i - 1] == ' ')
				i--;
			lastSpace = i;
			break;
		}
	int split = lastSpace > 0 ? lastSpace : maxSize;
	str = str.substr(0, split);
    str += "...";
}

bool char_isRTL(const lChar16 c){
    return ( //(c==0x00A6) ||  // UNICODE "BROKEN BAR"
                   (c>=0x0590)&&(c<=0x05FF)) ||
           (c == 0x05BE) || (c == 0x05C0) || (c == 0x05C3) || (c == 0x05C6) ||
           ((c>=0x05D0)&&(c<=0x05F4)) ||
           (c==0x0608) || (c==0x060B) ||
           (c==0x060D) ||
           ((c>=0x0600)&&(c<=0x06FF)) ||
           ((c>=0x06FF)&&(c<=0x0710)) ||
           ((c>=0x0712)&&(c<=0x072F)) ||
           ((c>=0x074D)&&(c<=0x07A5)) ||
           ((c>=0x07B1)&&(c<=0x07EA)) ||
           ((c>=0x07F4)&&(c<=0x07F5)) ||
           ((c>=0x07FA)&&(c<=0x0815)) ||
           (c==0x081A) || (c==0x0824) ||
           (c==0x0828) ||
           ((c>=0x0830)&&(c<=0x0858)) ||
           ((c>=0x085E)&&(c<=0x08AC)) ||
           (c==0x200F) || (c==0xFB1D) ||
           ((c>=0xFB1F)&&(c<=0xFB28)) ||
           ((c>=0xFB2A)&&(c<=0xFD3D)) ||
           ((c>=0xFD50)&&(c<=0xFDFC)) ||
           ((c>=0xFE70)&&(c<=0xFEFC)) ||
           ((c>=0x10800)&&(c<=0x1091B)) ||
           ((c>=0x10920)&&(c<=0x10A00)) ||
           ((c>=0x10A10)&&(c<=0x10A33)) ||
           ((c>=0x10A40)&&(c<=0x10B35)) ||
           ((c>=0x10B40)&&(c<=0x10C48)) ||
           ((c>=0x1EE00)&&(c<=0x1EEBB));
}

bool char_isPunct(const lChar16 c){
    return ((c>=33) && (c<=47)) ||
           ((c>=58) && (c<=64)) ||
           ((c>=91) && (c<=96)) ||
           ((c>=123)&& (c<=126))||
           (c == 0x00A6)||
           (c == 0x060C)||
           (c == 0x060D)||
           (c == 0x060E)||
           (c == 0x060F)||
           (c == 0x061F)||
           (c == 0x066D)||
           (c == 0x06DD)||
           (c == 0x06DE)||
           (c == 0x06E9)||
           (c == 0xFD3E)||
           (c == 0x0621)|| // hamza
           (c == 0xFD3F);
}

lString16 lString16::LigatureCheck(lString16 text)
{
    for (int i = 0 ; i < LAM_ALEF_COMBOS_LENGTH; i++) {
        text = LigatureCheck(text, lamAlefCombos[i][0], lamAlefCombos[i][1], lamAlefCombos[i][2]);
    }
    return text;
}

lString16 lString16::LigatureCheck(lString16 text, lChar16 lam, lChar16 alef, lChar16 lig)
{
    if(text.length() < 2)
    {
        return text;
    }

    lChar16 * alef_a = &alef;
    lChar16 * lam_a = &lam;
    lChar16 * lig_a = &lig;
    lString16 lamstr = lString16(lam_a,1);
    lString16 alefstr = lString16(alef_a,1);
    lString16 ligstr = lString16(lig_a,1);
    lString16 search = lamstr + alefstr;

    if(text.pos(search)==-1)
    {
        return text;
    }

    while (text.pos(search) !=-1)
    {
        text = text.replace(text.pos(search),2,ligstr);
    }

    return text;
}

LetterMap arabicLetterMap = ArabicLetterMap();

bool char_is_RTL_left_unjoinable(lChar16 ch)
{
    return ((ch == 0x0627) ||
            (ch == 0x062F) ||
            (ch == 0x0630) ||
            (ch == 0x0631) ||
            (ch == 0x0632) ||
            (ch == 0x0648) ||
            (ch == 0x0621) || //Hamza
            (ch == 0x0623) || //alef + hamza above
            (ch == 0x0625) || //alef + hamza below
            (ch == 0x0623) || //alef + wavy hamza above
            (ch == 0x0672) || //alef + wavy hamza below
            (ch == 0x0673) || //alef + madda
            (ch == 0x0671) || //alef + wasla
            (ch == 0xFEFC) || //Lam-alif final
            (ch == 0xFEFB)); //Lam-alif isolated
}

lString16 lString16::PrettyLetters(lString16 text)
{
    if (text.empty())
    {
        return text;
    }

    lString16 restext;
    for (int i = 0; i < text.length(); i++)
    {
        lChar16 ch = text.at(i);
        if( arabicLetterMap.find(ch) != arabicLetterMap.end() )
        {
            unsigned int pos = arabic_isolated;
            if (text.length() == 1)
            {
                pos = arabic_isolated;
            }
            else if (i == 0)
            {
                pos = arabic_start;
            }
            else if (i > 0)
            {
                if (char_is_RTL_left_unjoinable(text.at(i - 1)))
                {
                    if (i == text.length() - 1)
                    {
                        pos = arabic_isolated;
                    }
                    else
                    {
                        pos = arabic_start;
                    }
                }
                else
                {
                    if (i == text.length() - 1)
                    {
                        pos = arabic_end;
                    }
                    else
                    {
                        pos = arabic_mid;
                    }
                }
            }

            lChar16 replace = arabicLetterMap.at(ch).at(pos);
            lChar16 * replace_a = &replace;
            lString16 replacestr = lString16(replace_a,1);
            restext += replacestr;
        }
        else
        {
            lChar16 * ch_a = &ch;
            lString16 ch_str = lString16(ch_a,1);
            restext += ch_str;
        }
        //restext += lString16::itoa(i);
    }
    return restext;
}

lString16Collection lString16::ArabicSplitToWords()
{
    lString16Collection result;

    lString16 line = *this;

    int start = 0;
    bool last_space = false;
    bool last_punct = false;

    lChar16 last_text = line.at(0);
    bool last_state = char_isRTL(last_text);

    for (int c = 0; c < line.length(); c++)
    {
        lChar16 ch = line.at(c);

        bool is_space = ch == ' ';
        bool is_punct = char_isPunct(ch);

        //bool curr_state = (is_space)? last_state : char_isRTL(ch);
        bool curr_state;
        if(is_space)
        {
            curr_state = last_state;
        }
        else if(is_punct)
        {
            curr_state = (last_space)? false : last_state ;
        }
        else
        {
            curr_state = char_isRTL(ch);
        }
        //curr_state = (is_punct && last_space)? : curr_state
        bool break_char = (is_space || last_space || is_punct || last_punct);

        //CRLog::error("letter = [%s]",LCSTR(curr_text));
        if (curr_state != last_state || break_char )
        {
            int len = c-start;
            if(len>0)
            {
                lString16 word;
                for (int i = start; i < c; i++)
                {
                    word += line.at(i);
                }
                result.add(word);
                start = c;
            }
        }
        last_state = curr_state;
        last_space = is_space;
        last_punct = is_punct;
    }
    lString16 word;
    for (int i = start; i < line.length(); i++)
    {
        word += line.at(i);
    }
    result.add(word);
    return result;
}

bool lString16::CheckRTL()
{
    for (int i = 0; i < this->length(); i++)
    {
        if (char_isRTL(this->at(i)))
        {
            return true;
        }
    }
    return false;
}

lString16 lString16::PrepareRTL()
{
    //CRLog::error("preparertl");
    if(this->length() < 0)
    {
        return *this;
    }
    if(!this->CheckRTL())
    {
        return *this;
    }
    lString16Collection words = this->ArabicSplitToWords();
    lString16 result;
    for (int i = 0; i < words.length(); i++)
    {
        lString16 word = PrettyLetters(LigatureCheck(words.at(i)));
        result += word;
    }
    *this = result;
    return *this;
}

ReverseLetterMap reverseLetterMap = ArabicReverseLetterMap();

lString16 lString16::ReversePrettyLetters()
{
    if (this->empty())
    {
        return *this;
    }

    lString16 result;
    for (int i = 0; i < this->length(); i++)
    {
        lChar16 lam = 0x0644;
        lChar16 alef = 0x003F; //question mark "?"

        lChar16 ch = this->at(i);
        if( reverseLetterMap.find(ch) != reverseLetterMap.end() )
        {
            lChar16 replace = reverseLetterMap.at(ch);
            lString16 replacestr;

            bool lamAlefFound = (ch == 0xFEFA || ch == 0xFEFB || ch == 0xFEFC || ch == 0xFEF5 ||
                                 ch == 0xFEF6 || ch == 0xFEF7 || ch == 0xFEF8 || ch == 0xFEF9) ;
            if(lamAlefFound)
            {
                if      (replace == 0x0627) { alef = 0x0627; }
                else if (replace == 0x0622) { alef = 0x0622; }
                else if (replace == 0x0623) { alef = 0x0623; }
                else if (replace == 0x0625) { alef = 0x0625; }
                replacestr = lString16(&lam,1)+  lString16(&alef,1);
            }
            else
            {
                replacestr = lString16(&replace,1);
            }
            result += replacestr;
        }
        else
        {
            result += lString16(&ch,1);
        }
    }
    return result;
}
