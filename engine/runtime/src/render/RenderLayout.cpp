#include <base/ZEngine.h>

#include <render/RenderLayout.h>
#include <render/RenderEffect.h>

namespace RenderWorker
{
RenderLayout::RenderLayout()
: topo_type_(TT_PointList),
    index_format_(EF_Unknown),
    force_num_vertices_(0xFFFFFFFF),
    force_num_indices_(0xFFFFFFFF),
    streams_dirty_(true)
{
    vertex_streams_.reserve(4);
}

RenderLayout::~RenderLayout() noexcept = default;

void RenderLayout::BindVertexStream(const GraphicsBufferPtr& buffer, VertexElement const& vet,
    stream_type type /*= ST_Geometry*/, uint32_t freq/* = 1*/)
{
    BindVertexStream(buffer, MakeSpan<1>(vet), type, freq);
}

void RenderLayout::BindVertexStream(const GraphicsBufferPtr& buffer, std::span<const VertexElement> vet,
			stream_type type /*= ST_Geometry*/, uint32_t freq/* = 1*/)
{
    COMMON_ASSERT(buffer);

    uint32_t size = 0;
    for (size_t i = 0; i < vet.size(); ++i)
    {
        size += vet[i].element_size();
    }

    if (ST_Geometry == type)
    {
        for (size_t i = 0; i < vertex_streams_.size(); ++ i)
        {
            if (MakeSpan(vertex_streams_[i].format) == vet)
            {
                vertex_streams_[i].stream = buffer;
                vertex_streams_[i].vertex_size = size;
                vertex_streams_[i].type = type;
                vertex_streams_[i].freq = freq;

                streams_dirty_ = true;
                return;
            }
        }

        auto& vs = vertex_streams_.emplace_back();
        vs.stream = buffer;
        vs.format.assign(vet.begin(), vet.end());
        vs.vertex_size = size;
        vs.type = type;
        vs.freq = freq;
    }
    else
    {
        instance_stream_.stream = buffer;
        instance_stream_.format.assign(vet.begin(), vet.end());
        instance_stream_.vertex_size = size;
        instance_stream_.type = type;
        instance_stream_.freq = freq;
    }

    streams_dirty_ = true;
}

void RenderLayout::BindIndexStream(const GraphicsBufferPtr& buffer, ElementFormat format)
{
    COMMON_ASSERT((EF_R16UI == format) || (EF_R32UI == format));

    index_stream_ = buffer;
    index_format_ = format;

    streams_dirty_ = true;
}

const GraphicsBufferPtr& RenderLayout::GetIndexStream() const
{
    COMMON_ASSERT(index_stream_);
    return index_stream_;
}

GraphicsBufferPtr const & RenderLayout::InstanceStream() const
{
    return instance_stream_.stream;
}

void RenderLayout::InstanceStream(GraphicsBufferPtr const & buffer)
{
    instance_stream_.stream = buffer;
    streams_dirty_ = true;
}

void RenderLayout::VertexStreamFormat(uint32_t index, std::span<const VertexElement> vet)
{
    vertex_streams_[index].format.assign(vet.begin(), vet.end());
    uint32_t size = 0;
    for (size_t i = 0; i < vet.size(); ++ i)
    {
        size += vet[i].element_size();
    }
    
    vertex_streams_[index].vertex_size = size;
    streams_dirty_ = true;
}

uint32_t RenderLayout::VertexStreamNum() const
{
    return static_cast<uint32_t>(vertex_streams_.size());
}

bool RenderLayout::UseIndices() const
{
    return this->NumIndices() != 0;
}

uint32_t RenderLayout::NumIndices() const
{
    uint32_t n = 0;
    if (index_stream_)
    {
        if (0xFFFFFFFF == force_num_indices_)
        {
            n = index_stream_->Size() / NumFormatBytes(index_format_);
        }
        else
        {
            n = force_num_indices_;
        }
    }
    return n;
}

void RenderLayout::NumIndices(uint32_t n)
{
    force_num_indices_ = n;
    streams_dirty_ = true;
}

void RenderLayout::NumVertices(uint32_t n)
{
    force_num_vertices_ = n;
    streams_dirty_ = true;
}

uint32_t RenderLayout::NumVertices() const
{
	uint32_t n;
    if (0xFFFFFFFF == force_num_vertices_)
    {
        n = vertex_streams_[0].stream->Size() / vertex_streams_[0].vertex_size;
    }
    else
    {
        n = force_num_vertices_;
    }
    return n;
}

void RenderLayout::NumInstances(uint32_t n)
{
    force_num_instances_ = n;
    streams_dirty_ = true;
}

uint32_t RenderLayout::NumInstances() const
{
    uint32_t n;
    if (0xFFFFFFFF == force_num_instances_)
    {
        if (vertex_streams_.empty())
        {
            n = 1;
        }
        else
        {
            n = vertex_streams_[0].freq;
        }
    }
    else
    {
        n = force_num_instances_;
    }
    return n;
}

void RenderLayout::StartVertexLocation(uint32_t location)
{
    start_vertex_location_ = location;
    streams_dirty_ = true;
}

uint32_t RenderLayout::StartVertexLocation() const
{
    return start_vertex_location_;
}

void RenderLayout::StartIndexLocation(uint32_t location)
{
    start_index_location_ = location;
    streams_dirty_ = true;
}

void RenderLayout::StartInstanceLocation(uint32_t location)
{
    start_instance_location_ = location;
    streams_dirty_ = true;
}

uint32_t RenderLayout::StartInstanceLocation() const
{
    return start_instance_location_;
}

uint32_t RenderLayout::StartIndexLocation() const
{
    return start_index_location_;
}

void RenderLayout::BindIndirectArgs(GraphicsBufferPtr const & args_buff)
{
    indirect_args_buff_ = args_buff;
    streams_dirty_ = true;
}

GraphicsBufferPtr const & RenderLayout::GetIndirectArgs() const
{
    return indirect_args_buff_;
}

void RenderLayout::IndirectArgsOffset(uint32_t offset)
{
    indirect_args_offset = offset;
    streams_dirty_ = true;
}

uint32_t RenderLayout::IndirectArgsOffset() const
{
    return indirect_args_offset;
}
}