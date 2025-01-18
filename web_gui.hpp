#pragma once

#include <string>

namespace gui {

struct WebGUIResult {
  std::string quality_preset;
  std::string style_preset;
  std::string text;
  std::optional<bool> set_dir_influence_image_enabled;
};

void web_gui_init();
WebGUIResult web_gui_update();

}