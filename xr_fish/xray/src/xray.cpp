#include "xray.h"
#include "gltexture.h"
#include "glcontext.h"

#pragma warning(disable:4996) // some depricates from OpenCL 1.1

/////////////////////////////////////////////////////////////////
// OpenCL source of ray-tracing kernels
extern const char XRAY_OPENCL_SOURCE[];

/////////////////////////////////////////////////////////////////
// Multiples matrix by 4D vector
template< typename T >
XRVec4f MatVecMult( const T x[16], const XRVec4f& v )
{
  return XRVec4f (
        static_cast<float>(x[ 0] * v.x() + x[ 4] * v.y() +
                           x[ 8] * v.z() + x[12] * v.w()),
        static_cast<float>(x[ 1] * v.x() + x[ 5] * v.y() +
                           x[ 9] * v.z() + x[13] * v.w()),
        static_cast<float>(x[ 2] * v.x() + x[ 6] * v.y() +
                           x[10] * v.z() + x[14] * v.w()),
        static_cast<float>(x[ 3] * v.x() + x[ 7] * v.y() +
                           x[11] * v.z() + x[15] * v.w())
        );
}

/////////////////////////////////////////////////////////////////
// XRayBoard implementation

//----------------------------------------------------------------
// Update the environment map for XRay tracing
//----------------------------------------------------------------
bool XRayBoard::UpdateXRayEnvironmentMap()
{
    if( glView.IsNull() )
        return false;

    if( glViewModStatus == glView->ModificationState() )
        return true;

    cl_int error = CL_SUCCESS;

    if ( xrEnvironment )
        clReleaseMemObject( xrEnvironment );

    int sizeX = 1, sizeY = 1;
                                          // Note: environnement mapping is enabled
    if( !glView->TextureEnv().IsNull() && glView->SurfaceDetail() != VIEW3D_TOD_NONE ) 
    {                                                                
        sizeX = (glView->TextureEnv()->SizeX() <= 0) ? 1 : glView->TextureEnv()->SizeX();
        sizeY = (glView->TextureEnv()->SizeY() <= 0) ? 1 : glView->TextureEnv()->SizeY();
    }

    cl_image_format imageFormat;

    imageFormat.image_channel_order = CL_RGBA;
    imageFormat.image_channel_data_type = CL_FLOAT;

    xrEnvironment = clCreateImage2D( xrComputeContext, CL_MEM_READ_ONLY,
                                     &imageFormat, sizeX, sizeY, 0, 0, &error );

    cl_float* pPixelData = new cl_float[sizeX * sizeY * 4];
    // Note: OpenCL image isn't compatible with GL texteres

    if( !glView->TextureEnv().IsNull() && glView->SurfaceDetail() != VIEW3D_TOD_NONE ) 
    {
        glView->TextureEnv()->Bind( GetGlContext() );
        glGetTexImage( GL_TEXTURE_2D,
                       0,
                       GL_RGBA,
                       GL_FLOAT,
                       pPixelData );

        glView->TextureEnv()->Unbind( GetGlContext() );
    }
    else
    {
        for( int pixel = 0; pixel < sizeX * sizeY * 4; ++pixel )
            pPixelData[pixel] = 0.f;
    }

    unsigned int imageOffset[] = {0,0,0};
    unsigned int imageRegion[] = { sizeX, sizeY, 1 };
    
    error |= clEnqueueWriteImage( xrQueue, xrEnvironment, CL_TRUE,
                                  imageOffset, imageRegion, 
                                  0, 0, pPixelData, 0, 0, 0 );
#ifdef XRAY_PRINT_INFO
    if( error != CL_SUCCESS )
        std::cout << "clEnqueueWriteImage: Failed to write map image!\n";
#endif

    delete[] pPixelData;
    glViewModStatus = glView->ModificationState();
    return (error == CL_SUCCESS);
}

//----------------------------------------------------------------
// Update XRay board geometry (use OpenCL)
//----------------------------------------------------------------
bool XRayBoard::UpdateXRaySceneGeometry( bool isCheck )
{
    if( glView.IsNull() )
        return false;

    // set 'isCheck' to false to disable analysis for modifications on scene per frame
    if( !isCheck ) 
    {
        xrSceneData.Clear();
        IsXRayDataValid = false;
    }
    else if (glLayersModStatus != glView->LayerList().ModificationState())
    {
        return UpdateXRaySceneGeometry( false );
    }

    float* pTransform = 0;

    // Remove records from the hash map what are out-of-date 
    std::set<const GLStructure*> elements;

    const GLLayerList& alist = glView->LayerList();
    for( GLSequenceOfLayers::Iterator listIt = alist.Layers(); listIt.More(); listIt.Next() )
    {
        const GLPriorityList& priorityList = listIt.Value().PriorityList();
        if( priorityList.NbStructures() == 0 )
            continue;

        const GLArrayOfStructure& glStructures = priorityList.ArrayOfStructures();
        int lenght = glStructures.Length();
        for( int index = 0; index < lenght; ++index )
        {
            GLSequenceOfStructure::Iterator structIt;
            for( structIt.Init( glStructures(index) ); structIt.More(); structIt.Next() )
            {
                const GLStructure* pStructure = structIt.Value();
                if( isCheck )
                {
                    if ( CheckXRayStructure( pStructure ) )
                        return UpdateXRaySceneGeometry (false);
                }
                else
                {
                    if( !pStructure->IsRaytracable() )
                        continue;

                    if( pStructure->Transformation()->mat )
                    {
                        if ( pTransform == 0 )
                            pTransform = new float[16];

                        for( int i = 0; i < 4; ++i )
                            for( int j = 0; j < 4; ++j )
                            {
                                pTransform[j * 4 + i] = pStructure->Transformation()->mat[i][j];
                            }
                    }
                    SetXRayStructure( pStructure, pTransform, elements );
                }
            }
        }
    }

    if( !isCheck )
    {
        // Update hash map: remove out-of-date records
        std::map<const GLStructure*, unsigned int>::iterator structIt = 
            glStructureStates.begin();

        while( structIt != glStructureStates.end() )
        {
            if( elements.find( structIt->first ) == elements.end() )
                glStructureStates.erase( structIt++ );
            else
                ++structIt;
        }

        // Renew GL layers list state
        glLayersModStatus = glView->LayerList().ModificationState();

#ifdef XRAY_TRACE_PRINT_INFO
        OSD_Timer timer;
        timer.Start();
#endif

        SAHBuilder.Build( xrSceneData );

#ifdef XRAY_TRACE_PRINT_INFO
        std::cout << " XRay build: " << aTimer.ElapsedTime() << " (ms) for "
                  << xrSceneData.xrTriangles.size() / 1000 << "K tgs\n";
#endif

        const float scaleFactor = 1.5f; // normal

        xrSceneRadius = scaleFactor *
                        Max( Max( fabsf(xrSceneData.AxBox.Min().x()),
                             Max( fabsf(xrSceneData.AxBox.Min().y()),
                                  fabsf(xrSceneData.AxBox.Min().z()) ) ),
                             Max( fabsf(xrSceneData.AxBox.Max().x()),
                             Max( fabsf(xrSceneData.AxBox.Max().y()),
                                  fabsf(xrSceneData.AxBox.Max().z()) ) ) );

        xrSceneEpsilon = Max( 1e-4f, xrSceneRadius * 1e-4f );
        return WriteXRaySceneToDevice();
    }
    delete [] pTransform;
    return true;
}

//----------------------------------------------------------------
// Set GL structure to scene geometry
//----------------------------------------------------------------
bool XRayBoard::CheckXRayStructure( const GLStructure* pStruct )
{
    if( !pStruct->IsRaytracable() )
    {
        // all elements were removed from the structure
        if( pStruct->ModificationState() > 0 )
        {
            pStruct->ResetModificationState();
            return true;
        }
        return false;
    }

    std::map<const GLStructure*,unsigned int>::iterator structState = 
        glStructureStates.find( pStruct );

    if( structState != glStructureStates.end() )
        return ( structState->second != pStruct->ModificationState() );

    return true;
}

