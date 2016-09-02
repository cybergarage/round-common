/******************************************************************
 *
 * Round for C
 *
 * Copyright (C) Satoshi Konno 2015
 *
 * This is licensed under BSD-style license, see file COPYING.
 *
 ******************************************************************/

#include <stdlib.h>
#include <round/const.h>
#include <round/error_internal.h>
#include <round/route.h>

/****************************************
 * round_route_manager_init
 ****************************************/

bool round_route_manager_init(RoundRouteManager* mgr)
{
  if (!mgr)
    return false;
  
  mgr->map = round_map_new();
  
  if (!mgr->map)
    return false;
  
  return true;
}

/****************************************
 * round_route_manager_new
 ****************************************/

RoundRouteManager* round_route_manager_new()
{
  RoundRouteManager* mgr = (RoundRouteManager*)malloc(sizeof(RoundRouteManager));
  if (!mgr)
    return NULL;

  if (!round_route_manager_init(mgr)) {
    round_route_manager_delete(mgr);
    return NULL;
  }

  return mgr;
}

/****************************************
 * round_route_manager_delete
 ****************************************/

bool round_route_manager_delete(RoundRouteManager* mgr)
{
  if (!mgr)
    return false;

  if (mgr->map) {
    round_map_delete(mgr->map);
  }
  
  free(mgr);

  return true;
}
