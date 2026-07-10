from __future__ import annotations

import xml.etree.ElementTree as ET
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
MAP_DIR = ROOT / "assets" / "maps" / "morning_rush_green"
ASSET_ROOT = ROOT / "assets" / "third_party" / "craftpix" / "green-zone-tileset" / "files"

TILE_SIZE = 32
MAP_WIDTH = 144
MAP_HEIGHT = 17
MAP_PIXEL_WIDTH = MAP_WIDTH * TILE_SIZE
GROUND_ROW = 13

OBJECT_TILES = {
    "Box": ("3 Objects/Other/Box.png", 32, 25),
    "RampLeft": ("3 Objects/Other/Ramp1.png", 92, 48),
    "RampRight": ("3 Objects/Other/Ramp2.png", 92, 48),
    "HalfPipe": ("3 Objects/Other/Rapm3.png", 144, 39),
    "FountainLow": ("3 Objects/Fountain/1.png", 72, 72),
    "FountainTall": ("3 Objects/Fountain/2.png", 72, 72),
    "FenceLeft": ("3 Objects/Fence/6.png", 48, 64),
    "FenceRight": ("3 Objects/Fence/7.png", 48, 64),
    "Skateboard": ("3 Objects/Other/Skateboard1.png", 26, 7),
}

CAMPUS_OBJECT_TILES = {
    "WetFloorSign": ("wet_floor_sign.png", 471, 512),
    "TrafficCone": ("traffic_cone.png", 364, 512),
    "RecyclingBin": ("recycling_bin.png", 411, 512),
    "CampusPlanter": ("campus_planter.png", 512, 370),
    "CrowdBarrier": ("crowd_barrier.png", 512, 445),
}

PICKUP_OBJECT_TILES = {
    "Bicycle": ("bicycle.png", 1231, 739),
}


def relative_asset(path: str) -> str:
    return "../../third_party/craftpix/green-zone-tileset/files/" + path


def relative_campus_asset(path: str) -> str:
    return "../../sprites/parkour/campus_obstacles/" + path


def relative_pickup_asset(path: str) -> str:
    return "../../sprites/parkour/campus_pickups/" + path


def build_tileset() -> dict[str, int]:
    tileset = ET.Element(
        "tileset",
        version="1.10",
        tiledversion="1.12.2",
        name="green_zone_collection",
        tilewidth="144",
        tileheight="72",
        tilecount=str(96 + len(OBJECT_TILES) + len(CAMPUS_OBJECT_TILES)
                      + len(PICKUP_OBJECT_TILES)),
        columns="0",
        objectalignment="bottomleft",
    )

    for index in range(96):
        tile = ET.SubElement(tileset, "tile", id=str(index))
        ET.SubElement(
            tile,
            "image",
            width="32",
            height="32",
            source=relative_asset(f"1 Tiles/Tile_{index + 1:02}.png"),
        )

    gids: dict[str, int] = {}
    for offset, (name, (path, width, height)) in enumerate(OBJECT_TILES.items(), start=96):
        tile = ET.SubElement(tileset, "tile", id=str(offset), type=name)
        ET.SubElement(
            tile,
            "image",
            width=str(width),
            height=str(height),
            source=relative_asset(path),
        )
        gids[name] = offset + 1

    campus_start = 96 + len(OBJECT_TILES)
    for offset, (name, (path, width, height)) in enumerate(
            CAMPUS_OBJECT_TILES.items(), start=campus_start):
        tile = ET.SubElement(tileset, "tile", id=str(offset), type=name)
        ET.SubElement(
            tile,
            "image",
            width=str(width),
            height=str(height),
            source=relative_campus_asset(path),
        )
        gids[name] = offset + 1

    pickup_start = campus_start + len(CAMPUS_OBJECT_TILES)
    for offset, (name, (path, width, height)) in enumerate(
            PICKUP_OBJECT_TILES.items(), start=pickup_start):
        tile = ET.SubElement(tileset, "tile", id=str(offset), type=name)
        ET.SubElement(
            tile,
            "image",
            width=str(width),
            height=str(height),
            source=relative_pickup_asset(path),
        )
        gids[name] = offset + 1

    ET.indent(tileset)
    ET.ElementTree(tileset).write(MAP_DIR / "green_zone.tsx", encoding="utf-8", xml_declaration=True)
    return gids


def blank_grid() -> list[list[int]]:
    return [[0 for _ in range(MAP_WIDTH)] for _ in range(MAP_HEIGHT)]