//----------------------------------------------------------------
// Helper to get material properties
//----------------------------------------------------------------
void GetMaterialProps( const GL_SURF_PROP& prop, XRayMaterial& material )
{
    const float* pSrcAmb = prop.isphysic ? prop.ambcol.rgb : prop.matcol.rgb;
    material.Ambient = XRVec4f( pSrcAmb[0] * prop.amb,
                                pSrcAmb[1] * prop.amb,
                                pSrcAmb[2] * prop.amb,
                                1.0f );

    const float* pSrcDif = prop.isphysic ? prop.difcol.rgb : prop.matcol.rgb;
    material.Diffuse = XRVec4f( pSrcDif[0] * prop.diff,
                                pSrcDif[1] * prop.diff,
                                pSrcDif[2] * prop.diff,
                                1.0f );

    const float  aDefSpecCol[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    const float* pSrcSpe = prop.isphysic ? prop.speccol.rgb : aDefSpecCol;
    material.Specular = XRVec4f( pSrcSpe[0] * prop.spec,
                                 pSrcSpe[1] * prop.spec,
                                 pSrcSpe[2] * prop.spec,
                                 prop.shine );

    const float* pSrcEms = prop.isphysic ? prop.emscol.rgb : prop.matcol.rgb;
    material.Emission = XRVec4f( pSrcEms[0] * prop.emsv,
                                 pSrcEms[1] * prop.emsv,
                                 pSrcEms[2] * prop.emsv,
                                 1.0f );

    // Note: using the sub-linear blending function
    // to produce realistic-looking transparency effect
    material.Transparency = XRVec4f( powf(prop.trans, 0.75f),
                                     1.f - prop.trans,
                                     1.f,
                                     1.f );

    const float maxRefl = Max( material.Diffuse.x() + material.Specular.x(),
                              Max( material.Diffuse.y() + material.Specular.y(),
                                   material.Diffuse.z() + material.Specular.z() ) );

    const float reflectionScale = 0.75f / maxRefl;

    material.Reflection = XRVec4f( prop.speccol.rgb[0] * prop.spec,
                                   prop.speccol.rgb[1] * prop.spec,
                                   prop.speccol.rgb[2] * prop.spec,
                                   0.f ) * reflectionScale;
}

//----------------------------------------------------------------
// Set GL structure to scene geometry
//----------------------------------------------------------------
bool XRayBoard::SetXRayStructure( const GLStructure* pStruct,
                                  const float* pTransform,
                                  std::set<const GLStructure*>& elements )
{
#ifdef XRAY_TRACE_PRINT_INFO
    std::cout << "SetXRayStructure\n";
#endif

    elements.insert( pStruct );
    if( !pStruct->IsVisible() )
    {
        glStructureStates[pStruct] = pStruct->ModificationState();
        return true;
    }

    // Get structure material
    int structMatID = -1;

    if( pStruct->AspectFace() )
    {
        structMatID = static_cast<int>( xrSceneData.xrMaterials.size() );

        XRayMaterial structMaterial;
        GetMaterialProps( pStruct->AspectFace()->IntFront(), structMaterial );
        xrSceneData.xrMaterials.push_back( structMaterial );
    }

    GLListOfGroup::Iterator groupsIt = pStruct->Groups();
    while( groupsIt.More() )
    {
        // Get group material
        int groupMatID = -1;
        if( groupsIt.Value()->AspectFace() )
        {
            groupMatID = static_cast<int>( xrSceneData.xrMaterials.size() );

            XRayMaterial groupMaterial;
            GetMaterialProps( groupsIt.Value()->AspectFace()->IntFront(), groupMaterial );
            xrSceneData.xrMaterials.push_back( groupMaterial );
        }

        int matID = groupMatID < 0 ? structMatID : groupMatID;
        if( structMatID < 0 && groupMatID < 0 )
        {
            matID = static_cast<int>( xrSceneData.xrMaterials.size() );
            xrSceneData.xrMaterials.push_back( XRayMaterial() );
        }

        // Set GL elements from group (primitives only)
        for( const GLElementNode* pNode = groupsIt.Value()->FirstNode(); pNode; pNode = pNode->next )
        {
            if( TelNil == pNode->type )
            {
                GLAspectFace* pAspect = dynamic_cast<GLAspectFace*>( pNode->elem );
                if( pAspect )
                {
                    matID = static_cast<int>( xrSceneData.xrMaterials.size() );
                    XRayMaterial material;
                    GetMaterialProps( pAspect->IntFront(), material );
                    xrSceneData.xrMaterials.push_back( material );
                }
            }
            else if (TelParray == pNode->type)
            {
                GLPrimitiveArray* pPrimArray = dynamic_cast<GLPrimitiveArray*>( pNode->elem );
                if( pPrimArray )
                    SetXRayPrimitiveArray( pPrimArray->PArray(), matID, pTransform );
            }
        }
        groupsIt.Next();
    }

    float* newTransform = 0;

    // Process all connected GL structures
    GLListOfStructure::Iterator Its( pStruct->ConnectedStructures() );

    while( Its.More() )
    {
        if( Its.Value()->Transformation()->mat )
        {
            newTransform = new float[16];
            for( int i = 0; i < 4; ++i )
                for ( int j = 0; j < 4; ++j )
                {
                    newTransform[j * 4 + i] = Its.Value()->Transformation()->mat[i][j];
                }
        }
        if( Its.Value()->IsRaytracable() )
            SetXRayStructure( Its.Value(), newTransform ? newTransform : pTransform, elements );

        Its.Next();
    }
    delete[] newTransform;

    glStructureStates[pStruct] = pStruct->ModificationState();
    return true;
}

//----------------------------------------------------------------
// Set GL primitives array to scene geometry
//----------------------------------------------------------------
bool XRayBoard::SetXRayPrimitiveArray( const PRIMITIVES_ARRAY* primitives,
                                       int matID,
                                       const float* pTransform)
{
    if( primitives->type != TelPolygonsArrayType &&
        primitives->type != TelTrianglesArrayType &&
        primitives->type != TelQuadranglesArrayType &&
        primitives->type != TelTriangleFansArrayType &&
        primitives->type != TelTriangleStripsArrayType &&
        primitives->type != TelQuadrangleStripsArrayType )
    {
        return true;
    }

    if( primitives->vertices == 0 )
        return false;

    xrSceneData.xrVertices.reserve( xrSceneData.xrVertices.size() + primitives->num_vertexs );

    const int firstVert = static_cast<int>( xrSceneData.xrVertices.size() );
    for( int vert = 0; vert < primitives->num_vertexs; ++vert )
    {
        XRVec4f vertex( primitives->vertices[vert].xyz[0],
                        primitives->vertices[vert].xyz[1],
                        primitives->vertices[vert].xyz[2],
                        1.f );

        if( pTransform )
            vertex = MatVecMult( pTransform, vertex );

        xrSceneData.xrVertices.push_back( vertex );
        xrSceneData.AxBox.Add( vertex );
    }

    xrSceneData.xrNormals.reserve( xrSceneData.xrNormals.size() + primitives->num_vertexs );
    for( int norm = 0; norm < primitives->num_vertexs; ++norm )
    {
        XRVec4f normal;
        if( primitives->vnormals )
        {
            normal = XRVec4f( primitives->vnormals[norm].xyz[0],
                              primitives->vnormals[norm].xyz[1],
                              primitives->vnormals[norm].xyz[2],
                              0.f );
            if( pTransform )
                normal = MatVecMult( pTransform, normal );
        }
        xrSceneData.xrNormals.push_back( normal );
    }

    if( primitives->num_bounds > 0 )
    {
#ifdef XRAY_TRACE_PRINT_INFO
        std::cout << "\tNumber of bounds = " << primitives->num_bounds << "\n";
#endif

        int vertOffset = 0;
        for( int nBound = 0; nBound < primitives->num_bounds; ++nBound )
        {
            const int vertNum = primitives->bounds[nBound];

#ifdef XRAY_TRACE_PRINT_INFO
            std::cout << "\tAdd indices from " << nBound << ": " <<
                            vertOffset << ", " << vertNum << "\n";
#endif

            if( !SetXRayVertexIndices( primitives, 
                                       firstVert, 
                                       vertOffset, 
                                       vertNum, 
                                       matID) )
            {
                return false;
            }
            vertOffset += vertNum;
        }
    }
    else
    {
        const int vertNum = primitives->num_edges > 0 ? 
            primitives->num_edges : primitives->num_vertexs;

#ifdef XRAY_TRACE_PRINT_INFO
        std::cout << "\tSet indices " << vertNum << "\n";
#endif

        return SetXRayVertexIndices( primitives, 
                                     firstVert, 
                                     0, 
                                     vertNum, 
                                     matID );
    }
    return true;
}

//----------------------------------------------------------------
// Set vertex indices to scene geometry
//----------------------------------------------------------------
bool XRayBoard::SetXRayVertexIndices( const PRIMITIVES_ARRAY* indicies,
                                      int firstVert,
                                      int vertOffset,
                                      int vertNum,
                                      int matID )
{
    xrSceneData.xrTriangles.reserve( xrSceneData.xrTriangles.size() + vertNum );
    switch( indicies->type )
    {
    case TelTrianglesArrayType:
        return SetXRayTriangleArray( indicies, firstVert, vertOffset, vertNum, matID );
    case TelQuadranglesArrayType:
        return SetXRayQuadrangleArray( indicies, firstVert, vertOffset, vertNum, matID );
    case TelTriangleFansArrayType:
        return SetXRayTriangleFanArray( indicies, firstVert, vertOffset, vertNum, matID );
    case TelTriangleStripsArrayType:
        return SetXRayTriangleStripArray( indicies, firstVert, vertOffset, vertNum, matID );
    case TelQuadrangleStripsArrayType: 
        return SetXRayQuadrangleStripArray( indicies, firstVert, vertOffset, vertNum, matID );
    case TelPolygonsArrayType:
        return SetXRayPolygonArray( indicies, firstVert, vertOffset, vertNum, matID );
    default:
        return false;
  }
}

//----------------------------------------------------------------
// Set GL triangle array to scene geometry
//----------------------------------------------------------------
bool XRayBoard::SetXRayTriangleArray( const PRIMITIVES_ARRAY* triangles,
                                      int firstVert,
                                      int vertOffset,
                                      int vertNum,
                                      int matID )
{
    if( vertNum < 3 )
        return true;

    if( triangles->num_edges > 0 )
    {
        for( int vert = vertOffset; vert < vertOffset + vertNum - 2; vert += 3 )
        {   
            xrSceneData.xrTriangles.push_back( XRVec4i( firstVert + triangles->edges[vert + 0],
                                                        firstVert + triangles->edges[vert + 1],
                                                        firstVert + triangles->edges[vert + 2],
                                                        matID ) );
        }
    }
    else
    {
        for( int vert = vertOffset; vert < vertOffset + vertNum - 2; vert += 3)
        {
            xrSceneData.xrTriangles.push_back( XRVec4i( firstVert + vert + 0,
                                                        firstVert + vert + 1,
                                                        firstVert + vert + 2,
                                                        matID ) );
        }
    }
    return true;
}

//----------------------------------------------------------------
// Set GL triangle fan array to scene geometry
//----------------------------------------------------------------
bool XRayBoard::SetXRayTriangleFanArray( const PRIMITIVES_ARRAY* triangulars,
                                         int firstVert,
                                         int vertOffset,
                                         int vertNum,
                                         int matID )
{
    if( vertNum < 3 )
        return true;

    if( triangulars->num_edges > 0 )
    {
        for( int vert = vertOffset; vert < vertOffset + vertNum - 2; ++vert )
        {
            xrSceneData.xrTriangles.push_back( XRVec4i( firstVert + triangulars->edges[vertOffset],
                                                        firstVert + triangulars->edges[vert + 1],
                                                        firstVert + triangulars->edges[vert + 2],
                                                        matID ) );
        }
    }
    else
    {
        for( int vert = vertOffset; vert < vertOffset + vertNum - 2; ++vert )
        {
            xrSceneData.xrTriangles.push_back( XRVec4i( firstVert + vertOffset,
                                                        firstVert + vert + 1,
                                                        firstVert + vert + 2,
                                                        matID ) );
        }
    }
    return true;
}

//----------------------------------------------------------------
// Set GL triangle strip array to scene geometry
//----------------------------------------------------------------
bool XRayBoard::SetXRayTriangleStripArray( const PRIMITIVES_ARRAY* triangulars,
                                           int firstVert,
                                           int vertOffset,
                                           int vertNum,
                                           int matID )
{
    if( vertNum < 3 )
        return true;

    if( triangulars->num_edges > 0 )
    {
        xrSceneData.xrTriangles.push_back( XRVec4i( firstVert + triangulars->edges[vertOffset + 0],
                                                    firstVert + triangulars->edges[vertOffset + 1],
                                                    firstVert + triangulars->edges[vertOffset + 2],
                                                    matID ) );

        for( int vert = vertOffset + 1, aTriNum = 1; vert < vertOffset + vertNum - 2; ++vert, ++aTriNum )
        {
            xrSceneData.xrTriangles.push_back( XRVec4i( firstVert + triangulars->edges[vert + (aTriNum % 2) ? 1 : 0],
                                                        firstVert + triangulars->edges[vert + (aTriNum % 2) ? 0 : 1],
                                                        firstVert + triangulars->edges[vert + 2],
                                                        matID ) );
        }
    }
    else
    {
        xrSceneData.xrTriangles.push_back( XRVec4i( firstVert + vertOffset + 0,
                                                    firstVert + vertOffset + 1,
                                                    firstVert + vertOffset + 2,
                                                    matID ) );

        for( int vert = vertOffset + 1, aTriNum = 1; vert < vertOffset + vertNum - 2; ++vert, ++aTriNum )
        {
            xrSceneData.xrTriangles.push_back( XRVec4i( firstVert + vert + ( aTriNum % 2 ) ? 1 : 0,
                                                        firstVert + vert + ( aTriNum % 2 ) ? 0 : 1,
                                                        firstVert + vert + 2,
                                                        matID ) );
        }
    }
    return true;
}

//----------------------------------------------------------------
// Set GL quad array to scene geometry
//----------------------------------------------------------------
bool XRayBoard::SetXRayQuadrangleArray( const PRIMITIVES_ARRAY* quads,
                                        int firstVert,
                                        int vertOffset,
                                        int vertNum,
                                        int matID )
{
    if( vertNum < 4 )
        return true;

    if( quads->num_edges > 0 )
    {
        for( int vert = vertOffset; vert < vertOffset + vertNum - 3; vert += 4 )
        {
            xrSceneData.xrTriangles.push_back( XRVec4i( firstVert + quads->edges[vert + 0],
                                                        firstVert + quads->edges[vert + 1],
                                                        firstVert + quads->edges[vert + 2],
                                                        matID ) );
  
            xrSceneData.xrTriangles.push_back( XRVec4i( firstVert + quads->edges[vert + 0],
                                                        firstVert + quads->edges[vert + 2],
                                                        firstVert + quads->edges[vert + 3],
                                                        matID ) );
        }
    }
    else
    {
        for( int vert = vertOffset; vert < vertOffset + vertNum - 3; vert += 4 )
        {
            xrSceneData.xrTriangles.push_back( XRVec4i( firstVert + vert + 0,
                                                      firstVert + vert + 1,
                                                      firstVert + vert + 2,
                                                      matID ) );

            xrSceneData.xrTriangles.push_back( XRVec4i( firstVert + vert + 0,
                                                        firstVert + vert + 2,
                                                        firstVert + vert + 3,
                                                        matID ) );
        }
    }
    return true;
}

//----------------------------------------------------------------
// Set GL quad strip array to scene geometry
//----------------------------------------------------------------
bool XRayBoard::SetXRayQuadrangleStripArray( const PRIMITIVES_ARRAY* quads,
                                             int firstVert,
                                             int vertOffset,
                                             int vertNum,
                                             int matID )
{
    if( vertNum < 4 )
        return true;

    if( quads->num_edges > 0 )
    {
        xrSceneData.xrTriangles.push_back( XRVec4i(
                                firstVert + quads->edges[vertOffset + 0],
                                firstVert + quads->edges[vertOffset + 1],
                                firstVert + quads->edges[vertOffset + 2],
                                matID ) );

        xrSceneData.xrTriangles.push_back( XRVec4i(
                                firstVert + quads->edges[vertOffset + 1],
                                firstVert + quads->edges[vertOffset + 3],
                                firstVert + quads->edges[vertOffset + 2],
                                matID ) );

        for( int vert = vertOffset + 2; vert < vertOffset + vertNum - 3; vert += 2 )
        {
            xrSceneData.xrTriangles.push_back( XRVec4i(
                                    firstVert + quads->edges[vert + 0],
                                    firstVert + quads->edges[vert + 1],
                                    firstVert + quads->edges[vert + 2],
                                    matID ) );

            xrSceneData.xrTriangles.push_back( XRVec4i(
                                    firstVert + quads->edges[vert + 1],
                                    firstVert + quads->edges[vert + 3],
                                    firstVert + quads->edges[vert + 2],
                                    matID ) );
        }
    }
    else
    {
        xrSceneData.xrTriangles.push_back( XRVec4i( firstVert + 0,
                                                    firstVert + 1,
                                                    firstVert + 2,
                                                    matID ) );

        xrSceneData.xrTriangles.push_back( XRVec4i( firstVert + 1,
                                                    firstVert + 3,
                                                    firstVert + 2,
                                                    matID ) );

        for( int vert = vertOffset + 2; vert < vertOffset + vertNum - 3; vert += 2 )
        {
            xrSceneData.xrTriangles.push_back( XRVec4i( firstVert + vert + 0,
                                                        firstVert + vert + 1,
                                                        firstVert + vert + 2,
                                                        matID ) );

            xrSceneData.xrTriangles.push_back( XRVec4i( firstVert + vert + 1,
                                                        firstVert + vert + 3,
                                                        firstVert + vert + 2,
                                                        matID ) );
        }
    }
    return true;
}

//----------------------------------------------------------------
// Set GL polygon array to scene geometry
//----------------------------------------------------------------
bool XRayBoard::SetXRayPolygonArray( const PRIMITIVES_ARRAY* poligones,
                                     int firstVert,
                                     int vertOffset,
                                     int vertNum,
                                     int matID )
{
    if( poligones->num_vertexs < 3 )
        return true;

    if( poligones->edges )
    {
        for( int vert = vertOffset; vert < vertOffset + vertNum - 2; ++vert )
        {
            xrSceneData.xrTriangles.push_back( XRVec4i( firstVert + poligones->edges[vertOffset],
                                                        firstVert + poligones->edges[vert + 1],
                                                        firstVert + poligones->edges[vert + 2],
                                                        matID ) );
        }
    }
    else
    {
        for( int vert = vertOffset; vert < vertOffset + vertNum - 2; ++vert )
        {
            xrSceneData.xrTriangles.push_back( XRVec4i( firstVert + vertOffset,
                                                        firstVert + vert + 1,
                                                        firstVert + vert + 2,
                                                        matID ) );
        }
    }
    return true;
}

//----------------------------------------------------------------
// Update light sources of scene
//----------------------------------------------------------------
bool XRayBoard::UpdateXRayLightSources( const GLdouble theInvModelView[16] )
{
    xrSceneData.xrLightSources.clear();

    XRVec4f ambient( 0.0f, 0.0f, 0.0f, 0.0f );
    for( GLListOfLight::Iterator Itl( glView->LightList() ); Itl.More(); Itl.Next() )
    {
        const GLLight& light = Itl.Value();
        if( light.Type == Visual3d_TOLS_AMBIENT )
        {
            ambient += XRVec4f( light.Color.r(), light.Color.g(), light.Color.b(), 0.0f );
            continue;
        }

        XRVec4f diffuse( light.Color.r(), light.Color.g(), light.Color.b(), 1.0f );
        XRVec4f position( -light.Direction.x(), -light.Direction.y(), -light.Direction.z(), 0.0f );
        if( light.Type != Visual3d_TOLS_DIRECTIONAL )
            position = XRVec4f( light.Position.x(), light.Position.y(), light.Position.z(), 1.0f );

        if( light.IsHeadlight )
            position = MatVecMult( theInvModelView, position );
    
        xrSceneData.xrLightSources.push_back( XRayLight(diffuse, position) );
    }

    if( xrSceneData.xrLightSources.size() > 0 )
        xrSceneData.xrLightSources.front().Ambient += ambient;
    else
        xrSceneData.xrLightSources.push_back( XRayLight( XRVec4f(ambient.rgb(), -1.0f) ) );

    cl_int error = CL_SUCCESS;

    if( xrLightSourceBuffer )
        clReleaseMemObject( xrLightSourceBuffer );

    const unsigned int myLightBufferSize = xrSceneData.xrLightSources.size() > 0
                                 ? xrSceneData.xrLightSources.size()
                                 : 1;

    xrLightSourceBuffer = clCreateBuffer( xrComputeContext, CL_MEM_READ_ONLY,
                                          myLightBufferSize * sizeof(XRayLight),
                                          0, &error);

    if( xrSceneData.xrLightSources.size() > 0 )
    {
        const void* pData = xrSceneData.xrLightSources.front().Packed();
        error |= clEnqueueWriteBuffer( xrQueue, xrLightSourceBuffer, CL_TRUE, 0,
                                       myLightBufferSize * sizeof(XRayLight), pData,
                                       0, 0, 0 );
    }

#ifdef XRAY_TRACE_PRINT_INFO
    if (error != CL_SUCCESS)
    {
        std::cout << "Failed to set light sources!\n";
        return false;
    }
#endif

    return true;
}

//----------------------------------------------------------------
// Get an info about OpenCL libraries availability
//----------------------------------------------------------------
bool CheckOpenCL()
{
#ifdef _WIN32

    __try
    {
        cl_uint platforms;
        clGetPlatformIDs( 0, 0, &platforms );
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        return false;
    }

#endif
    return true;
}

//----------------------------------------------------------------
// Init OpenCL framework to use in XRay 
//----------------------------------------------------------------
bool XRayBoard::Init()
{
    if( xrInitStatus != CL_CLIS_NONE )
        return (xrInitStatus == CL_CLIS_INIT);

    if( !CheckOpenCL() )
    {
        xrInitStatus = CL_CLIS_FAIL; 
        GetGlContext()->PushMessage( GL_DEBUG_SOURCE_APPLICATION_ARB,
                                     GL_DEBUG_TYPE_ERROR_ARB,
                                     0,
                                     GL_DEBUG_SEVERITY_HIGH_ARB,
                                     "Feild to create OpenCL or library not found!" );
        return false;
    }

    // Obtain the platforms available
    cl_uint nPlatforms = 0;
    cl_int  error = clGetPlatformIDs( 0, 0, &nPlatforms );
    cl_platform_id* platformsIds = (cl_platform_id*)alloca( nPlatforms * sizeof(cl_platform_id) );
    error |= clGetPlatformIDs( nPlatforms, platformsIds, 0 );
    if( error != CL_SUCCESS || nPlatforms == 0 )
    {
        xrInitStatus = CL_CLIS_FAIL;
        GetGlContext()->PushMessage( GL_DEBUG_SOURCE_APPLICATION_ARB,
                                     GL_DEBUG_TYPE_ERROR_ARB,
                                     0,
                                     GL_DEBUG_SEVERITY_HIGH_ARB,
                                     "No such OpenCL platform installed!" );
        return false;
    }

    // NVIDIA or AMD platforms with GPU
    cl_platform_id prefPlatform = 0;
    for( cl_uint nPlatIter = 0; nPlatIter < nPlatforms; ++nPlatIter )
    {
        char name[256];
        error = clGetPlatformInfo( platformsIds[nPlatIter], CL_PLATFORM_NAME,
                                   sizeof(name), name, 0 );
        if( error != CL_SUCCESS )
            continue;

        if( strstr( name, "NVIDIA" ) != 0 )
        {
            prefPlatform = platformsIds[nPlatIter];

            // Use optimizations for NVIDIA GPUs
            IsNvidiaPlatform = true;
        }
        else if( strstr( name, "AMD" ) != 0 )
        {
            prefPlatform = prefPlatform ? prefPlatform : platformsIds[nPlatIter];

            // Use optimizations for ATI/AMD platform
            IsNvidiaPlatform = false;
        }
    }

    if( prefPlatform == 0 )
        prefPlatform = platformsIds[0];

    // Obtain the list of devices available in the selected platform
    cl_uint nDevices = 0;
    error = clGetDeviceIDs( prefPlatform, CL_DEVICE_TYPE_GPU, 0, 0, &nDevices );

    cl_device_id* devicesIds = (cl_device_id*)alloca( nDevices * sizeof(cl_device_id) );
    error |= clGetDeviceIDs( prefPlatform, CL_DEVICE_TYPE_GPU, nDevices, devicesIds, 0 );
    if (error != CL_SUCCESS)
    {
        xrInitStatus = CL_CLIS_FAIL;
        GetGlContext()->PushMessage( GL_DEBUG_SOURCE_APPLICATION_ARB,
                                     GL_DEBUG_TYPE_ERROR_ARB,
                                     0,
                                     GL_DEBUG_SEVERITY_HIGH_ARB,
                                     "Failed to get OpenCL GPU device!" );
        return false;
    }

    // Simply get first available GPU
    cl_device_id device = devicesIds[0];

    // detect old contexts
    char verClStr[256];
    clGetDeviceInfo( device, CL_DEVICE_VERSION, sizeof(verClStr), verClStr, 0 );
    verClStr[strlen("OpenCL 1.0")] = 0;
    const bool isVer10 = strncmp( verClStr, "OpenCL 1.0", strlen("OpenCL 1.0") ) == 0;

    // Create OpenCL context
    cl_context_properties ctxProp[] =
    {
#ifdef _WIN32
        CL_CONTEXT_PLATFORM, (cl_context_properties)prefPlatform,
        CL_GL_CONTEXT_KHR,   (cl_context_properties)wglGetCurrentContext(),
        CL_WGL_HDC_KHR,      (cl_context_properties)wglGetCurrentDC(),
#else
        CL_GL_CONTEXT_KHR,   (cl_context_properties)glXGetCurrentContext(),
        CL_GLX_DISPLAY_KHR,  (cl_context_properties)glXGetCurrentDisplay(),
        CL_CONTEXT_PLATFORM, (cl_context_properties)aPrefPlatform,
#endif
        0
    };

    xrComputeContext = clCreateContext( ctxProp, 1, &device, 0, 0, &error );
    if( error != CL_SUCCESS )
    {
        xrInitStatus = CL_CLIS_FAIL;
        GetGlContext()->PushMessage( GL_DEBUG_SOURCE_APPLICATION_ARB,
                                     GL_DEBUG_TYPE_ERROR_ARB,
                                     0,
                                     GL_DEBUG_SEVERITY_HIGH_ARB,
                                     "Failed to initialize OpenCL context!" );
        return false;
    }

    // Create OpenCL program
    const char* sources[] =
    {
        isVer10 ? "#define M_PI_F ( float )( 3.14159265359f )\n" : "",
        XRAY_OPENCL_SOURCE
    };

    xrProgram = clCreateProgramWithSource( xrComputeContext, 2, sources, 0, &error );
    if( error != CL_SUCCESS )
    {
        xrInitStatus = CL_CLIS_FAIL;
        GetGlContext()->PushMessage( GL_DEBUG_SOURCE_APPLICATION_ARB,
                                     GL_DEBUG_TYPE_ERROR_ARB,
                                     0,
                                     GL_DEBUG_SEVERITY_HIGH_ARB,
                                     "Failed to create OpenCL ray-tracing program!" );
        return false;
    }

    error = clBuildProgram( xrProgram, 0, 0, 0, 0, 0 );
    {
        // Fetch build log
        unsigned int logLen = 0;
        cl_int result = clGetProgramBuildInfo( xrProgram, 
                                               device,
                                               CL_PROGRAM_BUILD_LOG, 
                                               0, 0, 
                                               &logLen );

        char* buildLog = (char*)alloca( logLen );
        result |= clGetProgramBuildInfo( xrProgram, 
                                         device, 
                                         CL_PROGRAM_BUILD_LOG, 
                                         logLen, 
                                         buildLog, 
                                         0 );
        if( result == CL_SUCCESS )
        {
            if( error != CL_SUCCESS )
            {
                GetGlContext()->PushMessage( GL_DEBUG_SOURCE_APPLICATION_ARB,
                                             GL_DEBUG_TYPE_ERROR_ARB,
                                             0,
                                             GL_DEBUG_SEVERITY_HIGH_ARB,
                                             buildLog );
            }
            else
            {
#ifdef XRAY_TRACE_PRINT_INFO
            std::cout << aBuildLog << "\n";
#endif
            }
        }
    }

    if( error != CL_SUCCESS )
        return false;

    // Create OpenCL ray tracing kernels
    xrRenderKernel = clCreateKernel( xrProgram, "Main", &error );
    if( error != CL_SUCCESS )
    {
        xrInitStatus = CL_CLIS_FAIL;
        GetGlContext()->PushMessage( GL_DEBUG_SOURCE_APPLICATION_ARB,
                                     GL_DEBUG_TYPE_ERROR_ARB,
                                     0,
                                     GL_DEBUG_SEVERITY_HIGH_ARB,
                                     "Failed to create raytrace kernel!" );
        return false;
    }

    xrSmoothKernel = clCreateKernel( xrProgram, "MainSmoothing", &error );
    if( error != CL_SUCCESS )
    {
        xrInitStatus = CL_CLIS_FAIL;
        GetGlContext()->PushMessage( GL_DEBUG_SOURCE_APPLICATION_ARB,
                                     GL_DEBUG_TYPE_ERROR_ARB,
                                     0,
                                     GL_DEBUG_SEVERITY_HIGH_ARB,
                                     "Failed to create raytrace kernel!" );
        return false;
    }

    // Create OpenCL command queue
    // Note: For profiling set CL_QUEUE_PROFILING_ENABLE

    cl_command_queue_properties props = CL_QUEUE_PROFILING_ENABLE;
    xrQueue = clCreateCommandQueue( xrComputeContext, device, props, &error );
    if( error != CL_SUCCESS )
    {
        xrInitStatus = CL_CLIS_FAIL;
        GetGlContext()->PushMessage( GL_DEBUG_SOURCE_APPLICATION_ARB,
                                     GL_DEBUG_TYPE_ERROR_ARB,
                                     0,
                                     GL_DEBUG_SEVERITY_HIGH_ARB,
                                     "Failed to create command queue!" );
        return false;
    }

    xrInitStatus = CL_CLIS_INIT; // initialized in normal way
    return true;
}

//------------------------------------------------------------------------
// Returns information about device used for computations
//------------------------------------------------------------------------
bool XRayBoard::GetXRayDeviceInfo( GCollectionDataMap<GCollectionAnsi, GCollectionAnsi>& info ) const
{
    info.Clear();
    if( xrComputeContext == 0 )
        return false;

    unsigned int devicesSize = 0;
    cl_int error = clGetContextInfo( xrComputeContext, CL_CONTEXT_DEVICES, 0, 0, &devicesSize );
    cl_device_id* pDevices = (cl_device_id*)alloca( devicesSize );
    error |= clGetContextInfo( xrComputeContext, CL_CONTEXT_DEVICES, devicesSize, pDevices, 0 );
    if( error != CL_SUCCESS )
        return false;

    char deviceName[256];
    error |= clGetDeviceInfo( pDevices[0], CL_DEVICE_NAME, sizeof(deviceName), deviceName, 0 );
    info.Bind( "Name", deviceName );

    char deviceVendor[256];
    error |= clGetDeviceInfo( pDevices[0], CL_DEVICE_VENDOR, sizeof(deviceVendor), deviceVendor, 0 );
    info.Bind( "Vendor", deviceVendor );

    cl_device_type deviceType;
    error |= clGetDeviceInfo( pDevices[0], CL_DEVICE_TYPE, sizeof(deviceType), &deviceType, 0 );
    info.Bind( "Type", deviceType == CL_DEVICE_TYPE_GPU ? "GPU" : "CPU" );
    return true;
}

//----------------------------------------------------------------
// Release OpenCL instance and devices
//----------------------------------------------------------------
void XRayBoard::Release()
{
    clReleaseKernel( xrRenderKernel );
    clReleaseKernel( xrSmoothKernel );

    clReleaseProgram( xrProgram );
    clReleaseCommandQueue( xrQueue );

    clReleaseMemObject( xrOutputImage );
    clReleaseMemObject( xrEnvironment );
    clReleaseMemObject( xrOutputImageSmooth );

    clReleaseMemObject( xrVertexBuffer );
    clReleaseMemObject( xrNormalBuffer );
    clReleaseMemObject( xrTriangleBuffer );

    clReleaseMemObject( xrMaterialBuffer );
    clReleaseMemObject( xrLightSourceBuffer );

    clReleaseMemObject( xrNodeMinPointBuffer );
    clReleaseMemObject( xrNodeMaxPointBuffer );
    clReleaseMemObject( xrNodeDataRcrdBuffer );

    clReleaseContext( xrComputeContext );

    if( glIsTexture( *xrOutputTexture ) )
        glDeleteTextures( 2, xrOutputTexture );
}

//----------------------------------------------------------------
// Resizes OpenCL output image
//----------------------------------------------------------------
bool XRayBoard::ResizeXRayOutputBuffer( const cl_int sizeX, const cl_int sizeY )
{
    if( xrComputeContext == 0 )
        return false;

    bool toResize = true;
    GLint X = -1, Y = -1;

    if( *xrOutputTexture )
    {
        if( !GetGlContext()->IsGlGreaterEqual (2, 1) )
            return false;

        glBindTexture( GL_TEXTURE_RECTANGLE, *xrOutputTexture );

        glGetTexLevelParameteriv( GL_TEXTURE_RECTANGLE, 0, GL_TEXTURE_WIDTH,  &X );
        glGetTexLevelParameteriv( GL_TEXTURE_RECTANGLE, 0, GL_TEXTURE_HEIGHT, &Y );

        toResize = (X != sizeX) || (Y != sizeY);
        if( toResize )
            glDeleteTextures( 2, xrOutputTexture );
    }

    if( !toResize )
        return true;

    glGenTextures( 2, xrOutputTexture );
    for( int nTexIter = 0; nTexIter < 2; ++nTexIter )
    {
        glBindTexture( GL_TEXTURE_RECTANGLE, xrOutputTexture[nTexIter] );

        glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP );
        glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP );
        glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_R, GL_CLAMP );

        glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

        glTexImage2D( GL_TEXTURE_RECTANGLE, 
                      0, 
                      GL_RGBA32F,
                      sizeX, 
                      sizeY, 
                      0, 
                      GL_RGBA, 
                      GL_FLOAT, 
                      0 );
    }

    cl_int error = CL_SUCCESS;

    if( xrOutputImage )
        clReleaseMemObject( xrOutputImage );

    if( xrOutputImageSmooth )
        clReleaseMemObject( xrOutputImageSmooth );

    xrOutputImage = clCreateFromGLTexture2D( xrComputeContext, 
                                             CL_MEM_READ_WRITE,
                                             GL_TEXTURE_RECTANGLE, 
                                             0,
                                             xrOutputTexture[0], 
                                             &error );
    if( error != CL_SUCCESS )
    {
#ifdef XRAY_TRACE_PRINT_INFO
        std::cout << "Failed to create output image!\n";
#endif
        return false;
    }

    xrOutputImageSmooth = clCreateFromGLTexture2D( xrComputeContext, 
                                                   CL_MEM_READ_WRITE,
                                                   GL_TEXTURE_RECTANGLE, 
                                                   0,
                                                   xrOutputTexture[1], 
                                                   &error );
    if (error != CL_SUCCESS)
    {
#ifdef XRAY_TRACE_PRINT_INFO
        std::cout << "Failed to create antialiasing output image!\n";
#endif
        return false;
    }
    return true;
}

