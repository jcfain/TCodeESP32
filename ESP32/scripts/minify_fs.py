"""Bundle the web UI before PlatformIO builds the filesystem image.

data/www/index-min.html.gz is a generated artifact: minify.js bundles every
file under dataEdit/www into it. Nothing else regenerates it, so without this
hook "Build Filesystem Image" happily packages whatever gz happened to be on
disk and the device serves a stale UI - a new button can be in the source,
committed, and still missing from the running device.

Runs on the filesystem image target only, so ordinary firmware builds are not
slowed down by it.
"""

import os
import subprocess
import sys

Import("env")  # noqa: F821 - injected by PlatformIO/SCons

PROJECT_DIR = env["PROJECT_DIR"]  # noqa: F821
MINIFY_JS = os.path.join(PROJECT_DIR, "minify.js")


def minify_web_ui(source, target, env):
    if not os.path.isfile(MINIFY_JS):
        print("minify_fs: %s not found, packaging data/ unchanged" % MINIFY_JS)
        return

    print("minify_fs: bundling dataEdit/www -> data/www/index-min.html.gz")
    try:
        subprocess.run(["node", MINIFY_JS], cwd=PROJECT_DIR, check=True)
    except FileNotFoundError:
        sys.stderr.write(
            "minify_fs: 'node' is not on PATH. Install Node.js, or run "
            "`node minify.js` by hand before building the filesystem image - "
            "otherwise the image would ship a stale web UI.\n")
        env.Exit(1)
    except subprocess.CalledProcessError as exc:
        sys.stderr.write("minify_fs: minify.js failed with exit code %s\n" % exc.returncode)
        env.Exit(1)


# board_build.filesystem picks which of these actually gets built; registering
# both keeps the hook working if an environment switches.
for image in ("$BUILD_DIR/littlefs.bin", "$BUILD_DIR/spiffs.bin"):
    env.AddPreAction(image, minify_web_ui)  # noqa: F821
