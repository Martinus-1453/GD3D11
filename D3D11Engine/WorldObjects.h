#pragma once
#pragma warning( push )
#pragma warning( disable : 26495 )

#include "pch.h"
#include "GothicGraphicsState.h"
#include "D3D11ConstantBuffer.h"
#include "D3D11Texture.h"
#include "zTypes.h"
#include "ConstantBufferStructs.h"
#include "zCPolygon.h"
#include "BaseShadowedPointLight.h"
#include "D3D11VertexBuffer.h"

class zCMaterial;
class zCPolygon;
class D3D11VertexBuffer;
class zCVob;
class zCTexture;
class zCLightmap;
struct zCModelNodeInst;
struct BspInfo;
class zCQuadMark;
struct MaterialInfo;

struct ParticleRenderInfo
{
	GothicBlendStateInfo BlendState;
	int BlendMode;
};

struct ParticleInstanceInfo
{
	float3 position;
	float4 color;
	float2 scale;
	int drawMode; // 0 = billboard, 1 = y-locked billboard, 2 = y-plane, 3 = velo aligned
	float3 velocity;
};

struct MeshKey
{
	zCTexture * Texture;
	zCMaterial* Material;
	MaterialInfo* Info;
	//zCLightmap* Lightmap;
};

struct cmpMeshKey {
    bool operator()(const MeshKey& a, const MeshKey& b) const 
	{
        return (a.Texture<b.Texture);
    }
};

/*struct MeshKey
{
	zCMaterial* Material;
	zCLightmap* Lightmap;
};

struct cmpMeshKey {
    bool operator()(const MeshKey& a, const MeshKey& b) const 
	{
        return (a.Material<b.Material) || (a.Material == b.Material && a.Lightmap<b.Lightmap);
    }
};*/

struct VisualTesselationSettings
{
	VisualTesselationSettings()
	{
		buffer.VT_DisplacementStrength = 0.0f;
		buffer.VT_Roundness = 1.0f;
		buffer.VT_TesselationFactor = 0.0f;
		Constantbuffer = nullptr;
	}

	~VisualTesselationSettings()
	{
		delete Constantbuffer;
	}

	struct Buffer
	{
		float VT_TesselationFactor;
		float VT_Roundness;
		float VT_DisplacementStrength;
		float VT_Time;
	};

	/** creates/updates the constantbuffer */
	void UpdateConstantbuffer();

	D3D11ConstantBuffer* Constantbuffer;
	std::string TesselationShader;
	Buffer buffer;
};

/** Holds information about a mesh, ready to be loaded into the renderer */
struct MeshInfo
{
	MeshInfo()
	{
		MeshVertexBuffer = nullptr;
		MeshIndexBuffer = nullptr;
		BaseIndexLocation = 0;
		MeshIndexBufferPNAEN = nullptr;

		WrappedVB = nullptr;
		WrappedIB = nullptr;
	}

	virtual ~MeshInfo();

	/** Creates buffers for this mesh info */
	XRESULT Create(ExVertexStruct* vertices, unsigned int numVertices, VERTEX_INDEX* indices, unsigned int numIndices);

	D3D11VertexBuffer* MeshVertexBuffer;
	D3D11VertexBuffer* MeshIndexBuffer;
	std::vector<ExVertexStruct> Vertices;
	std::vector<VERTEX_INDEX> Indices;	

	D3D11VertexBuffer* MeshIndexBufferPNAEN;
	std::vector<VERTEX_INDEX> IndicesPNAEN;	
	std::vector<ExVertexStruct> VerticesPNAEN;
	unsigned int BaseIndexLocation;

	unsigned int VertexBufferOffset;
	unsigned int IndexBufferOffset;
	D3D11VertexBuffer* WrappedVB;
	D3D11VertexBuffer* WrappedIB;
};

struct WorldMeshInfo : public MeshInfo
{
	WorldMeshInfo()
	{
		SaveInfo = false;
	}

	/** Saves the info for this visual */
	void SaveWorldMeshInfo(const std::string & name);

	/** Loads the info for this visual */
	void LoadWorldMeshInfo(const std::string & name);

	VisualTesselationSettings TesselationSettings;

	/** If true we will save an info-file on next zen-resource-save */
	bool SaveInfo;
};

