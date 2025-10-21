/*
@author : Roy Meoded
@author : Yarin Keshet

@date : 20-10-2025


@description: Header-only thread-safe blocking queue for pipeline stages.
- Unbounded by default; optional bounded capacity via constructor.
- push() blocks if bounded and full, returns false if queue is closed.
- pop() blocks until an item is available or the queue is closed and empty; returns false on closed+empty.
- close() wakes all waiters; further push() calls fail and pops drain remaining items.

Usage:
- BlockingQueue<Job> q;
- q.push(job);           // producer
- Job j; while (q.pop(j)) { ... }  // consumer until closed
*/




#pragma once

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <queue>

template <typename T>
class BlockingQueue 
{
public:
	
	// Constructor: optional capacity (0 = unbounded):
	explicit BlockingQueue(std::size_t capacity = 0)
		: capacity_(capacity) {}

	// Disable copy constructor:
	BlockingQueue(const BlockingQueue&) = delete; 

	//Assignment operator:
	BlockingQueue& operator=(const BlockingQueue&) = delete; 

	// Push by const reference-using by the producer:
	bool push(const T& value) 
    {
		std::unique_lock<std::mutex> lk(mu_); 

		// Wait until not full (if bounded) or closed:
		not_full_cv_.wait(lk, [&]
        {
			return closed_ || capacity_ == 0 || q_.size() < capacity_;
		});

		// If closed, reject the push:
		if (closed_) return false;

		q_.push(value); 
		not_empty_cv_.notify_one(); // notify one waiting popper
		return true; // success 
	}

	// Push by rvalue reference
	bool push(T&& value) 
    {
		std::unique_lock<std::mutex> lk(mu_); // Lock mutex


		// Wait until not full (if bounded) or closed:
		not_full_cv_.wait(lk, [&]
        {
			return closed_ || capacity_ == 0 || q_.size() < capacity_;
		});
		if (closed_) return false;
		q_.push(std::move(value));
		not_empty_cv_.notify_one();
		return true;
	}

	// Pop into 'out'. Blocks until an item is available or the queue is closed and empty.
	// Returns true if an item was popped; false if closed and no more items will arrive.
	bool pop(T& out) 
    {
		std::unique_lock<std::mutex> lk(mu_); // Lock mutex

		// Wait until not empty or closed:
		not_empty_cv_.wait(lk, [&]{ return closed_ || !q_.empty(); });
		if (q_.empty()) 
        {
			// closed_ and empty => no more items
			return false;
		}
		out = std::move(q_.front()); // get the item
		q_.pop(); // remove it from the queue
		not_full_cv_.notify_one(); // notify one waiting pusher
		return true;
	}

	// Non-blocking try_pop. Returns true if an item was popped, false otherwise.
	bool try_pop(T& out) 
    {
		std::lock_guard<std::mutex> lk(mu_); // Lock mutex

		// If empty, return false:
		if (q_.empty()) return false;
		out = std::move(q_.front()); // get the item
		q_.pop(); // remove it from the queue
		not_full_cv_.notify_one(); // notify one waiting pusher
		return true;
	}

	// Close the queue: wake all waiters; further push() will fail; pop() drains until empty then returns false.
	void close() 
    {
		std::lock_guard<std::mutex> lk(mu_); // Lock mutex
		closed_ = true; // mark as closed
		not_empty_cv_.notify_all(); // wake all waiting poppers
		not_full_cv_.notify_all(); // wake all waiting pushers
	}

	// Check if the queue is closed:
	bool closed() const 
    {
		std::lock_guard<std::mutex> lk(mu_);
		return closed_;
	}

	// Get current size (non-blocking):
	std::size_t size() const 
    {
		std::lock_guard<std::mutex> lk(mu_);
		return q_.size();
	}

	// Check if the queue is empty (non-blocking):
	bool empty() const 
    {
		std::lock_guard<std::mutex> lk(mu_);
		return q_.empty();
	}

private:
	mutable std::mutex mu_; // mutex for thread-safety
	std::condition_variable not_empty_cv_; // condition variable for not empty
	std::condition_variable not_full_cv_; // condition variable for not full
	std::queue<T> q_; 
	bool closed_ = false; // indicates if the queue is closed
	std::size_t capacity_ = 0; // 0 == unbounded
};
