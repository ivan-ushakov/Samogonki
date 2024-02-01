#pragma once

#include <memory>
#include <string>
#include <vector>

#include "screen.h"

namespace editor::ui
{

class MainScreen final
{
public:
	MainScreen();
	void Render();

private:
	void DrawMenuBar();
	void DrawChildren();
	void DrawOpenFileDialog();

	void OpenFile();

private:
	bool _is_open_file = false;
	int _selected_code_page = 0;
	std::string _path;
	std::vector<std::unique_ptr<Screen>> _tab_screens;
};

}
