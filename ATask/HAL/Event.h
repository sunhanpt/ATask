#pragma once
#include <limits>
/**
* Interface for waitable events.
*
* This interface has platform-specific implementations that are used to wait for another
* thread to signal that it is ready for the waiting thread to do some work. It can also
* be used for telling groups of threads to exit.
*/
class FEvent
{
public:

	/**
	* Creates the event.
	*
	* Manually reset events stay triggered until reset.
	* Named events share the same underlying event.
	*
	* @param bIsManualReset Whether the event requires manual reseting or not.
	* @return true if the event was created, false otherwise.
	*/
	virtual bool Create(bool bIsManualReset = false) = 0;

	/**
	* Whether the signaled state of this event needs to be reset manually.
	*
	* @return true if the state requires manual resetting, false otherwise.
	* @see Reset
	*/
	virtual bool IsManualReset() = 0;

	/**
	* Triggers the event so any waiting threads are activated.
	*
	* @see IsManualReset, Reset
	*/
	virtual void Trigger() = 0;

	/**
	* Resets the event to an untriggered (waitable) state.
	*
	* @see IsManualReset, Trigger
	*/
	virtual void Reset() = 0;

	/**
	* Waits the specified amount of time for the event to be triggered.
	*
	* A wait time of MAX_uint32 is treated as infinite wait.
	*
	* @param WaitTime The time to wait (in milliseconds).
	* @param bIgnoreThreadIdleStats If true, ignores ThreadIdleStats
	* @return true if the event was triggered, false if the wait timed out.
	*/
	virtual bool Wait(unsigned int WaitTime, const bool bIgnoreThreadIdleStats = false) = 0;

	/**
	* Waits an infinite amount of time for the event to be triggered.
	*
	* @return true if the event was triggered.
	*/
	bool Wait()
	{
		return Wait(UINT_MAX);
	}

	/** Default constructor. */
	FEvent()
		: EventId(0)
		, EventStartCycles(0)
	{}

	/** Virtual destructor. */
	virtual ~FEvent()
	{}

	// DO NOT MODIFY THESE

	/** Advances stats associated with this event. Used to monitor wait->trigger history. */
	void AdvanceStats() {}

protected:

	/** Counter used to generate an unique id for the events. */
	static unsigned int EventUniqueId;

	/** An unique id of this event. */
	unsigned int EventId;

	/** Greater than 0, if the event called wait. */
	unsigned int EventStartCycles;
};
