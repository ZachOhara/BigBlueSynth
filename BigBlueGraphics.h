#pragma once

#include "IPlug_include_in_plug_hdr.h"

#include "IGraphicsStructs.h"
#include "IControls.h"

#include <vector>

using namespace iplug;
using namespace igraphics;

const IColor GetShadeColor(const IColor& litColor);

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
    IColor(255, 0, 176, 255), //DEFAULT_X1COLOR,
    IColor(255, 61, 90, 254), //DEFAULT_X2COLOR,
    IColor(255, 0, 230, 118), //DEFAULT_X3COLOR
  },

  //const IText DEFAULT_LABEL_TEXT{ DEFAULT_TEXT_SIZE + 5.f, EVAlign::Top };
  //const IText DEFAULT_VALUE_TEXT{ DEFAULT_TEXT_SIZE, EVAlign::Bottom };

  IText(15.f, EVAlign::Top, COLOR_WHITE), // DEFAULT_LABEL_TEXT,
  IText(12.f, EVAlign::Bottom, COLOR_WHITE), //DEFAULT_VALUE_TEXT,
  true, //DEFAULT_HIDE_CURSOR,
  false, //DEFAULT_DRAW_FRAME,
  false, //DEFAULT_DRAW_SHADOWS,
  DEFAULT_EMBOSS,
  DEFAULT_ROUNDNESS,
  DEFAULT_FRAME_THICKNESS,
  DEFAULT_SHADOW_OFFSET,
  DEFAULT_WIDGET_FRAC,
  DEFAULT_WIDGET_ANGLE
 );

static const double RING_SHADE_ALPHA = 0.15;
static const float RING_SIZE = 4.f;

class BigBlueKnobControl : public IVKnobControl
{
public:
  BigBlueKnobControl
  (
    const IRECT& bounds,
    int paramIdx,
    const char* label = "",
    bool valueIsEditable = false,
    bool valueInWidget = false,
    float a1 = -135.f, float a2 = 135.f, float aAnchor = -135.f,
    double gearing = DEFAULT_GEARING
  );

  void DrawIndicatorTrack(IGraphics& g, float angle, float cx, float cy, float radius) override;
};

class BigBlueInternalSlider : public IVSliderControl
{
public:
  BigBlueInternalSlider
  (
    const IRECT& bounds,
    int paramIdx = kNoParameter,
    const char* label = "",
    const IVStyle& style = DEFAULT_STYLE,
    bool valueIsEditable = false,
    EDirection dir = EDirection::Vertical,
    double gearing = DEFAULT_GEARING,
    float handleSize = 8.f,
    float trackSize = 3.f,
    bool handleInsideTrack = false
  );

  void Draw(IGraphics& g) override;
  void DrawTrack(IGraphics& g, const IRECT& r) override;
  void DrawHandle(IGraphics& g, const IRECT& bounds) override;

  IRECT& GetTrackBounds() { return mTrackBounds; };
};

class BigBlueSelectSliderControl : public IControl
{
public:
  BigBlueSelectSliderControl
  (
    IGraphics* pGraphics,
    const IRECT& bounds,
    int paramIdx,
    const std::vector<const char*>& listItems,
    const char* label = ""
  );

  void Draw(IGraphics& g) override;

  void OnResize() override;
  void OnMouseDown(float x, float y, const IMouseMod& mod) override;
  void OnMouseUp(float x, float y, const IMouseMod& mod) override;
  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override;
  void OnMouseWheel(float x, float y, const IMouseMod& mod, float d) override;

private:
  BigBlueInternalSlider* mSlider = nullptr;
  std::vector<IVLabelControl*> mLabels;
};

class BigBlueSliderControl : public IVSliderControl
{
public:

};
