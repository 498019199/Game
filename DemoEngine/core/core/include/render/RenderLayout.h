#pragma once
#include <render/GraphicsBuffer.h>

namespace RenderWorker
{

class RenderEffect;

// 顶点布局和顶点缓存
enum VertexElementUsage
{
    // vertex positions
    VEU_Position = 0,
    // vertex normals included (for lighting)
    VEU_Normal,
    // Vertex colors - diffuse
    VEU_Diffuse,
    // Vertex colors - specular
    VEU_Specular,
    // Vertex blend weights
    VEU_BlendWeight,
    // Vertex blend indices
    VEU_BlendIndex,
    // at least one set of texture coords (exact number specified in class)
    VEU_TextureCoord,
    // Vertex tangent
    VEU_Tangent,
    // Vertex binormal
    VEU_Binormal
};

struct VertexElement
{
    VertexElement()
    {
    }

    VertexElement(VertexElementUsage u, uint8_t ui, ElementFormat f)
        : usage(u), usage_index(ui), format(f)
    {
    }

    friend bool operator==(VertexElement const & lhs, VertexElement const & rhs) noexcept
    {
        return (lhs.usage == rhs.usage)
            && (lhs.usage_index == rhs.usage_index)
            && (lhs.format == rhs.format);
    }

    uint16_t element_size() const
    {
        return NumFormatBytes(format);
    }
    
    VertexElementUsage usage;
	uint8_t usage_index;
    ElementFormat format;
};

class ZENGINE_CORE_API RenderLayout
{
    ZENGINE_NONCOPYABLE(RenderLayout);
public:
   enum topology_type
    {
        TT_PointList,
        TT_LineList,
        TT_LineStrip,
        TT_TriangleList,
        TT_TriangleStrip,
        TT_LineList_Adj,
        TT_LineStrip_Adj,
        TT_TriangleList_Adj,
        TT_TriangleStrip_Adj,
        TT_1_Ctrl_Pt_PatchList,
        TT_2_Ctrl_Pt_PatchList,
        TT_3_Ctrl_Pt_PatchList,
        TT_4_Ctrl_Pt_PatchList,
        TT_5_Ctrl_Pt_PatchList,
        TT_6_Ctrl_Pt_PatchList,
        TT_7_Ctrl_Pt_PatchList,
        TT_8_Ctrl_Pt_PatchList,
        TT_9_Ctrl_Pt_PatchList,
        TT_10_Ctrl_Pt_PatchList,
        TT_11_Ctrl_Pt_PatchList,
        TT_12_Ctrl_Pt_PatchList,
        TT_13_Ctrl_Pt_PatchList,
        TT_14_Ctrl_Pt_PatchList,
        TT_15_Ctrl_Pt_PatchList,
        TT_16_Ctrl_Pt_PatchList,
        TT_17_Ctrl_Pt_PatchList,
        TT_18_Ctrl_Pt_PatchList,
        TT_19_Ctrl_Pt_PatchList,
        TT_20_Ctrl_Pt_PatchList,
        TT_21_Ctrl_Pt_PatchList,
        TT_22_Ctrl_Pt_PatchList,
        TT_23_Ctrl_Pt_PatchList,
        TT_24_Ctrl_Pt_PatchList,
        TT_25_Ctrl_Pt_PatchList,
        TT_26_Ctrl_Pt_PatchList,
        TT_27_Ctrl_Pt_PatchList,
        TT_28_Ctrl_Pt_PatchList,
        TT_29_Ctrl_Pt_PatchList,
        TT_30_Ctrl_Pt_PatchList,
        TT_31_Ctrl_Pt_PatchList,
        TT_32_Ctrl_Pt_PatchList
    };

    enum stream_type
    {
        ST_Geometry,
        ST_Instance
    };
    
    struct StreamUnit
    {
        GraphicsBufferPtr stream;
        std::vector<VertexElement> format;
        uint32_t vertex_size;

        stream_type type;
        uint32_t freq;
    };
public:
    RenderLayout();
    virtual ~RenderLayout() noexcept;
    
    void TopologyType(topology_type type)
    {
        topo_type_ = type;
    }
    topology_type TopologyType() const
    {
        return topo_type_;
    }

    uint32_t VertexStreamNum() const;

    bool UseIndices() const;
    void NumIndices(uint32_t n);
    uint32_t NumIndices() const;

    void NumVertices(uint32_t n);
    uint32_t NumVertices() const;

    const GraphicsBufferPtr& GetVertexStream(uint32_t index) const
    {
        return vertex_streams_[index].stream;
    }
    void SetVertexStream(uint32_t index, const GraphicsBufferPtr& gb)
    {
        vertex_streams_[index].stream = gb;
        streams_dirty_ = true;
    }

    const GraphicsBufferPtr& GetIndexStream() const
    {
		return index_stream_;
	}

    void VertexStreamFormat(uint32_t index, std::span<const VertexElement> vet);

    std::vector<VertexElement> const & VertexStreamFormat(uint32_t index) const
    {
        return vertex_streams_[index].format;
    }
    uint32_t VertexSize(uint32_t index) const
    {
        return vertex_streams_[index].vertex_size;
    }
    stream_type VertexStreamType(uint32_t index) const
    {
        return vertex_streams_[index].type;
    }
    uint32_t VertexStreamFrequency(uint32_t index) const
    {
        return vertex_streams_[index].freq;
    }

    ElementFormat IndexStreamFormat() const
    {
        return index_format_;
    }
    
    void BindVertexStream(const GraphicsBufferPtr& buffer, const VertexElement& vet,
		stream_type type = ST_Geometry, uint32_t freq = 1);
	void BindVertexStream(const GraphicsBufferPtr& buffer, std::span<VertexElement const> vet,
			stream_type type = ST_Geometry, uint32_t freq = 1);

    void BindIndexStream(const GraphicsBufferPtr& buffer, ElementFormat format);

    void Active() const;

    void StartVertexLocation(uint32_t location);
    uint32_t StartVertexLocation() const;

    void StartIndexLocation(uint32_t location);
    uint32_t StartIndexLocation() const;
protected:
    topology_type topo_type_;

	GraphicsBufferPtr index_stream_;// 索引缓冲区
	ElementFormat index_format_; // 索引格式

    std::vector<StreamUnit> vertex_streams_;; // 顶点缓冲区

    uint32_t force_num_vertices_{0xFFFFFFFF};
	uint32_t force_num_indices_{0xFFFFFFFF};

    uint32_t start_vertex_location_ {0};
    uint32_t start_index_location_ {0};

    mutable bool streams_dirty_;
};
using RenderLayoutPtr = std::shared_ptr<RenderLayout>;
}
