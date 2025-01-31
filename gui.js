const instance = new Module.GUIData();

const gui = document.createElement('div');
gui.style.position = 'absolute';
gui.style.right = '0';
gui.style.top = '0';
gui.style.width = '368px';
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
let debug_gui_enabled = false;

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
  const debug_button = document.createElement('button');
  debug_button.innerText = "enable debug gui";
  debug_button.onclick = e => {
    debug_gui_enabled = !debug_gui_enabled;
    instance.do_set_debug_gui_enabled(debug_gui_enabled);
  };
  show_hide.appendChild(button);
  show_hide.appendChild(debug_button);
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
  let eg_index = 0;
  const eg_texts = ([
    "Warping, or warped; tugging bits of self by lines, anchors set down shallow.",
    "Approaching the sea, the self curves",
    "into a shell's ear, to sing something vague like a memory."
  ]);
  const eg_paths = ([
    "images/337AA033.jpeg",
    "images/336AA021.jpeg",
    "images/336AA030.jpeg"
  ]);

  const row = document.createElement('div');
  const text_entry = document.createElement('input');
  const submit = document.createElement('button');
  const disable = document.createElement('button');
  const clear = document.createElement('button');
  const eg_toggle = document.createElement('button');
  eg_toggle.innerText = 'toggle example';
  eg_toggle.onclick = e => {
    let ind = eg_index++;
    ind = ind % eg_paths.length;
    instance.set_text(eg_texts[ind]);
    instance.set_direction_influence_image_path(eg_paths[ind]);
  };
  clear.onclick = e => { text_entry.value = " "; instance.set_text(text_entry.value); };
  clear.innerText = 'clear';
  submit.innerText = 'submit';
  submit.onclick = e => instance.set_text(text_entry.value);
  disable.innerText = 'disable';
  disable.onclick = e => instance.disable_direction_influence_image();
  row.appendChild(text_entry);
  row.appendChild(submit);
  row.appendChild(disable);
  row.appendChild(clear);
  row.appendChild(eg_toggle);
  div.appendChild(row);
})();

(function() {
  const funcs = ([
    {f: f => instance.set_time_scale(f), l: 'time'},
    {f: f => instance.set_direction_influence_scale(f), l: 'img sens.'},
    {f: f => instance.set_direction_influence_render_mix(f), l: 'img mix'}
  ]);

  funcs.map(func => {
    const row = document.createElement('div');
    row.style.display = 'flex';
    row.style.width = '100%';
    const input = document.createElement('input');
    const label = document.createElement('div');
    label.style.width = '20%';
    label.innerText = func.l;
    input.type = 'range';
    input.min = '0';
    input.max = '100';
    input.value = '50';
    input.style.width = '80%';
    input.style.height = '32px';
    input.oninput = function() {
      const v = parseInt(input.value) / 100;
      func.f(v);
    };
    row.appendChild(label);
    row.appendChild(input);
    div.appendChild(row);
  });
})();

(function() {
  const row = document.createElement('div');
  row.style.display = 'flex';
  row.style.width = '100%';
  (function() {
    let render_bw = true;
    const button = document.createElement('button');
    button.innerText = 'color';
    button.onclick = function() {
      render_bw = !render_bw;
      instance.set_render_bw(render_bw);
      button.innerText = render_bw ? 'color' : 'b&w';
    }
    row.appendChild(button);
  })();
  (function() {
    let enabled = true;
    const button = document.createElement('button');
    button.innerText = 'pause';
    button.onclick = function() {
      enabled = !enabled;
      instance.set_enabled(enabled);
      button.innerText = enabled ? 'pause' : 'play';
    }
    row.appendChild(button);
  })();
  div.appendChild(row);
})();

gui.appendChild(div);
document.body.appendChild(gui);