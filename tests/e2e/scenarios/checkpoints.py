"""Golden screenshot checkpoints.

The original game records each checkpoint into tests/e2e/golden/<test>/.
The rebuild must then match it within tolerance."""

from __future__ import annotations

from pathlib import Path

from PIL import Image

from harness.screen import diff_ratio


def checkpoint(game, golden_dir: Path, name: str, max_diff: float = 0.02, mask=None, settle: float = 1.0,
               region=None, timeout: float = 30.0):
    """Wait for the screen (or `region`, x/y/w/h) to settle, save it, and
    compare it to the original's golden image of the same checkpoint."""
    img = game.wait_stable(settle=settle, region=region, timeout=timeout)
    game.shots += 1
    img.save(game.out_dir / f"{game.shots:03d}-{name}.png")
    golden = golden_dir / f"{name}.png"
    # Some screens legitimately vary (the main menu is a day or night scene
    # depending on the system clock), so a checkpoint may have variants:
    # name.png, name.alt1.png, ...
    variants = ([golden] if golden.exists() else []) + sorted(golden_dir.glob(f"{name}.alt*.png"))

    def matches(path: Path) -> tuple[bool, float]:
        ref = Image.open(path)
        d = diff_ratio(ref, img, region=region)
        if mask:
            from harness.screen import similar
            return similar(ref, img, max_diff=max_diff, mask=mask, region=region), d
        return d <= max_diff, d

    results = [matches(p) for p in variants]
    if game.target == "original" and not getattr(game, "virtual", False):
        # Record the first image, and any later one that matches no variant.
        # (Never in virtual mode: there the original is checked like the
        # rebuild, so a parallel run cannot add variants.)
        if not variants:
            img.save(golden)
        elif not any(ok for ok, _ in results):
            img.save(golden_dir / f"{name}.alt{len(variants)}.png")
        return img
    if not variants:
        raise AssertionError(f"no golden image {golden}; run with --target original first")
    best = min(d for _, d in results)
    assert any(ok for ok, _ in results), (
        f"checkpoint {name}: {best:.1%} of pixels differ from the original (limit {max_diff:.1%}, "
        f"{len(variants)} golden variant(s))")
    return img
