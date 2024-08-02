#include "iscreen_editor.h"

#include <cassert>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include <unordered_map>

#include <imgui.h>

#include "aci_parser.h"
#include "task.h"
#include "text_converter.h"

// Script keyword IDs...
#define ASCR_POS_X		1
#define ASCR_POS_Y		2

#define ASCR_RESOURCE		3
#define ASCR_TYPE		4
#define ASCR_FILE_NAME		5

#define ASCR_KEY_OBJ		6
#define ASCR_CODE		7
#define ASCR_OPT_CODE		8

#define ASCR_EV_COMMAND 	9
#define ASCR_EV_COMM_DATA	10

#define ASCR_EVENT		11
#define ASCR_TIME		12

#define ASCR_FRAME_SEQ		13
#define ASCR_ID 		14
#define ASCR_RES_ID		15
#define ASCR_FON_RES_ID		16
#define ASCR_START_TIME 	17
#define ASCR_END_TIME		18
#define ASCR_DELTA		19

// aciScreenObject...
#define ASCR_OBJECT		20
#define ASCR_SIZE_X		21
#define ASCR_SIZE_Y		22
#define ASCR_DEFAULT_RES	23
#define ASCR_CUR_FRAME		24
#define ASCR_OBJ_INDEX		25  
#define ASCR_ALIGN_X		26  
#define ASCR_ALIGN_Y		27  

// aciScreenInputField...
#define ASCR_STRING		28
#define ASCR_COLOR0		29
#define ASCR_COLOR1		30
#define ASCR_COLOR2		31
#define ASCR_FONT		32
#define ASCR_SPACE		33
#define ASCR_MAX_STATE		34
#define ASCR_CUR_STATE		35
#define ASCR_STATE_STR		36

// aciScreenScroller...
#define ASCR_CUR_VALUE		37
#define ASCR_MAX_VALUE		38

#define ASCR_FLAG		39

// aciScreen
#define ASCR_SCREEN		40
#define ASCR_NEXT_SCREEN	41
#define ASCR_PREV_SCREEN	42

// Parameters...
#define ASCR_FONTS_DIR		43
#define ASCR_FONTS_PREFIX	44
#define ASCR_FONTS_NUM		45

#define ASCR_PALETTE_ID 	46

#define ASCR_STR_LEN		47

// Alpha
#define ASCR_ALPHA0		48
#define ASCR_ALPHA1		49
#define ASCR_ALPHA2		50
#define ASCR_D_ALPHA		51

// Arcane interface...
#define ARC_SCR_SYMBOL		1000
#define ARC_SCR_CRITICAL	1001

// AI Arcanes
#define ARC_PRM			1002
#define ARC_PRM_DATA		1003

// SpriteDispatcher
#define SPRD_SLOT		2000
#define SPRD_SPRITE		2001

// Doggy
#define IW_DOGGY_PHRASE			3000
#define IW_DOGGY_PHRASE_PRIORITY	3001

// iTexts
#define ASCR_iTEXT		3002

// aciScreenObject:type values...
#define ACS_BASE_OBJ		0x01
#define ACS_INPUT_FIELD_OBJ	0x02
#define ACS_SCROLLER_OBJ	0x03

// aciScreenObject::flags...
#define ACS_REDRAW_OBJECT	0x01
#define ACS_ISCREEN_FONT	0x02
#define ACS_ALIGN_CENTER	0x04
#define ACS_ACTIVE_STRING	0x08
#define ACS_BACK_RES		0x10
#define ACS_BLACK_FON		0x20		
#define ACS_MOVED_OBJECT	0x40
#define ACS_ALPHA_MASK		0x80
#define ACS_TEXT		0x100
#define ACS_BLOCKED		0x200
#define ACS_HIDDEN		0x400
#define ACS_MOUSE_SELECT	0x800
#define ACS_NO_RESIZE		0x1000
#define ACS_KEYB_SELECT		0x2000

