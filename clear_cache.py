import os
import shutil

script_dir = os.path.dirname(os.path.abspath(__file__))
target_dir = os.path.join(script_dir, ".scons_cache")

if os.path.exists(target_dir) and os.path.isdir(target_dir):
    shutil.rmtree(target_dir)
    print(f"Deleted folder: {target_dir}")