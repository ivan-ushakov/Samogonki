#pragma once

#include <memory>
#include <string>

namespace editor::ui
{

class InterfaceScreenEditor final
{
public:
	InterfaceScreenEditor(const std::string &file_path, const char *codepage);
	~InterfaceScreenEditor();
	void Render();

private:
	void DrawCanvas();
	void DrawProperties();

private:
	const std::string _file_path;
	size_t _selected_screen = 0;
	bool _is_busy = false;

	struct Internal;
	std::unique_ptr<Internal> _internal;
};

}
