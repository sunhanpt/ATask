#pragma once

class FRunnable
{
public:
	FRunnable() {}
	virtual ~FRunnable() {}

public:
	virtual bool		Init() { return true; }
	virtual int			Run() = 0; // return exit code
	virtual void		Stop() {}
	virtual void		Exit() {}
};