struct QuadMarkInfo
{
	QuadMarkInfo()
	{
		Mesh = nullptr;
		NumVertices = 0;
	}

	~QuadMarkInfo()
	{
		delete Mesh;
	}

	D3D11VertexBuffer* Mesh;
	int NumVertices;

	zCQuadMark* Visual;
	float3 Position;
};

/** Holds information about a skeletal mesh */
class zCMeshSoftSkin;
struct SkeletalMeshInfo
{
	SkeletalMeshInfo()
	{
		MeshVertexBuffer = nullptr;
		MeshIndexBuffer = nullptr;
		visual = nullptr;
		MeshIndexBufferPNAEN = nullptr;
	}

	~SkeletalMeshInfo();

	D3D11VertexBuffer* MeshVertexBuffer;
	D3D11VertexBuffer* MeshIndexBuffer;
	std::vector<ExSkelVertexStruct> Vertices;
	std::vector<VERTEX_INDEX> Indices;

	D3D11VertexBuffer* MeshIndexBufferPNAEN;
	std::vector<VERTEX_INDEX> IndicesPNAEN;	

	/** Actual visual containing this */
	zCMeshSoftSkin* visual;
};

class zCVisual;
struct BaseVisualInfo
{
	BaseVisualInfo()
	{
		Visual = nullptr;
	}

	virtual ~BaseVisualInfo()
	{
		for(std::map<zCMaterial *, std::vector<MeshInfo *>>::iterator it = Meshes.begin(); it != Meshes.end(); it++)
		{
			for(unsigned int i=0;i<it->second.size();i++)
				delete it->second[i];
		}
	}

	/** Creates PNAEN-Info for all meshes if not already there */
	virtual void CreatePNAENInfo(bool softNormals = false){}

	/** Removes PNAEN info from this visual */
	virtual void ClearPNAENInfo();

	/** Saves the info for this visual */
	virtual void SaveMeshVisualInfo(const std::string & name);

	/** Loads the info for this visual */
	virtual void LoadMeshVisualInfo(const std::string & name);

	std::map<zCMaterial *, std::vector<MeshInfo *>> Meshes;

	/** Tesselation settings for this vob */
	VisualTesselationSettings TesselationInfo;

	/** "size" of the mesh. The distance between it's bbox min and bbox max */
	float MeshSize;

	/** Meshes bounding box */
	zTBBox3D BBox;

	/** Meshes midpoint */
	DirectX::XMFLOAT3 MidPoint;

	/** Games visual */
	zCVisual* Visual;

	/** Name of this visual */
	std::string VisualName;
};

/** Holds the converted mesh of a VOB */
class zCProgMeshProto;
class zCTexture;
struct MeshVisualInfo : public BaseVisualInfo
{
	MeshVisualInfo()
	{
		Visual = nullptr;
		UnloadedSomething = false;
		StartInstanceNum = 0;
		FullMesh = nullptr;
	}

	~MeshVisualInfo()
	{
		delete FullMesh;
	}

	/** Starts a new frame for this mesh */
	void StartNewFrame()
	{
		Instances.clear();
	}

	/** Creates PNAEN-Info for all meshes if not already there */
	void CreatePNAENInfo(bool softNormals = false);

	std::map<MeshKey, std::vector<MeshInfo *>, cmpMeshKey> MeshesByTexture;

	// Vector of the MeshesByTexture-Map for faster access, since map iterations aren't Cache friendly
	std::vector<std::pair<MeshKey, std::vector<MeshInfo *>>> MeshesCached;

	//zCProgMeshProto* Visual;
	std::vector<VobInstanceInfo> Instances;
	unsigned int StartInstanceNum;

	/** Full mesh of this */
	MeshInfo * FullMesh;

	/** This is true if we can't actually render something on this. TODO: Try to fix this! */
	bool UnloadedSomething;
};

/** Holds the converted mesh of a VOB */
class zCMeshSoftSkin;
class zCModel;
struct SkeletalMeshVisualInfo : public BaseVisualInfo
{
	SkeletalMeshVisualInfo()
	{
		Visual = nullptr;
	}

