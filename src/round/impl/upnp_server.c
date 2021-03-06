/******************************************************************
 *
 * Round for C
 *
 * Copyright (C) Satoshi Konno 2015
 *
 * This is licensed under BSD-style license, see file COPYING.
 *
 ******************************************************************/

#include <round/const.h>
#include <round/util/strings.h>
#include <round/impl/upnp_server.h>
#include <round/impl/upnp_server_desc.h>
#include <round/rpc_server.h>

/****************************************
 * round_upnp_server_getrpcserverfromrequest
 ****************************************/

#define round_upnp_server_getrpcserverfromrequest(httpReq) round_upnp_device_getrpcserver(((mUpnpDevice*)mupnp_http_request_getuserdata(httpReq)))

/****************************************
 * round_upnp_server_actionreceived
 ****************************************/

bool round_upnp_server_actionreceived(mUpnpAction* action)
{
  return false;
}

/****************************************
 * round_upnp_server_queryreceived
 ****************************************/

bool round_upnp_server_queryreceived(mUpnpStateVariable* statVar)
{
  return false;
}

/****************************************
 * round_upnp_server_isrootrequest
 ****************************************/

bool round_upnp_server_isrootrequest(mUpnpHttpRequest* httpReq)
{
  if (!mupnp_http_request_isgetrequest(httpReq))
    return false;
  
  char* uri = mupnp_http_request_geturi(httpReq);
  if (!round_streq(uri, "/"))
    return false;
  
  return true;
}

/****************************************
 * round_upnp_server_isjsonrpcrequest
 ****************************************/

bool round_upnp_server_isjsonrpcrequest(mUpnpHttpRequest* httpReq)
{
  if (!mupnp_http_request_ispostrequest(httpReq))
    return false;

  char* uri = mupnp_http_request_geturi(httpReq);
  if (!round_streq(uri, ROUND_RPC_HTTP_ENDPOINT))
    return false;

  // TODO : Check content type

  return true;
}

/****************************************
* round_upnp_server_httprequestrecieved
****************************************/

void round_upnp_server_httprequestrecieved(mUpnpHttpRequest* httpReq)
{
  RoundRpcServer* server = round_upnp_server_getrpcserverfromrequest(httpReq);

  if (server && round_upnp_server_isjsonrpcrequest(httpReq)) {
    // TODO : Change to post message queue
    // round_upnp_server_postrpcrequest(NULL, httpReq);
    server->upnpServer->rpcReqListener(httpReq);
    return;
  }
  else if (round_upnp_server_isrootrequest(httpReq)) {
    mUpnpHttpResponse* httpRes = mupnp_http_response_new();
    mupnp_http_response_setstatuscode(httpRes, MUPNP_HTTP_STATUS_OK);
    mupnp_http_response_setcontentlength(httpRes, 0);
    mupnp_http_request_postresponse(httpReq, httpRes);
    mupnp_http_response_delete(httpRes);
    return;
  }

  mupnp_device_httprequestrecieved(httpReq);
}

/****************************************
* round_upnp_server_new
****************************************/

RoundUpnpServer* round_upnp_server_new(void)
{
  RoundUpnpServer* server = (RoundUpnpServer*)malloc(sizeof(RoundUpnpServer));

  if (!server)
    return NULL;

  server->dev = mupnp_device_new();

  if (!server->dev) {
    round_upnp_server_delete(server);
    return NULL;
  }

  // Setup UPnP Device

  if (!mupnp_device_parsedescription(server->dev, ROUND_UPNP_SERVER_DEVICE_DESCRIPTION, strlen(ROUND_UPNP_SERVER_DEVICE_DESCRIPTION))) {
    mupnp_device_delete(server->dev);
    return NULL;
  }

  mUpnpService* upnpSrv = mupnp_device_getservicebyexacttype(server->dev, ROUND_UPNP_SERVICE_TYPE);
  if (upnpSrv == NULL) {
    mupnp_device_delete(server->dev);
    return NULL;
  }

  if (!mupnp_service_parsedescription(upnpSrv, ROUND_UPNP_SERVER_SERVICE_DESCRIPTION, strlen(ROUND_UPNP_SERVER_SERVICE_DESCRIPTION))) {
    mupnp_device_delete(server->dev);
    return NULL;
  }

  mupnp_device_setactionlistener(server->dev, round_upnp_server_actionreceived);
  mupnp_device_setquerylistener(server->dev, round_upnp_server_queryreceived);
  mupnp_device_sethttplistener(server->dev, round_upnp_server_httprequestrecieved);

  // Set Listener

  round_upnp_server_setrpcrequestlistener(server, NULL);

  return server;
}

/****************************************
 * round_upnp_server_delete
 ****************************************/

bool round_upnp_server_delete(RoundUpnpServer* server)
{
  if (!server)
    return false;

  round_upnp_server_stop(server);

  mupnp_device_delete(server->dev);

  free(server);

  return true;
}

/****************************************
 * round_upnp_server_setbindaddress
 ****************************************/

bool round_upnp_server_setbindaddress(RoundUpnpServer* server, const char* addr)
{
  if (!server)
    return NULL;

  // TODO : Support single interface

  return false;
}

/****************************************
 * round_upnp_server_getbindaddress
 ****************************************/

const char* round_upnp_server_getbindaddress(RoundUpnpServer* server)
{
  if (!server)
    return NULL;

  mUpnpHttpServerList* httpServers = mupnp_device_gethttpserverlist(server->dev);
  if (!httpServers)
    return NULL;

  mUpnpHttpServer* httpServer = mupnp_http_serverlist_gets(httpServers);
  if (!httpServer)
    return NULL;

  // TODO : Support multiple interface

  return mupnp_http_request_getlocaladdress(httpServer);
}

/****************************************
 * round_upnp_server_setbindport
 ****************************************/

bool round_upnp_server_setbindport(RoundUpnpServer* server, int port)
{
  if (!server)
    return NULL;

  return mupnp_device_sethttpport(server->dev, port);
}

/****************************************
 * round_upnp_server_getbindport
 ****************************************/

int round_upnp_server_getbindport(RoundUpnpServer* server)
{
  if (!server)
    return 0;

  return mupnp_device_gethttpport(server->dev);
}