//----------------------------------------------------------------
// Change scene geometry in XRay (use GL)
//----------------------------------------------------------------
bool XRayBoard::WriteXRaySceneToDevice()
{
    if( xrComputeContext == 0 )
        return false;

    cl_int error = CL_SUCCESS;

    if( xrNormalBuffer )
        error |= clReleaseMemObject( xrNormalBuffer );

    if( xrVertexBuffer )
        error |= clReleaseMemObject( xrVertexBuffer );

    if( xrTriangleBuffer )
        error |= clReleaseMemObject( xrTriangleBuffer );

    if( xrNodeMinPointBuffer )
        error |= clReleaseMemObject( xrNodeMinPointBuffer );

    if( xrNodeMaxPointBuffer )
        error |= clReleaseMemObject( xrNodeMaxPointBuffer );

    if( xrNodeDataRcrdBuffer )
        error |= clReleaseMemObject( xrNodeDataRcrdBuffer );

    if( xrMaterialBuffer )
        error |= clReleaseMemObject( xrMaterialBuffer );

    if( error != CL_SUCCESS )
    {
#ifdef XRAY_TRACE_PRINT_INFO
        std::cout << "Failed to release scene buffers!\n";
#endif
        return false;
    }

    // geometry buffers
    cl_int errorTemp = CL_SUCCESS;
    const unsigned int myVertexBufferSize = xrSceneData.xrVertices.size() > 0
                                            ? xrSceneData.xrVertices.size() : 1;

    xrVertexBuffer = clCreateBuffer( xrComputeContext, 
                                     CL_MEM_READ_ONLY,
                                     myVertexBufferSize * sizeof(cl_float4), 
                                     0, &errorTemp );
    error |= errorTemp;

    const unsigned int normalBufferSize = xrSceneData.xrNormals.size() > 0
                                          ? xrSceneData.xrNormals.size() : 1;

    xrNormalBuffer = clCreateBuffer( xrComputeContext, 
                                     CL_MEM_READ_ONLY,
                                     normalBufferSize * sizeof(cl_float4), 
                                     0, &errorTemp );
    error |= errorTemp;

    const unsigned int triangleBufferSize = xrSceneData.xrTriangles.size() > 0
                                            ? xrSceneData.xrTriangles.size() : 1;

    xrTriangleBuffer = clCreateBuffer( xrComputeContext, 
                                       CL_MEM_READ_ONLY,
                                       triangleBufferSize * sizeof(cl_int4), 
                                       0, &errorTemp );
    error |= errorTemp;
    if( error != CL_SUCCESS )
    {
#ifdef XRAY_TRACE_PRINT_INFO
        std::cout << "Failed to create geometry buffers!\n";
#endif
        return false;
    }

    // Create material buffer
    const unsigned int materialBufferSize = xrSceneData.xrMaterials.size() > 0
                                            ? xrSceneData.xrMaterials.size() : 1;

    xrMaterialBuffer = clCreateBuffer( xrComputeContext, 
                                       CL_MEM_READ_ONLY,
                                       materialBufferSize * sizeof(XRayMaterial), 
                                       0, &errorTemp);
    if( errorTemp != CL_SUCCESS )
    {
#ifdef XRAY_TRACE_PRINT_INFO
        std::cout << "Failed to create material buffer!\n";
#endif
        return false;
    }

    // Create SAH buffers
    XRaySAH tree = SAHBuilder.Tree;
    const unsigned int nodeMinPointBufferSize = tree.MinPointBuffer.size() > 0
                                                ? tree.MinPointBuffer.size() : 1;

    xrNodeMinPointBuffer = clCreateBuffer( xrComputeContext, 
                                           CL_MEM_READ_ONLY,
                                           nodeMinPointBufferSize * sizeof(cl_float4), 
                                           0, &errorTemp );
    error |= errorTemp;

    const unsigned int nodeMaxPointBufferSize = tree.MaxPointBuffer.size() > 0
                                                ? tree.MaxPointBuffer.size() : 1;

    xrNodeMaxPointBuffer = clCreateBuffer( xrComputeContext, 
                                           CL_MEM_READ_ONLY, 
                                           nodeMaxPointBufferSize * sizeof(cl_float4), 
                                           0, &error );
    error |= errorTemp;

    const unsigned int nodeDataRecordBufferSize = tree.DataRcrdBuffer.size() > 0
                                                  ? tree.DataRcrdBuffer.size() : 1;

    xrNodeDataRcrdBuffer = clCreateBuffer( xrComputeContext, 
                                           CL_MEM_READ_ONLY,
                                           nodeDataRecordBufferSize * sizeof(cl_int4), 
                                           0, &error );
    error |= errorTemp;
    if( error != CL_SUCCESS )
    {
#ifdef XRAY_TRACE_PRINT_INFO
        std::cout << "Failed to create SAH buffers!\n";
#endif
        return false;
    }

    if( xrSceneData.xrTriangles.size() > 0 )
    {
        error |= clEnqueueWriteBuffer( xrQueue, xrVertexBuffer, CL_FALSE,
                                       0, xrSceneData.xrVertices.size() * sizeof(cl_float4),
                                       &xrSceneData.xrVertices.front(),
                                       0, 0, 0 );
        error |= clEnqueueWriteBuffer( xrQueue, xrNormalBuffer, CL_FALSE,
                                       0, xrSceneData.xrNormals.size() * sizeof(cl_float4),
                                       &xrSceneData.xrNormals.front(),
                                       0, 0, 0 );
        error |= clEnqueueWriteBuffer( xrQueue, xrTriangleBuffer, CL_FALSE,
                                       0, xrSceneData.xrTriangles.size() * sizeof(cl_int4),
                                       &xrSceneData.xrTriangles.front(),
                                       0, 0, 0 );
        if( error != CL_SUCCESS )
        {
  #ifdef XRAY_TRACE_PRINT_INFO
            std::cout << "Failed to write geometry buffers!\n";
  #endif
            return false;
        }
    }

    if( tree.DataRcrdBuffer.size() > 0 )
    {
        error |= clEnqueueWriteBuffer( xrQueue, xrNodeMinPointBuffer, CL_FALSE,
                                       0, tree.MinPointBuffer.size() * sizeof(cl_float4),
                                       &tree.MinPointBuffer.front(),
                                       0, 0, 0 );
        error |= clEnqueueWriteBuffer( xrQueue, xrNodeMaxPointBuffer, CL_FALSE,
                                       0, tree.MaxPointBuffer.size() * sizeof(cl_float4),
                                       &tree.MaxPointBuffer.front(),
                                       0, 0, 0 );
        error |= clEnqueueWriteBuffer( xrQueue, xrNodeDataRcrdBuffer, CL_FALSE,
                                       0, tree.DataRcrdBuffer.size() * sizeof(cl_int4),
                                       &tree.DataRcrdBuffer.front(),
                                       0, 0, 0 );
        if( error != CL_SUCCESS )
        {
  #ifdef XRAY_TRACE_PRINT_INFO
            std::cout << "Failed to write OpenCL SAH buffers!\n";
  #endif
            return false;
        }
    }

    if( xrSceneData.xrMaterials.size() > 0 )
    {
        const unsigned int nSize = xrSceneData.xrMaterials.size();
        const void* pData        = xrSceneData.xrMaterials.front().Packed();

        error |= clEnqueueWriteBuffer( xrQueue, xrMaterialBuffer, CL_FALSE,
                                       0, nSize * sizeof(XRayMaterial), pData,
                                       0, 0, 0 );
        if( error != CL_SUCCESS )
        {
  #ifdef XRAY_TRACE_PRINT_INFO
            std::cout << "Failed to write material buffer!\n";
  #endif
            return false;
        }
    }

    error |= clFinish( xrQueue );
#ifdef XRAY_TRACE_PRINT_INFO
    if( error != CL_SUCCESS )
        std::cout << "Failed to set scene data buffers!\n";
#endif

    if( error == CL_SUCCESS )
        IsXRayDataValid = xrSceneData.xrTriangles.size() > 0;

#ifdef XRAY_TRACE_PRINT_INFO

    float memUsed = static_cast<float>( xrSceneData.xrMaterials.size() ) * sizeof (XRayMaterial);

    aMemUsed += static_cast<float>(
                        xrSceneData.xrTriangles.size() * sizeof(XRVec4i) +
                        xrSceneData.xrVertices.size() * sizeof(XRVec4f) +
                        xrSceneData.xrNormals.size() * sizeof(XRVec4f)
                        );

    aMemUsed += static_cast<float>(
                        tree.MinPointBuffer().size() * sizeof(XRVec4f) +
                        tree.MaxPointBuffer().size() * sizeof(XRVec4f) +
                        tree.DataRcrdBuffer().size() * sizeof(XRVec4i)
                        );

    std::cout << "In GPU: " << memUsed / 1e6f << " Mb";
#endif

    xrSceneData.Clear();
    SAHBuilder.CleanUp();

    return (CL_SUCCESS == error);
}

