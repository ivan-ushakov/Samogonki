#pragma once

namespace editor::ui
{

struct Screen
{
	virtual ~Screen() = default;
	virtual const char *title() = 0;
	virtual void Render() = 0;    
};

}
