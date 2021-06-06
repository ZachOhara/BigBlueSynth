#pragma once

#include "IPlug_include_in_plug_hdr.h"

#include "IGraphicsStructs.h"
#include "IControls.h"

#include <vector>

using namespace iplug;
using namespace igraphics;

/*
const IVStyle BigBlueHouseStyle
(
  true, //DEFAULT_SHOW_LABEL,
  true, //DEFAULT_SHOW_VALUE,
  {
    DEFAULT_BGCOLOR,
    IColor(255, 10, 10, 10), //DEFAULT_FGCOLOR,
    IColor(255, 10, 10, 10), //DEFAULT_PRCOLOR,
    COLOR_WHITE, //DEFAULT_FRCOLOR,
    DEFAULT_HLCOLOR,
    COLOR_TRANSLUCENT, //DEFAULT_SHCOLOR,
    DEFAULT_X1COLOR, //IColor(255, 0, 176, 255), //DEFAULT_X1COLOR,
    DEFAULT_X2COLOR, //IColor(255, 61, 90, 254), //DEFAULT_X2COLOR,
    DEFAULT_X3COLOR //IColor(255, 0, 230, 118), //DEFAULT_X3COLOR
  },

  IText(15.f, EVAlign::Top, COLOR_WHITE), // DEFAULT_LABEL_TEXT,
  IText(12.f, EVAlign::Bottom, COLOR_WHITE), //DEFAULT_VALUE_TEXT,
  DEFAULT_HIDE_CURSOR, //true, //DEFAULT_HIDE_CURSOR,
  DEFAULT_DRAW_FRAME, //false, //DEFAULT_DRAW_FRAME,
  DEFAULT_DRAW_SHADOWS, //false, //DEFAULT_DRAW_SHADOWS,
  DEFAULT_EMBOSS,
  DEFAULT_ROUNDNESS,
  DEFAULT_FRAME_THICKNESS,
  DEFAULT_SHADOW_OFFSET,
  DEFAULT_WIDGET_FRAC,
  DEFAULT_WIDGET_ANGLE
 );
 */


const IColor BB_COLOR_OFFBLACK(255, 10, 10, 10);
const IColor BB_COLOR_BGRAY_900(255, 38, 50, 56);
const IColor BB_COLOR_LBLUE_A400(255, 0, 176, 255);

// Todo: find a better way to calculate these
const IColor BB_COLOR_OFFBLACK_DISABLED(255, 0, 0, 0);
const IColor BB_COLOR_LBLUE_A400_DISABLED(255, 21, 76, 102);
const IColor BB_COLOR_WHITE_DISABLED(255, 64, 64, 64);
const IColor BB_COLOR_SHADE_DISABLED(255, 28, 43, 51);

const IText BB_LABEL_TEXT(15.f, EVAlign::Top, COLOR_WHITE);
const IText BB_VALUE_TEXT(13.f, EVAlign::Bottom, COLOR_WHITE);

#define BB_DEFAULT_ACCENT_COLOR BB_COLOR_LBLUE_A400

static const double SHADE_ALPHA = 0.15;

static const float KNOB_RING_SIZE = 4.f;

class BBControl
{
public:
  BBControl(const IColor& accentColor);

protected:
  const IColor& GetAccentColor();
  const IColor& GetShadeColor();

  const IVStyle GetHouseStyle();

private:
  const IColor mAccentColor;
  const IColor mShadeColor;

  static const IColor CalculateShadeColor(const IColor& accentColor);
};

class BBKnobControl : public BBControl, public IVKnobControl
{
public:
  BBKnobControl
  (
    const IRECT& bounds,
    int paramId,
    const char* label = "",
    const IColor& accentColor = BB_DEFAULT_ACCENT_COLOR,
    float a1 = -135.f, float a2 = 135.f, float aAnchor = -135.f
  );

  void DrawIndicatorTrack(IGraphics& g, float angle, float cx, float cy, float radius) override;

private:
};

class BBSliderControl : public BBControl, public IVSliderControl
{
public:
  BBSliderControl
  (
    const IRECT& bounds,
    int paramId,
    const char* label = "",
    const IColor& accentColor = BB_DEFAULT_ACCENT_COLOR,
    float handleSize = 8.f,
    float trackSize = 2.f
  );

  void DrawHandle(IGraphics& g, const IRECT& bounds) override;

  IRECT& GetTrackBounds();
};

class BBUnfilledSliderControl : public BBSliderControl
{
public:
  BBUnfilledSliderControl
  (
    const IRECT& bounds,
    int paramId,
    const char* label = "",
    const IColor& accentColor = BB_DEFAULT_ACCENT_COLOR,
    float handleSize = 8.f,
    float trackSize = 2.f
  );

  void DrawTrack(IGraphics& g, const IRECT& r) override;
};

class BBSlideSelectControl : public BBControl, public IControl, public IVectorBase
{
public:
  BBSlideSelectControl
  (
    IGraphics* pGraphics,
    const IRECT& bounds,
    int paramId,
    const std::vector<const char*>& options,
    const char* label = "",
    bool useFilledSlider = true,
    const IColor& accentColor = BB_DEFAULT_ACCENT_COLOR
  );

  void Draw(IGraphics& g) override;

  void OnInit() override;
  void OnResize() override;
  void OnMouseDown(float x, float y, const IMouseMod& mod) override;
  void OnMouseUp(float x, float y, const IMouseMod& mod) override;
  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override;
  void OnMouseWheel(float x, float y, const IMouseMod& mod, float d) override;

  void SetDisabled(bool disable) override;

private:
  const float SLIDER_WIDTH = 20.f; // Width of the slider in px
  const float SLIDER_VGAP = 5.f; // Vertical gap between label and slider

  const char* mLabelText;
  BBSliderControl* mSlider = nullptr;
  std::vector<IVLabelControl*> mLabels;
};