namespace editor::ui::aci
{

struct Object final
{
	int id = 0;
	std::string id_value;
	ImVec2 position{};
	ImVec2 size{};
	ImVec2 align{};
	scrDataBlock *string_block = nullptr;
	std::string string_value;
	std::vector<std::string> state_strings;
	int current_state = 0;

	std::unordered_map<int, scrDataBlock *> properties;
	scrDataBlock *block = nullptr;

	scrDataBlock *GetProperty(int id) const
	{
		auto p = properties.find(id);
		if (p == properties.end())
		{
			throw std::runtime_error("property not found");
		}

		if (p->second->dataSize == 0)
		{
			throw std::runtime_error("empty property");
		}

		return p->second;
	}

	int GetIntegerValue(int id) const
	{
		auto p = GetProperty(id);
		if (p->dataType != 1)
		{
			throw std::runtime_error("wrong property type");
		}

		return p->i_dataPtr[0];
	}

	const char *GetStringValue(int id) const
	{
		auto p = GetProperty(id);
		if (p->dataType != 3)
		{
			throw std::runtime_error("wrong property type");
		}

		return p->c_dataPtr;
	}
};

struct Screen final
{
	std::string id;
	std::unordered_map<int, scrDataBlock *> properties;
	std::vector<Object> objects;
	scrDataBlock *block = nullptr;
};

}

using namespace editor::ui;

namespace
{

struct ScreenStore final
{
	scrDataBlock *root_block = nullptr;
	std::vector<aci::Screen> screens;

