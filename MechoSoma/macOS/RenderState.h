#pragma once

#include <array>
#include <optional>
#include <unordered_map>

#include "Md3d.h"
#include "ShaderCommon.h"

namespace graphics::d3d
{
	class RenderState final
	{
	public:
		void set_option(D3DRENDERSTATETYPE type, DWORD value)
		{
			_options[type] = value;
		}

		[[nodiscard]] DWORD get_option(D3DRENDERSTATETYPE type) const
		{
			return _options.at(type);
		}

		void set_texture(DWORD handle, DWORD stage)
		{
			_texture_stages[stage].texture_handle = handle;
		}

		[[nodiscard]] std::optional<DWORD> get_texture(DWORD stage) const
		{
			return _texture_stages[stage].texture_handle;
		}

		void set_texture_stage_state(DWORD stage, D3DTEXTURESTAGESTATETYPE state, DWORD value)
		{
			_texture_stages[stage].states[state] = value;
		}

		[[nodiscard]] DWORD get_texture_stage_state(DWORD stage, D3DTEXTURESTAGESTATETYPE state) const
		{
			return _texture_stages[stage].states.at(state);
		}

		[[nodiscard]] FragmentShaderParameters get_fragment_shader_parameters() const
		{
			FragmentShaderParameters result;
			switch (get_texture_stage_state(0, D3DTSS_COLOROP))
			{
				case D3DTOP_DISABLE:
					result.color_operation_1 = TextureColorOperation::Disable;
					break;

				case D3DTOP_SELECTARG1:
					result.color_operation_1 = TextureColorOperation::Texture;
					break;

				case D3DTOP_MODULATE:
					result.color_operation_1 = TextureColorOperation::Modulate;
					break;

				default:
					throw std::runtime_error("unsupported color operation");
			}

			switch (get_texture_stage_state(1, D3DTSS_COLOROP))
			{
				case D3DTOP_DISABLE:
					result.color_operation_2 = TextureColorOperation::Disable;
					break;

				case D3DTOP_MODULATE:
					result.color_operation_2 = TextureColorOperation::Modulate;
					break;

				default:
					throw std::runtime_error("unsupported color operation");
			}

			return result;
		}

		bool operator==(const RenderState& other) const
		{
			return _options == other._options && _texture_stages == other._texture_stages;
		}

	private:
		std::unordered_map<D3DRENDERSTATETYPE, DWORD> _options;

		struct TextureStage
		{
			std::optional<DWORD> texture_handle;
			std::unordered_map<D3DTEXTURESTAGESTATETYPE, DWORD> states;

			TextureStage() : texture_handle(std::nullopt) {}

			bool operator==(const TextureStage& other) const
			{
				return texture_handle == other.texture_handle && states == other.states;
			}
		};
		std::array<TextureStage, 10> _texture_stages;
	};
}
