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


#ifndef EERUNNABLE_H
#define EERUNNABLE_H

#include<string>
#include<libmutil/MemObject.h>

#ifdef _MSC_VER
#ifdef LIBMUTIL_EXPORTS
#define LIBMUTIL_API __declspec(dllexport)
#else
#define LIBMUTIL_API __declspec(dllimport)
#endif
#else
#define LIBMUTIL_API
#endif

class LIBMUTIL_API ThreadException{
public:
	ThreadException(std::string description);
	std::string what();
private:
	std::string desc;
};


class LIBMUTIL_API Runnable : public virtual MObject{
public:
	virtual void run()=0;
};


/**
 * 
 * TODO: Add join capability.
 *       Add support for interrupting system calls (easy on all platforms?)
 */
class LIBMUTIL_API Thread : public MObject{
public:
	Thread(MRef<Runnable *>runnable);
	~Thread();

	std::string getMemObjectType(){return "Thread";}

	/**
	 * Purpose: Create thread that executes a function and return handle 
	 * that can be passed to the static join method
         * @return Handle to the created thread.
	 */
	static int createThread( void f());

	/**
	 * Purpose: Create thread that executes a function that takes an
	 * argument.
	 * TODO: The return value of the function can not be retrieved.
	 *       Change the API to let the join method return this value
	 *       under both MSVC and pthread.
	 */
	static int createThread( void* f(void*), void *arg);

	
	/**
	Block the calling thread, waiting for the called thread to 
	be terminated.
	*/
	void * join();
	
	/**
	See non-static join
	*/
	static void join(int handle);

	/**
	Dangerous function
	In linux (uses pthreads), it cancels the called thread
	  (pthread_cancel), exiting the running function. Use with
	  care.
	Under windows, it uses the TerminateThread ... see the msdn lib
	  reference and google for it ... everywhere it is described as
	  a very baad function. Use at own risk. 
	  //TODO: Test under windows ... i have not.
	*/
	bool kill( );
	
	/**
	See non-static kill()
	*/
	static bool kill( int handle );

	void * getHandle() {return handle_ptr;}
	
	/**
	 * Purpose: Implement a platform independent way of delaying 
	 *		execution of a thread.
	 * @arg msec	The thread will suspend sleep for at least the 
	 *		requestex number of milliseconds.
	 * @return -1 if error, else otherwise
	*/
	static int msleep(int msec);

private:
	void *handle_ptr;
};

LIBMUTIL_API void setupDefaultSignalHandling();

#endif
