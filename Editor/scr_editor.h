#pragma once

#include <memory>
#include <string>

namespace editor::ui
{

class SCREditor final
{
public:
	SCREditor(const std::string &file_path, const char *codepage);
	~SCREditor();
	void Render();

private:
	const std::string _file_path;

	struct Internal;
	std::unique_ptr<Internal> _internal;
};

}
