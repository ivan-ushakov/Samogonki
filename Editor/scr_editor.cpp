#include "scr_editor.h"

#include <cassert>
#include <cstring>
#include <sstream>

#include <imgui.h>

#include "aci_parser.h"
#include "task.h"
#include "text_converter.h"

using namespace editor::ui;

struct SCREditor::Internal final
{
	scrDataBlock *root_block = nullptr;
	scrDataBlock *selected_block = nullptr;
	editor::util::TextConverter converter;
	std::stringstream data_buffer;

	explicit Internal(const char *codepage);
	~Internal();
	void DrawTreeNode(scrDataBlock *block);
};

SCREditor::Internal::Internal(const char *codepage) : converter(codepage, "utf-8")
{
}

SCREditor::Internal::~Internal()
{
	if (root_block != nullptr)
	{
		delete root_block;
	}
}

void SCREditor::Internal::DrawTreeNode(scrDataBlock *block)
{
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
		| ImGuiTreeNodeFlags_SpanAvailWidth;
	const bool is_leaf = block->nextLevel == nullptr || block->nextLevel->first() == nullptr;
	if (is_leaf)
	{
		node_flags |= ImGuiTreeNodeFlags_Leaf;
		node_flags |= ImGuiTreeNodeFlags_NoTreePushOnOpen;
	}

	if (block == selected_block)
	{
		node_flags |= ImGuiTreeNodeFlags_Selected;
	}

	const bool is_open = ImGui::TreeNodeEx(block, node_flags, "ID %d", block->ID);
	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
	{
		selected_block = block;
	}

	if (is_open && !is_leaf)
	{
		auto child = block->nextLevel->first();
		while (child != nullptr)
		{
			DrawTreeNode(child);
			child = child->next;
		}

		ImGui::TreePop();
	}
}

SCREditor::SCREditor(const std::string &file_path, const char *codepage)
	: _file_path(file_path), _internal(std::make_unique<SCREditor::Internal>(codepage))
{
	editor::task::Task t;
	t.run = [this]() -> void * {
		XStream file;
		file.open(_file_path.data(), XS_IN);
		return loadScript(file);
	};

	t.finish = [this](void *result) {
		this->_internal->root_block = static_cast<scrDataBlock *>(result);
	};
	assert(editor::task::Run(t));
}

SCREditor::~SCREditor() {}

void SCREditor::Render()
{
	if (_internal->root_block == nullptr)
	{
		return;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::BeginTable("##split", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY);
	ImGui::TableSetupScrollFreeze(0, 1);
	ImGui::TableSetupColumn("Object");
	ImGui::TableSetupColumn("Contents");
	ImGui::TableHeadersRow();
	ImGui::TableNextRow();

	{
		ImGui::TableSetColumnIndex(0);
		if (_internal->root_block != nullptr)
		{
			if (ImGui::TreeNode("Root"))
			{
				auto block = _internal->root_block->nextLevel->first();
				while (block != nullptr)
				{
					_internal->DrawTreeNode(block);
					block = block->next;
				}

				ImGui::TreePop();
			}
		}
	}

	{
		ImGui::TableSetColumnIndex(1);
		if (_internal->selected_block != nullptr)
		{
			const auto block = _internal->selected_block;
			assert(ImGui::GetIO().Fonts->Fonts.size() > 1);
			const auto regular_font = ImGui::GetIO().Fonts->Fonts[1];

			ImGui::PushFont(regular_font);
			ImGui::Text("Data type");
			ImGui::PopFont();
			switch (block->dataType)
			{
				case 1:
					ImGui::Text("integer");
					break;

				case 2:
					ImGui::Text("double");
					break;

				case 3:
					ImGui::Text("string");
					break;

				default:
					ImGui::Text("unknown");
					break;
			}
			ImGui::NewLine();

			ImGui::PushFont(regular_font);
			ImGui::Text("Data size");
			ImGui::PopFont();
			ImGui::Text("%d", block->dataSize);
			ImGui::NewLine();

			ImGui::PushFont(regular_font);
			ImGui::Text("Name");
			ImGui::PopFont();
			if (block->name != nullptr)
			{
				ImGui::TextUnformatted(block->name);
			}
			ImGui::NewLine();

			ImGui::PushFont(regular_font);
			ImGui::Text("Data");
			ImGui::PopFont();
			if (block->dataType == 3)
			{
				const auto text = _internal->converter.Convert(std::string_view(block->c_dataPtr, block->dataSize));
				ImGui::TextUnformatted(text.c_str());
			}
			else
			{
				_internal->data_buffer.str("");
				_internal->data_buffer.clear();

				for (int i = 0; i < block->dataSize; i++)
				{
					if (i != 0)
					{
						_internal->data_buffer << ", ";
					}
					switch (block->dataType)
					{
						case 1:
							_internal->data_buffer << block->i_dataPtr[i];
							break;

						case 2:
							_internal->data_buffer << block->d_dataPtr[i];
							break;

						default:
							break;
					}
				}
				ImGui::TextUnformatted(_internal->data_buffer.str().c_str());
			}
		}
	}

	ImGui::EndTable();
	ImGui::PopStyleVar();
}
