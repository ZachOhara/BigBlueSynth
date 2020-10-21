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
    .WithColor(EVColor::kSH, GetShadeColor())
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

BBSliderControl::BBSliderControl(const IRECT& bounds, int paramId, const char* label, const IColor& accentColor, float handleSize, float trackSize) :
  BBControl(accentColor),
  IVSliderControl(bounds, paramId, label, GetHouseStyle(), false, EDirection::Vertical, DEFAULT_GEARING, handleSize, trackSize, false)
{
}

void BBSliderControl::DrawHandle(IGraphics& g, const IRECT& bounds)
{
  // Call superclass to draw the base handle (the larger black circle)
  IVSliderControl::DrawHandle(g, bounds);
  // Now add the middle dot
  float dotSize = mHandleSize * (3.0 / 4.0);
  const IRECT dotBounds = bounds.GetCentredInside(dotSize, dotSize);
  g.FillEllipse(GetColor(EVColor::kFR), dotBounds);
}

IRECT& BBSliderControl::GetTrackBounds()
{
  return mTrackBounds;
}

BBUnfilledSliderControl::BBUnfilledSliderControl(const IRECT& bounds, int paramId, const char* label, const IColor& accentColor, float handleSize, float trackSize) :
  BBSliderControl(bounds, paramId, label, accentColor, handleSize, trackSize)
{
}

void BBUnfilledSliderControl::DrawTrack(IGraphics& g, const IRECT& filledArea)
{
  g.FillRect(GetShadeColor(), mTrackBounds);
}

BBSlideSelectControl::BBSlideSelectControl(IGraphics* pGraphics, const IRECT& bounds, int paramId, const std::vector<const char*>& options, const char* label, bool useFilledSlider, const IColor& accentColor) :
  BBControl(BB_DEFAULT_ACCENT_COLOR),
  IControl(bounds),
  IVectorBase(GetHouseStyle(), false, false),
  mLabelText(label)
{
  // Add the slider
  IRECT sliderBox = bounds.GetFromLeft(SLIDER_WIDTH);
  if (useFilledSlider)
    mSlider = new BBSliderControl(sliderBox, paramId, "dummy label");
  else
    mSlider = new BBUnfilledSliderControl(sliderBox, paramId, "dummy label");
  // the slider needs to have a label for spacing calculations,
  // but we hide it by making the font color transparent
  float sliderLabelSize = GetStyle().labelText.mSize + SLIDER_VGAP;
  mSlider->SetStyle(mSlider->GetStyle()
    .WithShowLabel(true)
    .WithLabelText(IText(sliderLabelSize, COLOR_TRANSPARENT))
    .WithShowValue(false)
  );
  pGraphics->AttachControl(mSlider);

  // With the slider in place, calculate the box for the labels
  IRECT& trackBox = mSlider->GetTrackBounds();
  int nDivisions = options.size() - 1;
  float dHeight = trackBox.H() / nDivisions;
  IRECT labelBounds = trackBox.GetHShifted(trackBox.W() + (SLIDER_WIDTH / 2));

  // Make the style for the labels
  float fontSize = GetStyle().valueText.mSize;
  IText text = IText(fontSize, EVAlign::Middle, COLOR_WHITE).WithAlign(EAlign::Near);
  IVStyle labelStyle = GetHouseStyle().WithLabelText(text);

  // Add the labels
  for (int i = 0; i < options.size(); i++)
  {
    // Get the center position (going bottom to top)
    int vCenterPos = dHeight * (options.size() - i - 1);
    // Build a bounding box around the center
    IRECT box = labelBounds.GetReducedFromTop(vCenterPos);
    box.B = box.T;
    box = box.GetCentredInside(box.W(), fontSize).GetVShifted(-1.f);
    pGraphics->MeasureText(text, options[i], box);
    // Construct and add the label
    IVLabelControl* label = new IVLabelControl(box, options[i], labelStyle);
    pGraphics->AttachControl(label);
  }

  // This is necessary for all controls
  AttachIControl(this, label);
}

void BBSlideSelectControl::Draw(IGraphics& g)
{
  DrawLabel(g);
  // For testing:
  //g.DrawRect(COLOR_RED, mLabelBounds);
  g.DrawRect(COLOR_ORANGE, mWidgetBounds);
}

void BBSlideSelectControl::OnInit()
{
  // This can't happen in the constructor without causing a null pointer exception
  SetLabelStr(mLabelText);
}

void BBSlideSelectControl::OnResize()
{
  SetTargetRECT(MakeRects(mRECT));
  mSlider->OnResize();
}

void BBSlideSelectControl::OnMouseDown(float x, float y, const IMouseMod& mod)
{
  mSlider->OnMouseDown(x, y, mod);
}

void BBSlideSelectControl::OnMouseUp(float x, float y, const IMouseMod& mod)
{
  mSlider->OnMouseUp(x, y, mod);
}

void BBSlideSelectControl::OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod)
{

  mSlider->OnMouseDrag(x, y, dX, dY, mod);
}

void BBSlideSelectControl::OnMouseWheel(float x, float y, const IMouseMod& mod, float d)
{
  mSlider->OnMouseWheel(x, y, mod, d);
}
