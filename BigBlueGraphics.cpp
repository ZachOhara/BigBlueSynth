
#include "BigBlueGraphics.h"

const IColor GetShadeColor(const IColor& litColor)
{
  const IColor shadeColor(
    255,
    (int)(litColor.R * RING_SHADE_ALPHA),
    (int)(litColor.G * RING_SHADE_ALPHA),
    (int)(litColor.B * RING_SHADE_ALPHA)
  );
  return shadeColor;
}

BigBlueKnobControl::BigBlueKnobControl(const IRECT& bounds, int paramIdx, const char* label, bool valueIsEditable, bool valueInWidget, float a1, float a2, float aAnchor, double gearing)
  : IVKnobControl(bounds, paramIdx, label, BigBlueHouseStyle, valueIsEditable, valueInWidget, a1, a2, aAnchor, EDirection::Vertical, gearing, RING_SIZE)
{
  SetPointerThickness(RING_SIZE);
}

void BigBlueKnobControl::DrawIndicatorTrack(IGraphics& g, float angle, float cx, float cy, float radius)
{
  // Draw the indicator ring shade
  if (mTrackSize > 0.f)
  {
    const IColor& ringColor = GetColor(kX1);
    const IColor shadeColor = GetShadeColor(ringColor);
    g.DrawArc(shadeColor, cx, cy, radius, mAngle1, mAngle2, &mBlend, mTrackSize);
  }
  // Call superclass to draw the actual indicator ring
  IVKnobControl::DrawIndicatorTrack(g, angle, cx, cy, radius);
}

BigBlueInternalSlider::BigBlueInternalSlider(const IRECT& bounds, int paramIdx, const char* label, const IVStyle& style, bool valueIsEditable, EDirection dir, double gearing, float handleSize, float trackSize, bool handleInsideTrack)
  : IVSliderControl(bounds, paramIdx, label, style, valueIsEditable, dir, gearing, handleSize, trackSize, handleInsideTrack)
{
}

void BigBlueInternalSlider::Draw(IGraphics& g)
{
  DrawBackground(g, mRECT);
  DrawLabel(g);
  DrawWidget(g);
  // Overridden so we can exclude the label here
}

void BigBlueInternalSlider::DrawTrack(IGraphics& g, const IRECT& filledArea)
{
  IColor trackColor = GetShadeColor(GetColor(kX1));
  g.FillRoundRect(trackColor, mTrackBounds, 0.f);
}


void BigBlueInternalSlider::DrawHandle(IGraphics& g, const IRECT& bounds)
{
  // Draw the base handle (the larger black circle)
  IVSliderControl::DrawHandle(g, bounds);
  // Now add the white dot
  const IRECT dot = bounds.GetCentredInside(6.f, 6.f);
  g.FillEllipse(COLOR_WHITE, dot);
}


BigBlueSelectSliderControl::BigBlueSelectSliderControl(IGraphics* pGraphics, const IRECT& bounds, int paramIdx, const std::vector<const char*>& listItems, const char* label)
  : IControl(bounds),
  mLabels()
{
  float sliderWidth = 25.f;
  IRECT sliderBox = bounds.GetFromLeft(sliderWidth);
  mSlider = new BigBlueInternalSlider(sliderBox, paramIdx, label, BigBlueHouseStyle, false, EDirection::Vertical, DEFAULT_GEARING);
  pGraphics->AttachControl(mSlider);

  IRECT& track = mSlider->GetTrackBounds();
  int nDivs = listItems.size() - 1;
  float sectionHeight = track.H() / nDivs;
  //IRECT labelBox = bounds.GetReducedFromLeft(sliderWidth);
  IRECT labelBox = track.GetHShifted(track.W() + (sliderWidth / 2));
  for (int i = 0; i < listItems.size(); i++)
  {
    IRECT box = labelBox.GetReducedFromTop(sectionHeight * i);
    box.B = box.T;
    box = box.GetCentredInside(box.W(), 20.f).GetAltered(0, 0, 50.f, 0).GetVShifted(-1.f);
    IText text = IText(12.f, EVAlign::Middle, COLOR_WHITE).WithAlign(EAlign::Near);
    IVStyle style = BigBlueHouseStyle.WithLabelText(text);
    pGraphics->MeasureText(text, listItems[i], box);
    IVLabelControl* label = new IVLabelControl(box, listItems[i], style);
    pGraphics->AttachControl(label);
    mLabels.push_back(label);
  }
}

void BigBlueSelectSliderControl::Draw(IGraphics& g)
{
  g.DrawRect(COLOR_ORANGE, mRECT);
}

void BigBlueSelectSliderControl::OnResize()
{
  mSlider->OnResize();
}

void BigBlueSelectSliderControl::OnMouseDown(float x, float y, const IMouseMod& mod)
{
  mSlider->OnMouseDown(x, y, mod);
}

void BigBlueSelectSliderControl::OnMouseUp(float x, float y, const IMouseMod& mod)
{
  mSlider->OnMouseUp(x, y, mod);
}

void BigBlueSelectSliderControl::OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod)
{

  mSlider->OnMouseDrag(x, y, dX, dY, mod);
}

void BigBlueSelectSliderControl::OnMouseWheel(float x, float y, const IMouseMod& mod, float d)
{
  mSlider->OnMouseWheel(x, y, mod, d);
}