#define OPENCL_GROUP_SIZE_TEST


//----------------------------------------------------------------
// Runs OpenCL kernels
//----------------------------------------------------------------
bool XRayBoard::RunXRayOpenCLKernels( const Graphic3d_CView& view,
                                                  const GLfloat origins[16],
                                                  const GLfloat directions[16],
                                                  const int sizeX,
                                                  const int sizeY )
{
    if( !xrRenderKernel || !xrQueue )
        return false;

    // kernel parameters

    cl_uint index = 0;
    cl_int  error = 0;

    error  = clSetKernelArg( xrRenderKernel, index++,
                             sizeof(cl_mem), &xrOutputImage );
    error |= clSetKernelArg( xrRenderKernel, index++,
                             sizeof(cl_mem), &xrEnvironment );
    error |= clSetKernelArg( xrRenderKernel, index++,
                             sizeof(cl_mem), &xrNodeMinPointBuffer );
    error |= clSetKernelArg( xrRenderKernel, index++,
                             sizeof(cl_mem), &xrNodeMaxPointBuffer );
    error |= clSetKernelArg( xrRenderKernel, index++,
                             sizeof(cl_mem), &xrNodeDataRcrdBuffer );
    error |= clSetKernelArg( xrRenderKernel, index++,
                             sizeof(cl_mem), &xrLightSourceBuffer );
    error |= clSetKernelArg( xrRenderKernel, index++,
                             sizeof(cl_mem), &xrMaterialBuffer );
    error |= clSetKernelArg( xrRenderKernel, index++,
                             sizeof(cl_mem), &xrVertexBuffer );
    error |= clSetKernelArg( xrRenderKernel, index++,
                             sizeof(cl_mem), &xrNormalBuffer );
    error |= clSetKernelArg( xrRenderKernel, index++,
                             sizeof(cl_mem), &xrTriangleBuffer );
    error |= clSetKernelArg( xrRenderKernel, index++,
                             sizeof(cl_float16), origins );
    error |= clSetKernelArg( xrRenderKernel, index++,
                             sizeof(cl_float16), directions );

    cl_int lightCount =  static_cast<cl_int>( xrSceneData.xrLightSources.size() );

    error |= clSetKernelArg( xrRenderKernel, index++,
                             sizeof(cl_int), &lightCount );
    error |= clSetKernelArg( xrRenderKernel, index++,
                             sizeof(cl_float), &xrSceneEpsilon );
    error |= clSetKernelArg( xrRenderKernel, index++,
                             sizeof(cl_float), &xrSceneRadius );
    error |= clSetKernelArg( xrRenderKernel, index++,
                             sizeof(cl_int), &view.IsShadowsEnabled );
    error |= clSetKernelArg( xrRenderKernel, index++,
                             sizeof(cl_int), &view.IsReflectionsEnabled );
    error |= clSetKernelArg( xrRenderKernel, index++,
                             sizeof(cl_int), &sizeX );
    error |= clSetKernelArg( xrRenderKernel, index++,
                             sizeof(cl_int), &sizeY );
    if( error != CL_SUCCESS )
    {
        const GCollectionAnsi msg = "Failed to set arguments for kernel!";
        GetGlContext()->PushMessage( GL_DEBUG_SOURCE_APPLICATION_ARB,
                                     GL_DEBUG_TYPE_ERROR_ARB,
                                     0,
                                     GL_DEBUG_SEVERITY_HIGH_ARB,
                                     msg );
        return false;
    }

    // for antialiasing view mode
    if( view.IsSmoothingEnabled )
    {
        index = 0;
        error  = clSetKernelArg( xrSmoothKernel, index++,
                                 sizeof(cl_mem), &xrOutputImage );
        error |= clSetKernelArg( xrSmoothKernel, index++,
                                 sizeof(cl_mem), &xrOutputImageSmooth );
        error |= clSetKernelArg( xrSmoothKernel, index++,
                                 sizeof(cl_mem), &xrEnvironment );
        error |= clSetKernelArg( xrSmoothKernel, index++,
                                 sizeof(cl_mem), &xrNodeMinPointBuffer );
        error |= clSetKernelArg( xrSmoothKernel, index++,
                                 sizeof(cl_mem), &xrNodeMaxPointBuffer );
        error |= clSetKernelArg( xrSmoothKernel, index++,
                                 sizeof(cl_mem), &xrNodeDataRcrdBuffer );
        error |= clSetKernelArg( xrSmoothKernel, index++,
                                 sizeof(cl_mem), &xrLightSourceBuffer );
        error |= clSetKernelArg( xrSmoothKernel, index++,
                                 sizeof(cl_mem), &xrMaterialBuffer );
        error |= clSetKernelArg( xrSmoothKernel, index++,
                                 sizeof(cl_mem), &xrVertexBuffer );
        error |= clSetKernelArg( xrSmoothKernel, index++,
                                 sizeof(cl_mem), &xrNormalBuffer );
        error |= clSetKernelArg( xrSmoothKernel, index++,
                                 sizeof(cl_mem), &xrTriangleBuffer );
        error |= clSetKernelArg( xrSmoothKernel, index++,
                                 sizeof(cl_float16), origins );
        error |= clSetKernelArg( xrSmoothKernel, index++,
                                 sizeof(cl_float16), directions );
        error |= clSetKernelArg( xrSmoothKernel, index++,
                                 sizeof(cl_int), &lightCount );
        error |= clSetKernelArg( xrSmoothKernel, index++,
                                 sizeof(cl_float), &xrSceneEpsilon );
        error |= clSetKernelArg( xrSmoothKernel, index++,
                                 sizeof(cl_float), &xrSceneRadius );
        error |= clSetKernelArg( xrSmoothKernel, index++,
                                 sizeof(cl_int), &view.IsShadowsEnabled );
        error |= clSetKernelArg( xrSmoothKernel, index++,
                                 sizeof(cl_int), &view.IsReflectionsEnabled );
        error |= clSetKernelArg( xrSmoothKernel, index++,
                                 sizeof(cl_int), &sizeX );
        error |= clSetKernelArg( xrSmoothKernel, index++,
                                 sizeof(cl_int), &sizeY );

        if( error != CL_SUCCESS )
        {
            const GCollectionAnsi msg = "Failed to set arguments of 'antialias' kernel!";
            GetGlContext()->PushMessage( GL_DEBUG_SOURCE_APPLICATION_ARB,
                                         GL_DEBUG_TYPE_ERROR_ARB,
                                         0,
                                         GL_DEBUG_SEVERITY_HIGH_ARB,
                                         msg );
            return false;
        }
    }

    unsigned int locSizeRender[] = { IsNvidiaPlatform ? 4 : 2, 32 };

#ifdef OPENCL_GROUP_SIZE_TEST
    for( unsigned int locX = 2; locX <= 32; locX <<= 1 )
    for( unsigned int locY = 2; locY <= 32; locY <<= 1 )
#endif
    {
#ifdef OPENCL_GROUP_SIZE_TEST
        locSizeRender[0] = locX;
        locSizeRender[1] = locY;
#endif

        unsigned int workSizeX = sizeX;
        if( workSizeX % locSizeRender[0] != 0 )
            workSizeX += locSizeRender[0] - workSizeX % locSizeRender[0];

        unsigned int workSizeY = sizeY;
        if( workSizeY % locSizeRender[1] != 0 )
            workSizeY += locSizeRender[1] - workSizeY % locSizeRender[1];

        unsigned int glbSizeRender[] = { workSizeX, workSizeY };

        // Run kernel
        cl_event anEvent = 0, anEventSmooth = 0;
        error = clEnqueueNDRangeKernel( xrQueue, xrRenderKernel,
                                        2, 0, glbSizeRender, locSizeRender,
                                        0, 0, &anEvent );
        if( error != CL_SUCCESS)
        {
            const GCollectionAnsi msg = "Failed to execute the ray-tracing kernel!";
            GetGlContext()->PushMessage( GL_DEBUG_SOURCE_APPLICATION_ARB,
                                         GL_DEBUG_TYPE_ERROR_ARB,
                                         0,
                                         GL_DEBUG_SEVERITY_HIGH_ARB,
                                         msg );
            return false;
        }
        clWaitForEvents( 1, &anEvent );

        if( view.IsSmoothingEnabled )
        {
            unsigned int locSizeSmooth[] = { IsNvidiaPlatform ? 4  : 8,
                                             IsNvidiaPlatform ? 32 : 8 };

#ifdef OPENCL_GROUP_SIZE_TEST
            locSizeSmooth[0] = locX;
            locSizeSmooth[1] = locY;
#endif

            workSizeX = sizeX;
            if( workSizeX % locSizeSmooth[0] != 0)
                workSizeX += locSizeSmooth[0] - workSizeX % locSizeSmooth[0];

            workSizeY = sizeY;
            if( workSizeY % locSizeSmooth[1] != 0 )
                workSizeY += locSizeSmooth[1] - workSizeY % locSizeSmooth[1];

            unsigned int glbSizeSmooth[] = { workSizeX, workSizeY };
            error = clEnqueueNDRangeKernel( xrQueue, xrSmoothKernel,
                                            2, 0, glbSizeSmooth, locSizeSmooth,
                                            0, 0, &anEventSmooth );
            clWaitForEvents( 1, &anEventSmooth);

            if( error != CL_SUCCESS)
            {
                const GCollectionAnsi msg = "Failed to run the 'antialias' kernel!";
                GetGlContext()->PushMessage( GL_DEBUG_SOURCE_APPLICATION_ARB,
                                             GL_DEBUG_TYPE_ERROR_ARB,
                                             0,
                                             GL_DEBUG_SEVERITY_HIGH_ARB,
                                             msg );
                return false;
            }
        }

        // Get the profiling data
#if defined(XRAY_TRACE_PRINT_INFO) || defined(OPENCL_GROUP_SIZE_TEST)

        cl_ulong timeStart, timeFinal;

        clGetEventProfilingInfo( anEvent, CL_PROFILING_COMMAND_START,
                                 sizeof(timeStart), &timeStart, 0 );
        clGetEventProfilingInfo( anEvent, CL_PROFILING_COMMAND_END,
                                 sizeof(timeFinal), &timeFinal, 0 );
        std::cout << "\tRender time (ms): " << (timeFinal - timeStart) / 1e6f << "\n";

        if( view.IsSmoothingEnabled )
        {
            clGetEventProfilingInfo( anEventSmooth, CL_PROFILING_COMMAND_START,
                                     sizeof(timeStart), &timeStart, 0 );
            clGetEventProfilingInfo( anEventSmooth, CL_PROFILING_COMMAND_END,
                                     sizeof(timeFinal), &timeFinal, 0 );
            std::cout << "\tAntialias time (ms): " 
                      << (timeFinal - timeStart) / 1e6f << "\n";
        }
#endif

        if( anEvent )
            clReleaseEvent( anEvent );

        if( anEventSmooth )
            clReleaseEvent( anEventSmooth );
    }
    return true;
}