	~SkeletalMeshVisualInfo()
	{
		for(std::map<zCMaterial *, std::vector<SkeletalMeshInfo*>>::iterator it = SkeletalMeshes.begin(); it != SkeletalMeshes.end(); it++)
		{
			for(unsigned int i=0;i<it->second.size();i++)
				delete it->second[i];
		}
	}

	/** Creates PNAEN-Info for all meshes if not already there */
	void CreatePNAENInfo(bool softNormals = false);

	/** Removes PNAEN info from this visual */
	void ClearPNAENInfo();

	/** Submeshes of this visual */
	std::map<zCMaterial *, std::vector<SkeletalMeshInfo*>> SkeletalMeshes;


};

struct BaseVobInfo
{
	virtual ~BaseVobInfo()
	{
	}
	/** Visual for this vob */
	BaseVisualInfo * VisualInfo;

	/** Vob the data came from */
	zCVob * Vob;
};

struct WorldMeshSectionInfo;
struct VobInfo : public BaseVobInfo
{
	VobInfo()
	{
		//Vob = nullptr;
		VobConstantBuffer = nullptr;
		IsIndoorVob = false;
		VisibleInRenderPass = false;
		VobSection = nullptr;
	}

	~VobInfo()
	{
		//delete VisualInfo;
		delete VobConstantBuffer;
	}

	/** Updates the vobs constantbuffer */
	void UpdateVobConstantBuffer();

	/** Constantbuffer which holds this vobs world matrix */
	D3D11ConstantBuffer* VobConstantBuffer;

	/** Position the vob was at while being rendered last time */
	DirectX::XMFLOAT3 LastRenderPosition;

	/** True if this is an indoor-vob */
	bool IsIndoorVob;

	/** Flag to see if this vob was drawn in the current render pass. Used to collect the same vob only once. */
	bool VisibleInRenderPass;

	/** Section this vob is in */
	WorldMeshSectionInfo* VobSection;

	/** Current world transform */
	DirectX::XMFLOAT4X4 WorldMatrix;

	/** BSP-Node this is stored in */
	std::vector<BspInfo*> ParentBSPNodes;

	/** Color the underlaying polygon has */
	DWORD GroundColor;
};

class zCVobLight;
class BaseShadowedPointLight;
struct VobLightInfo
{
	VobLightInfo()
	{
		Vob = nullptr;
		LightShadowBuffers = nullptr;
		VisibleInRenderPass = false;
		IsIndoorVob = false;
		DynamicShadows = false;
		UpdateShadows = true;
	}

	~VobLightInfo()
	{
		delete LightShadowBuffers;
	}

	/** Vob the data came from */
	zCVobLight* Vob;

	/** Flag to see if this vob was drawn in the current render pass. Used to collect the same vob only once. */
	bool VisibleInRenderPass;

	/** True if this is an indoor-vob */
	bool IsIndoorVob;

	/** BSP-Node this is stored in */
	std::vector<BspInfo*> ParentBSPNodes;

	/** Buffers for doing shadows on this light */
	BaseShadowedPointLight* LightShadowBuffers;
	bool DynamicShadows; // Whether this light should be able to have dynamic shadows
	bool UpdateShadows; // Whether to update this lights shadows on the next occasion

	/** Position where we were rendered the last time */
	DirectX::XMFLOAT3 LastRenderedPosition;
};


/** Holds the converted mesh of a VOB */
struct SkeletalVobInfo : public BaseVobInfo
{
	SkeletalVobInfo()
	{
		Vob = nullptr;
		VisualInfo = nullptr;
		IndoorVob = false;
		VisibleInRenderPass = false;
		VobConstantBuffer = nullptr;
	}

	~SkeletalVobInfo()
	{
		//delete VisualInfo;
		
		for(std::map<int, std::vector<MeshVisualInfo *>>::iterator it = NodeAttachments.begin(); it != NodeAttachments.end(); it++)
		{
			for(unsigned int i=0;i<it->second.size();i++)
				delete it->second[i];
		}

		delete VobConstantBuffer;
	}

	/** Updates the vobs constantbuffer */
	void UpdateVobConstantBuffer();

	/** Constantbuffer which holds this vobs world matrix */
	D3D11ConstantBuffer* VobConstantBuffer;

	/** Map of visuals attached to nodes */
	std::map<int, std::vector<MeshVisualInfo *>> NodeAttachments;