	~ScreenStore()
	{
		if (root_block != nullptr)
		{
			delete root_block;
		}
	}
};

aci::Object CreateObject(scrDataBlock *block, editor::util::TextConverter &converter)
{
	aci::Object result;
	result.block = block;

	auto b = block->nextLevel->first();
	while (b != nullptr)
	{
		switch (b->ID)
		{
			case ASCR_ID:
				assert(b->dataType == 1);
				assert(b->dataSize == 1);
				result.id = b->i_dataPtr[0];
				result.id_value = std::to_string(b->i_dataPtr[0]);
				break;

			case ASCR_POS_X:
				assert(b->dataType == 1);
				assert(b->dataSize == 1);
				result.position.x = b->i_dataPtr[0];
				break;

			case ASCR_POS_Y:
				assert(b->dataType == 1);
				assert(b->dataSize == 1);
				result.position.y = b->i_dataPtr[0];
				break;

			case ASCR_SIZE_X:
				assert(b->dataType == 1);
				assert(b->dataSize == 1);
				result.size.x = b->i_dataPtr[0];
				break;

			case ASCR_SIZE_Y:
				assert(b->dataType == 1);
				assert(b->dataSize == 1);
				result.size.y = b->i_dataPtr[0];
				break;

			case ASCR_ALIGN_X:
				assert(b->dataType == 1);
				assert(b->dataSize == 1);
				result.align.x = b->i_dataPtr[0];
				break;

			case ASCR_ALIGN_Y:
				assert(b->dataType == 1);
				assert(b->dataSize == 1);
				result.align.y = b->i_dataPtr[0];
				break;

			case ASCR_MAX_STATE:
				assert(b->dataType == 1);
				assert(b->dataSize == 1);
				result.state_strings.resize(b->i_dataPtr[0] + 1);
				break;

			case ASCR_CUR_STATE:
				assert(b->dataType == 1);
				assert(b->dataSize == 1);
				result.current_state = b->i_dataPtr[0];
				break;

			default:
				result.properties.emplace(b->ID, b);
				break;
		}
		b = b->next;
	}

	b = block->nextLevel->first();
	size_t state_string_index = 0;
	while (b != nullptr)
	{
		switch (b->ID)
		{
			case ASCR_STRING:
				assert(b->dataType == 3);
				result.string_value = converter.Convert(std::string_view(b->c_dataPtr, b->dataSize));
				result.string_block = b;
				break;

			case ASCR_STATE_STR:
				assert(b->dataType == 3);
				assert(!result.state_strings.empty() && state_string_index < result.state_strings.size());
				result.state_strings[state_string_index] = 
					converter.Convert(std::string_view(b->c_dataPtr, b->dataSize));
				state_string_index += 1;
				break;

			default:
				break;
		}
		b = b->next;
	}

	return result;
}

aci::Screen CreateScreen(scrDataBlock *block, editor::util::TextConverter &converter)
{
	aci::Screen result;
	result.block = block;

	auto b = block->nextLevel->first();	
	while (b != nullptr)
	{
		switch (b->ID)
		{
			case ASCR_ID:
				assert(b->dataType == 1);
				assert(b->dataSize == 1);
				result.id = std::to_string(b->i_dataPtr[0]);
				break;

			case ASCR_OBJECT:
				result.objects.emplace_back(CreateObject(b, converter));
				break;

			default:
				result.properties.emplace(b->ID, b);
				break;
		}
		b = b->next;
	}
	return result;
}

ScreenStore *LoadScreenStore(const std::string &file_path, const char *codepage)
{
	editor::util::TextConverter converter(codepage, "utf-8");

	XStream file;
	file.open(file_path.c_str(), XS_IN);
	auto root_block = loadScript(file);

	auto result = new ScreenStore();
	result->root_block = root_block;

	auto p = root_block->nextLevel->first();
	while (p != nullptr)
	{
		switch (p->ID)
		{
			case ASCR_SCREEN:
				result->screens.emplace_back(CreateScreen(p, converter));
				break;

			default:
				break;
		}
		p = p->next;
	}

	return result;
}

void DrawVec2Property(const char *title, ImVec2 value)
{
	assert(ImGui::GetIO().Fonts->Fonts.size() > 1);
	const auto regular_font = ImGui::GetIO().Fonts->Fonts[1];
	
	ImGui::PushFont(regular_font);
	ImGui::TextUnformatted(title);
	ImGui::PopFont();
	ImGui::Text("%d, %d", static_cast<int>(value.x), static_cast<int>(value.y));
	ImGui::NewLine();
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

struct InterfaceScreenEditor::Internal final
{
	editor::util::TextConverter converter;
	ScreenStore *screen_store = nullptr;
	std::optional<size_t> selected_object_index;
	std::string edit_buffer;

	explicit Internal(const char *codepage) : converter("utf-8", codepage)
	{
	}

	~Internal()
	{
		if (screen_store != nullptr)
		{
			delete screen_store;
		}
	}
};

InterfaceScreenEditor::InterfaceScreenEditor(const std::string &file_path, const char *codepage)
	: _file_path(file_path), _internal(std::make_unique<Internal>(codepage))
{
	editor::task::Task t;
	t.run = [this, codepage]() -> void *{
		return LoadScreenStore(_file_path, codepage);
	};

	t.finish = [this](void *result) {
		this->_internal->screen_store = static_cast<ScreenStore *>(result);
	};
	assert(editor::task::Run(t));
}

InterfaceScreenEditor::~InterfaceScreenEditor()
{
}

void InterfaceScreenEditor::Render()
{
	if (_internal->screen_store == nullptr)
	{
		return;
	}

	if (_internal->screen_store->screens.empty())
	{
		return;
	}

	const auto &screens = _internal->screen_store->screens;
	const auto &selected_screen = screens[_selected_screen];

	ImGui::SetNextItemWidth(ImGui::GetFontSize() * 8);
	if (ImGui::BeginCombo("Screen ID", selected_screen.id.c_str()))
	{
		for (size_t i = 0; i < screens.size(); i++)
		{
			const bool is_selected = (_selected_screen == i);
			if (ImGui::Selectable(screens[i].id.c_str(), is_selected))
			{
				_internal->selected_object_index = std::nullopt;
				_internal->edit_buffer.clear();
				_selected_screen = i;
			}

			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	ImGui::NewLine();

	if (ImGui::BeginTable("##split", 2))
	{
		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		DrawCanvas();

		ImGui::TableSetColumnIndex(1);
		DrawProperties();

		ImGui::EndTable();
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
			saveScript(_file_path.c_str(), _internal->screen_store->root_block);
			return nullptr;
		};

		t.finish = [this](void *result) {
			this->_is_busy = false;
		};
		assert(editor::task::Run(t));
	}
}

void InterfaceScreenEditor::DrawCanvas()
{
	const auto &screens = _internal->screen_store->screens;
	const auto &selected_screen = screens[_selected_screen];

	ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 50, 50, 255));
	if (!ImGui::BeginChild("##child", ImVec2(640, 480)))
	{
		ImGui::PopStyleColor();
		return;
	}

	for (size_t i = 0; i < selected_screen.objects.size(); i++)
	{
		const auto &object = selected_screen.objects[i];
		if (object.GetIntegerValue(ASCR_TYPE) != ACS_INPUT_FIELD_OBJ)
		{
			continue;
		}

		if (_internal->selected_object_index == i)
		{
			ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(150, 150, 150, 255));
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(100, 100, 100, 255));
		}
		ImGui::SetCursorPos(object.position);
		if (!ImGui::BeginChild(object.id_value.c_str(), object.size))
		{
			ImGui::PopStyleColor();
			continue;
		}

