#include <libs/lvgl/lvgl.h>
#include "FitVaardigLook.h"
#include "../DisplayApp.h"

using namespace Pinetime::Applications::Screens;

FitVaardigLook::FitVaardigLook(DisplayApp* app, Controllers::MotionController& motionController, System::SystemTask& systemTask)
  : Screen(app), motionController {motionController}, systemTask {systemTask} {

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "FV LOOK");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);

  systemTask.PushMessage(System::Messages::DisableSleeping);
}

FitVaardigLook::~FitVaardigLook() {
  lv_obj_clean(lv_scr_act());
  systemTask.PushMessage(System::Messages::EnableSleeping);
}

void FitVaardigLook::Refresh() {
}