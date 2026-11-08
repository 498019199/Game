Shader "Lib/Material"
{
    // Converted from Material.fxml — material cbuffer + textures (library, no Pass).
    CBuffer "klayge_material"
    {
        Float4 albedo_clr
        Float3 metalness_glossiness_factor
        Float4 emissive_clr
        Int albedo_map_enabled
        Int normal_map_enabled
        Int height_map_parallax_enabled
        Int height_map_tess_enabled
        Int occlusion_map_enabled
        Float alpha_test_threshold
        Float normal_scale
        Float occlusion_strength
        Float2 height_offset_scale
        Float4 tess_factors
    }

    Texture2D albedo_tex
    Texture2D metalness_glossiness_tex
    Texture2D emissive_tex
    Texture2D normal_tex
    Texture2D height_tex
    Texture2D occlusion_tex
}
