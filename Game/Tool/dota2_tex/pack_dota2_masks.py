#!/usr/bin/env python3
"""Pack Dota2 unpacked hero maps into the standard 4 textures.

Outputs per material prefix:
  *_color.tga   RGB from color, A from translucency (or opaque)
  *_normal.tga  copy / passthrough
  *_mask1.tga   R=detailMask G=diffuseFresnel B=metalness A=selfIllum
  *_mask2.tga   R=specular  G=rim  B=tintByBase  A=specularExponent

Optional extras kept as-is (not packed): detail, detail2, cubeMap.

Usage:
  python pack_dota2_masks.py --root resources/Models/chaos_knight
  python pack_dota2_masks.py --root resources/Models --all
  python pack_dota2_masks.py --root resources/Models/ogre_magi --sync ../../ZEngine/Assets/Models
"""

from __future__ import annotations

import argparse
import re
import shutil
import sys
from pathlib import Path

from PIL import Image


# Suffix -> role. Longer / more specific names first when matching.
CHANNEL_SUFFIXES = [
	("detailMask", "detail_mask"),
	("metalnessMask", "metalness"),
	("selfIllumMask", "self_illum"),
	("specularExponent", "spec_exp"),
	("specularMask", "specular"),
	("tintByBaseMask", "tint_by_base"),
	("rimMask", "rim"),
	("translucency", "translucency"),
	("diffuseWarp", "diffuse_fresnel"),  # fallback for mask1.G if present as spatial map
	("normal", "normal"),
	("color", "color"),
	("cubeMap", "cubemap"),
	("detail2", "detail2"),
	("detail", "detail"),
]

KEEP_EXTRA = {"detail", "detail2", "cubemap"}
ARCHIVE_ROLES = {
	"detail_mask",
	"metalness",
	"self_illum",
	"spec_exp",
	"specular",
	"tint_by_base",
	"rim",
	"translucency",
	"diffuse_fresnel",
}


def load_gray(path: Path | None, size: tuple[int, int], default: int) -> Image.Image:
	if path is None or not path.is_file():
		return Image.new("L", size, default)
	img = Image.open(path)
	if img.size != size:
		img = img.resize(size, Image.Resampling.BILINEAR)
	if img.mode != "L":
		img = img.convert("L")
	return img


def load_rgb(path: Path, size: tuple[int, int] | None = None) -> Image.Image:
	img = Image.open(path)
	if img.mode not in ("RGB", "RGBA"):
		img = img.convert("RGBA" if "A" in img.getbands() else "RGB")
	if size and img.size != size:
		img = img.resize(size, Image.Resampling.BILINEAR)
	return img


def ensure_nonzero_alpha(gray: Image.Image) -> Image.Image:
	"""Workshop rule: pure-black alpha is stripped; leave a speck if fully black."""
	extrema = gray.getextrema()
	if extrema[1] == 0:
		pix = gray.load()
		pix[0, 0] = 1
	return gray


def _classify_map(path: Path) -> tuple[str, str] | None:
	"""Return (prefix, role) for a channel/map file, or None if not a known map."""
	if not path.is_file():
		return None
	if path.suffix.lower() not in {".tga", ".png", ".jpg", ".jpeg"}:
		return None
	stem = path.stem
	if stem.endswith("_mask1") or stem.endswith("_mask2"):
		return None
	for suffix, role_name in CHANNEL_SUFFIXES:
		token = f"_{suffix}"
		if stem.endswith(token):
			return stem[: -len(token)], role_name
	return None


def match_prefix_maps(directory: Path) -> dict[str, dict[str, Path]]:
	"""Group files by material prefix -> role -> path.

	Also reads archived channel sources from sibling `_source_masks/` so re-runs
	still pack correctly after the first archive pass.
	"""
	groups: dict[str, dict[str, Path]] = {}
	search_dirs = [directory]
	archive = directory / "_source_masks"
	if archive.is_dir():
		search_dirs.append(archive)

	for search_dir in search_dirs:
		for path in sorted(search_dir.iterdir()):
			classified = _classify_map(path)
			if classified is None:
				continue
			prefix, role = classified
			# Prefer live materials/ over archived copies when both exist.
			groups.setdefault(prefix, {}).setdefault(role, path)
	return groups


