#include "BigBlueGraphics.h"

BBControl::BBControl(const IColor& accentColor) :
  mAccentColor(accentColor),
  mShadeColor(CalculateShadeColor(accentColor))
{
  int i;
}

const IColor& BBControl::GetAccentColor()
{
  return mAccentColor;
}

const IColor& BBControl::GetShadeColor()
{
  return mShadeColor;
}

const IVStyle BBControl::GetHouseStyle()
{
  return DEFAULT_STYLE
    .WithColor(EVColor::kFG, BB_COLOR_OFFBLACK)
    .WithColor(EVColor::kPR, BB_COLOR_OFFBLACK)
    .WithColor(EVColor::kFR, COLOR_WHITE)
    .WithColor(EVColor::kX1, GetAccentColor())
    .WithLabelText(BB_LABEL_TEXT)
    .WithValueText(BB_VALUE_TEXT)
    .WithDrawFrame(false)
    .WithDrawShadows(false);
}

const IColor BBControl::CalculateShadeColor(const IColor& accentColor)
{
  const IColor shadeColor(
    255,
    (int)(accentColor.R * SHADE_ALPHA),
    (int)(accentColor.G * SHADE_ALPHA),
    (int)(accentColor.B * SHADE_ALPHA)
  );
  return shadeColor;
}

BBKnobControl::BBKnobControl(const IRECT& bounds, int paramId, const char* label, const IColor& accentColor, float a1, float a2, float aAnchor):
  BBControl(accentColor),
  IVKnobControl(bounds, paramId, label, GetHouseStyle(), false, false, a1, a2, aAnchor, EDirection::Vertical, DEFAULT_GEARING, KNOB_RING_SIZE)
{
  SetPointerThickness(KNOB_RING_SIZE);
}

void BBKnobControl::DrawIndicatorTrack(IGraphics& g, float angle, float cx, float cy, float radius)
{
  // Draw the ring shade for the entire ring
  if (mTrackSize > 0.f)
  {
    g.DrawArc(GetShadeColor(), cx, cy, radius, mAngle1, mAngle2, &mBlend, mTrackSize);
  }
  // Call superclass to draw the filled portion of the ring
  IVKnobControl::DrawIndicatorTrack(g, angle, cx, cy, radius);

}

BigBlueInternalSlider::BigBlueInternalSlider(const IRECT& bounds, int paramIdx, const char* label, const IVStyle& style, bool valueIsEditable, EDirection dir, double gearing, float handleSize, float trackSize, bool handleInsideTrack) :
  IVSliderControl(bounds, paramIdx, label, style, valueIsEditable, dir, gearing, handleSize, trackSize, handleInsideTrack),
  BBControl(BB_DEFAULT_ACCENT_COLOR)
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
  IColor trackColor = GetShadeColor();  //GetShadeColor(GetColor(kX1));
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


BigBlueSelectSliderControl::BigBlueSelectSliderControl(IGraphics* pGraphics, const IRECT& bounds, int paramIdx, const std::vector<const char*>& listItems, const char* label) :
  IControl(bounds),
  BBControl(BB_DEFAULT_ACCENT_COLOR),
  mLabels()
{
  float sliderWidth = 25.f;
  IRECT sliderBox = bounds.GetFromLeft(sliderWidth);
  mSlider = new BigBlueInternalSlider(sliderBox, paramIdx, label, GetHouseStyle(), false, EDirection::Vertical, DEFAULT_GEARING);
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
    IVStyle style = GetHouseStyle().WithLabelText(text);
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