//----------------------------------------------------------------
// Inversion of 4 x 4 matrix
//----------------------------------------------------------------
template <typename T>
void InverseMatrix( const T x[16], T inv[16] )
{
  inv[ 0] = x[ 5] * (x[10] * x[15] - x[11] * x[14]) -
            x[ 9] * (x[ 6] * x[15] - x[ 7] * x[14]) -
            x[13] * (x[ 7] * x[10] - x[ 6] * x[11]);

  inv[ 1] = x[ 1] * (x[11] * x[14] - x[10] * x[15]) -
            x[ 9] * (x[ 3] * x[14] - x[ 2] * x[15]) -
            x[13] * (x[ 2] * x[11] - x[ 3] * x[10]);

  inv[ 2] = x[ 1] * (x[ 6] * x[15] - x[ 7] * x[14]) -
            x[ 5] * (x[ 2] * x[15] - x[ 3] * x[14]) -
            x[13] * (x[ 3] * x[ 6] - x[ 2] * x[ 7]);

  inv[ 3] = x[ 1] * (x[ 7] * x[10] - x[ 6] * x[11]) -
            x[ 5] * (x[ 3] * x[10] - x[ 2] * x[11]) -
            x[ 9] * (x[ 2] * x[ 7] - x[ 3] * x[ 6]);

  inv[ 4] = x[ 4] * (x[11] * x[14] - x[10] * x[15]) -
            x[ 8] * (x[ 7] * x[14] - x[ 6] * x[15]) -
            x[12] * (x[ 6] * x[11] - x[ 7] * x[10]);

  inv[ 5] = x[ 0] * (x[10] * x[15] - x[11] * x[14]) -
            x[ 8] * (x[ 2] * x[15] - x[ 3] * x[14]) -
            x[12] * (x[ 3] * x[10] - x[ 2] * x[11]);

  inv[ 6] = x[ 0] * (x[ 7] * x[14] - x[ 6] * x[15]) -
            x[ 4] * (x[ 3] * x[14] - x[ 2] * x[15]) -
            x[12] * (x[ 2] * x[ 7] - x[ 3] * x[ 6]);

  inv[ 7] = x[ 0] * (x[ 6] * x[11] - x[ 7] * x[10]) -
            x[ 4] * (x[ 2] * x[11] - x[ 3] * x[10]) -
            x[ 8] * (x[ 3] * x[ 6] - x[ 2] * x[ 7]);

  inv[ 8] = x[ 4] * (x[ 9] * x[15] - x[11] * x[13]) -
            x[ 8] * (x[ 5] * x[15] - x[ 7] * x[13]) -
            x[12] * (x[ 7] * x[ 9] - x[ 5] * x[11]);

  inv[ 9] = x[ 0] * (x[11] * x[13] - x[ 9] * x[15]) -
            x[ 8] * (x[ 3] * x[13] - x[ 1] * x[15]) -
            x[12] * (x[ 1] * x[11] - x[ 3] * x[ 9]);

  inv[10] = x[ 0] * (x[ 5] * x[15] - x[ 7] * x[13]) -
            x[ 4] * (x[ 1] * x[15] - x[ 3] * x[13]) -
            x[12] * (x[ 3] * x[ 5] - x[ 1] * x[ 7]);

  inv[11] = x[ 0] * (x[ 7] * x[ 9] - x[ 5] * x[11]) -
            x[ 4] * (x[ 3] * x[ 9] - x[ 1] * x[11]) -
            x[ 8] * (x[ 1] * x[ 7] - x[ 3] * x[ 5]);

  inv[12] = x[ 4] * (x[10] * x[13] - x[ 9] * x[14]) -
            x[ 8] * (x[ 6] * x[13] - x[ 5] * x[14]) -
            x[12] * (x[ 5] * x[10] - x[ 6] * x[ 9]);

  inv[13] = x[ 0] * (x[ 9] * x[14] - x[10] * x[13]) -
            x[ 8] * (x[ 1] * x[14] - x[ 2] * x[13]) -
            x[12] * (x[ 2] * x[ 9] - x[ 1] * x[10]);

  inv[14] = x[ 0] * (x[ 6] * x[13] - x[ 5] * x[14]) -
            x[ 4] * (x[ 2] * x[13] - x[ 1] * x[14]) -
            x[12] * (x[ 1] * x[ 6] - x[ 2] * x[ 5]);

  inv[15] = x[ 0] * (x[ 5] * x[10] - x[ 6] * x[ 9]) -
            x[ 4] * (x[ 1] * x[10] - x[ 2] * x[ 9]) -
            x[ 8] * (x[ 2] * x[ 5] - x[ 1] * x[ 6]);

    T det = x[0] * inv[ 0] +
            x[1] * inv[ 4] +
            x[2] * inv[ 8] +
            x[3] * inv[12];

    if( det == T(0.0) ) 
        return;

    det = T(1.0) / det;

    for (int i = 0; i < 16; ++i)
        inv[i] *= det;
}

