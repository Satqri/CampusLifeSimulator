from pathlib import Path
from statistics import median
from collections import deque

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
PLAYER_DIR = ROOT / "assets" / "sprites" / "player" / "morning_rush_student"
PLAYER_RAW = PLAYER_DIR / "raw"
OBSTACLE_DIR = ROOT / "assets" / "sprites" / "parkour" / "campus_obstacles"

TARGET_LEFT = PLAYER_DIR / "student_actions_left.png"
TARGET_RIGHT = PLAYER_DIR / "student_actions_right.png"
BACKUP_LEFT = PLAYER_DIR / "student_actions_left_before_user_material_20260702.png"
BACKUP_RIGHT = PLAYER_DIR / "student_actions_right_before_user_hit_20260702.png"
PREVIEW = PLAYER_DIR / "user_action_rows_20260702.png"

FRAME_WIDTH = 360
FRAME_HEIGHT = 260
TARGET_COLUMNS = 9


def crop_alpha(image: Image.Image) -> Image.Image:
    bbox = image.getchannel("A").getbbox()
    if bbox is None:
        raise RuntimeError("empty source sprite")
    return image.crop(bbox)


def remove_border_fragments(cell: Image.Image) -> Image.Image:
    alpha = cell.getchannel("A")
    width, height = cell.size
    pixels = alpha.load()
    visited = bytearray(width * height)
    remove = Image.new("L", cell.size, 0)
    remove_pixels = remove.load()

    for y in range(height):
        for x in range(width):
            index = y * width + x
            if visited[index] or pixels[x, y] < 16:
                continue
            queue = deque([(x, y)])
            visited[index] = 1
            component = []
            touches_side = False
            while queue:
                px, py = queue.popleft()
                component.append((px, py))
                touches_side = touches_side or px <= 2 or px >= width - 3
                for ny in range(max(0, py - 1), min(height, py + 2)):
                    for nx in range(max(0, px - 1), min(width, px + 2)):
                        next_index = ny * width + nx
                        if not visited[next_index] and pixels[nx, ny] >= 16:
                            visited[next_index] = 1
                            queue.append((nx, ny))
            if touches_side and len(component) < 3000:
                for px, py in component:
                    remove_pixels[px, py] = 255

    if remove.getbbox() is not None:
        cell.paste(Image.new("RGBA", cell.size, (0, 0, 0, 0)), mask=remove)
    return cell


def main_component_centers(strip: Image.Image, count: int) -> list[float]:
    alpha = strip.getchannel("A")
    width, height = strip.size
    pixels = alpha.load()
    visited = bytearray(width * height)
    components = []

    for y in range(height):
        for x in range(width):
            index = y * width + x
            if visited[index] or pixels[x, y] < 16:
                continue
            queue = deque([(x, y)])
            visited[index] = 1
            area = 0
            left = right = x
            while queue:
                px, py = queue.popleft()
                area += 1
                left = min(left, px)
                right = max(right, px)
                for ny in range(max(0, py - 1), min(height, py + 2)):
                    for nx in range(max(0, px - 1), min(width, px + 2)):
                        next_index = ny * width + nx
                        if not visited[next_index] and pixels[nx, ny] >= 16:
                            visited[next_index] = 1
                            queue.append((nx, ny))
            components.append((area, (left + right) * 0.5))

    if len(components) < count:
        raise RuntimeError(f"found only {len(components)} components for {count} frames")
    main = sorted(components, reverse=True)[:count]
    return sorted(center for _, center in main)


def split_strip(path: Path, count: int) -> list[Image.Image]:
    strip = Image.open(path).convert("RGBA")
    centers = main_component_centers(strip, count)
    boundaries = [0]
    boundaries.extend(round((centers[index] + centers[index + 1]) * 0.5)
                      for index in range(count - 1))
    boundaries.append(strip.width)
    frames = []
    for column in range(count):
        left = boundaries[column]
        right = boundaries[column + 1]
        cell = remove_border_fragments(strip.crop((left, 0, right, strip.height)))
        frames.append(crop_alpha(cell))
    return frames


def scale_frames(frames: list[Image.Image], reference_height: float) -> list[Image.Image]:
    scale = reference_height / median(frame.height for frame in frames)
    result = []
    for frame in frames:
        width = round(frame.width * scale)
        height = round(frame.height * scale)
        fit = min(1.0, (FRAME_WIDTH - 20) / width, (FRAME_HEIGHT - 8) / height)
        result.append(frame.resize(
            (round(width * fit), round(height * fit)),
            Image.Resampling.LANCZOS,
        ))
    return result


