#!/usr/bin/env python3
"""Generate NpcConfig C++ class from npc.json."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

# Single-path spellings come first so single-model prefabs keep their original mesh order.
MODEL_PATH_KEYS = ("model", "path", "models", "paths")

MODEL_COMPONENT_TYPE = "model"


def load_json(path: Path):
	try:
		# utf-8-sig also reads plain utf-8, and tolerates the BOM some editors add.
		with path.open("r", encoding="utf-8-sig") as fp:
			return json.load(fp)
	except json.JSONDecodeError as exc:
		raise ValueError(
			f"{path}:{exc.lineno}:{exc.colno}: invalid JSON: {exc.msg}"
		) from exc


def load_prefab(item: dict, index: int, json_path: Path) -> tuple[dict, Path]:
	prefab = item.get("prefab")
	if prefab is None:
		return item, json_path
	if not isinstance(prefab, str) or not prefab:
		raise ValueError(f"{json_path}: entry[{index}].prefab must be a non-empty string")

	prefab_relative = Path(prefab)
	if prefab_relative.is_absolute():
		raise ValueError(f"{json_path}: entry[{index}].prefab must be relative to the Assets directory")

	assets_path = json_path.parent.parent.resolve()
	prefab_path = (assets_path / prefab_relative).resolve()
	try:
		prefab_path.relative_to(assets_path)
	except ValueError as exc:
		raise ValueError(f"{json_path}: entry[{index}].prefab escapes the Assets directory") from exc
	if not prefab_path.is_file():
		raise ValueError(f"{json_path}: entry[{index}].prefab not found: {prefab_path}")

	prefab_data = load_json(prefab_path)
	if not isinstance(prefab_data, dict):
		raise ValueError(f"{prefab_path}: root must be a JSON object")

	# The table owns identity and may override prefab defaults when needed.
	resolved = dict(prefab_data)
	resolved.update({key: value for key, value in item.items() if key != "prefab"})
	return resolved, prefab_path


def string_list(value, context: str, json_path: Path) -> list[str]:
	if value is None:
		return []
	if isinstance(value, str):
		return [value] if value else []
	if isinstance(value, list) and all(isinstance(path, str) for path in value):
		return [path for path in value if path]
	raise ValueError(f"{json_path}: {context} must be a string or an array of strings")


def model_paths(container: dict, context: str, json_path: Path) -> list[str]:
	paths: list[str] = []
	for key in MODEL_PATH_KEYS:
		for path in string_list(container.get(key), f"{context}.{key}", json_path):
			if path not in paths:
				paths.append(path)
	return paths


def component_type(component: dict, context: str, json_path: Path) -> str:
	value = component.get("type", component.get("Type"))
	if not isinstance(value, str) or not value:
		raise ValueError(f"{json_path}: {context}.type must be a non-empty string")
	return value


def cpp_escape(value: str) -> str:
	return (
		value.replace("\\", "\\\\")
		.replace('"', '\\"')
		.replace("\n", "\\n")
		.replace("\r", "\\r")
		.replace("\t", "\\t")
	)


def optional_string(container: dict, key: str, context: str, json_path: Path) -> str:
	value = container.get(key)
	if value is None:
		return ""
	if not isinstance(value, str):
		raise ValueError(f"{json_path}: {context}.{key} must be a string")
	return value


def empty_textures() -> dict[str, str]:
	return {
		"albedo": "",
		"metalness_glossiness": "",
		"normal": "",
		"emissive": "",
		"detail": "",
		"detail2": "",
		"detail_mask": "",
		"cubemap": "",
		"translucency": "",
		"mask1": "",
		"mask2": "",
		"diffuse_warp": "",
		"fresnel_warp_color": "",
		"fresnel_warp_rim": "",
		"fresnel_warp_spec": "",
	}


def load_textures(container: dict, context: str, json_path: Path) -> dict[str, str]:
	textures = container.get("textures")
	if textures is None:
		return empty_textures()
	if not isinstance(textures, dict):
		raise ValueError(f"{json_path}: {context}.textures must be an object")

	def pick(*keys: str) -> str:
		for key in keys:
			value = textures.get(key)
			if value is None:
				continue
			if not isinstance(value, str):
				raise ValueError(f"{json_path}: {context}.textures.{key} must be a string")
			return value
		return ""

	# Engine slot names preferred; Dota2 / UE-style aliases accepted.
	return {
		"albedo": pick("albedo", "color", "da", "DA"),
		"metalness_glossiness": pick(
			"metalness_glossiness", "metalnessMask", "metalness_mask", "dcse", "DCSE"
		),
		"normal": pick("normal", "nr", "NR"),
		"emissive": pick("emissive", "selfIllumMask", "selfillum_mask", "SelfillumMask"),
		"detail": pick("detail"),
		"detail2": pick("detail2"),
		"detail_mask": pick("detail_mask", "detailMask"),
		"cubemap": pick("cubemap", "cubeMap", "cube_map"),
		"translucency": pick("translucency"),
		"mask1": pick("mask1"),
		"mask2": pick("mask2"),
		"diffuse_warp": pick("diffuse_warp", "diffuseWarp"),
		"fresnel_warp_color": pick("fresnel_warp_color", "fresnelWarpColor"),
		"fresnel_warp_rim": pick("fresnel_warp_rim", "fresnelWarpRim"),
		"fresnel_warp_spec": pick("fresnel_warp_spec", "fresnelWarpSpec"),
	}


def load_parts(container: dict, context: str, json_path: Path) -> list[dict]:
	parts = container.get("parts")
	if parts is None:
		return []
	if not isinstance(parts, dict):
		raise ValueError(f"{json_path}: {context}.parts must be an object")

	resolved: list[dict] = []
	for part_name, part_value in parts.items():
		if not isinstance(part_name, str) or not part_name:
			raise ValueError(f"{json_path}: {context}.parts keys must be non-empty strings")
		if not isinstance(part_value, dict):
			raise ValueError(f"{json_path}: {context}.parts.{part_name} must be an object")
		# Accept either {"textures": {...}} or textures fields directly on the part.
		tex_container = part_value
		if "textures" not in part_value and any(
			key in part_value
			for key in (
				"albedo",
				"color",
				"da",
				"DA",
				"normal",
				"nr",
				"NR",
				"metalness_glossiness",
				"metalnessMask",
				"dcse",
				"DCSE",
				"emissive",
				"selfIllumMask",
				"detail",
				"detail2",
				"detailMask",
				"cubemap",
				"cubeMap",
				"translucency",
				"mask1",
				"mask2",
				"diffuseWarp",
				"fresnelWarpColor",
				"fresnelWarpRim",
				"fresnelWarpSpec",
			)
		):
			tex_container = {"textures": part_value}
		resolved.append(
			{
				"name": part_name,
				"textures": load_textures(tex_container, f"{context}.parts.{part_name}", json_path),
			}
		)

	# Longer keys first so "shoulder" wins over a hypothetical shorter substring.
	resolved.sort(key=lambda part: len(part["name"]), reverse=True)
	return resolved


def load_model_component(container: dict, context: str, json_path: Path) -> dict:
	models = model_paths(container, context, json_path)
	if not models:
		raise ValueError(f"{json_path}: {context} needs model or models")

	return {
		"models": models,
		"material": optional_string(container, "material", context, json_path),
		# Prefer snake_case; accept PascalCase aliases used in engine naming.
		"render_effect": optional_string(container, "render_effect", context, json_path)
		or optional_string(container, "RenderEffect", context, json_path),
		"render_technique": optional_string(container, "render_technique", context, json_path)
		or optional_string(container, "RenderTechnique", context, json_path),
		"textures": load_textures(container, context, json_path),
		"parts": load_parts(container, context, json_path),
	}


def load_components(data: dict, context: str, json_path: Path) -> list[dict]:
	"""Each component names the class the spawner builds; "model" carries AModel data.

	A new type needs a payload loader here, a payload struct in the generated header,
	and a matching spawner registered in NpcSpawner.
	"""
	raw = data.get("components", data.get("Components"))
	if raw is None:
		# Prefabs without components describe a single model inline.
		return [{"type": MODEL_COMPONENT_TYPE, "model": load_model_component(data, context, json_path)}]
	if not isinstance(raw, list):
		raise ValueError(f"{json_path}: {context}.components must be an array")

	components: list[dict] = []
	for comp_index, component in enumerate(raw):
		comp_context = f"{context}.components[{comp_index}]"
		if not isinstance(component, dict):
			raise ValueError(f"{json_path}: {comp_context} must be an object")

		kind = component_type(component, comp_context, json_path)
		model = None
		if kind.lower() == MODEL_COMPONENT_TYPE:
			model = load_model_component(component, comp_context, json_path)
		else:
			print(
				f"warning: {json_path}: {comp_context} type '{kind}' has no payload loader; "
				"only the type name is emitted",
				file=sys.stderr,
			)
		components.append({"type": kind, "model": model})

	if not any(component["model"] for component in components):
		raise ValueError(f"{json_path}: {context} needs a '{MODEL_COMPONENT_TYPE}' component")

	return components


def load_entries(json_path: Path) -> list[dict]:
	data = load_json(json_path)

	if not isinstance(data, list):
		raise ValueError(f"{json_path}: root must be a JSON array")

	entries: list[dict] = []
	for index, item in enumerate(data):
		if not isinstance(item, dict):
			raise ValueError(f"{json_path}: entry[{index}] must be an object")

		item, item_path = load_prefab(item, index, json_path)
		npc_id = item.get("id", 0)
		name = item.get("name", "")
		context = f"entry[{index}]"

		if not isinstance(npc_id, int):
			raise ValueError(f"{json_path}: {context}.id must be an int")
		if not isinstance(name, str):
			raise ValueError(f"{item_path}: {context}.name must be a string")

		entries.append(
			{
				"id": npc_id,
				"name": name,
				"components": load_components(item, context, item_path),
			}
		)

	return entries


def write_header(path: Path) -> None:
	path.parent.mkdir(parents=True, exist_ok=True)
	path.write_text(
		"""#pragma once

