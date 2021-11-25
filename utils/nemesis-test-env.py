from pathlib import Path, PurePath, PurePosixPath, WindowsPath
import os
import shutil
import subprocess
import re
import argparse

def copyDirTree(src_path: Path, dst_path: Path, copy_masks = [('INCLUDE', r'*')], create_dirs = True) -> bool:
    """
    copy directory tree starting from src_path to directory pointed by dst_path
    files to copy are selected by entries in copy_masks list.
    copy_mask = [ tuple ( 'INCLUDE' | 'EXCLUDE', 'glob-like expression mathing filenames')]
    list is processed in order, if no 'INCLUDE' match is found file does NOT get copied
    """
    # convert paths to pathlib.Path if they're strings
    if type(src_path) == str:
        src_path = Path(src_path)
    if type(dst_path) == str:
        dst_path = Path(dst_path)

    if not src_path.exists():
        print("ERROR: cannot copy - file", str(src_path), "does not exists!")
        return False
    print("copyDir entering:", str(src_path))

    # iterate over directory content
    with os.scandir(src_path) as dir_it:
        for entry in dir_it:
            if entry.is_file():
                # match against copy_masks
                for copy_mask_type, copy_mask in copy_masks:
                    if Path(entry.name).match(copy_mask):
                        if copy_mask_type == 'INCLUDE':
                            # create destination path
                            if not dst_path.exists():
                                if create_dirs:
                                    #dst_path.mkdir(parents=True)
                                    print("making path:", str(dst_path))
                                else:
                                    print("ERROR: destinition directory ", str(dst_path), "does not exists!")
                                    return False

                            print("COPY", str(src_path / entry.name), "->", str(dst_path))
                            # rv = shutil.copy2(src_path, dst_path)
                            #print("COPY", str(src_path), "->", str(rv))
                        elif copy_mask_type == 'EXCLUDE':
                            # file is excluded, skip next rules
                            print("SKIP", str(src_path / entry.name))
                            break
            elif entry.is_dir():
                # recurse into directory
                copyDirTree(src_path/entry.name, dst_path/entry.name, copy_masks)

    return True






if __name__ == "__main__":

    arg_parser = argparse.ArgumentParser("Utility for making Nemesis test environments, and testing results")
    arg_parser.add_argument("-s", "--src", help="full path to skyrim 'Data' directory")
    arg_parser.add_argument("-d", "--dst", help="full path to directory where test directory hierarchy will be created")
    args = arg_parser.parse_args()

    print("-s ", args.src)
    print("-d ", args.dst)

    if args.src and args.dst:
        copyDirTree(args.src, args.dst, copy_masks=[("INCLUDE", "*.txt")])

