#pragma once

#include <cstdint>
#include <chrono>
#include <bits/unique_ptr.h>
#include "Screen.h"
#include <libs/lvgl/src/lv_core/lv_style.h>
#include <libs/lvgl/src/lv_core/lv_obj.h>
#include <components/motion/MotionController.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class FitVaardigLook : public Screen {
      public:
        FitVaardigLook(DisplayApp* app, Controllers::MotionController& motionController, System::SystemTask& systemTask);
        ~FitVaardigLook() override;

        void Refresh() override;

      private:
        Controllers::MotionController& motionController;
        System::SystemTask& systemTask;
      };
    }
  }
}