// AUTO-GENERATED by Game/Tool/gen_npc_config.py — do not edit by hand.

#include <game/GameApi.h>

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

struct NpcConfigTextures
{
	char const* albedo;
	char const* metalness_glossiness;
	char const* normal;
	char const* emissive;
	char const* detail;
	char const* detail2;
	char const* detail_mask;
	char const* cubemap;
	char const* translucency;
	char const* mask1;
	char const* mask2;
	char const* diffuse_warp;
	char const* fresnel_warp_color;
	char const* fresnel_warp_rim;
	char const* fresnel_warp_spec;
};

struct NpcConfigPart
{
	char const* name;
	NpcConfigTextures textures;
};

// Payload of a "model" component: the meshes and materials of one AModel.
struct NpcConfigModel
{
	char const* const* models;
	std::size_t model_count;
	char const* material;
	char const* render_effect;
	char const* render_technique;
	NpcConfigTextures textures;
	NpcConfigPart const* parts;
	std::size_t part_count;
};

// `type` selects the class the spawner builds; "model" builds an AModel.
// A new type adds its payload pointer here and a spawner in NpcSpawner.
struct NpcConfigComponent
{
	char const* type;
	NpcConfigModel const* model;
};

struct NpcConfigEntry
{
	int32_t id;
	char const* name;
	NpcConfigComponent const* components;
	std::size_t component_count;
};

