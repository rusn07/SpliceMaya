//
// Copyright (c) 2010-2016, Fabric Software Inc. All rights reserved.
//

#include "FabricDFGBaseInterface.h"
#include "FabricSpliceRenderCallback.h"
#include "FabricSpliceBaseInterface.h"
#include "FabricSpliceHelpers.h"

#include <maya/MGlobal.h>
#include <maya/M3dView.h>
#include <maya/MFnCamera.h>
#include <maya/MFnDagNode.h>
#include <maya/MDagPath.h>
#include <maya/MMatrix.h>
#include <maya/MAnimControl.h>

bool gHostToRTRCallbackEnabled = true;
M3dView::RendererName gRenderName = M3dView::kViewport2Renderer;
FabricCore::RTVal FabricSpliceRenderCallback::sHostToRTRCallback;
FabricCore::RTVal FabricSpliceRenderCallback::sDrawContext;

bool isHostToRTRCallbackEnabled() {
  return gHostToRTRCallbackEnabled;
}

void enableHostToRTRCallback(bool enable) {
  gHostToRTRCallbackEnabled = enable;
}
 
void FabricSpliceRenderCallback::invalidateHostToRTRCallback() {
  sHostToRTRCallback.invalidate(); 
}
 
void FabricSpliceRenderCallback::setCameraTranformFromMaya(M3dView &view, FabricCore::RTVal &camera/*, FabricCore::RTVal &camera*/) {

  try
  {
    MDagPath mayaCameraDag;
    view.getCamera(mayaCameraDag);

    MMatrix mayaCameraMatrix = mayaCameraDag.inclusiveMatrix();
    FabricCore::RTVal cameraMat = FabricSplice::constructRTVal("Mat44");

    FabricCore::RTVal cameraMatData = cameraMat.callMethod("Data", "data", 0, 0);
    float * cameraMatFloats = (float*)cameraMatData.getData();
    if(cameraMat) {
      cameraMatFloats[0]  = (float)mayaCameraMatrix[0][0];
      cameraMatFloats[1]  = (float)mayaCameraMatrix[1][0];
      cameraMatFloats[2]  = (float)mayaCameraMatrix[2][0];
      cameraMatFloats[3]  = (float)mayaCameraMatrix[3][0];
      cameraMatFloats[4]  = (float)mayaCameraMatrix[0][1];
      cameraMatFloats[5]  = (float)mayaCameraMatrix[1][1];
      cameraMatFloats[6]  = (float)mayaCameraMatrix[2][1];
      cameraMatFloats[7]  = (float)mayaCameraMatrix[3][1];
      cameraMatFloats[8]  = (float)mayaCameraMatrix[0][2];
      cameraMatFloats[9]  = (float)mayaCameraMatrix[1][2];
      cameraMatFloats[10] = (float)mayaCameraMatrix[2][2];
      cameraMatFloats[11] = (float)mayaCameraMatrix[3][2];
      cameraMatFloats[12] = (float)mayaCameraMatrix[0][3];
      cameraMatFloats[13] = (float)mayaCameraMatrix[1][3];
      cameraMatFloats[14] = (float)mayaCameraMatrix[2][3];
      cameraMatFloats[15] = (float)mayaCameraMatrix[3][3];
    }

    camera.callMethod("", "setTransform", 1, &cameraMat);
  }
  catch (FabricCore::Exception e)
  {
    mayaLogErrorFunc(e.getDesc_cstr());
    return;
  }
}

void FabricSpliceRenderCallback::setCameraProjectionFromMaya(M3dView &view, FabricCore::RTVal &camera/*, FabricCore::RTVal &camera*/) {

  try
  {
    MMatrix projectionMatrix;
    view.projectionMatrix(projectionMatrix);
    projectionMatrix = projectionMatrix.transpose();

    FabricCore::RTVal projectionMat = FabricSplice::constructRTVal("Mat44");
    FabricCore::RTVal projectionData = projectionMat.callMethod("Data", "data", 0, 0);
    float * projectionFloats = (float*)projectionData.getData();
    if(projectionMat) {
      projectionFloats[0]  = (float)projectionMatrix[0][0];
      projectionFloats[1]  = (float)projectionMatrix[0][1];
      projectionFloats[2]  = (float)projectionMatrix[0][2];
      projectionFloats[3]  = (float)projectionMatrix[0][3];
      projectionFloats[4]  = (float)projectionMatrix[1][0];
      projectionFloats[5]  = (float)projectionMatrix[1][1];
      projectionFloats[6]  = (float)projectionMatrix[1][2];
      projectionFloats[7]  = (float)projectionMatrix[1][3];
      projectionFloats[8]  = (float)projectionMatrix[2][0];
      projectionFloats[9]  = (float)projectionMatrix[2][1];
      projectionFloats[10] = (float)projectionMatrix[2][2];
      projectionFloats[11] = (float)projectionMatrix[2][3];
      projectionFloats[12] = (float)projectionMatrix[3][0];
      projectionFloats[13] = (float)projectionMatrix[3][1];
      projectionFloats[14] = (float)projectionMatrix[3][2];
      projectionFloats[15] = (float)projectionMatrix[3][3];
    }
    camera.callMethod("", "setProjMatrix", 1, &projectionMat);
  }
  catch (FabricCore::Exception e)
  {
    mayaLogErrorFunc(e.getDesc_cstr());
    return;
  }
}