	/** Indoor* */
	bool IndoorVob;

	/** Flag to see if this vob was drawn in the current render pass. Used to collect the same vob only once. */
	bool VisibleInRenderPass;

	/** Current world transform */
	DirectX::XMFLOAT4X4 WorldMatrix;

	/** BSP-Node this is stored in */
	std::vector<BspInfo*> ParentBSPNodes;
};

struct SectionInstanceCache
{
	SectionInstanceCache()
	{

	}

	~SectionInstanceCache();

	/** Clears the cache for the given progmesh */
	void ClearCacheForStatic(MeshVisualInfo* pm);

	std::map<MeshVisualInfo *, std::vector<VS_ExConstantBuffer_PerInstance>> InstanceCacheData;
	std::map<MeshVisualInfo *, D3D11VertexBuffer*> InstanceCache;

};

class D3D11Texture;

/** Describes a world-section for the renderer */
struct WorldMeshSectionInfo
{
	WorldMeshSectionInfo()
	{
		BoundingBox.Min = DirectX::XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
		BoundingBox.Max = DirectX::XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		FullStaticMesh = nullptr;
	}

	~WorldMeshSectionInfo()
	{
		for(std::map<MeshKey, WorldMeshInfo*>::iterator it = WorldMeshes.begin(); it != WorldMeshes.end(); it++)
		{
			delete it->second;
		}
		WorldMeshes.clear();

		for(std::map<MeshKey, MeshInfo *>::iterator it = SuppressedMeshes.begin(); it != SuppressedMeshes.end(); it++)
		{
			delete it->second;
		}
		SuppressedMeshes.clear();
		
		for(std::map<D3D11Texture *, std::vector<MeshInfo *>>::iterator it = WorldMeshesByCustomTexture.begin(); it != WorldMeshesByCustomTexture.end(); it++)
		{
			delete it->first; // Meshes are stored in "WorldMeshes". Only delete the texture
		}
		WorldMeshesByCustomTexture.clear();

		for(std::list<VobInfo *>::iterator it = Vobs.begin(); it != Vobs.end(); it++)
		{
			delete (*it);
		}
		Vobs.clear();

		for(auto it = SectionPolygons.begin(); it != SectionPolygons.end(); it++)
		{
			delete (*it);
		}
		SectionPolygons.clear();

		

		delete FullStaticMesh;
	}

	/** Saves this sections mesh to a file */
	void SaveSectionMeshToFile(const std::string & name);

	/** Saves the mesh infos for this section */
	void SaveMeshInfos(const std::string & worldName, INT2 sectionPos);

	/** Saves the mesh infos for this section */
	void LoadMeshInfos(const std::string & worldName, INT2 sectionPos);

	std::map<MeshKey, WorldMeshInfo*, cmpMeshKey> WorldMeshes;
	std::map<D3D11Texture *, std::vector<MeshInfo *>> WorldMeshesByCustomTexture;
	std::map<zCMaterial *, std::vector<MeshInfo *>> WorldMeshesByCustomTextureOriginal;
	std::map<MeshKey, MeshInfo *, cmpMeshKey> SuppressedMeshes;
	std::list<VobInfo*> Vobs;

	/** Loaded ocean-polys of this section */
	std::vector<DirectX::XMFLOAT3> OceanPoints;

	// This is filled in case we have loaded a custom worldmesh
	std::vector<zCPolygon *> SectionPolygons;

	/** The whole section as one single mesh, without alpha-test materials */
	MeshInfo * FullStaticMesh;

	/** This sections bounding box */
	zTBBox3D BoundingBox;

	/** XY-Coord on the section array */
	INT2 WorldCoordinates;

	SectionInstanceCache InstanceCache;

	unsigned int BaseIndexLocation;
	unsigned int NumIndices;
};

class zCBspTree;
class zCWorld;
struct WorldInfo
{
	WorldInfo()
	{
		BspTree = nullptr;
		CustomWorldLoaded = false;
	}

	DirectX::XMFLOAT2 MidPoint;
	float LowestVertex;
	float HighestVertex;
	zCBspTree* BspTree;
	zCWorld* MainWorld;
	std::string WorldName;
	bool CustomWorldLoaded;
};

#pragma warning( pop )