class GAME_API NpcConfig
{
public:
	static std::span<NpcConfigEntry const> All() noexcept;
	static std::size_t Count() noexcept;
	static NpcConfigEntry const* FindById(int32_t id) noexcept;
	static NpcConfigEntry const* FindByName(std::string_view name) noexcept;
};
""",
		encoding="utf-8",
		newline="\n",
	)


def format_textures(tex: dict[str, str]) -> str:
	return (
		f'{{ "{cpp_escape(tex["albedo"])}", '
		f'"{cpp_escape(tex["metalness_glossiness"])}", '
		f'"{cpp_escape(tex["normal"])}", '
		f'"{cpp_escape(tex["emissive"])}", '
		f'"{cpp_escape(tex["detail"])}", '
		f'"{cpp_escape(tex["detail2"])}", '
		f'"{cpp_escape(tex["detail_mask"])}", '
		f'"{cpp_escape(tex["cubemap"])}", '
		f'"{cpp_escape(tex["translucency"])}", '
		f'"{cpp_escape(tex["mask1"])}", '
		f'"{cpp_escape(tex["mask2"])}", '
		f'"{cpp_escape(tex["diffuse_warp"])}", '
		f'"{cpp_escape(tex["fresnel_warp_color"])}", '
		f'"{cpp_escape(tex["fresnel_warp_rim"])}", '
		f'"{cpp_escape(tex["fresnel_warp_spec"])}" }}'
	)


def write_source(path: Path, entries: list[dict], json_path: Path) -> None:
	path.parent.mkdir(parents=True, exist_ok=True)
	repo_root = Path(__file__).resolve().parents[2]
	try:
		source_path = json_path.resolve().relative_to(repo_root).as_posix()
	except ValueError:
		source_path = json_path.name

	lines = [
		"#include <Data/generated/NpcConfig.gen.h>",
		"",
		"// AUTO-GENERATED by Game/Tool/gen_npc_config.py — do not edit by hand.",
		f"// Source: {source_path}",
		"",
		"namespace",
		"{",
	]

	if entries:
		for entry in entries:
			for comp_index, component in enumerate(entry["components"]):
				model = component["model"]
				if model is None:
					continue

				prefix = f"kNpc_{entry['id']}_C{comp_index}"
				lines.append(f"\tchar const* const {prefix}_Models[] =")
				lines.append("\t{")
				for model_path in model["models"]:
					lines.append(f'\t\t"{cpp_escape(model_path)}",')
				lines.append("\t};")
				lines.append("")

				parts = model["parts"]
				if parts:
					lines.append(f"\tNpcConfigPart const {prefix}_Parts[] =")
					lines.append("\t{")
					for part in parts:
						lines.append(
							f'\t\t{{ "{cpp_escape(part["name"])}", {format_textures(part["textures"])} }},'
						)
					lines.append("\t};")
					lines.append("")

				lines.append(f"\tNpcConfigModel const {prefix}_Model =")
				lines.append("\t{")
				lines.append(f'\t\t{prefix}_Models, {len(model["models"])},')
				lines.append(f'\t\t"{cpp_escape(model["material"])}",')
				lines.append(f'\t\t"{cpp_escape(model["render_effect"])}",')
				lines.append(f'\t\t"{cpp_escape(model["render_technique"])}",')
				lines.append(f'\t\t{format_textures(model["textures"])},')
				lines.append(f'\t\t{f"{prefix}_Parts" if parts else "nullptr"}, {len(parts)},')
				lines.append("\t};")
				lines.append("")

			lines.append(f"\tNpcConfigComponent const kNpc_{entry['id']}_Components[] =")
			lines.append("\t{")
			for comp_index, component in enumerate(entry["components"]):
				payload = f"&kNpc_{entry['id']}_C{comp_index}_Model" if component["model"] else "nullptr"
				lines.append(f'\t\t{{ "{cpp_escape(component["type"])}", {payload} }},')
			lines.append("\t};")
			lines.append("")

		lines.append("\tNpcConfigEntry const kNpcEntries[] =")
		lines.append("\t{")
		for entry in entries:
			lines.append(
				"\t\t{ "
				f'{entry["id"]}, "{cpp_escape(entry["name"])}", '
				f'kNpc_{entry["id"]}_Components, {len(entry["components"])} }},'
			)
		lines.append("\t};")
	else:
		lines.extend(
			[
				'\tNpcConfigComponent const kNpc_Empty_Components[] = { { "", nullptr } };',
				"",
				"\tNpcConfigEntry const kNpcEntries[] =",
				"\t{",
				'\t\t{ 0, "", kNpc_Empty_Components, 0 },',
				"\t};",
			]
		)

	lines.extend(
		[
			"} // namespace",
			"",
			"std::span<NpcConfigEntry const> NpcConfig::All() noexcept",
			"{",
			"\treturn std::span<NpcConfigEntry const>(kNpcEntries, Count());",
			"}",
			"",
			"std::size_t NpcConfig::Count() noexcept",
			"{",
			f"\treturn {len(entries)};",
			"}",
			"",
			"NpcConfigEntry const* NpcConfig::FindById(int32_t id) noexcept",
			"{",
			"\tfor (NpcConfigEntry const& entry : All())",
			"\t{",
			"\t\tif (entry.id == id)",
			"\t\t{",
			"\t\t\treturn &entry;",
			"\t\t}",
			"\t}",
			"\treturn nullptr;",
			"}",
			"",
			"NpcConfigEntry const* NpcConfig::FindByName(std::string_view name) noexcept",
			"{",
			"\tfor (NpcConfigEntry const& entry : All())",
			"\t{",
			"\t\tif (name == entry.name)",
			"\t\t{",
			"\t\t\treturn &entry;",
			"\t\t}",
			"\t}",
			"\treturn nullptr;",
			"}",
			"",
		]
	)

	path.write_text("\n".join(lines), encoding="utf-8", newline="\n")


def main(argv: list[str] | None = None) -> int:
	parser = argparse.ArgumentParser(description="Generate NpcConfig C++ class from npc.json")
	parser.add_argument("--input", type=Path, required=True, help="Path to npc.json")
	parser.add_argument("--header", type=Path, required=True, help="Output header path")
	parser.add_argument("--source", type=Path, required=True, help="Output source path")
	args = parser.parse_args(argv)

	entries = load_entries(args.input.resolve())
	write_header(args.header.resolve())
	write_source(args.source.resolve(), entries, args.input.resolve())

	print(f"Generated {args.header} and {args.source} ({len(entries)} npc entries)")
	return 0


if __name__ == "__main__":
	sys.exit(main())
