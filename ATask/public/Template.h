#pragma once

/**
* utility template for a class that should not be copyable.
* Derive from this class to make your class non-copyable
*/
class FNoncopyable
{
protected:
	FNoncopyable() {};
	~FNoncopyable() {};
private:
	FNoncopyable(const FNoncopyable&) = delete;
	FNoncopyable& operator=(const FNoncopyable&) = delete;
};