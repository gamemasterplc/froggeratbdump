#pragma once

#include <string>

class AnimFormat
{
public:
	AnimFormat() {}
	virtual ~AnimFormat() {}
public:
	virtual void Dump(std::string dir, std::string name) = 0;
};

