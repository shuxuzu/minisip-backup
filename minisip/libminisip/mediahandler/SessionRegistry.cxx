/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* Copyright (C) 2004, 2005
 *
 * Authors: Erik Eliasson <eliasson@it.kth.se>
 *          Johan Bilien <jobi@via.ecp.fr>
*/

#include"SessionRegistry.h"

#include"Session.h"
#include"MediaStream.h"

#ifdef _WIN32_WCE
#	include"../include/minisip_wce_extra_includes.h"
#endif

using namespace std;

MRef<Session *> SessionRegistry::getSession( std::string callId ){
        list<MRef<Session *> >::iterator iSession;

        sessionsLock.lock();
        for( iSession = sessions.begin(); iSession != sessions.end(); iSession++ ){
                if( (*iSession)->getCallId() == callId ){
                        sessionsLock.unlock();
                        return *iSession;
                }
        }

        sessionsLock.unlock();
        return NULL;
}
std::list<MRef<Session *> > SessionRegistry::getAllSessions() { 
	std::list<MRef<Session *> > sessionsRet;
	sessionsLock.lock();
	sessionsRet = sessions;
	sessionsLock.unlock();
	return sessionsRet; 
}

void SessionRegistry::registerSession( MRef<Session *> session ){
        sessionsLock.lock();
        sessions.push_back( session );
        sessionsLock.unlock();
}

void SessionRegistry::unregisterSession( MRef<Session *> session ){
        sessionsLock.lock();
        sessions.remove( session );
        sessionsLock.unlock();
}