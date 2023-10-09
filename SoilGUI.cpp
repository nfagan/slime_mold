#include "SoilGUI.hpp"
#include "SoilComponent.hpp"
#include "slime_mold.hpp"
#include <imgui.h>

SoilGUIUpdateResult SoilGUI::render(const SoilComponent& component, float fps) {
  SoilGUIUpdateResult result;

  ImGui::Begin("SoilGUI");

  ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / fps, fps);

  bool enabled = component.params.enabled;
  if (ImGui::Checkbox("Enabled", &enabled)) {
    result.enabled = enabled;
  }

  const auto& soil_config = *component.get_soil()->read_config();
  auto decay = soil_config.decay;
  if (ImGui::SliderFloat("Decay", &decay, 0.001f, 0.5f)) {
    result.decay = decay;
  }

  auto ds = soil_config.diffuse_speed;
  if (ImGui::SliderFloat("DiffuseSpeed", &ds, 0.01f, 1.0f)) {
    result.diffuse_speed = ds;
  }

  bool diff_enabled = soil_config.diffuse_enabled;
  if (ImGui::Checkbox("DiffuseEnabled", &diff_enabled)) {
    result.diffuse_enabled = diff_enabled;
  }

  bool allow_perturb = soil_config.allow_perturb_event;
  if (ImGui::Checkbox("AllowPerturbEvent", &allow_perturb)) {
    result.allow_perturb_event = allow_perturb;
  }

  auto ts = soil_config.time_scale;
  if (ImGui::SliderFloat("TimeScale", &ts, 0.01f, 8.0f)) {
    result.time_scale = ts;
  }

  bool circ_world = soil_config.circular_world;
  if (ImGui::Checkbox("CircularWorld", &circ_world)) {
    result.circular_world = circ_world;
  }

  bool only_right_turns = soil_config.only_right_turns;
  if (ImGui::Checkbox("OnlyRightTurns", &only_right_turns)) {
    result.only_right_turns = only_right_turns;
  }

  ImGui::Text("TS Power %d", soil_config.turn_speed_power);
  if (ImGui::SmallButton("ScaleTurnSpeed2")) {
    result.turn_speed_power = soil_config.turn_speed_power + 1;
  }
  ImGui::SameLine();
  if (ImGui::SmallButton("ScaleTurnSpeed0.5")) {
    result.turn_speed_power = soil_config.turn_speed_power - 1;
  }

  ImGui::Text("Speed Power %d", soil_config.scale_speed_power);
  if (ImGui::SmallButton("ScaleSpeed2")) {
    result.speed_power = soil_config.scale_speed_power + 1;
  }
  ImGui::SameLine();
  if (ImGui::SmallButton("ScaleSpeed0.5")) {
    result.speed_power = soil_config.scale_speed_power - 1;
  }

  ImGui::End();
  return result;
}