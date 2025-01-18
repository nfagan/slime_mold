#include "web_gui.hpp"
#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

/*
 * https://stackoverflow.com/questions/74755250/how-pass-a-large-array-from-js-to-c-using-emscripten
 * https://stackoverflow.com/questions/50615377/how-do-you-call-a-c-function-that-takes-or-returns-a-struct-by-value-from-js-v
 * https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html
 */

class GUIData {
public:
  void set_quality_preset(std::string p) { quality_preset = p; }
  std::string check_quality_preset() { auto p = quality_preset; quality_preset = ""; return p; }

  void set_style_preset(std::string p) { style_preset = p; }
  std::string check_style_preset() { auto p = style_preset; style_preset = ""; return p; }

  void set_text(std::string p) { text = p; }
  std::string check_text() { auto p = text; text = ""; return p; }

  void disable_direction_influence_image() { set_dir_influence_image_enabled = false; }
  int should_disable_direction_influence_image() {
    auto v =
      set_dir_influence_image_enabled.has_value() && !set_dir_influence_image_enabled.value();
    set_dir_influence_image_enabled = std::nullopt;
    return v;
  }

private:
  std::string quality_preset;
  std::string style_preset;
  std::string text;
  std::optional<bool> set_dir_influence_image_enabled;
};

EMSCRIPTEN_BINDINGS(my_class_example) {
  class_<GUIData>("GUIData")
    .constructor<>()
    .function("check_quality_preset", &GUIData::check_quality_preset)
    .function("set_quality_preset", &GUIData::set_quality_preset)
    .function("check_style_preset", &GUIData::check_style_preset)
    .function("set_style_preset", &GUIData::set_style_preset)
    .function("check_text", &GUIData::check_text)
    .function("set_text", &GUIData::set_text)
    .function("disable_direction_influence_image", &GUIData::disable_direction_influence_image)
    .function("should_disable_direction_influence_image", &GUIData::should_disable_direction_influence_image)
  ;
}

void gui::web_gui_init() {
  EM_ASM(
    const instance = new Module.GUIData();

    const gui = document.createElement('div');
    gui.style.position = 'fixed';
    gui.style.left = '0';
    gui.style.top = '0';
    gui.style.width = '512px';
    gui.style.backgroundColor = 'gray';
    gui.style.opacity = '0.5';
    gui.style.display = 'flex';
    gui.style.flexDirection = 'column';

    gui.instance = instance;
    gui.id = 'gui';

    const div = document.createElement('div');
    div.style.display = 'flex';
    div.style.flexDirection = 'column';
    div.style.width = '100%';

    let gui_enabled = true;

    (function() {
      function on_click(e) {
        if (gui_enabled) {
          div.style.display = 'none';
        } else {
          div.style.display = 'flex';
        }
        gui_enabled = !gui_enabled;
      }

      const show_hide = document.createElement('div');
      show_hide.style.width = '100%';
      const button = document.createElement('button');
      button.innerText = "show or hide";
      button.onclick = on_click;
      show_hide.appendChild(button);
      gui.appendChild(show_hide);
    })();

    const qual_row = document.createElement('div');
    qual_row.style.width = '100%';
    div.appendChild(qual_row);
    (["low", "med", "high"]).map(p => {
      const button1 = document.createElement('button');
      button1.innerText = p;
      button1.onclick = e => instance.set_quality_preset(p);
      qual_row.appendChild(button1);
    });

    const style_row = document.createElement('div');
    style_row.style.width = '100%';
    div.appendChild(style_row);
    (["mid_coh", "high_coh", "chaotic", "fragile", "clustered"]).map(p => {
      const button1 = document.createElement('button');
      button1.innerText = p;
      button1.onclick = e => instance.set_style_preset(p);
      style_row.appendChild(button1);
    });

    (function() {
      const row = document.createElement('div');
      const text_entry = document.createElement('input');
      const submit = document.createElement('button');
      const disable = document.createElement('button');
      submit.innerText = 'submit';
      submit.onclick = e => instance.set_text(text_entry.value);
      disable.innerText = 'disable';
      disable.onclick = e => instance.disable_direction_influence_image();
      row.appendChild(text_entry);
      row.appendChild(submit);
      row.appendChild(disable);
      div.appendChild(row);
    })();

    gui.appendChild(div);
    document.body.appendChild(gui);
  );
}

gui::WebGUIResult gui::web_gui_update() {
  gui::WebGUIResult res{};

  {
    char* str = (char*) EM_ASM_PTR({
      const div = document.getElementById('gui');
      const preset = div.instance.check_quality_preset();
      return stringToNewUTF8(preset);
    });
    res.quality_preset = str;
    free(str);
  }
  {
    char* str = (char*) EM_ASM_PTR({
      const div = document.getElementById('gui');
      const preset = div.instance.check_style_preset();
      return stringToNewUTF8(preset);
    });
    res.style_preset = str;
    free(str);
  }
  {
    char* str = (char*) EM_ASM_PTR({
      const div = document.getElementById('gui');
      const preset = div.instance.check_text();
      return stringToNewUTF8(preset);
    });
    res.text = str;
    free(str);
  }
  {
    int v = EM_ASM_INT({
      const div = document.getElementById('gui');
      return div.instance.should_disable_direction_influence_image();
    });
    if (v) { res.set_dir_influence_image_enabled = false; }
  }

  return res;
}
