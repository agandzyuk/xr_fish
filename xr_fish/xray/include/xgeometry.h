#ifndef __xray_geometry_h__
#define __xray_geometry_h__

#include "gl_cl.h"
#include "axbbox.h"

#include "gstructures.h"
#include "gprimitives.h"

namespace XRaySceneNamespace
{
    // is the group contains xray geometry
    bool IsXRGroup(const GLGroup* pGroup);

    // is the element contains xray geometry
    bool IsXRElement(const GLElementNode* pNode);

    // is the structure contains xray geometry
    bool IsXRStructure(const GLStructure* pStruct);
}

// material attributes GL
class XRayMaterial
{
public:

    XRVec4f Ambient;
    XRVec4f Diffuse;
    XRVec4f Specular;
    XRVec4f Emission;
    XRVec4f Reflection;
    XRVec4f Refraction;
    XRVec4f Transparency;

public:

    // default material
    XRayMaterial();

    // material with properties
    XRayMaterial( const XRVec4f& theAmbient,
                  const XRVec4f& theDiffuse,
                  const XRVec4f& theSpecular );

    // material with properties
    XRayMaterial( const XRVec4f& theAmbient,
                  const XRVec4f& theDiffuse,
                  const XRVec4f& theSpecular,
                  const XRVec4f& theEmission,
                  const XRVec4f& theTranspar );

    // material with properties
    XRayMaterial( const XRVec4f& theAmbient,
                  const XRVec4f& theDiffuse,
                  const XRVec4f& theSpecular,
                  const XRVec4f& theEmission,
                  const XRVec4f& theTranspar,
                  const XRVec4f& theReflection,
                  const XRVec4f& theRefraction );

    const float* serialize() { return reinterpret_cast<float*> (this); }
};

// light source GL
class XRayLight
{
public:

    XRVec4f Ambient;
    XRVec4f Diffuse;
    XRVec4f Position;

public:
    // ambient light
    XRayLight( const XRVec4f& theAmbient );

    // or diffuse light
    XRayLight( const XRVec4f& theDiffuse,
               const XRVec4f& thePosition );

    const float* serialize() { return reinterpret_cast<float*> (this); }
};

// GL geometry data
struct XRayScene
{
    // axis bounding box of scene
    AxisBBox AxBox;

    // array of normals
    XRArray4f xrNormals;

    // array of vertices
    XRArray4f xrVertices;

    // array of triangles
    XRArray4i xrTriangles;

    // array of 'front' material properties
    std::vector<XRayMaterial,
              GCollectionAllocator<XRayMaterial> > xrMaterials;

    // array of properties of light sources
    std::vector<XRayLight,
              GCollectionAllocator<XRayLight> > xrLightSources;

    // clear scene geometry and material data
    void Clear();

    // AxisBBox of triangle
    AxisBBox Box( const int triangle ) const;

    // centroid of triangle
    XRVec4f Centerid( const int triangle ) const;
};

// parameters of SAH tree node (currently BVH only)
class XRaySAHNode
{
    friend class XRaySAH;

public:

    // Minimum point of node's bounding box
    XRVec4f MinPoint;
    // Maximum point of node's bounding box
    XRVec4f MaxPoint;
    // Data vector (stores data fields of the node)
    XRVec4i DataRcrd;

public:
    // empty SAH node
    XRaySAHNode();

    // new SAH node by specified data
    XRaySAHNode( const XRVec4f& minPoint,
                 const XRVec4f& maxPoint,
                 const XRVec4i& dataRcrd );

    // new SAH node by specified triangle
    XRaySAHNode( const XRVec4f& minPoint,
                 const XRVec4f& maxPoint,
                 const int begTriangle,
                 const int endTriangle );

    // new SAH node by specified triangle
    XRaySAHNode( const AxisBBox& axBBox,
                 const int begTriangle,
                 const int endTriangle );

    // left child of noleaf node
    int LeftChild() const { return DataRcrd.y(); }
    void SetLeftChild( int ñhild ) { DataRcrd.y() = ñhild; }

    // right child of noleaf node
    int RightChild() const { return DataRcrd.z(); }
    void SetRightChild( int ñhild ) { DataRcrd.z() = ñhild; }

    // index of begin point for triangle of leaf node
    int BegTriangle() const { return DataRcrd.y(); }
    void SetBegTriangle( int index ) { DataRcrd.y() = index; }

    // index of end point for triangle of leaf node
    int EndTriangle() const { return DataRcrd.z(); }
    void SetEndTriangle( int index ) { DataRcrd.z() = index; }

    // level of node in SAH tree
    int Level() const { return DataRcrd.w(); }
    void SetLevel( int level ) { DataRcrd.w() = level; }

    // is outer node (leaf)?
    bool IsOuter() const { return DataRcrd.x() == 1; }

    // sets type leaf 
    void SetOuter() { DataRcrd.x() = 1; }
    // sets type noleaf
    void SetInner() { DataRcrd.x() = 0; }
};

// parameters of SAH tree
class XRaySAH
{
public:

    void CleanUp();
    int AddNode( const XRaySAHNode& node );
    XRaySAHNode Node( const int index ) const;
    void SetNode( const int index, const XRaySAHNode& node );

public:
    XRArray4f MinPointBuffer;
    XRArray4f MaxPointBuffer;
    XRArray4i DataRcrdBuffer;
};

// parameters of single node bin (slice of AxisBBox)
struct XRaySAHBin
{
    // Creates new node bin
    XRaySAHBin(): Count(0) { }

    int Count;
    AxisBBox Volume;
};

// building task of node
struct XRaySAHNodeTask
{
    XRaySAHNodeTask();
    XRaySAHNodeTask( const int nodeToBuild,
                     const int begTriangle,
                     const int endTriangle );

    // Index of building tree node
    int NodeToBuild;
    // Index of start node triangle
    int BegTriangle;
    // Index of final node triangle
    int EndTriangle;
};

// The array of bins of SAH tree node
typedef std::vector<XRaySAHBin,
                    GCollectionAllocator<XRaySAHBin> > XRBinVector;

// SAH builder
class BinSAHBuilder
{
public:
    // Builded tree
    XRaySAH Tree;

    // Maximum depth in tree
    int MaxDepth;

public:

    BinSAHBuilder();
    ~BinSAHBuilder();

    void Build( XRayScene& geometry, const float epsilon = 1e-3f );
    void CleanUp();

private:
    void Build( XRayScene& geometry, const int task );
    void Arrange( XRayScene& geometry, 
                  const XRaySAHNode& node,
                  XRBinVector& bins, 
                  const int axis );
    int SplitByIntervals( XRayScene& geometry, 
                          const int first, 
                          const int last,
                          XRaySAHNode& node, 
                          int bin, const int axis );

private:
    std::vector<XRaySAHNodeTask> TaskQueue;
};

#endif // __xray_geometry_h__
