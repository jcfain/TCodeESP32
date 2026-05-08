"""Optional GitHub Releases lookup so the wizard can offer "newer firmware
available" even when the bundled payload is months old.

Disabled silently if the network is unavailable — bundled payload always
remains a valid fallback.
"""
from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import List, Optional

import requests


GITHUB_REPO = "jcfain/TCodeESP32"


@dataclass
class ReleaseAsset:
    name: str
    url: str
    size: int


@dataclass
class Release:
    tag: str
    name: str
    assets: List[ReleaseAsset]
    is_prerelease: bool


def latest_release(repo: str = GITHUB_REPO, timeout: float = 5.0) -> Optional[Release]:
    """Return the latest non-draft release, or ``None`` on any failure."""
    url = f"https://api.github.com/repos/{repo}/releases/latest"
    try:
        resp = requests.get(url, timeout=timeout, headers={"Accept": "application/vnd.github+json"})
        resp.raise_for_status()
        data = resp.json()
    except Exception:
        return None
    return Release(
        tag=data.get("tag_name", ""),
        name=data.get("name", ""),
        is_prerelease=bool(data.get("prerelease")),
        assets=[
            ReleaseAsset(
                name=a.get("name", ""),
                url=a.get("browser_download_url", ""),
                size=int(a.get("size", 0)),
            )
            for a in data.get("assets", [])
        ],
    )


def download_asset(asset: ReleaseAsset, dest: Path, timeout: float = 60.0,
                   progress=None) -> Path:
    """Stream *asset* to *dest*. ``progress(bytes_done, bytes_total)`` if given."""
    dest.parent.mkdir(parents=True, exist_ok=True)
    with requests.get(asset.url, timeout=timeout, stream=True) as r:
        r.raise_for_status()
        total = int(r.headers.get("Content-Length") or asset.size or 0)
        done = 0
        with dest.open("wb") as fp:
            for chunk in r.iter_content(chunk_size=64 * 1024):
                if not chunk:
                    continue
                fp.write(chunk)
                done += len(chunk)
                if progress:
                    try:
                        progress(done, total)
                    except Exception:
                        pass
    return dest
