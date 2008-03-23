/////////////////////////////////////////////////////////////////////////////
//
//  Captury - http://rm-rf.in/captury
//  $Id$
//  (libcaptury - captury framework library)
//
//  Copyright (c) 2007 by Christian Parpart <trapni@gentoo.org>
//
//  This file as well as its whole library is licensed under
//  the terms of GPL. See the file COPYING.
//
/////////////////////////////////////////////////////////////////////////////
#ifndef sw_fifo_h
#define sw_fifo_h

#include <pthread.h>

template<typename T, int CAPACITY = 8>
class TFifo {
private:
	int FHead;
	int FTail;
	int FSize;
	T FBuffer[CAPACITY];

	pthread_mutex_t FMutex;
	pthread_cond_t FCond;

public:
	explicit TFifo(int ACapacity) : 
		FHead(0), FTail(0), FSize(0), FBuffer(), FMutex(), FCond() {

		pthread_mutex_init(&FMutex, 0);
		pthread_cond_init(&FCond, 0);
	}

	~TFifo() {
		pthread_mutex_destroy(&FMutex);
		pthread_cond_destroy(&FCond);
	}

	inline bool empty() const {
		return !FSize;
	}

	inline int size() const {
		return FSize;
	}

	inline int capacity() const {
		return CAPACITY;
	}

	inline bool full() const {
		return FSize + 1 == CAPACITY;
	}

	inline T& head() {
		pthread_mutex_lock(&FMutex);

		while (size() == capacity())
			pthread_cond_wait(&FCond, &FMutex);

		T& result = FBuffer[FHead];

		pthread_mutex_unlock(&FMutex);
		return result;
	}

	inline void head_advance() {
		pthread_mutex_lock(&FMutex);

		FHead = (FHead + 1) % capacity();
		++FSize;

		pthread_mutex_unlock(&FMutex);
		pthread_cond_broadcast(&FCond);
	}

	inline T& head_reverse_advance() {
		pthread_mutex_lock(&FMutex);

		if (!FHead) {
			FHead = capacity() - 1;
		} else {
			FHead = (FHead - 1) % capacity();
		}

		if (FSize)
			--FSize;
		else
			FTail = FHead;

		T& result = FBuffer[FHead];

		pthread_mutex_unlock(&FMutex);
		pthread_cond_broadcast(&FCond);

		return result;
	}

	inline T& tail() {
		pthread_mutex_lock(&FMutex);

		while (empty())
			pthread_cond_wait(&FCond, &FMutex);

		T& result = FBuffer[FTail];
		--FSize;

		pthread_mutex_unlock(&FMutex);
		return result;
	}

	inline void tail_advance() {
		pthread_mutex_lock(&FMutex);

		FTail = (FTail + 1) % capacity();

		pthread_mutex_unlock(&FMutex);
		pthread_cond_broadcast(&FCond);
	}

	inline T& operator[](int AIndex) {
		return FBuffer[AIndex];
	}
};

#endif
