#include "itext_editor.h"

#include <cassert>
#include <cstring>
#include <vector>

#include <imgui.h>

#include "aci_parser.h"
#include "task.h"
#include "text_converter.h"

using namespace editor::ui;

#define ASCR_iTEXT 3002
#define ASCR_STRING	28

namespace
{

struct TextItem final
{
	std::string id;
	std::string text;
	scrDataBlock *text_block = nullptr;
};

struct TextItemStore final
{
	scrDataBlock *root_block = nullptr;
	std::vector<TextItem> items;

	~TextItemStore();
};

TextItemStore::~TextItemStore()
{
	if (root_block != nullptr)
	{
		delete root_block;
	}
}

TextItemStore *LoadItemStore(const std::filesystem::path &file_path, const char *codepage)
{
	editor::util::TextConverter converter(codepage, "utf-8");

	XStream file;
	file.open(file_path.c_str(), XS_IN);
	auto root_block = loadScript(file);

	auto result = new TextItemStore();
	result->root_block = root_block;

	auto p = root_block->nextLevel->first();
	while (p)
	{
		if (p->ID == ASCR_iTEXT)
		{
			auto p1 = p->nextLevel->first();
			while (p1)
			{
				if (p1->ID == ASCR_STRING)
				{
					result->items.emplace_back(TextItem{
						std::to_string(p->i_dataPtr[0]),
						converter.Convert(std::string_view(p1->c_dataPtr, p1->dataSize)),
						p1
					});
				}
				p1 = p1->next;
			}
		}
		p = p->next;
	}

	return result;
}

int EditBufferResizeCallback(ImGuiInputTextCallbackData *data)
{
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	{
		std::string *buffer = static_cast<std::string *>(data->UserData);
		assert(buffer->data() == data->Buf);
		buffer->resize(data->BufSize);
		data->Buf = buffer->data();
	}
	return 0;
}

}

struct InterfaceTextEditor::Internal final
{
	editor::util::TextConverter converter;
	TextItemStore *item_store = nullptr;
	std::string edit_buffer;

	explicit Internal(const char *codepage) : converter("utf-8", codepage)
	{
	}

	~Internal();
};

InterfaceTextEditor::Internal::~Internal()
{
	if (item_store != nullptr)
	{
		delete item_store;
	}
}

InterfaceTextEditor::InterfaceTextEditor(const std::string &file_path, const char *codepage)
	: _file_path(file_path), _internal(std::make_unique<InterfaceTextEditor::Internal>(codepage))
{
	editor::task::Task t;
	t.run = [this, codepage]() -> void *{
		return LoadItemStore(_file_path, codepage);
	};

	t.finish = [this](void *result) {
		_internal->item_store = static_cast<TextItemStore *>(result);
		if (!_internal->item_store->items.empty())
		{
			_internal->edit_buffer = _internal->item_store->items[0].text;
		}
	};
	assert(editor::task::Run(t));
}

InterfaceTextEditor::~InterfaceTextEditor() {}

void InterfaceTextEditor::Render()
{
	if (_internal->item_store == nullptr)
	{
		return;
	}

	auto &items = _internal->item_store->items;
	auto &item = items[_selected_item];

	ImGui::SetNextItemWidth(ImGui::GetFontSize() * 8);
	if (ImGui::BeginCombo("Text ID", item.id.c_str()))
	{
		for (int i = 0; i < items.size(); i++)
		{
			const bool is_selected = (_selected_item == i);
			if (ImGui::Selectable(items[i].id.c_str(), is_selected))
			{
				_selected_item = i;
				_internal->edit_buffer = items[i].text;
			}

			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	assert(ImGui::GetIO().Fonts->Fonts.size() > 1);
	const auto regular_font = ImGui::GetIO().Fonts->Fonts[1];

	ImGui::PushFont(regular_font);
	ImGui::Text("Text");
	ImGui::PopFont();
	ImGui::InputTextMultiline(
		"##text",
		_internal->edit_buffer.data(),
		_internal->edit_buffer.size(),
		ImVec2(0, 0),
		ImGuiInputTextFlags_CallbackResize,
		EditBufferResizeCallback,
		&_internal->edit_buffer
	);

	const bool is_changed = _internal->edit_buffer != item.text;
	if (is_changed && ImGui::Button("Update"))
	{
		item.text = _internal->edit_buffer;
		delete[] item.text_block->c_dataPtr;

		const auto result = _internal->converter.Convert(_internal->edit_buffer);
		item.text_block->dataSize = result.size() + 1;
		item.text_block->c_dataPtr = new char[item.text_block->dataSize];
		item.text_block->c_dataPtr[item.text_block->dataSize - 1] = 0;
		result.copy(item.text_block->c_dataPtr, item.text.size());
	}

	ImGui::NewLine();

	if (ImGui::Button("Save"))
	{
		if (_is_busy)
		{
			return;
		}
		_is_busy = true;

		editor::task::Task t;
		t.run = [this]() -> void *{
			saveScript(_file_path.c_str(), _internal->item_store->root_block);
			return nullptr;
		};

		t.finish = [this](void *result) {
			this->_is_busy = false;
		};
		assert(editor::task::Run(t));
	}
}
