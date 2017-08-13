/*
 Copyright (c) 2008 TrueCrypt Developers Association. All rights reserved.

 Governed by the TrueCrypt License 3.0 the full text of which is contained in
 the file License.txt included in TrueCrypt binary and source code distribution
 packages.
*/

#include <pthread.h>
#include "../Mutex.h"
#include "../SystemException.h"

namespace CipherShed
{
	Mutex::Mutex ()
	{
#ifndef CS_UNITTESTING
		pthread_mutexattr_t attributes;

		int status = pthread_mutexattr_init (&attributes);
		if (status != 0)
			throw SystemException (SRC_POS, status);

		status = pthread_mutexattr_settype (&attributes, PTHREAD_MUTEX_RECURSIVE);
		if (status != 0)
			throw SystemException (SRC_POS, status);

		status = pthread_mutex_init (&SystemMutex, &attributes);
		if (status != 0)
			throw SystemException (SRC_POS, status);
#endif
		Initialized = true;
	}

	Mutex::~Mutex ()
	{
		Initialized = false;
#ifndef CS_UNITTESTING
#ifdef DEBUG
		int status =
#endif
		pthread_mutex_destroy (&SystemMutex);

#ifdef DEBUG
		if (status != 0)
			SystemLog::WriteException (SystemException (SRC_POS, status));
#endif
#endif
	}

	void Mutex::Lock ()
	{
		assert (Initialized);
#ifndef CS_UNITTESTING
		int status = pthread_mutex_lock (&SystemMutex);
		if (status != 0)
			throw SystemException (SRC_POS, status);
#endif
	}

	void Mutex::Unlock ()
	{
#ifndef CS_UNITTESTING
		int status = pthread_mutex_unlock (&SystemMutex);
		if (status != 0)
			throw SystemException (SRC_POS, status);
#endif
	}
}
