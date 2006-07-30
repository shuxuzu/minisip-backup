/*
  Copyright (C) 2006 Mikael Magnusson
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
 * Authors: Mikael Magnusson <mikma@users.sourceforge.net>
*/

#ifndef SIPAUTHENTICATIONDIGEST_H
#define SIPAUTHENTICATIONDIGEST_H

#include<libmsip/libmsip_config.h>
#include<libmutil/MemObject.h>

#include<string>

class SipHeaderValueProxyAuthenticate;
class SipHeaderValueAuthorization;
class SipRequest;

/**
 * Holds user credentials and digest parameters
 * for www and proxy authentication of SIP.
 */
class LIBMSIP_API SipAuthenticationDigest: public MObject{
	public:
		SipAuthenticationDigest( MRef<SipHeaderValueProxyAuthenticate*> auth );

		/**
		 * Update all authentication values except type and realm.
		 */
		bool update( MRef<SipHeaderValueProxyAuthenticate*> auth );

		/**
		 * Build an authorization header for a given SIP REQUEST,
		 */
		MRef<SipHeaderValueAuthorization*> createAuthorization( MRef<SipRequest*> req) const;

		const std::string &getRealm() const{ return realm; }
		bool getStale() const{ return stale; }

		/**
		 * @return SIP_HEADER_TYPE_WWWAUTHENTICATE or
		 * SIP_HEADER_TYPE_PROXYAUTHENTICATE depending on type
		 */
		int getType() const{ return type; }

		/**
		 * Set credential used when creating authorization headers.
		 */
		void setCredential(const std::string &username, const std::string &password);

	private:
		int type;
		std::string realm;
// 		std::string domain;
		std::string nonce;
		std::string opaque;
		bool stale;
		std::string algorithm;
		std::string qop;

		std::string username;
		std::string password;

		static std::string nullStr;
};

#endif	// SIPAUTHENTICATIONDIGEST_H