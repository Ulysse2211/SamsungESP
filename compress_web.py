import os
import gzip
import shutil
Import("env")

def compress_assets(source, target, item):
    print("Compressing...")
    src_dir = os.path.join(env.get("PROJECT_DIR"), "data_src")
    dist_dir = os.path.join(env.get("PROJECT_DIR"), "data")

    if os.path.exists(dist_dir):
        shutil.rmtree(dist_dir)
    os.makedirs(dist_dir)

    for root, dirs, files in os.walk(src_dir):
        for file in files:
            src_path = os.path.join(root, file)
            dist_path = os.path.join(dist_dir, file)

            if file.endswith((".html", ".css", ".js", ".svg")):
                with open(src_path, 'rb') as f_in:
                    with gzip.open(dist_path + '.gz', 'wb') as f_out:
                        shutil.copyfileobj(f_in, f_out)
            else:
                shutil.copy2(src_path, dist_path)
    print("Compressed")

env.AddPreAction("buildfs", compress_assets)
compress_assets(None, None, None)