//----------------------------------------------------------------
// Creates primary rays for corners of screen quad
//----------------------------------------------------------------
void CreateCornerRays( const GLdouble invModelProj[16],
                       float origins[16],
                       float directions[16])
{
    int originIndex = 0;
    int directIndex = 0;

    for (int y = -1; y <= 1; y += 2)
    {
        for (int x = -1; x <= 1; x += 2)
        {
            XRVec4f origin ( float(x),
                              float(y),
                             -1.f,
                              1.f );

            origin = MatVecMult( invModelProj, origin );

            XRVec4f direct( float(x),
                            float(y),
                            1.f,
                            1.f );

            direct = MatVecMult( invModelProj, direct ) - origin;

            GLdouble invLen = 1.f / sqrt( direct.x() * direct.x() +
                                          direct.y() * direct.y() +
                                          direct.z() * direct.z() );

            origins[originIndex++] = static_cast<GLfloat>( origin.x() );
            origins[originIndex++] = static_cast<GLfloat>( origin.y() );
            origins[originIndex++] = static_cast<GLfloat>( origin.z() );
            origins[originIndex++] = 1.f;

            directions[directIndex++] = static_cast<GLfloat>( direct.x() * invLen );
            directions[directIndex++] = static_cast<GLfloat>( direct.y() * invLen );
            directions[directIndex++] = static_cast<GLfloat>( direct.z() * invLen );
            directions[directIndex++] = 0.f;
        }
    }
}

