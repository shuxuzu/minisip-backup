/*
  Copyright (C) 2005, 2004 Erik Eliasson, Johan Bilien
  
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
 * Authors: Erik Eliasson <eliasson@it.kth.se>
 *          Johan Bilien <jobi@via.ecp.fr>
*/


/* Name
 * 	SipHeaderTo.h
 * Author
 * 	Erik Eliasson, eliasson@it.kth.se
 * Purpose
 * 
*/

#ifndef SIPHEADERTO_H
#define SIPHEADERTO_H

#ifdef _MSC_VER
#ifdef LIBMSIP_EXPORTS
#define LIBMSIP_API __declspec(dllexport)
#else
#define LIBMSIP_API __declspec(dllimport)
#endif
#else
#define LIBMSIP_API
#endif

#include<libmsip/SipHeader.h>
#include<libmsip/SipURI.h>

/**
 * @author Erik Eliasson
*/

class LIBMSIP_API SipHeaderValueTo: public SipHeaderValue{
	public:
		SipHeaderValueTo();
		SipHeaderValueTo(const string &build_from);
		SipHeaderValueTo(const string &username, const string &ip);

		virtual ~SipHeaderValueTo();
		
                virtual std::string getMemObjectType(){return "SipHeaderTo";}
		
		/**
		 * returns string representation of the header
		 */
		string getString(); 

		/**
		 * returns the protocol used. This can be either UDP or TCP
		 */
		SipURI &getUri();
		void setUri(const SipURI &uri);
		
		void setTag(const string &tag);
		string getTag();
	private:
		SipURI uri;
		string tag;
};

#endif
