# Dota2 texture pack tool

Packs unpacked hero maps into the Workshop-style **4 textures**:

| Output | Contents |
|--------|----------|
| `*_color.tga` | RGB = base color, **A = translucency** |
| `*_normal.tga` | Normal |
| `*_mask1.tga` | R detailMask, G diffuse/fresnel mask, B metalness, A selfIllum |
| `*_mask2.tga` | R specular, G rim, B tintByBase, A specularExponent |

**Not packed** (kept as optional extras): `detail`, `detail2`, `cubeMap`, `diffuseWarp`, `fresnelWarpColor`, `fresnelWarpRim`, `fresnelWarpSpec`.

## Layout

```
dota2_tex/
  pack_dota2_masks.py
  resources/models/chaos_knight/...
  resources/models/ogre_magi/...
  resources/models/ancient_apparition/...
```

Channel sources absorbed into masks are moved to `_source_masks/` beside each materials folder.

## Usage

```bat
cd game\tool\dota2_tex
python pack_dota2_masks.py --root resources\models
python pack_dota2_masks.py --root resources\models --sync ..\..\..\zengine\assets\models
```