		if (ImGui::InvisibleButton("##button", object.size))
		{
			_internal->selected_object_index = i;
			_internal->edit_buffer = object.string_value;
		}

		ImGui::PopStyleColor();
		ImGui::EndChild();
	}

	ImGui::PopStyleColor();
	ImGui::EndChild();	
}

void InterfaceScreenEditor::DrawProperties()
{
	if (!_internal->selected_object_index)
	{
		return;
	}

	auto &screens = _internal->screen_store->screens;
	auto &selected_screen = screens[_selected_screen];
	auto &object = selected_screen.objects[_internal->selected_object_index.value()];

	if (object.GetIntegerValue(ASCR_TYPE) != ACS_INPUT_FIELD_OBJ)
	{
		return;
	}

	assert(ImGui::GetIO().Fonts->Fonts.size() > 1);
	const auto regular_font = ImGui::GetIO().Fonts->Fonts[1];

	ImGui::PushFont(regular_font);
	ImGui::Text("ID");
	ImGui::PopFont();
	ImGui::TextUnformatted(object.id_value.c_str());
	ImGui::NewLine();

	DrawVec2Property("Position", object.position);
	DrawVec2Property("Size", object.size);
	DrawVec2Property("Align", object.align);

	ImGui::PushFont(regular_font);
	ImGui::Text("State strings");
	ImGui::PopFont();
	if (ImGui::BeginListBox("##state"))
	{
		for (size_t i = 0; i < object.state_strings.size(); i++)
		{
			const bool is_selected = i == object.current_state;
			const auto &state = object.state_strings[i];
			if (ImGui::Selectable(state.c_str(), is_selected))
			{
				object.current_state = i;
			}

			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndListBox();
	}

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

	const bool is_changed = _internal->edit_buffer != object.string_value;

	if (ImGui::Button("Delete"))
	{
		for (const auto &i : selected_screen.objects)
		{
			auto object_index = i.block->find(ASCR_OBJ_INDEX);
			if (object_index == nullptr)
			{
				continue;
			}

			assert(object_index->dataType == 1);
			assert(object_index->dataSize == 2);
			assert(object_index->i_dataPtr[1] != object.id);
		}
		
		selected_screen.block->nextLevel->remove(object.block);
		delete object.block;

		const auto p = selected_screen.objects.begin() + _internal->selected_object_index.value();
		selected_screen.objects.erase(p);

		_internal->selected_object_index = std::nullopt;
	}

	ImGui::SameLine();
	if (is_changed && ImGui::Button("Update"))
	{
		object.string_value = _internal->edit_buffer;
		delete[] object.string_block->c_dataPtr;

		const auto result = _internal->converter.Convert(_internal->edit_buffer);
		object.string_block->dataSize = result.size() + 1;
		object.string_block->c_dataPtr = new char[object.string_block->dataSize];
		object.string_block->c_dataPtr[object.string_block->dataSize - 1] = 0;
		result.copy(object.string_block->c_dataPtr, object.string_value.size());
	}
}