def add_ground(grid: list[list[int]], segments: list[tuple[int, int]]) -> None:
    for start_x, end_x in segments:
        start_col = max(0, start_x // TILE_SIZE)
        end_col = min(MAP_WIDTH, (end_x + TILE_SIZE - 1) // TILE_SIZE)
        for col in range(start_col, end_col):
            grid[GROUND_ROW][col] = 1
            for row in range(GROUND_ROW + 1, MAP_HEIGHT):
                grid[row][col] = 10


def add_platform_tiles(grid: list[list[int]], platforms: list[tuple[int, int, int]]) -> None:
    for x, y, width in platforms:
        row = max(0, min(MAP_HEIGHT - 1, y // TILE_SIZE))
        start_col = max(0, x // TILE_SIZE)
        count = max(1, width // TILE_SIZE)
        for col in range(start_col, min(MAP_WIDTH, start_col + count)):
            grid[row][col] = 22


def csv_data(grid: list[list[int]]) -> str:
    rows = [",".join(str(value) for value in row) for row in grid]
    return "\n" + ",\n".join(rows) + "\n"


def add_rect(group: ET.Element, object_id: int, name: str, x: float, y: float,
             width: float, height: float, class_name: str = "") -> int:
    attrs = {
        "id": str(object_id),
        "name": name,
        "x": str(x),
        "y": str(y),
        "width": str(width),
        "height": str(height),
    }
    if class_name:
        attrs["class"] = class_name
    ET.SubElement(group, "object", attrs)
    return object_id + 1


def add_polygon(group: ET.Element, object_id: int, name: str, x: float, y: float,
                points: str) -> int:
    obj = ET.SubElement(
        group,
        "object",
        id=str(object_id),
        name=name,
        **{"class": "Slope", "x": str(x), "y": str(y)},
    )
    ET.SubElement(obj, "polygon", points=points)
    return object_id + 1


def add_tile_object(group: ET.Element, object_id: int, name: str, gid: int,
                    x: float, bottom: float, width: float, height: float) -> int:
    ET.SubElement(
        group,
        "object",
        id=str(object_id),
        name=name,
        gid=str(gid),
        x=str(x),
        y=str(bottom),
        width=str(width),
        height=str(height),
    )
    return object_id + 1


def create_map(stage: int, gids: dict[str, int], ground_segments: list[tuple[int, int]],
               platforms: list[tuple[int, int, int]], visual_objects: list[tuple[str, str, int, int, int, int]],
               solids: list[tuple[str, int, int, int, int]],
               hazards: list[tuple[str, int, int, int, int]],
               slopes: list[tuple[str, int, int, str]],
               pickup: tuple[int, int, int, int]) -> None:
    root = ET.Element(
        "map",
        version="1.10",
        tiledversion="1.12.2",
        orientation="orthogonal",
        renderorder="right-down",
        width=str(MAP_WIDTH),
        height=str(MAP_HEIGHT),
        tilewidth=str(TILE_SIZE),
        tileheight=str(TILE_SIZE),
        infinite="0",
        nextlayerid="11",
        nextobjectid="200",
    )
    properties = ET.SubElement(root, "properties")
    ET.SubElement(properties, "property", name="stage", type="int", value=str(stage))
    ET.SubElement(properties, "property", name="backgroundMode", value="existing_campus")
    ET.SubElement(root, "tileset", firstgid="1", source="green_zone.tsx")

    grid = blank_grid()
    add_ground(grid, ground_segments)
    add_platform_tiles(grid, platforms)
    tile_layer = ET.SubElement(root, "layer", id="1", name="GreenZoneTerrain",
                               width=str(MAP_WIDTH), height=str(MAP_HEIGHT))
    ET.SubElement(tile_layer, "data", encoding="csv").text = csv_data(grid)

    object_id = 1
    visuals = ET.SubElement(root, "objectgroup", id="2", name="VisualObjects", draworder="topdown")
    pickup_x, pickup_y, pickup_width, pickup_height = pickup
    object_id = add_tile_object(visuals, object_id, "BicycleVisual",
                                gids["Bicycle"], pickup_x, pickup_y + pickup_height,
                                pickup_width, pickup_height)
    for name, kind, x, bottom, width, height in visual_objects:
        object_id = add_tile_object(visuals, object_id, name, gids[kind], x, bottom, width, height)

    solid_group = ET.SubElement(root, "objectgroup", id="3", name="CollisionSolid", visible="0")
    for index, (start_x, end_x) in enumerate(ground_segments, start=1):
        object_id = add_rect(solid_group, object_id, f"GroundSegment{index}", start_x, 420,
                             end_x - start_x, 124, "Ground")
    for item in solids:
        object_id = add_rect(solid_group, object_id, *item, "Solid")

    one_way_group = ET.SubElement(root, "objectgroup", id="4", name="CollisionOneWay", visible="0")
    for index, (x, y, width) in enumerate(platforms, start=1):
        object_id = add_rect(one_way_group, object_id, f"Platform{index}", x, y, width, 28, "OneWay")

    hazard_group = ET.SubElement(root, "objectgroup", id="5", name="Hazard", visible="0")
    for item in hazards:
        object_id = add_rect(hazard_group, object_id, *item, "Hazard")

    slope_group = ET.SubElement(root, "objectgroup", id="6", name="Slope", visible="0")
    for item in slopes:
        object_id = add_polygon(slope_group, object_id, *item)

    spawn = ET.SubElement(root, "objectgroup", id="7", name="Spawn", visible="0")
    object_id = add_rect(spawn, object_id, "PlayerSpawn", 96, 364, 34, 56, "Spawn")
    pickup = ET.SubElement(root, "objectgroup", id="8", name="Pickup", visible="0")
    object_id = add_rect(pickup, object_id, "Bicycle", pickup_x, pickup_y,
                         pickup_width, pickup_height, "Pickup")
    finish = ET.SubElement(root, "objectgroup", id="9", name="Finish", visible="0")
    add_rect(finish, object_id, "ClassDoor", 4512, 330, 64, 90, "Finish")

    ET.indent(root)
    path = MAP_DIR / f"morning_rush_stage_{stage}.tmx"
    ET.ElementTree(root).write(path, encoding="utf-8", xml_declaration=True)


def main() -> None:
    MAP_DIR.mkdir(parents=True, exist_ok=True)
    gids = build_tileset()

    create_map(
        1,
        gids,
        ground_segments=[(0, MAP_PIXEL_WIDTH)],
        platforms=[(2440, 356, 192), (2712, 308, 192), (2992, 356, 192), (3264, 308, 192)],
        visual_objects=[
            ("WetFloorSign", "WetFloorSign", 620, 420, 54, 76),
            ("BoxA", "Box", 960, 420, 64, 50),
            ("TrafficCone", "TrafficCone", 1300, 420, 44, 56),
            ("BoxStackLow", "Box", 2200, 420, 64, 50),
            ("BoxStackHigh", "Box", 2200, 370, 64, 50),
            ("BoxFinish", "Box", 4000, 420, 64, 50),
        ],
        solids=[
            ("Hurdle WetFloorSign", 624, 344, 46, 76),
            ("Hurdle BoxA", 964, 370, 56, 50),
            ("Hurdle TrafficCone", 1304, 364, 36, 56),
            ("Hurdle BoxStack", 2204, 320, 56, 100),
            ("Hurdle BoxFinish", 4004, 370, 56, 50),
        ],
        hazards=[
            ("Hurdle WetFloorSign", 620, 344, 54, 76),
            ("Hurdle BoxA", 960, 370, 64, 50),
            ("Hurdle TrafficCone", 1300, 364, 44, 56),
            ("Hurdle BoxStack", 2200, 320, 64, 100),
            ("Hurdle BoxFinish", 4000, 370, 64, 50),
        ],
        slopes=[],
        pickup=(2756, 244, 104, 64),
    )

    create_map(
        2,
        gids,
        ground_segments=[(0, MAP_PIXEL_WIDTH)],
        platforms=[
            (400, 356, 192), (680, 308, 192), (960, 260, 192),
            (1280, 340, 192), (1760, 308, 192), (2220, 260, 192),
            (2760, 324, 192), (3320, 276, 192), (3890, 324, 192),
        ],
        visual_objects=[
            ("RecyclingBin", "RecyclingBin", 1536, 420, 58, 82),
            ("FenceA", "FenceLeft", 2520, 420, 96, 128),
            ("CampusPlanter", "CampusPlanter", 3072, 420, 104, 62),
            ("FenceB", "FenceRight", 3650, 420, 96, 128),
        ],
        solids=[
            ("Hurdle RecyclingBin", 1540, 338, 50, 82),
            ("Hurdle FenceA", 2524, 300, 88, 120),
            ("Hurdle CampusPlanter", 3076, 358, 96, 62),
            ("Hurdle FenceB", 3654, 300, 88, 120),
        ],
        hazards=[
            ("Hurdle RecyclingBin", 1536, 338, 58, 82),
            ("Hurdle FenceA", 2520, 300, 96, 120),
            ("Hurdle CampusPlanter", 3072, 358, 104, 62),
            ("Hurdle FenceB", 3650, 300, 96, 120),
        ],
        slopes=[],
        pickup=(2264, 196, 104, 64),
    )

    create_map(
        3,
        gids,
        ground_segments=[(0, 900), (1420, 1980), (2500, 3060), (3600, MAP_PIXEL_WIDTH)],
        platforms=[
            (760, 330, 184), (1018, 276, 176), (1238, 276, 168),
            (1788, 334, 184), (2040, 282, 184), (2280, 268, 176),
            (2868, 332, 156), (3148, 278, 148), (3404, 260, 140),
        ],
        visual_objects=[
            ("WetFloorSign", "WetFloorSign", 520, 420, 54, 76),
            ("RecyclingBin", "RecyclingBin", 1500, 420, 58, 82),
            ("CrowdBarrier", "CrowdBarrier", 2600, 420, 104, 86),
        ],
        solids=[
            ("Hurdle WetFloorSign", 524, 344, 46, 76),
            ("Hurdle RecyclingBin", 1504, 338, 50, 82),
            ("Hurdle CrowdBarrier", 2604, 334, 96, 86),
        ],
        hazards=[
            ("Hurdle WetFloorSign", 520, 344, 54, 76),
            ("Hurdle RecyclingBin", 1500, 338, 58, 82),
            ("Hurdle CrowdBarrier", 2600, 334, 104, 86),
            ("Pit Gap1", 900, 420, 520, 124),
            ("Pit Gap2", 1980, 420, 520, 124),
            ("Pit Gap3", 3060, 420, 540, 124),
        ],
        slopes=[],
        pickup=(2318, 204, 104, 64),
    )

if __name__ == "__main__":
    main()
