#ifndef __xray_h__
#define __xray_h__

#include "xgeometry.h"
#include "glwindow.h"
#include "glview.h"

#include <map>
#include <set>

/////////////////////////////////////////////////////////////////////
class XRayBoard : public GLWindow
{
public:

    // Get information about OpenCL device
    bool GetXRayDeviceInfo( GCollectionDataMap<GCollectionAnsi,GCollectionAnsi>& info ) const;

protected:

    // 
    enum XRayInitStatus
    {
        CL_CLIS_NONE = 0,
        CL_CLIS_INIT,
        CL_CLIS_FAIL
    };

protected: // methods related to XRay tracing

    // Set GL structure to scene geometry
    bool SetXRayStructure( const GLStructure* pStruct,
                           const float* pTrans, 
                           std::set<const GLStructure*>& elements );
 
    // Set GL primitive array to scene geometry
    bool SetXRayPrimitiveArray( const PRIMITIVES_ARRAY* primitives, 
                                int matID, 
                                const float* pTrans );

    // Set vertex indices from GL primitive array to scene geometry
    bool SetXRayVertexIndices( const PRIMITIVES_ARRAY* indices,
                               int firstVert, 
                               int vertOffset, 
                               int vertNum, 
                               int matID );

    // Set GL triangle array to scene geometry
    bool SetXRayTriangleArray( const PRIMITIVES_ARRAY* triangular,
                               int firstVert, 
                               int vertOffset, 
                               int vertNum, 
                               int matID );

    // Set GL triangle fan array to scene geometry
    bool SetXRayTriangleFanArray( const PRIMITIVES_ARRAY* triangulars,
                                  int firstVert, 
                                  int vertOffset, 
                                  int vertNum, 
                                  int matID );

    // Set GL triangle fan array to scene geometry
    bool SetXRayTriangleStripArray( const PRIMITIVES_ARRAY* triangulars,
                                    int firstVert, 
                                    int vertOffset, 
                                    int vertNum, 
                                    int matID );

    // Set GL quadrangle array to scene geometry
    bool SetXRayQuadrangleArray( const PRIMITIVES_ARRAY* quads,
                                 int firstVert, 
                                 int vertOffset, 
                                 int vertNum, 
                                 int matID );

    // Set GL quadrangle strip array to scene geometry
    bool SetXRayQuadrangleStripArray( const PRIMITIVES_ARRAY* quads,
                                      int firstVert, 
                                      int vertOffset, 
                                      int vertNum, 
                                      int matID );

    // Set GL polygon array to scene geometry
    bool SetXRayPolygonArray( const PRIMITIVES_ARRAY* poligones,
                              int firstVert, 
                              int vertOffset, 
                              int vertNum, 
                              int matID );

    // Init OpenCL framework to use in XRay 
    bool Init();
  
    // Release OpenCL and its devices
    void Release();

    // Change scene geometry in XRay (use GL)
    bool WriteXRaySceneToDevice();

    // Update XRay board geometry (use OpenCL)
    bool UpdateXRaySceneGeometry( bool isCheck );

    // Check to see is the structure modified
    bool CheckXRayStructure( const GlStructure* pStruct );

    // Update light sources of scene
    bool UpdateXRayLightSources( const GLdouble aInvModelView[16] );

    // Update the environment map for XRay tracing
    bool UpdateXRayEnvironmentMap();

    // Resize XRay output image
    bool ResizeXRayOutputBuffer( const cl_int sizeX, const cl_int sizeY );


    // Runs OpenCL kernels
    bool RunXRayOpenCLKernels( const Graphic3d_CView& view,
                               const GLfloat origins[16],
                               const GLfloat directions[16],
                               const int sizeX,
                               const int sizeY );

    // Use OpenCL raytracing
    bool Raytrace( const Graphic3d_CView& view,
                   const int sizeX, 
                   int sizeY, 
                   const Tint toSwap );

protected:

    BinSAHBuilder SAHBuilder;

    // Result of OpenCL initialization.
    XRayInitStatus xrInitStatus;
    // Nvidia OpenCL used?
    bool IsNvidiaPlatform;
    // geometry data valid?
    bool IsXRayDataValid;

    // scene geometry data for raytrace
    XRayScene xrSceneData;

    // bounding sphere of the scene
    float xrSceneRadius;
    float xrSceneEpsilon;

    cl_context xrComputeContext;
    cl_command_queue xrQueue;
    cl_program xrProgram;
    cl_kernel xrRenderKernel;
    cl_kernel xrSmoothKernel;
    cl_mem xrEnvironment;
    cl_mem xrOutputImage;
    cl_mem xrOutputImageSmooth;
    cl_mem xrNormalBuffer;
    cl_mem xrVertexBuffer;
    cl_mem xrTriangleBuffer;
    cl_mem xrNodeMinPointBuffer;
    cl_mem xrNodeMaxPointBuffer;
    cl_mem xrNodeDataRcrdBuffer;
    cl_mem xrMaterialBuffer;
    cl_mem xrLightSourceBuffer;
    GLuint xrOutputTexture[2];

    unsigned int glViewModStatus;
    unsigned int glLayersModStatus;
    std::map<const GlStructure*,unsigned int> glStructureStates;

protected:
    Handle(GLView) glView; // just one view is supported

protected: 
    Handle(GLRenderFilter) glRenderFilter;
    Handle(GLTexture)      glTextureBound;
};

#endif // __xray_h__