def pack_group(prefix: str, maps: dict[str, Path], archive_dir: Path, dry_run: bool) -> bool:
	color_path = maps.get("color")
	if color_path is None:
		return False

	color = load_rgb(color_path)
	size = color.size
	if color.mode != "RGBA":
		color = color.convert("RGBA")

	trans = load_gray(maps.get("translucency"), size, 255)
	r, g, b, _ = color.split()
	color_out = Image.merge("RGBA", (r, g, b, trans))

	normal_path = maps.get("normal")
	if normal_path is None:
		normal_out = Image.new("RGB", size, (128, 128, 255))
	else:
		normal_out = load_rgb(normal_path, size).convert("RGB")

	mask1 = Image.merge(
		"RGBA",
		(
			load_gray(maps.get("detail_mask"), size, 0),
			load_gray(maps.get("diffuse_fresnel"), size, 0),
			load_gray(maps.get("metalness"), size, 0),
			ensure_nonzero_alpha(load_gray(maps.get("self_illum"), size, 0)),
		),
	)
	mask2 = Image.merge(
		"RGBA",
		(
			load_gray(maps.get("specular"), size, 0),
			load_gray(maps.get("rim"), size, 0),
			# Stored as authored; shader treats B as inverted tint-by-base.
			load_gray(maps.get("tint_by_base"), size, 0),
			load_gray(maps.get("spec_exp"), size, 128),
		),
	)

	out_dir = color_path.parent
	out_color = out_dir / f"{prefix}_color.tga"
	out_normal = out_dir / f"{prefix}_normal.tga"
	out_mask1 = out_dir / f"{prefix}_mask1.tga"
	out_mask2 = out_dir / f"{prefix}_mask2.tga"

	print(f"  pack {prefix}")
	print(f"    -> {out_color.name}, {out_normal.name}, {out_mask1.name}, {out_mask2.name}")

	if dry_run:
		return True

	archive_dir.mkdir(parents=True, exist_ok=True)
	# Archive channel sources that are absorbed into mask1/mask2/color.A
	for role, path in maps.items():
		if role in ARCHIVE_ROLES and path.is_file():
			dest = archive_dir / path.name
			if path.resolve() != dest.resolve():
				shutil.move(str(path), str(dest))

	color_out.save(out_color)
	normal_out.save(out_normal)
	mask1.save(out_mask1)
	mask2.save(out_mask2)
	return True


def pack_tree(root: Path, dry_run: bool) -> int:
	count = 0
	for directory in [root, *sorted(p for p in root.rglob("*") if p.is_dir())]:
		# Skip archive folders
		if directory.name in {"_source_masks", "__pycache__"}:
			continue
		groups = match_prefix_maps(directory)
		if not groups:
			continue
		archive = directory / "_source_masks"
		print(f"[{directory}]")
		for prefix, maps in sorted(groups.items()):
			if pack_group(prefix, maps, archive, dry_run):
				count += 1
	return count


def sync_to_assets(src_models: Path, dst_models: Path) -> None:
	"""Copy packed model trees into ZEngine Assets/Models."""
	dst_models.mkdir(parents=True, exist_ok=True)
	for hero in ("chaos_knight", "ogre_magi"):
		src = src_models / hero
		dst = dst_models / hero
		if not src.is_dir():
			print(f"skip sync, missing {src}")
			continue
		if dst.exists():
			shutil.rmtree(dst)
		shutil.copytree(
			src,
			dst,
			ignore=shutil.ignore_patterns("_source_masks", "*.tga.dds", "*.kmeta", "__pycache__"),
		)
		print(f"synced {src} -> {dst}")


def main(argv: list[str] | None = None) -> int:
	parser = argparse.ArgumentParser(description="Pack Dota2 unpacked maps into color/normal/mask1/mask2")
	parser.add_argument(
		"--root",
		type=Path,
		default=Path(__file__).resolve().parent / "resources" / "Models",
		help="Hero root or Models directory under dota2_tex/resources",
	)
	parser.add_argument("--dry-run", action="store_true")
	parser.add_argument(
		"--sync",
		type=Path,
		default=None,
		help="Optional ZEngine/Assets/Models path to publish packed heroes into",
	)
	args = parser.parse_args(argv)

	root = args.root.resolve()
	if not root.is_dir():
		print(f"root not found: {root}", file=sys.stderr)
		return 1

	print(f"Packing under {root}")
	n = pack_tree(root, args.dry_run)
	print(f"Packed {n} material group(s)")

	if args.sync is not None and not args.dry_run:
		models_root = root if root.name == "Models" else root.parent
		if models_root.name != "Models":
			# single hero folder passed
			models_root = root.parent
			if (root / "materials").is_dir():
				# sync single hero: parent should be Models
				sync_parent = args.sync.resolve()
				hero_name = root.name
				dst = sync_parent / hero_name
				if dst.exists():
					shutil.rmtree(dst)
				shutil.copytree(
					root,
					dst,
					ignore=shutil.ignore_patterns("_source_masks", "*.tga.dds", "*.kmeta", "__pycache__"),
				)
				print(f"synced {root} -> {dst}")
			else:
				sync_to_assets(models_root, args.sync.resolve())
		else:
			sync_to_assets(models_root, args.sync.resolve())

	return 0


if __name__ == "__main__":
	sys.exit(main())
