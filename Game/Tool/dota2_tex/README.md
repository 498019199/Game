# Dota2 texture pack tool

Packs unpacked hero maps into the Workshop-style **4 textures**:

| Output | Contents |
|--------|----------|
| `*_color.tga` | RGB = base color, **A = translucency** |
| `*_normal.tga` | Normal |
| `*_mask1.tga` | R detailMask, G diffuse/fresnel mask, B metalness, A selfIllum |
| `*_mask2.tga` | R specular, G rim, B tintByBase, A specularExponent |

**Not packed** (kept as optional extras): `detail`, `detail2`, `cubeMap`.

## Layout

```
dota2_tex/
  pack_dota2_masks.py
  resources/Models/chaos_knight/...
  resources/Models/ogre_magi/...
```

Channel sources absorbed into masks are moved to `_source_masks/` beside each materials folder.

## Usage

```bat
cd Game\Tool\dota2_tex
python pack_dota2_masks.py --root resources\Models
python pack_dota2_masks.py --root resources\Models --sync ..\..\..\ZEngine\Assets\Models
```
