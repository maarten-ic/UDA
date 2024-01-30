#ifndef UDA_SECURITY_CLIENTAUTHENTICATION_H
#define UDA_SECURITY_CLIENTAUTHENTICATION_H

#include "structures/genStructs.h"
#include "clientserver/udaStructs.h"

#include "security.h"

int clientAuthentication(CLIENT_BLOCK* client_block, SERVER_BLOCK* server_block,
                                     LOGMALLOCLIST* logmalloclist, USERDEFINEDTYPELIST* userdefinedtypelist,
                                     AUTHENTICATION_STEP authenticationStep);

#endif // UDA_SECURITY_CLIENTAUTHENTICATION_H