//----------------------------------------------------------------
// Redraws the window using OpenCL re-tracing
//----------------------------------------------------------------
bool XRayBoard::Raytrace( const Graphic3d_CView& view,
                          const int sizeX,
                          int   sizeY,
                          const Tint toSwap)
{
    if( !Init() )
        return false;

    if( !ResizeXRayOutputBuffer( sizeX, sizeY ) )
        return false;

    if( !UpdateXRayEnvironmentMap() )
        return false;

    if( !UpdateXRaySceneGeometry(true) )
        return false;

    // model-view and projection matrices
    GCollectionArray2OfReal orientation( 0, 3, 0, 3 );
    GCollectionArray2OfReal viewMapping( 0, 3, 0, 3 );

    glView->GetMatrices( orientation, viewMapping, true );

    GLdouble orientationMatrix[16];
    GLdouble viewMappingMatrix[16];
    GLdouble orientationInvers[16];

    for( int j = 0; j < 4; ++j )
        for( int i = 0; i < 4; ++i )
        {
            orientationMatrix[4 * j + i] = orientation(i, j);
            viewMappingMatrix[4 * j + i] = viewMapping(i, j);
        }

    InverseMatrix( orientationMatrix, orientationInvers );

    if( !UpdateXRayLightSources (orientationInvers) )
        return false;

    // primary rays for corners of screen quad
    glMatrixMode( GL_MODELVIEW );

    glLoadMatrixd( viewMappingMatrix );
    glMultMatrixd( orientationMatrix );

    GLdouble modelProject[16];
    GLdouble invModelProj[16];

    glGetDoublev( GL_MODELVIEW_MATRIX, modelProject );

    InverseMatrix( modelProject, invModelProj );

    GLfloat origins[16];
    GLfloat directions[16];

    CreateCornerRays( invModelProj,
                      origins,
                      directions );

    // Performance raytraced image
    cl_mem images[] = { xrOutputImage, xrOutputImageSmooth };
    cl_int error = clEnqueueAcquireGLObjects( xrQueue,
                                              2, images,
                                              0, 0, 0 );
    clFinish( xrQueue );

    if ( IsXRayDataValid )
    {
        RunXRayOpenCLKernels( view,
                              origins,
                              directions,
                              sizeX,
                              sizeY );
    }

    error |= clEnqueueReleaseGLObjects( xrQueue,
                                        2, images,
                                        0, 0, 0 );
    clFinish( xrQueue );

    // background
    glPushAttrib( GL_ENABLE_BIT |
                  GL_CURRENT_BIT |
                  GL_COLOR_BUFFER_BIT |
                  GL_DEPTH_BUFFER_BIT );

    glDisable( GL_DEPTH_TEST );

    if( GetGLStatus() & GL_NS_WHITEBACK )
    {
        glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
    }
    else
    {
        glClearColor( BackgroundColor().rgb[0],
                      BackgroundColor().rgb[1],
                      BackgroundColor().rgb[2],
                      1.0f );
    }

    glClear( GL_COLOR_BUFFER_BIT );

    Handle( GLWorkspace ) aWorkspace( this );
    glView->DrawBackground( aWorkspace );

    // quad canvas to show result image
    glEnable( GL_COLOR_MATERIAL );
    glEnable( GL_BLEND );

    glDisable( GL_DEPTH_TEST );

    glBlendFunc( GL_ONE, GL_SRC_ALPHA );

    glEnable( GL_TEXTURE_RECTANGLE );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glColor3f (1.0f, 1.0f, 1.0f);

    glBindTexture( GL_TEXTURE_RECTANGLE, 
        xrOutputTexture[view.IsSmoothingEnabled ? 1 : 0] );

    if( IsXRayDataValid )
    {
        glBegin (GL_QUADS);
        {
            glTexCoord2i(    0,     0);   glVertex2f(-1.f, -1.f);
            glTexCoord2i(    0, sizeY);   glVertex2f(-1.f,  1.f);
            glTexCoord2i(sizeX, sizeY);   glVertex2f( 1.f,  1.f);
            glTexCoord2i(sizeX,     0);   glVertex2f( 1.f, -1.f);
        }
        glEnd();
    }

    glPopAttrib();

    // call glSwapBuffers
    if( toSwap )
    {
        GetGlContext()->SwapBuffers();
        BackBufferRestored( false );
    }
    else
        glFlush();

    return true;
}
