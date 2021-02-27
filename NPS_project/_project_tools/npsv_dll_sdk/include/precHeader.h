//-------------------- precHeader.h ---------------------------
//-----------------------------------------------------------
#pragma once
#ifndef PRECHEADER_H_
#define PRECHEADER_H_
//-----------------------------------------------------------
#ifndef STRICT
  #define STRICT 1
#endif
#define WIN32_LEAN_AND_MEAN
//-----------------------------------------------------------
#include <windows.h>
#include <winnt.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
//------------------------------------------------------------------
#ifdef __cplusplus
#define SIZE_A(arr) ( \
   0 * sizeof(reinterpret_cast<const ::Bad_arg_to_SIZE_A*>(arr)) + \
   0 * sizeof(::Bad_arg_to_SIZE_A::check_type((arr), &(arr))) + \
   sizeof(arr) / sizeof((arr)[0]) )

struct Bad_arg_to_SIZE_A {
   class Is_pointer; // incomplete
   class Is_array {};
   template <typename T>
   static Is_pointer check_type(const T*, const T* const*);
   static Is_array check_type(const void*, const void*);
};
#else
  #define SIZE_A(a) (sizeof(a) / sizeof(*a))
#endif
//------------------------------------------------------------------
#define NO_COPY_COSTR_OPER(a) \
  private:\
    a(const a&);\
    const a& operator=(const a&);
//------------------------------------------------------------------
#define toBool(a) ((a) ? true : false)
//-----------------------------------------------------------
#endif
