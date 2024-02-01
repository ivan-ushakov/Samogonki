#include "main_screen.h"

#include <array>
#include <filesystem>
#include <string_view>
#include <utility>

#include <imgui.h>

#include "iscreen_editor.h"
#include "itext_editor.h"
#include "scr_editor.h"

using namespace editor::ui;

namespace
{
	constexpr const char *open_file_dialog_title = "Open";

	using CodePagePair = std::pair<const char *, const char *>;
	constexpr std::array<CodePagePair, 4> codepage_list{
		CodePagePair{"cz", "cp1250"},
		CodePagePair{"en", "cp1250"},
		CodePagePair{"it", "cp1250"},
		CodePagePair{"ru", "cp1251"}
	};

	struct SCREditorTab final : public Screen
	{
		explicit SCREditorTab(std::filesystem::path file_path, const char *codepage)
			: file_name(file_path.filename()), editor(file_path.string(), codepage)
		{
		}

		const char *title() override
		{
			return file_name.c_str();
		}

		void Render() override
		{
			editor.Render();
		}

		const std::string file_name;
		SCREditor editor;
	};

	struct InterfaceScreenEditorTab final : public Screen
	{
		InterfaceScreenEditorTab(std::filesystem::path file_path, const char *codepage)
			: file_name(file_path.filename()), editor(file_path.string(), codepage)
		{
		}

		const char *title() override
		{
			return file_name.c_str();
		}

		void Render() override
		{
			editor.Render();
		}

		const std::string file_name;
		InterfaceScreenEditor editor;
	};

	struct InterfaceTextEditorTab final : public Screen
	{
		InterfaceTextEditorTab(std::filesystem::path file_path, const char *codepage)
			: file_name(file_path.filename()), editor(file_path.string(), codepage)
		{
		}

		const char *title() override
		{
			return file_name.c_str();
		}

		void Render() override
		{
			editor.Render();
		}

		const std::string file_name;
		InterfaceTextEditor editor;
	};
}

MainScreen::MainScreen() : _path(1024, 0)
{
}

void MainScreen::Render()
{
	const ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoSavedSettings;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);

	if (!ImGui::Begin("Main", nullptr, flags))
	{
		ImGui::End();
		return;
	}

	DrawMenuBar();
	DrawChildren();
	DrawOpenFileDialog();

	ImGui::End();
}

void MainScreen::DrawMenuBar()
{
	if (!ImGui::BeginMenuBar())
	{
		return;
	}

	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("Open"))
		{
			_is_open_file = true;
		}
		ImGui::EndMenu();
	}

	ImGui::EndMenuBar();
}

void MainScreen::DrawChildren()
{
	if (_tab_screens.empty() || !ImGui::BeginTabBar("##tabs"))
	{
		return;
	}

	for (const auto &screen : _tab_screens)
	{
		if (!ImGui::BeginTabItem(screen->title()))
		{
			continue;
		}

		screen->Render();
		ImGui::EndTabItem();
	}

	ImGui::EndTabBar();
}

void MainScreen::DrawOpenFileDialog()
{
	if (_is_open_file)
	{
		_is_open_file = false;
		_selected_code_page = 0;
		std::fill(_path.begin(), _path.end(), 0);
		ImGui::OpenPopup(open_file_dialog_title);
	}

	if (!ImGui::BeginPopupModal(open_file_dialog_title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		return;
	}

	ImGui::InputText("Path", _path.data(), _path.size());

	const auto combo_preview_value = codepage_list[_selected_code_page].first;
	if (ImGui::BeginCombo("##combo", combo_preview_value))
	{
		for (int i = 0; i < codepage_list.size(); i++)
		{
			const bool is_selected = (_selected_code_page == i);
			if (ImGui::Selectable(codepage_list[i].first, is_selected))
			{
				_selected_code_page = i;
			}

			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	if (ImGui::Button("Cancel"))
	{
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();

	if (ImGui::Button("Open"))
	{
		OpenFile();
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}

void MainScreen::OpenFile()
{
	auto i = _path.begin();
	while (i != _path.end())
	{
		if (*i == 0)
		{
			break;
		}

		if (*i > 127)
		{
			return;
		}

		i++;
	}
	std::filesystem::path file_path(_path.begin(), i);

	if (!std::filesystem::exists(file_path))
	{
		return;
	}

	const auto codepage = codepage_list[_selected_code_page].second;

	if (file_path.filename() == "iscreen.scb")
	{
		_tab_screens.emplace_back(std::make_unique<InterfaceScreenEditorTab>(file_path, codepage));
		return;
	}

	if (file_path.filename() == "iText.scb")
	{
		_tab_screens.emplace_back(std::make_unique<InterfaceTextEditorTab>(file_path, codepage));
		return;
	}

	if (file_path.extension() == ".scb")
	{
		_tab_screens.emplace_back(std::make_unique<SCREditorTab>(file_path, codepage));
		return;
	}
}
