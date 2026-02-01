/*
MIT License

Copyright (c) 2026 Jack Humbert

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include "Overlay.h"
#include "iracing.h"
#include "Config.h"
#include <windows.h>
#include <iostream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

struct Turn {
	std::string name;
	double start;
	double end;
};

class OverlayTurnNumber : public Overlay {
public:
  const float DefaultFontSize = 15.3f;

  OverlayTurnNumber(Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice)
      : Overlay("OverlayTurnNumber", d3dDevice) {}

protected:
  virtual float2 getDefaultSize() { return float2(200, 50); }

  virtual void onEnable() { onConfigChanged(); }
  
	virtual void onDisable() { m_text.reset(); }

  virtual void onConfigChanged() {

		m_text.reset(m_dwriteFactory.Get());

		const std::string font = g_cfg.getString(m_name, "font", "Microsoft YaHei UI");
		const float fontSize = g_cfg.getFloat(m_name, "font_size", DefaultFontSize);
		const int fontWeight = g_cfg.getInt(m_name, "font_weight", 500);
		HRCHECK(m_dwriteFactory->CreateTextFormat( toWide(font).c_str(), NULL, (DWRITE_FONT_WEIGHT)fontWeight, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, L"en-us", &m_textFormat));
		m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		m_textFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

		HRCHECK(m_dwriteFactory->CreateTextFormat(toWide(font).c_str(), NULL, (DWRITE_FONT_WEIGHT)fontWeight, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize * 0.8f, L"en-us", &m_textFormatSmall));
		m_textFormatSmall->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		m_textFormatSmall->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

		
		//// Background geometry
  //  {
  //    Microsoft::WRL::ComPtr<ID2D1GeometrySink> geometrySink;
  //    m_d2dFactory->CreatePathGeometry(&m_backgroundPathGeometry);
  //    m_backgroundPathGeometry->Open(&geometrySink);

  //    const float w = (float)m_width;
  //    const float h = (float)m_height;

  //    geometrySink->BeginFigure(float2(0, h), D2D1_FIGURE_BEGIN_FILLED);
  //    geometrySink->AddBezier(D2D1::BezierSegment(float2(0, -h / 3), float2(w, -h / 3), float2(w, h)));
  //    geometrySink->EndFigure(D2D1_FIGURE_END_CLOSED);

  //    geometrySink->Close();
  //  }
		//						
		//// Static background cache
  //  Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> bmpTarget;
  //  m_renderTarget->CreateCompatibleRenderTarget(&bmpTarget);
  //  bmpTarget->BeginDraw();
  //  bmpTarget->Clear();

  //  // Draw the background
  //  m_brush->SetColor(g_cfg.getFloat4(m_name, "background_col", float4(0, 0, 0, 0.5f)));
  //  bmpTarget->FillGeometry(m_backgroundPathGeometry.Get(), m_brush.Get());

		//
  //  bmpTarget->EndDraw();
  //  bmpTarget->GetBitmap(&m_backgroundBitmap);
	}

  fs::path get_turn_numbers_path() {
		char buffer[MAX_PATH];
		auto bytes = GetModuleFileNameA(NULL, buffer, MAX_PATH);

		if (bytes == 0)
			return L"";

		fs::path fullPath(buffer);

		return fullPath.parent_path() / "TurnNumbers";
  }

  void load_turn_numbers() { 
		auto filename = get_turn_numbers_path() / std::format("{}.json", g_ir_session->trackName);

		printf("Looking for turn numbers in %s\n", filename.string().c_str());

		std::string json;
		if (!loadFile(filename.string(), json))
			return;

    picojson::value pjval;
    std::string parseError = picojson::parse(pjval, json);
    if (!parseError.empty()) {
      printf("Turn number file is not valid JSON!\n%s\n", parseError.c_str());
      return;
    }


    picojson::object& obj = pjval.get<picojson::object>();
    picojson::array& turns_array = obj["turns"].get<picojson::array>();
    m_turns.reserve(turns_array.size());
    for (picojson::value &turn_value : turns_array) {
      picojson::object& turn = turn_value.get<picojson::object>();
      m_turns.push_back(Turn{
          turn["name"].get<std::string>(),
          turn["start"].get<double>(),
          turn["end"].get<double>(),
			});
		}

    m_trackNumbersLoaded = true;
	}

  virtual void onUpdate() {
		if (!g_ir_session->initialized)
			return;

		if (!m_trackNumbersLoaded)
			load_turn_numbers();

		const float4 textCol = g_cfg.getFloat4(m_name, "text_col", float4(1, 1, 1, 0.9f));
		
    m_renderTarget->BeginDraw();
    m_brush->SetColor(textCol);

		// Render the cached background
		//{
		//	m_renderTarget->Clear(float4(0, 0, 0, 0));
		//	m_renderTarget->DrawBitmap(m_backgroundBitmap.Get());
		//}
    auto dist = ir_LapDistPct.getFloat();
		//wchar_t s[16];
  //  swprintf(s, _countof(s), L"%3f", dist);
  //  m_text.render(m_renderTarget.Get(), s,
  //    m_textFormat.Get(), 0.f, 200.f, 0.f, m_brush.Get(),
  //    DWRITE_TEXT_ALIGNMENT_TRAILING);

		if (m_turns.size()) {

			for (Turn & turn : m_turns) {
				if (dist >= turn.start && dist < turn.end) {
          m_text.render(m_renderTarget.Get(), toWide(turn.name).c_str(),
              m_textFormat.Get(), 0.f, 200.f,
              25.f, m_brush.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);
					break;
				}
			}
		}

    m_renderTarget->EndDraw();
	}

  Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormat;
  Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormatSmall;

  Microsoft::WRL::ComPtr<ID2D1PathGeometry1> m_backgroundPathGeometry;

  TextCache m_text;
  Microsoft::WRL::ComPtr<ID2D1Bitmap> m_backgroundBitmap;
	bool m_trackNumbersLoaded = false;
  std::vector<Turn> m_turns;

};