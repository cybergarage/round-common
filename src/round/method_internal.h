/******************************************************************
 *
 * Round for C
 *
 * Copyright (C) Satoshi Konno 2015
 *
 * This is licensed under BSD-style license, see file COPYING.
 *
 ******************************************************************/

#ifndef _ROUNDC_METHOD_INTERNAL_H_
#define _ROUNDC_METHOD_INTERNAL_H_

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <round/typedef.h>
#include <round/const.h>
#include <round/util/mutex.h>
#include <round/util/map.h>
#include <round/method_internal.h>
#include <round/script_internal.h>

#ifdef  __cplusplus
extern "C" {
#endif

/****************************************
 * Data Type
 ****************************************/

typedef struct {
  char *lang;
  char *name;
  byte *code;
  size_t codeSize;
} RoundMethod;
  
typedef struct {
  RoundMutex *mutex;
  RoundMap *methodMap;
  RoundMap *engineMap;
} RoundMethodManager;

/****************************************
 * Function (Method)
 ****************************************/
  
RoundMethod *round_method_new();
bool round_method_delete(RoundMethod *method);
  
#define round_method_setlanguage(method, value) round_strloc(value, &method->lang)
#define round_method_getlanguage(method) (method->lang)
  
#define round_method_setname(method, value) round_strloc(value, &method->name)
#define round_method_getname(method) (method->name)
  
bool round_method_setcode(RoundMethod *method, byte *code, size_t codeLen);
#define round_method_getcode(method) (method->code)
#define round_method_getsource(method) ((const char *)method->code)
#define round_method_getcodeSize(method) (method->codeSize)
  
bool round_method_isvalid(RoundMethod *method);
  
/****************************************
 * Function (Method Map)
 ****************************************/
  
RoundMap *round_method_map_new();
  
#define round_method_map_delete(map) round_map_delete(map)
#define round_method_map_size(map) round_map_size(map)
#define round_method_map_set(map, method) round_map_setobject(map, method->name, method)
#define round_method_map_get(map, name) ((RoundMethod*)round_map_getobjectbykey(map, name))
#define round_method_map_remove(map, name) round_map_removeobjectbykey(map, name)

/****************************************
 * Function (Script Engine)
 ****************************************/
  
bool round_script_engine_execscript(RoundScriptEngine *engine, RoundMethod *method, const char *param, RoundString *result, RoundError *err);

/****************************************
 * Function (Method Manager)
 ****************************************/
  
RoundMethodManager *round_method_manager_new();
bool round_method_manager_delete(RoundMethodManager *mgr);

bool round_method_manager_addmethod(RoundMethodManager *mgr, RoundMethod *method);
RoundMethod *round_method_manager_getmethod(RoundMethodManager *mgr, const char *name);
bool round_method_manager_removemethod(RoundMethodManager *mgr, const char *name);
#define round_method_manager_getmethodsize(mgr) round_map_size(mgr->methodMap)

bool round_method_manager_addengine(RoundMethodManager *mgr, RoundScriptEngine *engine);
RoundScriptEngine *round_method_manager_getengine(RoundMethodManager *mgr, const char *name);
bool round_method_manager_removeengine(RoundMethodManager *mgr, const char *name);
#define round_method_manager_getenginsize(mgr) round_map_size(mgr->engineMap)
  
#ifdef  __cplusplus
}
#endif

#endif