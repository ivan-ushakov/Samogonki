#pragma once

#include <memory>
#include <string>

namespace editor::ui
{

class InterfaceTextEditor final
{
public:
	InterfaceTextEditor(const std::string &file_path, const char *codepage);
	~InterfaceTextEditor();
	void Render();

private:
	const std::string _file_path;
	int _selected_item = 0;
	bool _is_busy = false;

	struct Internal;
	std::unique_ptr<Internal> _internal;
};

}