def paste_row(sheet: Image.Image, row: int, frames: list[Image.Image],
              bounce: tuple[int, ...] = ()) -> None:
    clear = Image.new("RGBA", (sheet.width, FRAME_HEIGHT), (0, 0, 0, 0))
    sheet.paste(clear, (0, row * FRAME_HEIGHT))
    for column, frame in enumerate(frames):
        x = column * FRAME_WIDTH + (FRAME_WIDTH - frame.width) // 2
        offset = bounce[column] if column < len(bounce) else 0
        y = row * FRAME_HEIGHT + FRAME_HEIGHT - frame.height - 4 + offset
        sheet.alpha_composite(frame, (x, y))


def rebuild_action_sheets() -> None:
    left_sheet = Image.open(TARGET_LEFT).convert("RGBA")
    right_sheet = Image.open(TARGET_RIGHT).convert("RGBA")
    expected_size = (FRAME_WIDTH * TARGET_COLUMNS, FRAME_HEIGHT * 9)
    if left_sheet.size != expected_size or right_sheet.size != expected_size:
        raise RuntimeError(
            f"unexpected sheet sizes: left={left_sheet.size}, right={right_sheet.size}"
        )
    if not BACKUP_LEFT.exists():
        BACKUP_LEFT.write_bytes(TARGET_LEFT.read_bytes())
    if not BACKUP_RIGHT.exists():
        BACKUP_RIGHT.write_bytes(TARGET_RIGHT.read_bytes())

    idle = scale_frames([crop_alpha(Image.open(
        PLAYER_RAW / "left_idle_clean.png").convert("RGBA"))], 248.0)
    idle = idle * TARGET_COLUMNS
    run = scale_frames(split_strip(PLAYER_RAW / "left_run_user.png", 8), 242.0)
    crawl_raw = split_strip(PLAYER_RAW / "left_crawl_user.png", 9)
    standing_height = median([crawl_raw[0].height, crawl_raw[-1].height])
    crawl_scale = 242.0 / standing_height
    crawl = []
    for frame in crawl_raw:
        width = round(frame.width * crawl_scale)
        height = round(frame.height * crawl_scale)
        fit = min(1.0, (FRAME_WIDTH - 20) / width, (FRAME_HEIGHT - 8) / height)
        crawl.append(frame.resize(
            (round(width * fit), round(height * fit)),
            Image.Resampling.LANCZOS,
        ))

    hit_raw = split_strip(PLAYER_RAW / "hit_fall_user.png", 8)
    hit_scale = 230.0 / median(frame.height for frame in hit_raw[:3])
    hit_right = []
    for frame in hit_raw:
        width = round(frame.width * hit_scale)
        height = round(frame.height * hit_scale)
        fit = min(1.0, (FRAME_WIDTH - 20) / width, (FRAME_HEIGHT - 32) / height)
        hit_right.append(frame.resize(
            (round(width * fit), round(height * fit)),
            Image.Resampling.LANCZOS,
        ))
    hit_left = [frame.transpose(Image.Transpose.FLIP_LEFT_RIGHT) for frame in hit_right]

    paste_row(left_sheet, 0, idle)
    paste_row(left_sheet, 1, run)
    paste_row(left_sheet, 4, crawl)
    paste_row(left_sheet, 5, hit_left, (0, 0, -3, -14, 0, 0, 0, 0))
    paste_row(right_sheet, 5, hit_right, (0, 0, -3, -14, 0, 0, 0, 0))
    left_sheet.save(TARGET_LEFT, optimize=True)
    right_sheet.save(TARGET_RIGHT, optimize=True)

    preview = Image.new("RGBA", (left_sheet.width, FRAME_HEIGHT * 4), (26, 28, 32, 255))
    preview_rows = ((left_sheet, 1), (left_sheet, 4), (right_sheet, 5), (left_sheet, 5))
    for preview_row, (sheet, target_row) in enumerate(preview_rows):
        crop = sheet.crop((0, target_row * FRAME_HEIGHT,
                           sheet.width, (target_row + 1) * FRAME_HEIGHT))
        preview.alpha_composite(crop, (0, preview_row * FRAME_HEIGHT))
    preview.save(PREVIEW, optimize=True)


def trim_obstacles() -> None:
    for path in OBSTACLE_DIR.glob("*.png"):
        image = crop_alpha(Image.open(path).convert("RGBA"))
        max_edge = max(image.size)
        if max_edge > 512:
            scale = 512.0 / max_edge
            image = image.resize(
                (round(image.width * scale), round(image.height * scale)),
                Image.Resampling.LANCZOS,
            )
        image.save(path, optimize=True)


if __name__ == "__main__":
    rebuild_action_sheets()
    trim_obstacles()