void FabricSpliceRenderCallback::setCameraParamtersFromMaya(M3dView &view, FabricCore::RTVal &camera/*, FabricCore::RTVal &camera*/) {

  try
  {
    MDagPath mayaCameraDag;
    view.getCamera(mayaCameraDag);

    MFnCamera mayaCamera(mayaCameraDag);
    bool isOrthographic = mayaCamera.isOrtho();
    FabricCore::RTVal param = FabricSplice::constructBooleanRTVal(isOrthographic);
    camera.callMethod("", "setOrthographic", 1, &param);

    if(isOrthographic) 
    {
      double windowAspect = view.portWidth()/view.portHeight();
      double left = 0.0, right = 0.0, bottom = 0.0, top = 0.0;
      bool applyOverscan = false, applySqueeze = false, applyPanZoom = false;
      mayaCamera.getViewingFrustum ( windowAspect, left, right, bottom, top, applyOverscan, applySqueeze, applyPanZoom );
      param = FabricSplice::constructFloat32RTVal(top-bottom);
      camera.callMethod("", "setOrthographicFrustumHeight", 1, &param);
    }
    else 
    {
      double fovX, fovY;
      mayaCamera.getPortFieldOfView(view.portWidth(), view.portHeight(), fovX, fovY);    
      param = FabricSplice::constructFloat64RTVal(fovY);
      camera.callMethod("", "setFovY", 1, &param);
    }

    std::vector<FabricCore::RTVal> args(2);
    args[0] = FabricSplice::constructFloat32RTVal(mayaCamera.nearClippingPlane());
    args[1] = FabricSplice::constructFloat32RTVal(mayaCamera.farClippingPlane());
    camera.callMethod("", "setRange", 2, &args[0]);
  }
  catch (FabricCore::Exception e)
  {
    mayaLogErrorFunc(e.getDesc_cstr());
    return;
  }
}
 
void FabricSpliceRenderCallback::initHostToRTRCallback() {
 
  if(!sDrawContext.isValid()) {
    sDrawContext = FabricSplice::constructObjectRTVal("DrawContext");
    sDrawContext = sDrawContext.callMethod("DrawContext", "getInstance", 0, 0);
  }
  else if(sDrawContext.isObject() && sDrawContext.isNullObject()) {
    sDrawContext = FabricSplice::constructObjectRTVal("DrawContext");
    sDrawContext = sDrawContext.callMethod("DrawContext", "getInstance", 0, 0);
  }

  if(!sHostToRTRCallback.isValid()) {
    sHostToRTRCallback = FabricSplice::constructObjectRTVal("HostToRTRCallback");
    sHostToRTRCallback = sHostToRTRCallback.callMethod("HostToRTRCallback", "getOrCreateCallback", 0, 0);
  }
  else if(sHostToRTRCallback.isObject() && sHostToRTRCallback.isNullObject()) {
    sHostToRTRCallback = FabricSplice::constructObjectRTVal("HostToRTRCallback");
    sHostToRTRCallback = sHostToRTRCallback.callMethod("HostToRTRCallback", "getOrCreateCallback", 0, 0);
  }
}
 
void FabricSpliceRenderCallback::draw(const MString &str, uint32_t phase) {
  try
  {
    M3dView view;
    M3dView::getM3dViewFromModelPanel(str, view);

    // We don't want to draw when users are selecting objects in maya.
    if(view.selectMode()) return;

    // draw
    view.beginGL();
    FabricSplice::Logging::AutoTimer globalTimer("Maya::DrawOpenGL()"); 
    FabricCore::RTVal args[5] = {
      FabricSplice::constructUInt32RTVal(phase),
      FabricSplice::constructStringRTVal(str.asChar()),
      FabricSplice::constructFloat64RTVal(view.portWidth()),
      FabricSplice::constructFloat64RTVal(view.portHeight()),
      FabricSplice::constructUInt32RTVal(2)
    };
    sHostToRTRCallback.callMethod("", "render", 5, &args[0]);    
    view.endGL();
  }
  catch (FabricCore::Exception e)
  {
    mayaLogErrorFunc(e.getDesc_cstr());
    return;
  }
}

void FabricSpliceRenderCallback::preRenderCallback(const MString &str, void *clientData) {

  if(!gHostToRTRCallbackEnabled)
    return;

  if(!FabricSplice::SceneManagement::hasRenderableContent() && FabricDFGBaseInterface::getNumInstances() == 0)
    return;

  try
  {
    M3dView view;
    M3dView::getM3dViewFromModelPanel(str, view);

    FabricSpliceRenderCallback::initHostToRTRCallback();

    // RTR
    MStatus returnStatus;
    FabricCore::RTVal viewport;
    FabricCore::RTVal viewportName = FabricSplice::constructStringRTVal(str.asChar());
    if(gRenderName != view.getRendererName(&returnStatus))
    {
      gRenderName = view.getRendererName(&returnStatus);
      viewport = sHostToRTRCallback.callMethod("BaseRTRViewport", "resetViewport", 1, &viewportName);
    }
    else viewport = sHostToRTRCallback.callMethod("BaseRTRViewport", "getOrAddViewport", 1, &viewportName);
    FabricCore::RTVal camera = viewport.callMethod("RTRBaseCamera", "getRTRCamera", 0, 0);

    FabricSpliceRenderCallback::setCameraTranformFromMaya(view, camera);
    FabricSpliceRenderCallback::setCameraProjectionFromMaya(view, camera);
    FabricSpliceRenderCallback::setCameraParamtersFromMaya(view, camera);

    // draw
    //FabricSpliceRenderCallback::draw(str, 2);
  }
  catch (FabricCore::Exception e)
  {
    mayaLogErrorFunc(e.getDesc_cstr());
    return;
  }
}

void FabricSpliceRenderCallback::postRenderCallback(const MString &str, void *clientData) {

  if(!gHostToRTRCallbackEnabled)
    return;
 
  if(!FabricSplice::SceneManagement::hasRenderableContent() && FabricDFGBaseInterface::getNumInstances() == 0)
    return;

  // draw
  FabricSpliceRenderCallback::draw(str, 4);
}
 